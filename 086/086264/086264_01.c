#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

// Count {0,1} N x N matrices with determinant = 1.
// Brute force with pruning and exact integer determinant (Bareiss).

static int N;
static long long count_det1 = 0;

// Fraction-free Gaussian elimination (Bareiss).
// Exact for integer matrices if no overflow.
static long long determinant_bareiss(const uint64_t *rows) {
    long long a[20 * 20];

    for (int i = 0; i < N; i++) {
        uint64_t r = rows[i];
        for (int j = 0; j < N; j++) {
            a[i * N + j] = (r >> (N - 1 - j)) & 1ULL;
        }
    }

    long long det_sign = 1;
    long long prev_pivot = 1;

    for (int k = 0; k < N - 1; k++) {
        int pivot = k;
        while (pivot < N && a[pivot * N + k] == 0) pivot++;
        if (pivot == N) {
            return 0;
        }
        if (pivot != k) {
            for (int j = 0; j < N; j++) {
                long long tmp = a[k * N + j];
                a[k * N + j] = a[pivot * N + j];
                a[pivot * N + j] = tmp;
            }
            det_sign = -det_sign;
        }

        long long pivot_val = a[k * N + k];
        for (int i = k + 1; i < N; i++) {
            for (int j = k + 1; j < N; j++) {
                __int128 num = (__int128)a[i * N + j] * pivot_val
                             - (__int128)a[i * N + k] * a[k * N + j];
                if (k > 0) num /= prev_pivot;
                a[i * N + j] = (long long)num;
            }
        }
        prev_pivot = pivot_val;
    }

    long long det = det_sign * a[(N - 1) * N + (N - 1)];
    return det;
}

// Determinant modulo p using Gaussian elimination over Z_p.
// Returns det mod p in [0, p-1].
static int det_mod_p(const uint64_t *rows, int p) {
    int a[20 * 20];

    for (int i = 0; i < N; i++) {
        uint64_t r = rows[i];
        for (int j = 0; j < N; j++) {
            a[i * N + j] = (int)((r >> (N - 1 - j)) & 1ULL);
        }
    }

    int det = 1 % p;
    for (int k = 0; k < N; k++) {
        int pivot = k;
        while (pivot < N && a[pivot * N + k] % p == 0) pivot++;
        if (pivot == N) {
            return 0;
        }
        if (pivot != k) {
            for (int j = 0; j < N; j++) {
                int tmp = a[k * N + j];
                a[k * N + j] = a[pivot * N + j];
                a[pivot * N + j] = tmp;
            }
            det = (p - det) % p; // row swap flips sign
        }

        int pivot_val = a[k * N + k] % p;
        det = (det * pivot_val) % p;

        // Compute pivot inverse mod p (p is prime).
        int inv = 1;
        int base = pivot_val;
        int exp = p - 2;
        while (exp > 0) {
            if (exp & 1) inv = (int)((long long)inv * base % p);
            base = (int)((long long)base * base % p);
            exp >>= 1;
        }

        for (int i = k + 1; i < N; i++) {
            int factor = (int)((long long)a[i * N + k] * inv % p);
            if (factor == 0) continue;
            for (int j = k + 1; j < N; j++) {
                int v = a[i * N + j] - (int)((long long)factor * a[k * N + j] % p);
                if (v < 0) v += p;
                a[i * N + j] = v;
            }
        }
    }

    return det % p;
}

static bool add_to_basis(uint64_t v, uint64_t *basis, int *rank, int *changed_idx) {
    for (int b = N - 1; b >= 0; b--) {
        if (((v >> b) & 1ULL) == 0) continue;
        if (basis[b] == 0) {
            basis[b] = v;
            (*rank)++;
            *changed_idx = b;
            return true;
        }
        v ^= basis[b];
    }
    return false; // dependent over F2
}

static int mod_inv(int a, int p) {
    int res = 1;
    int base = a % p;
    int exp = p - 2;
    while (exp > 0) {
        if (exp & 1) res = (int)((long long)res * base % p);
        base = (int)((long long)base * base % p);
        exp >>= 1;
    }
    return res;
}

// Add row to basis over F_p. Basis is stored by pivot column (0..N-1).
// Returns true if rank increases; sets *changed_col to pivot column.
static bool add_to_basis_modp(uint64_t mask, int p, int basis[20][20],
                              int *rank, int *changed_col) {
    int row[20];
    for (int j = 0; j < N; j++) {
        row[j] = (int)((mask >> (N - 1 - j)) & 1ULL) % p;
    }

    for (int col = 0; col < N; col++) {
        if (row[col] == 0) continue;
        if (basis[col][col] == 0) {
            int inv = mod_inv(row[col], p);
            for (int j = col; j < N; j++) {
                basis[col][j] = (int)((long long)row[j] * inv % p);
            }
            (*rank)++;
            *changed_col = col;
            return true;
        } else {
            int factor = row[col];
            for (int j = col; j < N; j++) {
                int v = row[j] - (int)((long long)factor * basis[col][j] % p);
                if (v < 0) v += p;
                row[j] = v;
            }
        }
    }
    return false;
}

