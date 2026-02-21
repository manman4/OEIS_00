#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define MAX_PRIMES 6
#define BI_BASE 1000000000U

typedef struct {
    uint32_t *d;
    int len;
    int cap;
} BigInt;

/* Distinct 61-bit primes */
static const uint64_t PRIMES[MAX_PRIMES] = {
    2305843009213693951ULL, /* 2^61 - 1 */
    2305843009213693921ULL,
    2305843009213693907ULL,
    2305843009213693723ULL,
    2305843009213693693ULL,
    2305843009213693669ULL
};

static void bi_init(BigInt *a) {
    a->d = NULL;
    a->len = 0;
    a->cap = 0;
}

static void bi_free(BigInt *a) {
    free(a->d);
    a->d = NULL;
    a->len = 0;
    a->cap = 0;
}

static void bi_reserve(BigInt *a, int need) {
    if (need <= a->cap) return;
    int ncap = (a->cap == 0) ? 4 : a->cap;
    while (ncap < need) ncap <<= 1;
    uint32_t *nd = (uint32_t *)realloc(a->d, (size_t)ncap * sizeof(uint32_t));
    if (!nd) {
        fprintf(stderr, "allocation failed\n");
        exit(1);
    }
    a->d = nd;
    a->cap = ncap;
}

static void bi_trim(BigInt *a) {
    while (a->len > 0 && a->d[a->len - 1] == 0) a->len--;
}

static void bi_set_u64(BigInt *a, uint64_t v) {
    if (v == 0) {
        a->len = 0;
        return;
    }
    bi_reserve(a, 3);
    a->len = 0;
    while (v > 0) {
        a->d[a->len++] = (uint32_t)(v % BI_BASE);
        v /= BI_BASE;
    }
}

static uint64_t bi_mod_u64(const BigInt *a, uint64_t m) {
    uint64_t rem = 0;
    for (int i = a->len - 1; i >= 0; --i) {
        __uint128_t x = (__uint128_t)rem * BI_BASE + a->d[i];
        rem = (uint64_t)(x % m);
    }
    return rem;
}

static void bi_mul_u64(BigInt *a, uint64_t m) {
    if (a->len == 0 || m == 0) {
        a->len = 0;
        return;
    }

    bi_reserve(a, a->len + 8);
    __uint128_t carry = 0;
    for (int i = 0; i < a->len; ++i) {
        __uint128_t x = (__uint128_t)a->d[i] * m + carry;
        a->d[i] = (uint32_t)(x % BI_BASE);
        carry = x / BI_BASE;
    }
    while (carry > 0) {
        bi_reserve(a, a->len + 1);
        a->d[a->len++] = (uint32_t)(carry % BI_BASE);
        carry /= BI_BASE;
    }
}

static void bi_add_mul_u64(BigInt *a, const BigInt *b, uint64_t mul) {
    if (mul == 0 || b->len == 0) return;

    bi_reserve(a, b->len + 8);
    if (a->len < b->len) {
        memset(a->d + a->len, 0, (size_t)(b->len - a->len) * sizeof(uint32_t));
        a->len = b->len;
    }

    __uint128_t carry = 0;
    int i = 0;
    for (; i < b->len; ++i) {
        __uint128_t x = (__uint128_t)b->d[i] * mul + carry + a->d[i];
        a->d[i] = (uint32_t)(x % BI_BASE);
        carry = x / BI_BASE;
    }

    while (carry > 0) {
        if (i >= a->len) {
            bi_reserve(a, a->len + 1);
            a->d[a->len++] = 0;
        }
        __uint128_t x = (__uint128_t)a->d[i] + carry;
        a->d[i] = (uint32_t)(x % BI_BASE);
        carry = x / BI_BASE;
        i++;
    }

    if (i > a->len) a->len = i;
    bi_trim(a);
}

static void bi_print(const BigInt *a) {
    if (a->len == 0) {
        printf("0");
        return;
    }
    printf("%u", a->d[a->len - 1]);
    for (int i = a->len - 2; i >= 0; --i) {
        printf("%09u", a->d[i]);
    }
}

