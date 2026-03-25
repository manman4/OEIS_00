#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

// Term: exponents for a1..a15 packed into two uint64 (5 bits each)
// bits 0..63 -> lo, bits 64..127 -> hi
// exp(a_i) stored at bit position (i-1)*5
// max exponent must be <= 30 (5 bits). We reserve 31 as invalid.

typedef struct {
    uint64_t lo;
    uint64_t hi;
} Term;

static inline int term_eq(Term a, Term b) {
    return a.lo == b.lo && a.hi == b.hi;
}

// Hash: SplitMix64 over combined bits
static inline uint64_t splitmix64(uint64_t x) {
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
    x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
    x = x ^ (x >> 31);
    return x;
}

static inline uint64_t hash_term(Term t) {
    uint64_t h1 = splitmix64(t.lo);
    uint64_t h2 = splitmix64(t.hi);
    return h1 ^ (h2 + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2));
}

// Exponent accessors
static inline uint8_t get_exp(Term t, int var) {
    int p = (var - 1) * 5;
    if (p < 60) {
        return (uint8_t)((t.lo >> p) & 0x1FULL);
    } else if (p == 60) {
        uint8_t low4 = (uint8_t)((t.lo >> 60) & 0x0FULL);
        uint8_t high1 = (uint8_t)(t.hi & 0x1ULL);
        return (uint8_t)((high1 << 4) | low4);
    } else {
        int shift = p - 64;
        return (uint8_t)((t.hi >> shift) & 0x1FULL);
    }
}

static inline void set_exp(Term *t, int var, uint8_t val) {
    int p = (var - 1) * 5;
    if (p < 60) {
        uint64_t mask = 0x1FULL << p;
        t->lo = (t->lo & ~mask) | ((uint64_t)val << p);
    } else if (p == 60) {
        uint64_t mask_lo = 0xFULL << 60;
        t->lo = (t->lo & ~mask_lo) | (((uint64_t)val & 0x0FULL) << 60);
        t->hi = (t->hi & ~0x1ULL) | ((uint64_t)(val >> 4) & 0x1ULL);
    } else {
        int shift = p - 64;
        uint64_t mask = 0x1FULL << shift;
        t->hi = (t->hi & ~mask) | ((uint64_t)val << shift);
    }
}

static inline int add_pow(Term *t, int var, uint8_t inc) {
    uint8_t e = get_exp(*t, var);
    if (e + inc >= 31) return 0; // overflow
    set_exp(t, var, (uint8_t)(e + inc));
    return 1;
}

static inline int dec_var(Term *t, int var) {
    uint8_t e = get_exp(*t, var);
    if (e == 0) return 0;
    set_exp(t, var, (uint8_t)(e - 1));
    return 1;
}

// Poly: term list + hash table of indices + GMP coeffs

typedef struct {
    Term *terms;       // dense list of unique terms
    mpz_t *coeffs;     // coefficients (GMP)
    uint8_t *inited;   // coeffs[i] initialized?
    uint32_t *table;   // open addressing, stores index into terms
    size_t size;       // number of unique terms (including coeff=0)
    size_t cap_terms;
    size_t cap_table;  // power of two
} Poly;

static void die_oom(void) {
    fprintf(stderr, "Out of memory\n");
    exit(1);
}

static void poly_init(Poly *p, size_t cap_table, size_t cap_terms) {
    p->cap_table = cap_table;
    p->cap_terms = cap_terms;
    p->size = 0;

    p->terms = (Term *)malloc(sizeof(Term) * cap_terms);
    if (!p->terms) die_oom();

    p->coeffs = (mpz_t *)malloc(sizeof(mpz_t) * cap_terms);
    if (!p->coeffs) die_oom();

    p->inited = (uint8_t *)calloc(cap_terms, sizeof(uint8_t));
    if (!p->inited) die_oom();

    p->table = (uint32_t *)malloc(sizeof(uint32_t) * cap_table);
    if (!p->table) die_oom();

    for (size_t i = 0; i < cap_table; i++) p->table[i] = 0xFFFFFFFFu;
}