// Enumerate strictly increasing row masks to remove row permutations.
static void backtrack(int row, uint64_t start_mask, uint64_t *rows,
                      uint64_t *basis_f2, int rank_f2,
                      int basis_modp[][20][20], int *rank_modp,
                      const int *primes, int pcount) {
    if (row == N) {
        // Multi-modulus filter: reject quickly if det != ±1 (mod p)
        for (int i = 0; i < pcount; i++) {
            int p = primes[i];
            int dm = det_mod_p(rows, p);
            if (!(dm == 1 || dm == p - 1)) return;
        }
        long long d = determinant_bareiss(rows);
        if (d == 1 || d == -1) count_det1++;
        return;
    }

    // Need all rows independent over F2 for det to be odd.
    // If remaining rows cannot reach full rank, prune.
    int remaining = N - row;
    if (rank_f2 + remaining < N) return;
    for (int i = 0; i < pcount; i++) {
        if (rank_modp[i] + remaining < N) return;
    }

    uint64_t max_mask = (N == 64) ? UINT64_MAX : ((1ULL << N) - 1ULL);
    for (uint64_t mask = start_mask; mask <= max_mask; mask++) {
        int changed_f2 = -1;
        int rank2_f2 = rank_f2;
        if (!add_to_basis(mask, basis_f2, &rank2_f2, &changed_f2)) continue; // det even

        int changed_cols[8];
        for (int i = 0; i < pcount; i++) changed_cols[i] = -1;
        int rank2_modp[8];
        for (int i = 0; i < pcount; i++) rank2_modp[i] = rank_modp[i];

        bool ok = true;
        for (int i = 0; i < pcount; i++) {
            if (!add_to_basis_modp(mask, primes[i], basis_modp[i],
                                   &rank2_modp[i], &changed_cols[i])) {
                // rank did not increase; still ok
            }
            if (rank2_modp[i] + (remaining - 1) < N) {
                ok = false;
                break;
            }
        }

        if (ok) {
            rows[row] = mask;
            backtrack(row + 1, mask + 1, rows,
                      basis_f2, rank2_f2,
                      basis_modp, rank2_modp,
                      primes, pcount);
        }

        basis_f2[changed_f2] = 0;
        for (int i = 0; i < pcount; i++) {
            if (changed_cols[i] >= 0) {
                for (int j = 0; j < N; j++) basis_modp[i][changed_cols[i]][j] = 0;
            }
        }
    }
}

int main(void) {
    int Nmax;
    printf("Enter Nmax: ");
    if (scanf("%d", &Nmax) != 1 || Nmax < 0 || Nmax > 20) {
        fprintf(stderr, "Nmax must be in 0..20 for this implementation.\n");
        return 1;
    }

    FILE *out = fopen("b086264.txt", "w");
    if (!out) {
        perror("b086264.txt");
        return 1;
    }
    setvbuf(out, NULL, _IOLBF, 0); // line-buffered for sequential output

    for (N = 0; N <= Nmax; N++) {
        if (N == 0) {
            fprintf(out, "0 1\n");
            continue;
        }

        if (N >= 63) {
            fprintf(stderr, "N too large for 64-bit row masks.\n");
            fclose(out);
            return 1;
        }

        uint64_t *rows = (uint64_t *)malloc((size_t)N * sizeof(uint64_t));
        if (!rows) {
            fclose(out);
            return 1;
        }

        count_det1 = 0;
        uint64_t basis_f2[64] = {0};
        static const int primes[] = {2, 3, 5};
        const int pcount = (int)(sizeof(primes) / sizeof(primes[0]));
        int basis_modp[8][20][20] = {{{0}}};
        int rank_modp[8] = {0};

        backtrack(0, 1, rows,
                  basis_f2, 0,
                  basis_modp, rank_modp,
                  primes, pcount);

        long long total;
        if (N == 1) {
            total = count_det1;
        } else {
            // Each matrix with distinct rows has exactly n!/2 row permutations
            // that preserve det=1. (Odd permutations flip the sign.)
            long long fact = 1;
            for (int i = 2; i <= N; i++) fact *= i;
            total = count_det1 * (fact / 2);
        }

        fprintf(out, "%d %lld\n", N, total);
        fflush(out);

        free(rows);
    }

    fclose(out);
    return 0;
}
