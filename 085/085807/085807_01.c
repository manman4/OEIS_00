#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>

#ifdef _OPENMP
#include <omp.h>
#endif

#define MAX_PRIMES 6

static const uint64_t PRIMES[MAX_PRIMES] = {
    2305843009213693951ULL, /* 2^61 - 1 */
    2305843009213693921ULL,
    2305843009213693907ULL,
    2305843009213693723ULL,
    2305843009213693693ULL,
    2305843009213693669ULL
};

static inline uint64_t add_mod(uint64_t a, uint64_t b, uint64_t p) {
    uint64_t s = a + b;
    return (s >= p) ? (s - p) : s;
}

static inline uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t p) {
    return (a >= b) ? (a - b) : (a + (p - b));
}

static inline uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t p) {
    return (uint64_t)((__uint128_t)a * (__uint128_t)b % p);
}

static uint64_t pow_mod(uint64_t a, uint64_t e, uint64_t p) {
    uint64_t r = 1;
    while (e) {
        if (e & 1ULL) r = mul_mod(r, a, p);
        a = mul_mod(a, a, p);
        e >>= 1ULL;
    }
    return r;
}

static inline uint64_t mod_inv_prime(uint64_t a, uint64_t p) {
    return pow_mod(a, p - 2, p);
}

static double now_sec(void) {
#ifdef _OPENMP
    return omp_get_wtime();
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9;
#endif
}

static inline uint32_t popcount_u64(uint64_t x) {
    return (uint32_t)__builtin_popcountll(x);
}

static void fill_rowsum_from_gray_absdiff(int n, uint64_t gray, uint64_t *rowsum) {
    for (int i = 0; i < n; ++i) rowsum[i] = 0;
    uint64_t mask = gray;
    while (mask) {
        int bit = __builtin_ctzll(mask);
        for (int i = 0; i < n; ++i) {
            uint64_t v = (uint64_t)((i > bit) ? (i - bit) : (bit - i));
            rowsum[i] += v;
        }
        mask &= (mask - 1ULL);
    }
}

/* Ryser:
   per(A) = (-1)^n * sum_{S subset [n], S!=empty} (-1)^|S| prod_i sum_{j in S} a_{ij}
   sign term is + when (n - |S|) is even, else -.
*/
static uint64_t permanent_absdiff_mod_parallel(int n, uint64_t p) {
    if (n == 0) return 1;
    if (n == 1) return 0;
    if (n > 62) {
        fprintf(stderr, "n=%d too large for uint64_t Gray subsets\n", n);
        exit(1);
    }

    uint64_t total = 1ULL << n;
    uint64_t masks = total - 1ULL;

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

        uint64_t *rowsum = (uint64_t *)calloc((size_t)n, sizeof(uint64_t));
        if (!rowsum) {
            fprintf(stderr, "allocation failed\n");
            exit(1);
        }

        uint64_t g = start ^ (start >> 1);
        fill_rowsum_from_gray_absdiff(n, g, rowsum);

        int sign = ((n - (int)popcount_u64(g)) & 1) ? -1 : 1;
        uint64_t prev_g = g;
        uint64_t acc = 0;

        for (uint64_t t = start; t < end; ++t) {
            if (t > start) {
                g = t ^ (t >> 1);
                uint64_t diff = g ^ prev_g;
                int bit = __builtin_ctzll(diff);
                int add = (g & (1ULL << bit)) ? 1 : 0;
                for (int i = 0; i < n; ++i) {
                    uint64_t v = (uint64_t)((i > bit) ? (i - bit) : (bit - i));
                    rowsum[i] = add ? (rowsum[i] + v) : (rowsum[i] - v);
                }
                sign = -sign;
                prev_g = g;
            }

            uint64_t prod = 1;
            for (int i = 0; i < n; ++i) {
                prod = mul_mod(prod, rowsum[i] % p, p);
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

static double log2_factorial(int n) {
    if (n < 2) return 0.0;
    double s = 0.0;
    for (int i = 2; i <= n; ++i) s += log2((double)i);
    return s;
}

/* Upper bound:
   per(|i-j|) <= n! * (n-1)^n (each term <= (n-1)^n, n! terms)
*/
static int choose_prime_count_for_n(int n) {
    if (n == 0) return 1;
    if (n == 1) return 1;
    double bits = log2_factorial(n) + (double)n * log2((double)(n - 1));
    int need = (int)(bits + 6.0);
    int got = 0;
    for (int i = 0; i < MAX_PRIMES; ++i) {
        got += 61;
        if (got >= need) return i + 1;
    }
    return -1;
}

static void crt_reconstruct_big(const uint64_t *res, int pcnt, mpz_t out) {
    mpz_t x, mod;
    mpz_init_set_ui(x, 0);
    mpz_init_set_ui(mod, 1);

    for (int i = 0; i < pcnt; ++i) {
        uint64_t p = PRIMES[i];
        uint64_t x_mod = mpz_fdiv_ui(x, p);
        uint64_t mod_mod = mpz_fdiv_ui(mod, p);
        uint64_t rhs = (res[i] >= x_mod) ? (res[i] - x_mod) : (res[i] + (p - x_mod));
        uint64_t inv = mod_inv_prime(mod_mod, p);
        uint64_t t = mul_mod(rhs, inv, p);
        mpz_addmul_ui(x, mod, t);
        mpz_mul_ui(mod, mod, p);
    }

    mpz_set(out, x);
    mpz_clear(x);
    mpz_clear(mod);
}

int main(int argc, char **argv) {
    int max_n = 26;
    if (argc >= 2) {
        max_n = atoi(argv[1]);
        if (max_n < 0 || max_n > 60) {
            fprintf(stderr, "usage: %s [max_n<=60]\n", argv[0]);
            return 1;
        }
    }

    FILE *out = fopen("b085807.txt", "w");
    if (!out) {
        fprintf(stderr, "failed to open output file: b085807.txt\n");
        return 1;
    }

    mpz_t ans;
    mpz_init(ans);

    for (int n = 0; n <= max_n; ++n) {
        int pcnt = choose_prime_count_for_n(n);
        if (pcnt < 0) {
            fprintf(stderr, "not enough primes for n=%d\n", n);
            mpz_clear(ans);
            fclose(out);
            return 1;
        }

        uint64_t residues[MAX_PRIMES];
        for (int i = 0; i < pcnt; ++i) {
            residues[i] = permanent_absdiff_mod_parallel(n, PRIMES[i]);
        }

        crt_reconstruct_big(residues, pcnt, ans);
        char *s = mpz_get_str(NULL, 10, ans);
        if (!s) {
            fprintf(stderr, "mpz_get_str failed\n");
            mpz_clear(ans);
            fclose(out);
            return 1;
        }
        fprintf(out, "%d %s\n", n, s);
        free(s);
        fflush(out);
    }

    mpz_clear(ans);
    fclose(out);
    return 0;
}