static int igcd_int(int a, int b) {
    while (b != 0) {
        int t = a % b;
        a = b;
        b = t;
    }
    return (a < 0) ? -a : a;
}

static inline uint64_t add_mod(uint64_t a, uint64_t b, uint64_t p) {
    uint64_t s = a + b;
    return (s >= p) ? (s - p) : s;
}

static inline uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t p) {
    return (a >= b) ? (a - b) : (a + (p - b));
}

static inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t p) {
    __uint128_t z = (__uint128_t)a * (__uint128_t)b;
    return (uint64_t)(z % p);
}

static uint64_t pow_mod(uint64_t a, uint64_t e, uint64_t p) {
    uint64_t r = 1;
    while (e > 0) {
        if (e & 1ULL) r = mul_mod(r, a, p);
        a = mul_mod(a, a, p);
        e >>= 1ULL;
    }
    return r;
}

static uint64_t modinv_u64(uint64_t a, uint64_t p) {
    return pow_mod(a, p - 2, p);
}

static inline int popcount_u64(uint64_t x) {
    return __builtin_popcountll(x);
}

static void fill_rowsum_from_gray(const uint8_t *A, int rows, int cols, uint64_t gray, int *rowsum) {
    for (int i = 0; i < rows; ++i) rowsum[i] = 0;
    uint64_t mask = gray;
    while (mask) {
        int bit = __builtin_ctzll(mask);
        for (int i = 0; i < rows; ++i) rowsum[i] += A[(size_t)i * cols + bit];
        mask &= (mask - 1ULL);
    }
}

static uint64_t permanent_mod_u8_parallel(const uint8_t *A, int n, uint64_t p) {
    if (n == 0) return 1;

    uint64_t total = 1ULL << n;
    uint64_t masks = total - 1ULL; /* skip empty subset */

    int nthreads = 1;
#ifdef _OPENMP
    nthreads = omp_get_max_threads();
#endif
    int chunks = nthreads * 8;
    if ((uint64_t)chunks > masks) chunks = (int)masks;
    if (chunks < 1) chunks = 1;

    uint64_t *chunk_acc = (uint64_t *)calloc((size_t)chunks, sizeof(uint64_t));
    if (!chunk_acc) {
        fprintf(stderr, "allocation failed\n");
        exit(1);
    }

#pragma omp parallel for schedule(static) if(chunks > 1)
    for (int c = 0; c < chunks; ++c) {
        uint64_t start = 1ULL + ((uint64_t)c * masks) / (uint64_t)chunks;
        uint64_t end = 1ULL + ((uint64_t)(c + 1) * masks) / (uint64_t)chunks;
        if (start >= end) {
            chunk_acc[c] = 0;
            continue;
        }

        int *rowsum = (int *)calloc((size_t)n, sizeof(int));
        if (!rowsum) {
            fprintf(stderr, "allocation failed\n");
            exit(1);
        }

        uint64_t g = start ^ (start >> 1);
        fill_rowsum_from_gray(A, n, n, g, rowsum);

        int sign = ((n - popcount_u64(g)) & 1) ? -1 : 1;
        uint64_t prev_g = g;
        uint64_t acc = 0;

        for (uint64_t t = start; t < end; ++t) {
            if (t > start) {
                g = t ^ (t >> 1);
                uint64_t diff = g ^ prev_g;
                int bit = __builtin_ctzll(diff);
                int add = (g & (1ULL << bit)) ? 1 : 0;

                for (int i = 0; i < n; ++i) {
                    uint8_t v = A[(size_t)i * n + bit];
                    rowsum[i] += add ? v : -v;
                }
                sign = -sign;
                prev_g = g;
            }

            uint64_t prod = 1;
            for (int i = 0; i < n; ++i) {
                prod = mul_mod(prod, (uint64_t)rowsum[i], p);
                if (prod == 0) break;
            }

            acc = (sign > 0) ? add_mod(acc, prod, p) : sub_mod(acc, prod, p);
        }

        free(rowsum);
        chunk_acc[c] = acc;
    }

    uint64_t ans = 0;
    for (int c = 0; c < chunks; ++c) ans = add_mod(ans, chunk_acc[c], p);
    free(chunk_acc);
    return ans;
}

