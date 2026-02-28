#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef _OPENMP
#include <omp.h>
#endif

enum { MAXK = 64, NMOD = 3 };
static const uint32_t BI_BASE = 1000000000U;

typedef struct {
    uint32_t *d;
    int len;
    int cap;
} BigInt;

static void bi_init(BigInt *a) { a->d = NULL; a->len = 0; a->cap = 0; }
static void bi_clear(BigInt *a) { free(a->d); a->d = NULL; a->len = 0; a->cap = 0; }

static void bi_reserve(BigInt *a, int need) {
    if (need <= a->cap) return;
    int c = a->cap ? a->cap : 4;
    while (c < need) c <<= 1;
    uint32_t *nd = (uint32_t *)realloc(a->d, (size_t)c * sizeof(uint32_t));
    if (!nd) { fprintf(stderr, "OOM BigInt\n"); exit(1); }
    a->d = nd;
    a->cap = c;
}

static void bi_set_u64(BigInt *a, uint64_t x) {
    if (x == 0) { a->len = 0; return; }
    bi_reserve(a, 3);
    a->len = 0;
    while (x) {
        a->d[a->len++] = (uint32_t)(x % BI_BASE);
        x /= BI_BASE;
    }
}

static void bi_copy(BigInt *dst, const BigInt *src) {
    bi_reserve(dst, src->len);
    if (src->len) memcpy(dst->d, src->d, (size_t)src->len * sizeof(uint32_t));
    dst->len = src->len;
}

static void bi_add_inplace(BigInt *a, const BigInt *b) {
    int n = (a->len > b->len) ? a->len : b->len;
    bi_reserve(a, n + 2);
    uint64_t carry = 0;
    for (int i = 0; i < n; i++) {
        uint64_t av = (i < a->len) ? a->d[i] : 0;
        uint64_t bv = (i < b->len) ? b->d[i] : 0;
        uint64_t s = av + bv + carry;
        a->d[i] = (uint32_t)(s % BI_BASE);
        carry = s / BI_BASE;
    }
    a->len = n;
    while (carry) {
        a->d[a->len++] = (uint32_t)(carry % BI_BASE);
        carry /= BI_BASE;
    }
}

static void bi_mul_u64(BigInt *a, uint64_t m) {
    if (a->len == 0 || m == 1) return;
    if (m == 0) { a->len = 0; return; }
    bi_reserve(a, a->len + 4);
    __uint128_t carry = 0;
    for (int i = 0; i < a->len; i++) {
        __uint128_t v = (__uint128_t)a->d[i] * m + carry;
        a->d[i] = (uint32_t)(v % BI_BASE);
        carry = v / BI_BASE;
    }
    while (carry) {
        a->d[a->len++] = (uint32_t)(carry % BI_BASE);
        carry /= BI_BASE;
    }
}

static uint64_t bi_mod_u64(const BigInt *a, uint64_t mod) {
    __uint128_t r = 0;
    for (int i = a->len - 1; i >= 0; i--) {
        r = (r * BI_BASE + a->d[i]) % mod;
    }
    return (uint64_t)r;
}

static char *bi_to_string(const BigInt *a) {
    if (a->len == 0) {
        char *z = (char *)malloc(2);
        z[0] = '0'; z[1] = '\0';
        return z;
    }
    int est = a->len * 10 + 1;
    char *s = (char *)malloc((size_t)est);
    if (!s) { fprintf(stderr, "OOM str\n"); exit(1); }
    int p = snprintf(s, (size_t)est, "%u", a->d[a->len - 1]);
    for (int i = a->len - 2; i >= 0; i--) {
        p += snprintf(s + p, (size_t)(est - p), "%09u", a->d[i]);
    }
    return s;
}

static uint64_t add_mod(uint64_t a, uint64_t b, uint64_t mod) {
    a += b;
    if (a >= mod) a -= mod;
    return a;
}

