use std::fmt;
use std::thread;

const BASE: u64 = 1_000_000_000;

#[derive(Clone, Debug, PartialEq, Eq)]
struct Big {
    digits: Vec<u32>, // little-endian base 1e9
}

impl Big {
    fn zero() -> Self {
        Self { digits: vec![] }
    }

    fn from_u64(mut n: u64) -> Self {
        if n == 0 {
            return Self::zero();
        }
        let mut digits = Vec::new();
        while n > 0 {
            digits.push((n % BASE) as u32);
            n /= BASE;
        }
        Self { digits }
    }

    fn add_assign(&mut self, other: &Big) {
        let n = self.digits.len().max(other.digits.len());
        self.digits.resize(n, 0);

        let mut carry: u64 = 0;
        for i in 0..n {
            let a = self.digits[i] as u64;
            let b = if i < other.digits.len() { other.digits[i] as u64 } else { 0 };
            let sum = a + b + carry;
            self.digits[i] = (sum % BASE) as u32;
            carry = sum / BASE;
        }
        if carry > 0 {
            self.digits.push(carry as u32);
        }
    }

    fn mul_small_assign(&mut self, m: u64) {
        if self.digits.is_empty() || m == 0 {
            self.digits.clear();
            return;
        }
        if m == 1 {
            return;
        }

        let mut carry: u128 = 0;
        for d in &mut self.digits {
            let cur = (*d as u128) * (m as u128) + carry;
            *d = (cur % (BASE as u128)) as u32;
            carry = cur / (BASE as u128);
        }
        while carry > 0 {
            self.digits.push((carry % (BASE as u128)) as u32);
            carry /= BASE as u128;
        }
    }

    fn mod_u64(&self, m: u64) -> u64 {
        if m == 1 || self.digits.is_empty() {
            return 0;
        }
        let mut rem: u128 = 0;
        for &d in self.digits.iter().rev() {
            rem = (rem * (BASE as u128) + d as u128) % (m as u128);
        }
        rem as u64
    }
}

impl fmt::Display for Big {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        if self.digits.is_empty() {
            return write!(f, "0");
        }
        let last = *self.digits.last().unwrap();
        write!(f, "{}", last)?;
        for &d in self.digits[..self.digits.len() - 1].iter().rev() {
            write!(f, "{:09}", d)?;
        }
        Ok(())
    }
}

// --- 数論ユーティリティ ---

fn gcd(a: u64, b: u64) -> u64 {
    if b == 0 { a } else { gcd(b, a % b) }
}

fn mod_inv(a: u64, m: u64) -> u64 {
    let (mut t, mut new_t): (i128, i128) = (0, 1);
    let (mut r, mut new_r): (i128, i128) = (m as i128, a as i128);
    while new_r != 0 {
        let q = r / new_r;
        (t, new_t) = (new_t, t - q * new_t);
        (r, new_r) = (new_r, r - q * new_r);
    }
    if t < 0 {
        t += m as i128;
    }
    t as u64
}

// --- Ryser法 (Gray codeで差分更新 + mod p) ---

const PRIMES: [u64; 6] = [
    2305843009213693951, // 2^61 - 1
    2305843009213693921,
    2305843009213693907,
    2305843009213693723,
    2305843009213693693,
    2305843009213693669,
];

fn mul_mod(a: u64, b: u64, p: u64) -> u64 {
    ((a as u128 * b as u128) % (p as u128)) as u64
}

fn add_mod(a: u64, b: u64, p: u64) -> u64 {
    let s = a + b;
    if s >= p { s - p } else { s }
}

fn permanent_mod_p(matrix: &[Vec<u8>], p: u64) -> u64 {
    let n = matrix.len();
    if n == 0 {
        return 1;
    }
    if n == 1 {
        return (matrix[0][0] as u64) % p;
    }

    let p128 = p as u128;
    let total_subsets = 1u64 << n;
    let chunk_size: u64 = 512;
    let start0 = 1u64;

    let chunk_eval = |start: u64, end: u64| -> u128 {
        let start_gray = start ^ (start >> 1);
        let mut row_sums = vec![0i32; n];
        for bit in 0..n {
            if (start_gray >> bit) & 1 == 1 {
                for i in 0..n {
                    row_sums[i] += matrix[i][bit] as i32;
                }
            }
        }

        let mut local_sum = 0u128;
        let mut prev_gray = start_gray;

        for k in start..end {
            let gray = k ^ (k >> 1);

            if k > start {
                let diff = prev_gray ^ gray;
                let bit = diff.trailing_zeros() as usize;
                if (gray >> bit) & 1 == 1 {
                    for i in 0..n {
                        row_sums[i] += matrix[i][bit] as i32;
                    }
                } else {
                    for i in 0..n {
                        row_sums[i] -= matrix[i][bit] as i32;
                    }
                }
            }
            prev_gray = gray;

            let mut prod = 1u128;
            for &s in &row_sums {
                prod = (prod * (s as u128)) % p128;
                if prod == 0 {
                    break;
                }
            }

            let set_size = gray.count_ones();
            if (n as u32 + set_size) % 2 == 0 {
                local_sum += prod;
            } else {
                local_sum += p128 - prod;
            }
        }

        local_sum % p128
    };

    let threads = thread::available_parallelism().map(|n| n.get()).unwrap_or(1);
    if threads <= 1 || total_subsets <= (chunk_size * 2) {
        let mut total = 0u128;
        let mut start = start0;
        while start < total_subsets {
            let end = (start + chunk_size).min(total_subsets);
            total = (total + chunk_eval(start, end)) % p128;
            start += chunk_size;
        }
        return total as u64;
    }

    let stride = chunk_size * threads as u64;
    let total_sum_u128 = thread::scope(|scope| {
        let mut handles = Vec::with_capacity(threads);
        for tid in 0..threads {
            handles.push(scope.spawn(move || {
                let mut local_total = 0u128;
                let mut start = start0 + (tid as u64) * chunk_size;
                while start < total_subsets {
                    let end = (start + chunk_size).min(total_subsets);
                    local_total = (local_total + chunk_eval(start, end)) % p128;
                    start += stride;
                }
                local_total
            }));
        }

        let mut total = 0u128;
        for h in handles {
            total = (total + h.join().unwrap()) % p128;
        }
        total
    });

    total_sum_u128 as u64
}