static void poly_free(Poly *p) {
    if (p->coeffs && p->inited) {
        for (size_t i = 0; i < p->cap_terms; i++) {
            if (p->inited[i]) mpz_clear(p->coeffs[i]);
        }
    }
    free(p->terms);
    free(p->coeffs);
    free(p->inited);
    free(p->table);
    p->terms = NULL;
    p->coeffs = NULL;
    p->inited = NULL;
    p->table = NULL;
    p->size = p->cap_terms = p->cap_table = 0;
}

static void poly_clear(Poly *p) {
    p->size = 0;
    for (size_t i = 0; i < p->cap_table; i++) p->table[i] = 0xFFFFFFFFu;
}

static void poly_reserve_terms(Poly *p, size_t new_cap) {
    Term *nt = (Term *)realloc(p->terms, sizeof(Term) * new_cap);
    if (!nt) die_oom();
    p->terms = nt;

    mpz_t *nc = (mpz_t *)realloc(p->coeffs, sizeof(mpz_t) * new_cap);
    if (!nc) die_oom();
    p->coeffs = nc;

    uint8_t *ni = (uint8_t *)realloc(p->inited, sizeof(uint8_t) * new_cap);
    if (!ni) die_oom();
    if (new_cap > p->cap_terms) {
        memset(ni + p->cap_terms, 0, new_cap - p->cap_terms);
    }
    p->inited = ni;

    p->cap_terms = new_cap;
}

static inline void poly_add_term_mpz(Poly *p, Term t, const mpz_t add) {
    if (mpz_sgn(add) == 0) return;
    uint64_t h = hash_term(t) & (p->cap_table - 1);
    while (1) {
        uint32_t idx = p->table[h];
        if (idx == 0xFFFFFFFFu) {
            if (p->size >= p->cap_terms) {
                size_t new_cap = p->cap_terms * 2;
                if (new_cap < 1024) new_cap = 1024;
                poly_reserve_terms(p, new_cap);
            }
            size_t pos = p->size;
            p->terms[pos] = t;
            if (!p->inited[pos]) {
                mpz_init(p->coeffs[pos]);
                p->inited[pos] = 1;
            }
            mpz_set(p->coeffs[pos], add);
            p->table[h] = (uint32_t)pos;
            p->size++;
            return;
        }
        if (term_eq(p->terms[idx], t)) {
            mpz_add(p->coeffs[idx], p->coeffs[idx], add);
            return;
        }
        h = (h + 1) & (p->cap_table - 1);
    }
}

// dst += src * var^pow (coeffs unchanged)
static void poly_mul_pow_into(Poly *dst, const Poly *src, int var, uint8_t pow) {
    for (size_t i = 0; i < src->size; i++) {
        if (mpz_sgn(src->coeffs[i]) == 0) continue;
        Term t = src->terms[i];
        if (!add_pow(&t, var, pow)) {
            fprintf(stderr, "Exponent overflow (var %d)\n", var);
            exit(1);
        }
        poly_add_term_mpz(dst, t, src->coeffs[i]);
    }
}

// dst += scale * diff(src, diff_var) * (mult_var if >0)
static void poly_add_scaled_diff_terms(Poly *dst, const Poly *src, int diff_var, int mult_var, unsigned long scale) {
    mpz_t tmp;
    mpz_init(tmp);
    for (size_t i = 0; i < src->size; i++) {
        if (mpz_sgn(src->coeffs[i]) == 0) continue;

        uint8_t e = get_exp(src->terms[i], diff_var);
        if (e == 0) continue;

        Term t = src->terms[i];
        set_exp(&t, diff_var, (uint8_t)(e - 1));

        if (mult_var > 0) {
            if (!add_pow(&t, mult_var, 1)) {
                fprintf(stderr, "Exponent overflow (var %d)\n", mult_var);
                exit(1);
            }
        }

        mpz_mul_ui(tmp, src->coeffs[i], (unsigned long)e);
        if (scale != 1) mpz_mul_ui(tmp, tmp, scale);
        poly_add_term_mpz(dst, t, tmp);
    }
    mpz_clear(tmp);
}