static uint64_t mul_mod_u64(uint64_t a, uint64_t b, uint64_t mod) {
    return (uint64_t)((__uint128_t)a * b % mod);
}

static uint64_t pow_mod_u64(uint64_t a, uint64_t e, uint64_t mod) {
    uint64_t r = 1 % mod;
    while (e) {
        if (e & 1) r = mul_mod_u64(r, a, mod);
        a = mul_mod_u64(a, a, mod);
        e >>= 1;
    }
    return r;
}

static int is_prime_u64(uint64_t n) {
    if (n < 2) return 0;
    static const uint64_t small[] = {2,3,5,7,11,13,17,19,23,29,31,37};
    for (size_t i = 0; i < sizeof(small)/sizeof(small[0]); i++) {
        uint64_t p = small[i];
        if (n == p) return 1;
        if (n % p == 0) return 0;
    }
    uint64_t d = n - 1;
    int s = 0;
    while ((d & 1) == 0) { d >>= 1; s++; }
    static const uint64_t bases[] = {2ULL, 325ULL, 9375ULL, 28178ULL, 450775ULL, 9780504ULL, 1795265022ULL};
    for (size_t i = 0; i < sizeof(bases)/sizeof(bases[0]); i++) {
        uint64_t a = bases[i] % n;
        if (a == 0) continue;
        uint64_t x = pow_mod_u64(a, d, n);
        if (x == 1 || x == n - 1) continue;
        int witness = 1;
        for (int r = 1; r < s; r++) {
            x = mul_mod_u64(x, x, n);
            if (x == n - 1) { witness = 0; break; }
        }
        if (witness) return 0;
    }
    return 1;
}

static void pick_three_primes(uint64_t mods[NMOD]) {
    /* Verification build: intentionally use a disjoint prime range from the main source.
     * Main source scans in (2^60, 2^61); this one scans in (2^58, 2^59).
     */
    uint64_t start = (1ULL << 59) - 1;
    int f = 0;
    for (uint64_t x = start | 1ULL; x > (1ULL << 58); x -= 2) {
        if (is_prime_u64(x)) {
            mods[f++] = x;
            if (f == NMOD) return;
        }
    }
    fprintf(stderr, "failed to pick primes\n");
    exit(1);
}

static uint64_t inv_mod_u64(uint64_t a, uint64_t mod) {
    int64_t t = 0, nt = 1;
    int64_t r = (int64_t)mod, nr = (int64_t)(a % mod);
    while (nr != 0) {
        int64_t q = r / nr;
        int64_t tmp = t - q * nt; t = nt; nt = tmp;
        tmp = r - q * nr; r = nr; nr = tmp;
    }
    if (r != 1) { fprintf(stderr, "no inverse\n"); exit(1); }
    if (t < 0) t += (int64_t)mod;
    return (uint64_t)t;
}

