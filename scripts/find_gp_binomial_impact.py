from __future__ import annotations

import csv
import re
from bisect import bisect_right
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path

RESOLVED_DATE = "2026-06-25"


class NonAffineError(Exception):
    pass


def strip_gp_comments(text: str) -> str:
    stripped_lines = []
    for line in text.splitlines():
        comment_pos = line.find("\\\\")
        if comment_pos >= 0:
            line = line[:comment_pos]
        stripped_lines.append(line)
    return "\n".join(stripped_lines)


def normalize_expr(expr: str) -> str:
    expr = expr.strip()
    while expr.startswith("(") and expr.endswith(")") and matching_paren(expr, 0) == len(expr) - 1:
        expr = expr[1:-1].strip()
    return expr


def split_top_level(text: str, sep: str = ",") -> list[str]:
    parts: list[str] = []
    start = 0
    depth = 0
    for i, ch in enumerate(text):
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
        elif ch == sep and depth == 0:
            parts.append(text[start:i].strip())
            start = i + 1
    parts.append(text[start:].strip())
    return parts


def matching_paren(text: str, open_pos: int) -> int:
    depth = 0
    for i in range(open_pos, len(text)):
        ch = text[i]
        if ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return i
    raise ValueError("unmatched parenthesis")


@dataclass(frozen=True)
class Affine:
    coeffs: dict[str, Fraction]
    const: Fraction = Fraction(0)

    def __post_init__(self) -> None:
        object.__setattr__(
            self,
            "coeffs",
            {k: v for k, v in self.coeffs.items() if v != 0},
        )

    @staticmethod
    def const_value(value: int | Fraction) -> "Affine":
        return Affine({}, Fraction(value))

    @staticmethod
    def var(name: str) -> "Affine":
        return Affine({name: Fraction(1)}, Fraction(0))

    def is_constant(self) -> bool:
        return not self.coeffs

    def __add__(self, other: "Affine") -> "Affine":
        coeffs = dict(self.coeffs)
        for key, value in other.coeffs.items():
            coeffs[key] = coeffs.get(key, Fraction(0)) + value
        return Affine(coeffs, self.const + other.const)

    def __sub__(self, other: "Affine") -> "Affine":
        coeffs = dict(self.coeffs)
        for key, value in other.coeffs.items():
            coeffs[key] = coeffs.get(key, Fraction(0)) - value
        return Affine(coeffs, self.const - other.const)

    def __mul__(self, other: "Affine") -> "Affine":
        if self.is_constant():
            scale = self.const
            return Affine({k: scale * v for k, v in other.coeffs.items()}, scale * other.const)
        if other.is_constant():
            scale = other.const
            return Affine({k: scale * v for k, v in self.coeffs.items()}, scale * self.const)
        raise NonAffineError("non-affine multiplication")

    def __truediv__(self, other: "Affine") -> "Affine":
        if not other.is_constant() or other.const == 0:
            raise NonAffineError("non-affine division")
        scale = other.const
        return Affine({k: v / scale for k, v in self.coeffs.items()}, self.const / scale)

    def __neg__(self) -> "Affine":
        return Affine({k: -v for k, v in self.coeffs.items()}, -self.const)

    def substitute(self, name: str, replacement: "Affine") -> "Affine":
        if name not in self.coeffs:
            return self
        scale = self.coeffs[name]
        coeffs = dict(self.coeffs)
        del coeffs[name]
        base = Affine(coeffs, self.const)
        return base + Affine.const_value(scale) * replacement

    def unknown_vars(self, allowed: set[str]) -> set[str]:
        return {name for name in self.coeffs if name not in allowed}

    def int_linear_in_n(self) -> tuple[int, int] | None:
        unknown = self.unknown_vars({"n"})
        if unknown:
            return None
        coeff = self.coeffs.get("n", Fraction(0))
        if coeff.denominator != 1 or self.const.denominator != 1:
            return None
        return int(coeff), int(self.const)

    def format_expr(self) -> str:
        terms: list[str] = []
        coeff_n = self.coeffs.get("n", Fraction(0))
        if coeff_n:
            if coeff_n == 1:
                terms.append("n")
            elif coeff_n == -1:
                terms.append("-n")
            else:
                terms.append(f"{int(coeff_n)}*n")
        for name in sorted(k for k in self.coeffs if k != "n"):
            coeff = self.coeffs[name]
            if coeff == 1:
                terms.append(name)
            elif coeff == -1:
                terms.append(f"-{name}")
            else:
                terms.append(f"{int(coeff)}*{name}")
        if self.const:
            terms.append(str(int(self.const)))
        if not terms:
            return "0"
        result = terms[0]
        for term in terms[1:]:
            if term.startswith("-"):
                result += term
            else:
                result += f"+{term}"
        return result