static void all_column_minors_mod_u8_parallel(const uint8_t *A, int k, int m, uint64_t p, uint64_t *bout) {
    uint64_t total = 1ULL << m;
    uint64_t full_mask = total - 1ULL;

    for (int j = 0; j < m; ++j) bout[j] = 0;

    int nthreads = 1;
#ifdef _OPENMP
    nthreads = omp_get_max_threads();
#endif
    int chunks = nthreads * 8;
    if ((uint64_t)chunks > total) chunks = (int)total;
    if (chunks < 1) chunks = 1;

    uint64_t *chunk_b = (uint64_t *)calloc((size_t)chunks * m, sizeof(uint64_t));
    if (!chunk_b) {
        fprintf(stderr, "allocation failed\n");
        exit(1);
    }

#pragma omp parallel for schedule(static) if(chunks > 1)
    for (int c = 0; c < chunks; ++c) {
        uint64_t start = ((uint64_t)c * total) / (uint64_t)chunks;
        uint64_t end = ((uint64_t)(c + 1) * total) / (uint64_t)chunks;
        if (start >= end) continue;

        uint64_t *lb = chunk_b + (size_t)c * m;
        int *rowsum = NULL;
        if (k > 0) {
            rowsum = (int *)calloc((size_t)k, sizeof(int));
            if (!rowsum) {
                fprintf(stderr, "allocation failed\n");
                exit(1);
            }
        }

        uint64_t g = start ^ (start >> 1);
        if (k > 0) fill_rowsum_from_gray(A, k, m, g, rowsum);

        int sign = ((k + popcount_u64(g)) & 1) ? -1 : 1;
        uint64_t prev_g = g;

        for (uint64_t t = start; t < end; ++t) {
            if (t > start) {
                g = t ^ (t >> 1);
                uint64_t diff = g ^ prev_g;
                int bit = __builtin_ctzll(diff);
                int add = (g & (1ULL << bit)) ? 1 : 0;

                for (int i = 0; i < k; ++i) {
                    uint8_t v = A[(size_t)i * m + bit];
                    rowsum[i] += add ? v : -v;
                }
                sign = -sign;
                prev_g = g;
            }

            uint64_t prod = 1;
            for (int i = 0; i < k; ++i) {
                prod = mul_mod(prod, (uint64_t)rowsum[i], p);
                if (prod == 0) break;
            }

            uint64_t term = (sign > 0) ? prod : (prod == 0 ? 0 : (p - prod));
            uint64_t cmask = (~g) & full_mask;
            while (cmask) {
                int j = __builtin_ctzll(cmask);
                lb[j] = add_mod(lb[j], term, p);
                cmask &= (cmask - 1ULL);
            }
        }

        free(rowsum);
    }

    for (int c = 0; c < chunks; ++c) {
        uint64_t *lb = chunk_b + (size_t)c * m;
        for (int j = 0; j < m; ++j) bout[j] = add_mod(bout[j], lb[j], p);
    }

    free(chunk_b);
}

static uint64_t jackson2_mod(int n, uint64_t p) {
    if ((n & 1) == 0) {
        int m = n / 2;
        uint8_t *M = (uint8_t *)calloc((size_t)m * m, sizeof(uint8_t));
        if (!M) {
            fprintf(stderr, "allocation failed\n");
            exit(1);
        }

        for (int i = 1; i <= m; ++i) {
            for (int j = 1; j <= m; ++j) {
                if (igcd_int(2 * i, 2 * j - 1) == 1) M[(size_t)(i - 1) * m + (j - 1)] = 1;
            }
        }

        uint64_t s = permanent_mod_u8_parallel(M, m, p);
        free(M);
        return mul_mod(s, s, p);
    }

    int m = (n + 1) / 2;
    int k = m - 1;

    uint8_t *A = NULL;
    if (k > 0) {
        A = (uint8_t *)calloc((size_t)k * m, sizeof(uint8_t));
        if (!A) {
            fprintf(stderr, "allocation failed\n");
            exit(1);
        }
    }

    uint64_t *b = (uint64_t *)calloc((size_t)m, sizeof(uint64_t));
    if (!b) {
        fprintf(stderr, "allocation failed\n");
        exit(1);
    }

    for (int i = 1; i <= k; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (igcd_int(2 * i, 2 * j - 1) == 1) A[(size_t)(i - 1) * m + (j - 1)] = 1;
        }
    }

    all_column_minors_mod_u8_parallel(A, k, m, p, b);

    uint64_t s = 0;
    for (int i = 1; i <= m; ++i) {
        for (int j = 1; j <= m; ++j) {
            if (igcd_int(2 * i - 1, 2 * j - 1) == 1) {
                uint64_t t = mul_mod(b[i - 1], b[j - 1], p);
                s = add_mod(s, t, p);
            }
        }
    }

    free(A);
    free(b);
    return s;
}