static uint64_t mix64(uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

static size_t next_pow2(size_t x) {
    size_t p = 1;
    while (p < x) p <<= 1;
    return p;
}

typedef struct {
    uint64_t key;
    uint64_t v[NMOD];
    uint8_t used;
} HEntry;

typedef struct {
    HEntry *tab;
    size_t *touched;
    size_t touched_len;
    size_t cap;
    size_t size;
} Hash;

static void hash_init(Hash *h, size_t cap) {
    h->cap = next_pow2(cap < 16 ? 16 : cap);
    h->size = 0;
    h->touched_len = 0;
    h->tab = (HEntry *)calloc(h->cap, sizeof(HEntry));
    h->touched = (size_t *)malloc(h->cap * sizeof(size_t));
    if (!h->tab || !h->touched) { fprintf(stderr, "OOM hash\n"); exit(1); }
}

static void hash_zero(Hash *h) {
    for (size_t i = 0; i < h->touched_len; i++) {
        h->tab[h->touched[i]].used = 0;
    }
    h->touched_len = 0;
    h->size = 0;
}

static void hash_clear(Hash *h) {
    free(h->tab);
    free(h->touched);
    h->tab = NULL;
    h->touched = NULL;
    h->cap = 0;
    h->size = 0;
    h->touched_len = 0;
}

static void hash_rehash(Hash *h, size_t ncap) {
    Hash nh;
    hash_init(&nh, ncap);
    for (size_t i = 0; i < h->cap; i++) {
        if (!h->tab[i].used) continue;
        size_t mask = nh.cap - 1;
        size_t pos = (size_t)mix64(h->tab[i].key) & mask;
        while (nh.tab[pos].used) pos = (pos + 1) & mask;
        nh.tab[pos] = h->tab[i];
        nh.touched[nh.touched_len++] = pos;
        nh.size++;
    }
    free(h->tab);
    free(h->touched);
    *h = nh;
}

static void hash_add(Hash *h, uint64_t key, const uint64_t addv[NMOD], const uint64_t mods[NMOD]) {
    if ((h->size + 1) * 10 >= h->cap * 7) hash_rehash(h, h->cap * 2);
    size_t mask = h->cap - 1;
    size_t pos = (size_t)mix64(key) & mask;
    while (h->tab[pos].used) {
        if (h->tab[pos].key == key) {
            for (int i = 0; i < NMOD; i++) h->tab[pos].v[i] = add_mod(h->tab[pos].v[i], addv[i], mods[i]);
            return;
        }
        pos = (pos + 1) & mask;
    }
    h->tab[pos].used = 1;
    h->tab[pos].key = key;
    for (int i = 0; i < NMOD; i++) h->tab[pos].v[i] = addv[i] % mods[i];
    h->touched[h->touched_len++] = pos;
    h->size++;
}

static uint64_t *hash_get(Hash *h, uint64_t key) {
    if (!h->tab) return NULL;
    size_t mask = h->cap - 1;
    size_t pos = (size_t)mix64(key) & mask;
    while (h->tab[pos].used) {
        if (h->tab[pos].key == key) return h->tab[pos].v;
        pos = (pos + 1) & mask;
    }
    return NULL;
}

#ifdef _OPENMP
static void hash_merge_into(Hash *dst, const Hash *src, const uint64_t mods[NMOD]) {
    for (size_t i = 0; i < src->cap; i++) {
        if (!src->tab[i].used) continue;
        hash_add(dst, src->tab[i].key, src->tab[i].v, mods);
    }
}
#endif

typedef struct {
    int k;
    int counts[MAXK];
    uint64_t strides[MAXK];
    uint16_t type_mask[MAXK];
    uint8_t self_compat[MAXK];
    uint64_t compat[MAXK];
    uint64_t all_mask;
    uint64_t total_code;
    int total_len;
} Problem;

static int cmp_pair_desc(const void *a, const void *b) {
    const int *x = (const int *)a;
    const int *y = (const int *)b;
    if (x[1] != y[1]) return y[1] - x[1];
    return x[0] - y[0];
}

static int generate_primes(int n, int *pr) {
    int c = 0;
    for (int x = 2; x <= n; x++) {
        int ok = 1;
        for (int d = 2; d * d <= x; d++) {
            if (x % d == 0) { ok = 0; break; }
        }
        if (ok) pr[c++] = x;
    }
    return c;
}

static Problem build_problem_with_primes(int n, const int *pr, int pc) {
    Problem P;
    memset(&P, 0, sizeof(P));
    if (n <= 0) {
        return P;
    }
    int small_idx[64];
    int sc = 0;
    int bigbit = -1;

    for (int i = 0; i < pc; i++) {
        if (pr[i] == 2) {
            small_idx[i] = sc++;
        } else if (pr[i] <= n / 2) {
            small_idx[i] = sc++;
        } else {
            small_idx[i] = -1;
            if (bigbit < 0) bigbit = sc++;
        }
    }
    if (sc > 15) {
        fprintf(stderr, "too many compressed prime bits: %d\n", sc);
        exit(1);
    }

    uint16_t masks[MAXK];
    int cnt[MAXK];
    int k = 0;

    for (int v = 1; v <= n; v++) {
        int x = v;
        uint16_t m = 0;
        for (int i = 0; i < pc; i++) {
            int p = pr[i];
            if (x % p == 0) {
                int b = small_idx[i];
                if (b < 0) b = bigbit;
                m |= (uint16_t)(1u << b);
                while (x % p == 0) x /= p;
            }
            if (x == 1) break;
        }

        int idx = -1;
        for (int i = 0; i < k; i++) {
            if (masks[i] == m) { idx = i; break; }
        }
        if (idx < 0) {
            if (k >= MAXK) { fprintf(stderr, "too many types\n"); exit(1); }
            masks[k] = m;
            cnt[k] = 1;
            k++;
        } else {
            cnt[idx]++;
        }
    }

    int pair[MAXK][2];
    for (int i = 0; i < k; i++) {
        pair[i][0] = masks[i];
        pair[i][1] = cnt[i];
    }
    qsort(pair, (size_t)k, sizeof(pair[0]), cmp_pair_desc);

    P.k = k;
    P.total_len = n;

    for (int i = 0; i < k; i++) {
        P.type_mask[i] = (uint16_t)pair[i][0];
        P.counts[i] = pair[i][1];
        P.self_compat[i] = 0;
        if (P.type_mask[i] == 0) {
            P.self_compat[i] = 1; /* number 1 */
        } else if (bigbit >= 0 && P.type_mask[i] == (uint16_t)(1u << bigbit)) {
            P.self_compat[i] = 1;
        }
    }

    P.strides[0] = 1;
    for (int i = 1; i < k; i++) {
        __uint128_t s = (__uint128_t)P.strides[i - 1] * (uint64_t)(P.counts[i - 1] + 1);
        if (s > UINT64_MAX) {
            fprintf(stderr, "state code overflow\n");
            exit(1);
        }
        P.strides[i] = (uint64_t)s;
    }

    P.total_code = 0;
    for (int i = 0; i < k; i++) P.total_code += (uint64_t)P.counts[i] * P.strides[i];

    P.all_mask = (k == 64) ? ~0ULL : ((1ULL << k) - 1ULL);
    for (int i = 0; i < k; i++) {
        uint64_t cm = 0;
        for (int j = 0; j < k; j++) {
            if ((P.type_mask[i] & P.type_mask[j]) == 0) {
                cm |= (1ULL << j);
            } else if (i == j && P.self_compat[i]) {
                cm |= (1ULL << j);
            }
        }
        P.compat[i] = cm;
    }

    return P;
}
static inline uint64_t pack_key(uint64_t used_code, int last_plus, int k) {
    return used_code * (uint64_t)(k + 1) + (uint64_t)last_plus;
}

static inline void unpack_key(uint64_t key, int k, uint64_t *used_code, int *last_plus) {
    *last_plus = (int)(key % (uint64_t)(k + 1));
    *used_code = key / (uint64_t)(k + 1);
}

static void expand_state_to_hash(const Problem *P,
                                 uint64_t used_code, int last_plus,
                                 const uint64_t val[NMOD], Hash *dst, const uint64_t mods[NMOD]) {
    int used_cnt[MAXK];
    for (int t = 0; t < P->k; t++) {
        used_cnt[t] = (int)((used_code / P->strides[t]) % (uint64_t)(P->counts[t] + 1));
    }

    uint64_t cand = (last_plus == 0) ? P->all_mask : P->compat[last_plus - 1];
    while (cand) {
        int t = __builtin_ctzll(cand);
        cand &= cand - 1;

        if (used_cnt[t] >= P->counts[t]) continue;

        uint64_t ncode = used_code + P->strides[t];
        uint64_t nkey = pack_key(ncode, t + 1, P->k);
        hash_add(dst, nkey, val, mods);
    }
}

static void build_layer(const Problem *P, int depth, const uint64_t mods[NMOD], Hash *out) {
    Hash cur, nxt;
    hash_init(&cur, 1 << 15);
    hash_init(&nxt, 1 << 15);
#ifdef _OPENMP
    int nth = omp_get_max_threads();
    Hash *locals = (Hash *)malloc((size_t)nth * sizeof(Hash));
    if (!locals) { fprintf(stderr, "OOM locals\n"); exit(1); }
    for (int t = 0; t < nth; t++) hash_init(&locals[t], 1 << 13);
#endif

    uint64_t one[NMOD] = {1, 1, 1};
    uint64_t root = pack_key(0, 0, P->k);
    hash_add(&cur, root, one, mods);

    for (int step = 0; step < depth; step++) {
        hash_zero(&nxt);
#ifdef _OPENMP
        int use_parallel = (nth > 1 && cur.size >= 4096);
        if (use_parallel) {
            for (int t = 0; t < nth; t++) hash_zero(&locals[t]);
#pragma omp parallel
            {
                int tid = omp_get_thread_num();
                Hash *lh = &locals[tid];
#pragma omp for schedule(dynamic, 256)
                for (size_t i = 0; i < cur.cap; i++) {
                    if (!cur.tab[i].used) continue;
                    uint64_t used_code;
                    int last_plus;
                    unpack_key(cur.tab[i].key, P->k, &used_code, &last_plus);
                    expand_state_to_hash(P, used_code, last_plus, cur.tab[i].v, lh, mods);
                }
            }
            for (int t = 0; t < nth; t++) hash_merge_into(&nxt, &locals[t], mods);
        } else {
            for (size_t i = 0; i < cur.cap; i++) {
                if (!cur.tab[i].used) continue;
                uint64_t used_code;
                int last_plus;
                unpack_key(cur.tab[i].key, P->k, &used_code, &last_plus);
                expand_state_to_hash(P, used_code, last_plus, cur.tab[i].v, &nxt, mods);
            }
        }
#else
        for (size_t i = 0; i < cur.cap; i++) {
            if (!cur.tab[i].used) continue;
            uint64_t used_code;
            int last_plus;
            unpack_key(cur.tab[i].key, P->k, &used_code, &last_plus);
            expand_state_to_hash(P, used_code, last_plus, cur.tab[i].v, &nxt, mods);
        }
#endif
        Hash tmp = cur;
        cur = nxt;
        nxt = tmp;
    }

    *out = cur;
    hash_clear(&nxt);
#ifdef _OPENMP
    for (int t = 0; t < nth; t++) hash_clear(&locals[t]);
    free(locals);
#endif
}

static void multiply_label_factor_mod(uint64_t r[NMOD], const Problem *P, const uint64_t mods[NMOD]) {
    for (int i = 0; i < P->k; i++) {
        for (int x = 2; x <= P->counts[i]; x++) {
            for (int j = 0; j < NMOD; j++) r[j] = mul_mod_u64(r[j], (uint64_t)x, mods[j]);
        }
    }
}

static void crt_reconstruct(const uint64_t r[NMOD], const uint64_t mods[NMOD], BigInt *out) {
    BigInt X, M;
    bi_init(&X); bi_init(&M);
    bi_set_u64(&X, r[0]);
    bi_set_u64(&M, mods[0]);

    for (int i = 1; i < NMOD; i++) {
        uint64_t mi = mods[i];
        uint64_t xmod = bi_mod_u64(&X, mi);
        uint64_t mmod = bi_mod_u64(&M, mi);
        uint64_t t = (r[i] >= xmod) ? (r[i] - xmod) : (r[i] + mi - xmod);
        uint64_t inv = inv_mod_u64(mmod, mi);
        uint64_t c = mul_mod_u64(t, inv, mi);

        BigInt tmp;
        bi_init(&tmp);
        bi_copy(&tmp, &M);
        bi_mul_u64(&tmp, c);
        bi_add_inplace(&X, &tmp);
        bi_clear(&tmp);

        bi_mul_u64(&M, mi);
    }

    bi_copy(out, &X);
    bi_clear(&X);
    bi_clear(&M);
}

static void count_mitm(const Problem *P, const uint64_t mods[NMOD], uint64_t out[NMOD]) {
    for (int i = 0; i < NMOD; i++) out[i] = 0;
    if (P->total_len == 0) {
        for (int i = 0; i < NMOD; i++) out[i] = 1;
        return;
    }

    int left_len = P->total_len / 2;
    int right_len = P->total_len - left_len;

    if (left_len == 0 || right_len == 0) {
        Hash all;
        build_layer(P, P->total_len, mods, &all);
        for (size_t i = 0; i < all.cap; i++) {
            if (!all.tab[i].used) continue;
            uint64_t uc;
            int lastp;
            unpack_key(all.tab[i].key, P->k, &uc, &lastp);
            (void)uc; (void)lastp;
            for (int j = 0; j < NMOD; j++) out[j] = add_mod(out[j], all.tab[i].v[j], mods[j]);
        }
        hash_clear(&all);
        return;
    }

    Hash left, right;
    build_layer(P, left_len, mods, &left);
    build_layer(P, right_len, mods, &right);
    int compat_len[MAXK];
    int compat_idx[MAXK][MAXK];
    for (int i = 0; i < P->k; i++) {
        compat_len[i] = 0;
        uint64_t m = P->compat[i];
        while (m) {
            int t = __builtin_ctzll(m);
            m &= m - 1;
            compat_idx[i][compat_len[i]++] = t + 1;
        }
    }

    for (size_t i = 0; i < left.cap; i++) {
        if (!left.tab[i].used) continue;
        uint64_t uc;
        int la;
        unpack_key(left.tab[i].key, P->k, &uc, &la);
        if (la == 0 && P->total_len > 0) continue;

        uint64_t rc = P->total_code - uc;

        for (int ci = 0; ci < compat_len[la - 1]; ci++) {
            int rb = compat_idx[la - 1][ci];
            uint64_t rkey = pack_key(rc, rb, P->k);
            uint64_t *rv = hash_get(&right, rkey);
            if (!rv) continue;
            for (int j = 0; j < NMOD; j++) {
                uint64_t add = mul_mod_u64(left.tab[i].v[j], rv[j], mods[j]);
                out[j] = add_mod(out[j], add, mods[j]);
            }
        }
    }

    hash_clear(&left);
    hash_clear(&right);
}

static void compute_a076220_problem(const Problem *P, const uint64_t mods[NMOD], BigInt *ans) {
    uint64_t r[NMOD];
    count_mitm(P, mods, r);
    multiply_label_factor_mod(r, P, mods);
    crt_reconstruct(r, mods, ans);
}

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;

    const int nmax = 33;

    uint64_t mods[NMOD];
    pick_three_primes(mods);

    int primes[64];
    int pc = generate_primes(nmax, primes);

    int start_n = 0;
    FILE *fr = fopen("b076220_altcrt.txt", "r");
    if (fr) {
        int expect = 0;
        int n;
        char buf[4096];
        while (fscanf(fr, "%d %4095s", &n, buf) == 2) {
            if (n != expect) break;
            expect++;
            if (expect > nmax + 1) break;
        }
        fclose(fr);
        start_n = expect;
    }

    if (start_n > nmax) return 0;

    FILE *fp = fopen("b076220_altcrt.txt", start_n == 0 ? "w" : "a");
    if (!fp) {
        fprintf(stderr, "failed to open output: b076220_altcrt.txt\n");
        return 1;
    }

    BigInt ans;
    bi_init(&ans);
    for (int n = start_n; n <= nmax; n++) {
        Problem P = build_problem_with_primes(n, primes, pc);
        ans.len = 0;
        compute_a076220_problem(&P, mods, &ans);
        char *s = bi_to_string(&ans);
        fprintf(fp, "%d %s\n", n, s);
        fflush(fp);
        free(s);
    }

    bi_clear(&ans);
    fclose(fp);
    return 0;
}