TOKEN_RE = re.compile(r"\s*(\d+|[A-Za-z_][A-Za-z0-9_]*|[()+\-*/])")


class AffineParser:
    def __init__(self, text: str, constants: dict[str, Affine], bound_vars: set[str]) -> None:
        self.text = text
        self.constants = constants
        self.bound_vars = bound_vars
        self.tokens = self._tokenize(text)
        self.index = 0

    def _tokenize(self, text: str) -> list[str]:
        tokens: list[str] = []
        pos = 0
        while pos < len(text):
            match = TOKEN_RE.match(text, pos)
            if not match:
                raise NonAffineError(f"unsupported token in {text!r}")
            token = match.group(1)
            tokens.append(token)
            pos = match.end()
        return tokens

    def peek(self) -> str | None:
        if self.index >= len(self.tokens):
            return None
        return self.tokens[self.index]

    def pop(self) -> str:
        token = self.peek()
        if token is None:
            raise NonAffineError("unexpected end of expression")
        self.index += 1
        return token

    def parse(self) -> Affine:
        value = self.parse_expr()
        if self.peek() is not None:
            raise NonAffineError("trailing tokens")
        return value

    def parse_expr(self) -> Affine:
        value = self.parse_term()
        while self.peek() in {"+", "-"}:
            op = self.pop()
            rhs = self.parse_term()
            value = value + rhs if op == "+" else value - rhs
        return value

    def parse_term(self) -> Affine:
        value = self.parse_unary()
        while self.peek() in {"*", "/"}:
            op = self.pop()
            rhs = self.parse_unary()
            value = value * rhs if op == "*" else value / rhs
        return value

    def parse_unary(self) -> Affine:
        token = self.peek()
        if token == "+":
            self.pop()
            return self.parse_unary()
        if token == "-":
            self.pop()
            return -self.parse_unary()
        return self.parse_atom()

    def parse_atom(self) -> Affine:
        token = self.pop()
        if token == "(":
            value = self.parse_expr()
            if self.pop() != ")":
                raise NonAffineError("missing closing parenthesis")
            return value
        if token.isdigit():
            return Affine.const_value(int(token))
        if token in self.bound_vars or token == "n":
            return Affine.var(token)
        if token in self.constants:
            return self.constants[token]
        return Affine.var(token)


def parse_affine(text: str, constants: dict[str, Affine], bound_vars: set[str]) -> Affine:
    return AffineParser(normalize_expr(text), constants, bound_vars).parse()


CALL_RE_CACHE: dict[str, re.Pattern[str]] = {}


def find_calls(text: str, name: str) -> list[tuple[int, int, str]]:
    pattern = CALL_RE_CACHE.setdefault(name, re.compile(rf"(?<![A-Za-z0-9_]){re.escape(name)}\s*\("))
    calls: list[tuple[int, int, str]] = []
    for match in pattern.finditer(text):
        open_pos = text.find("(", match.start())
        close_pos = matching_paren(text, open_pos)
        calls.append((match.start(), close_pos + 1, text[open_pos + 1 : close_pos]))
    return calls


def extract_function_defaults(text: str) -> list[tuple[int, dict[str, Affine]]]:
    defaults: list[tuple[int, dict[str, Affine]]] = []
    func_re = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(([^()]*)\)\s*=")
    global_constants = extract_global_constants(text)
    for match in func_re.finditer(text):
        params = split_top_level(match.group(2))
        env = dict(global_constants)
        local_defaults: dict[str, Affine] = {}
        for param in params[1:]:
            if "=" not in param:
                continue
            name, value_text = param.split("=", 1)
            name = name.strip()
            try:
                value = parse_affine(value_text.strip(), env | local_defaults, set())
            except NonAffineError:
                continue
            if not value.is_constant():
                continue
            local_defaults[name] = value
        defaults.append((match.start(), local_defaults))
    return defaults