static double log2_factorial(int n) {
    if (n < 2) return 0.0;
    double s = 0.0;
    for (int i = 2; i <= n; ++i) s += log2((double)i);
    return s;
}

/* Conservative upper bound bits for Jackson2(n). */
static int required_bits_bound(int n) {
    double bits;
    if ((n & 1) == 0) {
        int m = n / 2;
        bits = 2.0 * log2_factorial(m);
    } else {
        int m = (n + 1) / 2;
        int k = m - 1;
        bits = 2.0 * log2_factorial(k) + 2.0 * log2((double)m);
    }
    return (int)(bits + 4.0); /* safety margin */
}

static int choose_prime_count_for_n(int n) {
    int need = required_bits_bound(n);
    int got = 0;
    for (int i = 0; i < MAX_PRIMES; ++i) {
        got += 61;
        if (got >= need) return i + 1;
    }
    return -1;
}

static void crt_reconstruct_big(const uint64_t *residues, int pcnt, BigInt *out) {
    BigInt x, mod;
    bi_init(&x);
    bi_init(&mod);
    bi_set_u64(&x, 0);
    bi_set_u64(&mod, 1);

    for (int i = 0; i < pcnt; ++i) {
        uint64_t p = PRIMES[i];
        uint64_t x_mod_p = bi_mod_u64(&x, p);
        uint64_t rhs = (residues[i] >= x_mod_p) ? (residues[i] - x_mod_p) : (residues[i] + (p - x_mod_p));

        uint64_t mod_mod_p = bi_mod_u64(&mod, p);
        uint64_t inv = modinv_u64(mod_mod_p, p);
        uint64_t t = mul_mod(rhs, inv, p);

        bi_add_mul_u64(&x, &mod, t);
        bi_mul_u64(&mod, p);
    }

    bi_free(out);
    *out = x;
    bi_free(&mod);
}

int main(int argc, char **argv) {
    int nmax = 60;
    if (argc >= 2) {
        nmax = atoi(argv[1]);
        if (nmax < 1 || nmax > 60) {
            fprintf(stderr, "usage: %s [nmax<=60]\n", argv[0]);
            return 1;
        }
    }

    int pcnt = choose_prime_count_for_n(nmax);
    if (pcnt < 0) {
        fprintf(stderr, "Not enough primes for bound at n=%d\n", nmax);
        return 1;
    }

#ifdef _OPENMP
    fprintf(stderr, "OpenMP threads: %d\n", omp_get_max_threads());
#else
    fprintf(stderr, "OpenMP disabled (build with -fopenmp for parallel execution)\n");
#endif
    fprintf(stderr, "Using %d CRT primes (~%d bits) for safe reconstruction up to n=%d\n", pcnt, pcnt * 61, nmax);

    BigInt value;
    bi_init(&value);

    for (int n = 1; n <= nmax; ++n) {
        uint64_t residues[MAX_PRIMES];
        for (int i = 0; i < pcnt; ++i) residues[i] = jackson2_mod(n, PRIMES[i]);

        crt_reconstruct_big(residues, pcnt, &value);
        printf("%d ", n);
        bi_print(&value);
        putchar('\n');
        fflush(stdout);
    }

    bi_free(&value);
    return 0;
}