fn crt_reconstruct_big(residues: &[u64]) -> Big {
    // Sequential CRT:
    // x = r0 (mod p0), then extend one modulus at a time.
    let mut x = Big::from_u64(residues[0]);
    let mut mod_prod = Big::from_u64(PRIMES[0]);

    for i in 1..residues.len() {
        let p = PRIMES[i];
        let xi = x.mod_u64(p);
        let rhs = (residues[i] + p - xi) % p;
        let inv = mod_inv(mod_prod.mod_u64(p), p);
        let t = ((rhs as u128 * inv as u128) % (p as u128)) as u64;

        let mut delta = mod_prod.clone();
        delta.mul_small_assign(t);
        x.add_assign(&delta);

        mod_prod.mul_small_assign(p);
    }

    x
}

fn all_column_minors_mod_p(matrix: &[Vec<u8>], m: usize, p: u64) -> Vec<u64> {
    let k = matrix.len();
    let total = 1u64 << m;
    let full_mask = total - 1;
    let mut out = vec![0u64; m];
    let mut row_sums = vec![0i32; k];
    let mut prev_gray = 0u64;
    let mut sign = if (k & 1) == 0 { 1 } else { -1 };

    for t in 0..total {
        let gray = t ^ (t >> 1);
        if t > 0 {
            let diff = gray ^ prev_gray;
            let bit = diff.trailing_zeros() as usize;
            if ((gray >> bit) & 1) == 1 {
                for i in 0..k {
                    row_sums[i] += matrix[i][bit] as i32;
                }
            } else {
                for i in 0..k {
                    row_sums[i] -= matrix[i][bit] as i32;
                }
            }
            sign = -sign;
        }
        prev_gray = gray;

        let mut prod = 1u64;
        for &s in &row_sums {
            prod = mul_mod(prod, s as u64, p);
            if prod == 0 {
                break;
            }
        }
        let term = if sign > 0 { prod } else if prod == 0 { 0 } else { p - prod };

        let mut cmask = (!gray) & full_mask;
        while cmask != 0 {
            let j = cmask.trailing_zeros() as usize;
            out[j] = add_mod(out[j], term, p);
            cmask &= cmask - 1;
        }
    }

    out
}

fn jackson2_mod(n: usize, p: u64) -> u64 {
    if (n & 1) == 0 {
        let m = n / 2;
        let mut matrix = vec![vec![0u8; m]; m];
        for i in 0..m {
            for j in 0..m {
                if gcd((2 * (i + 1)) as u64, (2 * (j + 1) - 1) as u64) == 1 {
                    matrix[i][j] = 1;
                }
            }
        }
        let s = permanent_mod_p(&matrix, p);
        return mul_mod(s, s, p);
    }

    let m = (n + 1) / 2;
    let k = m - 1;
    let mut a = vec![vec![0u8; m]; k];
    for i in 0..k {
        for j in 0..m {
            if gcd((2 * (i + 1)) as u64, (2 * (j + 1) - 1) as u64) == 1 {
                a[i][j] = 1;
            }
        }
    }

    let b = all_column_minors_mod_p(&a, m, p);
    let mut s = 0u64;
    for i in 0..m {
        for j in 0..m {
            if gcd((2 * (i + 1) - 1) as u64, (2 * (j + 1) - 1) as u64) == 1 {
                s = add_mod(s, mul_mod(b[i], b[j], p), p);
            }
        }
    }
    s
}

// --- Jackson2 ロジック ---

fn jackson2(n: u64) -> Big {
    let residues: Vec<u64> = PRIMES
        .iter()
        .map(|&p| jackson2_mod(n as usize, p))
        .collect();
    crt_reconstruct_big(&residues)
}

fn main() {
    use std::fs::File;
    use std::io::{BufWriter, Write};

    let file = File::create("b005326_01.txt").expect("ファイルを作成できません");
    let mut writer = BufWriter::new(file);

    for n in 1u64..=60 {
        writeln!(writer, "{} {}", n, jackson2(n)).expect("書き込みに失敗しました");
        writer.flush().expect("flushに失敗しました");
    }
}