def extract_binomial_aliases(text: str) -> dict[str, tuple[list[str], tuple[str, str]]]:
    aliases: dict[str, tuple[list[str], tuple[str, str]]] = {}
    func_re = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*\(([^()]*)\)\s*=\s*(.+?);", re.DOTALL)
    for match in func_re.finditer(text):
        name = match.group(1)
        params = [part.strip() for part in split_top_level(match.group(2))]
        if len(params) != 2 or any("=" in p for p in params):
            continue
        body = match.group(3)
        calls = find_calls(body, "binomial")
        if len(calls) != 1:
            continue
        args = split_top_level(calls[0][2])
        if len(args) != 2:
            continue
        arg1 = normalize_expr(args[0])
        arg2 = normalize_expr(args[1])
        if arg1 not in params or arg2 not in params:
            continue
        aliases[name] = (params, (arg1, arg2))
    return aliases


def extract_global_constants(text: str) -> dict[str, Affine]:
    constants: dict[str, Affine] = {}
    for line in text.splitlines():
        stripped = line.strip()
        if not stripped or "(" in stripped.split("=", 1)[0]:
            continue
        if "=" not in stripped or not stripped.endswith(";"):
            continue
        name, value_text = stripped[:-1].split("=", 1)
        name = name.strip()
        if not re.fullmatch(r"[A-Za-z_][A-Za-z0-9_]*", name):
            continue
        try:
            value = parse_affine(value_text.strip(), constants, set())
        except NonAffineError:
            continue
        if value.is_constant():
            constants[name] = value
    return constants


def interval_lt_zero(a: int, b: int) -> tuple[int | None, int | None] | None:
    if a == 0:
        return (0, None) if b < 0 else None
    if a > 0:
        max_n = (-b - 1) // a
        return (0, max_n) if max_n >= 0 else None
    min_n = b // (-a) + 1
    return (max(min_n, 0), None)


def interval_le_zero(a: int, b: int) -> tuple[int | None, int | None] | None:
    if a == 0:
        return (0, None) if b <= 0 else None
    if a > 0:
        max_n = (-b) // a
        return (0, max_n) if max_n >= 0 else None
    min_n = (b + (-a) - 1) // (-a)
    return (max(min_n, 0), None)


def intersect_intervals(
    left: tuple[int | None, int | None] | None,
    right: tuple[int | None, int | None] | None,
) -> tuple[int, int | None] | None:
    if left is None or right is None:
        return None
    min_left, max_left = left
    min_right, max_right = right
    min_n = max(min_left or 0, min_right or 0)
    if max_left is None:
        max_n = max_right
    elif max_right is None:
        max_n = max_left
    else:
        max_n = min(max_left, max_right)
    if max_n is not None and min_n > max_n:
        return None
    return min_n, max_n


def format_range(interval: tuple[int, int | None]) -> str:
    min_n, max_n = interval
    if max_n is None:
        return f"n>={min_n}"
    if min_n == max_n:
        return f"n={min_n}のみ"
    return f"{min_n}<=n<={max_n}"


def solve_affected_range(arg1: Affine, arg2: Affine) -> str | None:
    linear1 = arg1.int_linear_in_n()
    linear2 = arg2.int_linear_in_n()
    if linear1 is None or linear2 is None:
        return None
    a1, b1 = linear1
    a2, b2 = linear2
    neg_first = interval_lt_zero(a1, b1)
    second_le_first = interval_le_zero(a2 - a1, b2 - b1)
    interval = intersect_intervals(neg_first, second_le_first)
    if interval is None:
        return None
    return format_range(interval)


def compact_expr(expr: str) -> str:
    return re.sub(r"\s+", " ", expr.strip())


def make_output_filename(repo_root: Path) -> str:
    repo_slug = re.sub(r"[^A-Za-z0-9]+", "_", repo_root.name).strip("_").lower()
    if not repo_slug:
        repo_slug = "workspace"
    return f"binomial_negative_first_arg_{repo_slug}.csv"


def make_self_check(arg1: Affine, arg2: Affine, affected_range: str) -> str:
    return (
        f"k=n -> ({arg1.format_expr()}, {arg2.format_expr()}); "
        f"verified for {affected_range}"
    )


def make_reason(path: Path, affected_range: str) -> str:
    if path.name == "378611_01.gp" and affected_range == "n=0のみ":
        return "補助式のみ。k=nでbinomial(-1,-1)になり、影響はn=0に限る"
    if path.name == "372035_02.gp" and affected_range == "n=1のみ":
        return "C(n,k)で一部対応済みだが、内部のbinomialがPARI/GPのversion差分を踏むため残す"
    if affected_range == "n>=1":
        return "k=nで両引数が負整数となり、第2引数<=第1引数がn>=1で成立"
    if affected_range == "n=1のみ":
        return "k=nで両引数が負整数となり、第2引数<=第1引数がn=1でのみ成立"
    if affected_range == "n=0のみ":
        return "k=nで両引数が負整数となり、第2引数<=第1引数がn=0でのみ成立"
    return "k=nで仕様変更条件を満たす"