// dst = (-1/k) * src / var  (only terms with exp>0)
static void poly_div_var_scale_neg_ui(Poly *dst, const Poly *src, int var, unsigned long k) {
    mpz_t tmp;
    mpz_init(tmp);
    for (size_t i = 0; i < src->size; i++) {
        if (mpz_sgn(src->coeffs[i]) == 0) continue;
        Term t = src->terms[i];
        if (!dec_var(&t, var)) continue;

        mpz_divexact_ui(tmp, src->coeffs[i], k);
        mpz_neg(tmp, tmp);
        poly_add_term_mpz(dst, t, tmp);
    }
    mpz_clear(tmp);
}

static Term term_zero(void) {
    Term t = {0, 0};
    return t;
}

static Term term_with_exp(int var, uint8_t exp) {
    Term t = term_zero();
    set_exp(&t, var, exp);
    return t;
}

static size_t count_nonzero(const Poly *p) {
    size_t cnt = 0;
    for (size_t i = 0; i < p->size; i++) {
        if (mpz_sgn(p->coeffs[i]) != 0) cnt++;
    }
    return cnt;
}

int main(int argc, char **argv) {
    int max_n = 15;
    size_t table_pow = 28; // 2^28 slots by default

    if (argc >= 2) max_n = atoi(argv[1]);
    if (argc >= 3) table_pow = (size_t)atoi(argv[2]);

    if (max_n < 1 || max_n > 17) {
        fprintf(stderr, "max_n must be between 1 and 17\n");
        return 1;
    }
    if (table_pow < 20 || table_pow > 30) {
        fprintf(stderr, "table_pow should be in [20,30] (2^20..2^30 slots)\n");
        return 1;
    }

    size_t cap_table = (size_t)1 << table_pow;
    size_t cap_terms = cap_table / 2;

    Poly s, t0, u0;
    poly_init(&s, cap_table, cap_terms);
    poly_init(&t0, cap_table, cap_terms);
    poly_init(&u0, cap_table, cap_terms);

    // n=1: discriminant of linear is 1
    if (max_n >= 1) {
        printf("n=1 terms: 1\n");
    }

    // n=2: s = a1^2 - 4*a2
    poly_clear(&s);
    {
        mpz_t c;
        mpz_init(c);
        mpz_set_ui(c, 1);
        poly_add_term_mpz(&s, term_with_exp(1, 2), c); // a1^2
        mpz_set_si(c, -4);
        poly_add_term_mpz(&s, term_with_exp(2, 1), c); // -4*a2
        mpz_clear(c);
    }

    if (max_n >= 2) {
        printf("n=2 terms: %zu\n", count_nonzero(&s));
    }

    for (int n = 3; n <= max_n; n++) {
        // D0 = a_{n-1}^2 * s
        poly_clear(&t0);
        poly_mul_pow_into(&t0, &s, n - 1, 2);

        // s = D0
        poly_clear(&s);
        for (size_t i = 0; i < t0.size; i++) {
            if (mpz_sgn(t0.coeffs[i]) == 0) continue;
            poly_add_term_mpz(&s, t0.terms[i], t0.coeffs[i]);
        }

        for (int k = 1; k < n; k++) {
            // u0 = n*diff(t0,a1) + sum_{j=2..n-1} (n+1-j) a_{j-1} diff(t0,a_j)
            poly_clear(&u0);
            poly_add_scaled_diff_terms(&u0, &t0, 1, 0, (unsigned long)n);

            for (int j = 2; j <= n - 1; j++) {
                unsigned long scale = (unsigned long)(n + 1 - j);
                poly_add_scaled_diff_terms(&u0, &t0, j, j - 1, scale);
            }

            // t0 = -u0 / (k * a_{n-1})
            poly_clear(&t0);
            poly_div_var_scale_neg_ui(&t0, &u0, n - 1, (unsigned long)k);

            // s += t0 * a_n^k
            poly_mul_pow_into(&s, &t0, n, (uint8_t)k);
        }

        printf("n=%d terms: %zu\n", n, count_nonzero(&s));
        fflush(stdout);
    }

    poly_free(&s);
    poly_free(&t0);
    poly_free(&u0);
    return 0;
}
