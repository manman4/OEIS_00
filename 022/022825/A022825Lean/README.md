# A022825Lean

This Lean 4 project proves the following enumeration theorem related to
[OEIS A022825](https://oeis.org/A022825).

For every natural number `n`, the number of permutations `sigma` of
`{1, ..., n}` satisfying

```text
sigma(k) mod sigma(k+1) < sigma(k+1) mod sigma(k+2)
```

for all `1 <= k <= n - 2` is `A022825(n + 1)`.  A022825 is characterized on
the positive integers by

```text
a(1) = 1,
a(N) = sum_{j=2..N} a(floor(N / j))    for N > 1.
```

## Formal result

`RemainderPermutation n` is the finite type of the permutations in the
statement, and `a022825 N` is the finite cardinality of strict divisor chains
bounded by `N`.  The top-level theorem is

```lean
theorem remainderPermutation_card_eq_a022825 (n : Nat) :
    Fintype.card (RemainderPermutation n) = a022825 (n + 1)
```

The project also proves

```lean
@[simp] theorem a022825_one : a022825 1 = 1

theorem a022825_recurrence (N : Nat) (hN : 1 < N) :
    a022825 N = ∑ j ∈ Finset.Icc 2 N, a022825 (N / j)
```

and a uniqueness theorem showing that these two equations characterize
`a022825` on all positive natural numbers.

## Proof outline

The proof is general; it is not a finite verification of initial terms.

1. Every strict remainder permutation is decomposed into consecutive rotated
   blocks.
2. The block endpoints form a strict divisor chain with first endpoint at
   least `3`.
3. Conversely, every such divisor chain constructs a valid remainder
   permutation.  The two constructions are proved to give a bijection.
4. A second bijection decomposes every divisor chain bounded by `N` according
   to its last multiplier `j`.  Counting this bijection proves the A022825
   recurrence.
5. Strong induction proves uniqueness of the sequence determined by the
   initial value and recurrence.

## Reproducible build

The project pins both Lean and Mathlib to `v4.27.0`.

```sh
lake update
lake exe cache get
lake build
```

To display the axiom dependencies of the public declarations, run

```sh
lake build A022825Lean.AxiomAudit
```

The audit reports only Lean/Mathlib's standard axioms `propext`,
`Classical.choice`, and `Quot.sound`.  The project contains no `sorry`, custom
axioms, or bounded computation used in place of the theorem.

All project paths are relative, so the `A022825Lean` directory can be moved as
a unit and built in another location.