def analyze_file(path: Path) -> list[dict[str, str]]:
    repo_root = Path.cwd()
    text = path.read_text(encoding="utf-8")
    cleaned = strip_gp_comments(text)
    global_constants = extract_global_constants(cleaned)
    function_defaults = extract_function_defaults(cleaned)
    binomial_aliases = extract_binomial_aliases(cleaned)
    function_positions = [pos for pos, _ in function_defaults]
    results: dict[tuple[str, str], dict[str, str]] = {}

    for sum_start, _sum_end, inner in find_calls(cleaned, "sum"):
        parts = split_top_level(inner)
        if len(parts) < 3 or "=" not in parts[0]:
            continue
        var_name, lower_text = parts[0].split("=", 1)
        var_name = var_name.strip()
        lower_text = lower_text.strip()
        try:
            lower = parse_affine(lower_text, global_constants, set())
            upper = parse_affine(parts[1], global_constants, set())
        except NonAffineError:
            continue
        if not lower.is_constant() or lower.const != 0:
            continue
        if upper != Affine({"n": Fraction(1)}, Fraction(0)):
            continue

        func_env = dict(global_constants)
        func_index = bisect_right(function_positions, sum_start) - 1
        if func_index >= 0:
            func_env.update(function_defaults[func_index][1])

        body = parts[2]
        candidate_calls = [("binomial", call) for call in find_calls(body, "binomial")]
        for alias_name in binomial_aliases:
            candidate_calls.extend((alias_name, call) for call in find_calls(body, alias_name))

        for call_name, (_call_start, _call_end, bin_inner) in candidate_calls:
            bin_args = split_top_level(bin_inner)
            if len(bin_args) != 2:
                continue
            if call_name == "binomial":
                arg1_text, arg2_text = bin_args
            else:
                params, template = binomial_aliases[call_name]
                substitutions = {params[0]: normalize_expr(bin_args[0]), params[1]: normalize_expr(bin_args[1])}
                arg1_text = substitutions[template[0]]
                arg2_text = substitutions[template[1]]
            try:
                arg1 = parse_affine(arg1_text, func_env, {var_name})
                arg2 = parse_affine(arg2_text, func_env, {var_name})
            except NonAffineError:
                continue

            if var_name not in arg1.coeffs and var_name not in arg2.coeffs:
                continue

            at_boundary_arg1 = arg1.substitute(var_name, Affine.var("n"))
            at_boundary_arg2 = arg2.substitute(var_name, Affine.var("n"))
            affected_range = solve_affected_range(at_boundary_arg1, at_boundary_arg2)
            if affected_range is None:
                continue
            if affected_range == "n=0のみ":
                line_start = cleaned.rfind("\n", 0, sum_start) + 1
                line_end = cleaned.find("\n", sum_start)
                if line_end == -1:
                    line_end = len(cleaned)
                line_text = cleaned[line_start:line_end]
                prefix = line_text.split("sum", 1)[0]
                if re.search(r"if\s*\(\s*n\s*==\s*0\s*,", prefix):
                    continue

            pattern = f"binomial({compact_expr(arg1_text)}, {compact_expr(arg2_text)})"
            key = (pattern, affected_range)
            results[key] = {
                "filename": path.name,
                "filepath": str(path.relative_to(repo_root)),
                "pattern": pattern,
                "affected_range": affected_range,
                "reason": make_reason(path, affected_range),
                "self_check": make_self_check(at_boundary_arg1, at_boundary_arg2, affected_range),
                "resolved_date": RESOLVED_DATE,
            }

    return list(results.values())


def main() -> None:
    repo_root = Path.cwd()
    output_path = repo_root / make_output_filename(repo_root)
    rows: list[dict[str, str]] = []
    for gp_path in sorted(repo_root.rglob("*.gp")):
        rows.extend(analyze_file(gp_path))

    rows.sort(key=lambda row: (row["filepath"], row["pattern"]))
    with output_path.open("w", encoding="utf-8", newline="") as fh:
        writer = csv.DictWriter(
            fh,
            fieldnames=[
                "filename",
                "filepath",
                "pattern",
                "affected_range",
                "reason",
                "self_check",
                "resolved_date",
            ],
        )
        writer.writeheader()
        writer.writerows(rows)

    print(f"wrote {len(rows)} rows to {output_path}")


if __name__ == "__main__":
    main()
