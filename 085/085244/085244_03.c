/*
 * A085244 -- exact permanent of the n by n GCD matrix, n <= 35.
 *
 * This is deliberately a different algorithm from 085244_02.c.  Put
 * E(i,d)=[d|i] and Phi=diag(phi(1),...,phi(n)).  Smith's factorization is
 *
 *                 G=(gcd(i,j)) = E Phi E^T.
 *
 * Since E^{-1}(i,d)=mu(i/d)[d|i], the precision matrix H=G^{-1} is
 *
 * H(i,j)=sum_{lcm(i,j)|k<=n} mu(k/i) mu(k/j) / phi(k).              (1)
 *
 * H is sparse: with the descending elimination order its filled width is
 * only 7 at n=35.  If z is a centered formal complex Gaussian with precision
 * H, Wick's formula gives
 *
 *                 per(G) = E product_i z_i conjugate(z_i).         (2)
 *
 * We evaluate (2) by exact variable elimination over finite fields.  A tree
 * message is a sparse polynomial in z and conjugate(z) on the separator.
 * Eliminating v with pivot h and conditional linear form beta*z uses
 *
 * E[(y+B)^a (ybar+Bbar)^b]
 *   = sum_t binom(a,t)binom(b,t)t! h^(-t) B^(a-t) Bbar^(b-t).       (3)
 *
 * Formula (3), sparse LDL elimination, and message multiplication are the
 * whole modular algorithm; no value produced by 085244_02.c is consumed.
 * The 61-bit CRT residues are propagated two at a time, which lowers the
 * peak table size without weakening reconstruction.  The rigorous permanent
 * bound product(row sums) makes the combined CRT result unique.
 *
 * Safety and verification:
 *   - hard n<=35 and exponent/width checks;
 *   - all large allocations tracked against A085244_03_MEMORY_MIB (4 GiB);
 *   - exact rational support construction for H, not floating point;
 *   - nonzero modular pivots, tree-decomposition and exponent checks;
 *   - reference first-prime residues through n=38;
 *   - independent subset permanent DP through n=22;
 *   - exact built-in terms through n=29 and CRT residue replay;
 *   - the final b-file is installed only after complete success.
 *
 * Build:
 *   clang -O3 -march=native -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     085244_03.c -lgmp -o 085244_03
 *
 * Usage:
 *   ./085244_03 --term N
 *   ./085244_03 --check [N]
 *   ./085244_03 MAX_N [START_N]
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <gmp.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#if ULONG_MAX < UINT64_MAX
#error "085244_03 requires 64-bit unsigned long"
#endif

#define MAX_N 35
#define MAX_WIDTH 8U
#define COUNT_BITS 6U
#define COUNT_MASK UINT64_C(63)
#define MAX_MODULI 2U
#define MAX_CRT_MODULI 4U
#define KNOWN_RESIDUE_N 38
#define KNOWN_EXACT_N 29
#define DIRECT_CHECK_N 22
#define DEFAULT_MEMORY_MIB UINT64_C(4096)
#define MIN_MEMORY_MIB UINT64_C(64)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define EMPTY_KEY UINT64_MAX

_Static_assert(MAX_N <= COUNT_MASK, "packed exponent is too narrow");
_Static_assert(COUNT_BITS * MAX_WIDTH <= 64U,
               "one side of a separator key must fit uint64_t");

static const uint64_t known_residue[KNOWN_RESIDUE_N + 1] = {
    UINT64_C(0), UINT64_C(1), UINT64_C(3), UINT64_C(14),
    UINT64_C(112), UINT64_C(872), UINT64_C(14372), UINT64_C(154480),
    UINT64_C(3098480), UINT64_C(59710816), UINT64_C(1688186176),
    UINT64_C(27925409152), UINT64_C(1327833590272),
    UINT64_C(25675495200768), UINT64_C(1017195720916224),
    UINT64_C(47444016840290304), UINT64_C(1114109633706177503),
    UINT64_C(1140200724325347792), UINT64_C(605814141477190406),
    UINT64_C(779523081708593516), UINT64_C(872003706515250454),
    UINT64_C(737146661465650219), UINT64_C(1009727701421576126),
    UINT64_C(854537493059079376), UINT64_C(679821370357018645),
    UINT64_C(260526890240354531), UINT64_C(486942201144866324),
    UINT64_C(1067481194159174585), UINT64_C(591049534218902125),
    UINT64_C(990531435660320261), UINT64_C(907081723088034761),
    UINT64_C(279022517005155903), UINT64_C(630390056218630735),
    UINT64_C(1149930728939511561), UINT64_C(986412578526681865),
    UINT64_C(518400539548616480), UINT64_C(1147905397317651708),
    UINT64_C(334581868099078631), UINT64_C(796567356817866531)
};

static const char *const known_exact[KNOWN_EXACT_N + 1] = {
    NULL,
    "1", "3", "14", "112", "872", "14372", "154480",
    "3098480", "59710816", "1688186176", "27925409152",
    "1327833590272", "25675495200768", "1017195720916224",
    "47444016840290304", "2267031138313024512",
    "56480432945454004224", "4051971981329937580032",
    "112180041921327922569216", "9250427364885586859163648",
    "604870570906353696547307520", "37003949025135478872990547968",
    "1226734830877410684373175894016",
    "150982602863547637867136387383296",
    "8158813240249417966741428872675328",
    "586623207316450040729896176413835264",
    "42950237145098618016020059492435623936",
    "4982316241621537079365176128607596052480",
    "205652759887575032846825612479521764671488"
};

typedef struct {
    uint64_t limit, used, peak, required;
} MemoryTracker;

typedef struct {
    size_t count;
    uint64_t modulus[MAX_MODULI];
} ModulusPlan;

typedef struct {
    size_t count;
    uint64_t modulus[MAX_CRT_MODULI];
    mpz_t product;
} CrtPlan;

typedef struct {
    uint64_t row_key;
    uint64_t column_key;
    uint64_t value[MAX_MODULI];
} StateEntry;

_Static_assert(sizeof(StateEntry) == 32U,
               "StateEntry size is part of memory accounting");

typedef struct {
    void *block;
    StateEntry *entry;
    uint32_t *active;
    size_t capacity, count, modulus_count;
} StateMap;

typedef struct {
    uint64_t separator_mask;
    uint8_t separator[MAX_WIDTH];
    uint8_t separator_size;
    int parent, first_child, next_sibling;
    StateMap message;
} Node;

typedef struct {
    unsigned width;
    size_t peak_states;
    uint64_t products, transforms;
    double seconds;
} Statistics;

typedef enum {
    STATUS_OK = 0, STATUS_MEMORY, STATUS_WIDTH,
    STATUS_OVERFLOW, STATUS_PIVOT, STATUS_INTERNAL
} Status;

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        die("clock_gettime failed");
    }
    return (double)now.tv_sec + (double)now.tv_nsec / 1.0e9;
}

static int parse_int(const char *text, int low, int high,
                     const char *label)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno || end == text || *end != '\0' ||
        value < low || value > high) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, low, high, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static uint64_t memory_limit_bytes(void)
{
    const char *text = getenv("A085244_03_MEMORY_MIB");
    uint64_t mib = DEFAULT_MEMORY_MIB;
    if (text && *text) {
        char *end = NULL;
        errno = 0;
        unsigned long long value = strtoull(text, &end, 10);
        if (errno || end == text || *end != '\0' ||
            value < MIN_MEMORY_MIB || value > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A085244_03_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)value;
    }
    return mib << 20;
}

static bool tracked_reserve(MemoryTracker *memory, uint64_t bytes)
{
    if (bytes > memory->limit || memory->used > memory->limit - bytes) {
        memory->required = bytes;
        return false;
    }
    memory->used += bytes;
    if (memory->used > memory->peak) memory->peak = memory->used;
    return true;
}

static void tracked_release(MemoryTracker *memory, uint64_t bytes)
{
    if (bytes > memory->used) die("memory accounting underflow");
    memory->used -= bytes;
}

static int gcd_int(int a, int b)
{
    while (b) { int r = a % b; a = b; b = r; }
    return a;
}

static unsigned totient(unsigned n)
{
    unsigned result = n;
    for (unsigned p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            while (n % p == 0) n /= p;
            result -= result / p;
        }
    }
    if (n > 1) result -= result / n;
    return result;
}

static int mobius(unsigned n)
{
    unsigned factors = 0;
    for (unsigned p = 2; p * p <= n; ++p) {
        if (n % p == 0) {
            n /= p;
            ++factors;
            if (n % p == 0) return 0;
            while (n % p == 0) n /= p;
        }
    }
    if (n > 1) ++factors;
    return (factors & 1U) ? -1 : 1;
}

static uint64_t add_mod(uint64_t a, uint64_t b, uint64_t p)
{
    uint64_t s = a + b;
    return s >= p ? s - p : s;
}

static uint64_t sub_mod(uint64_t a, uint64_t b, uint64_t p)
{
    return a >= b ? a - b : p - (b - a);
}

static uint64_t mul_mod(uint64_t a, uint64_t b, uint64_t p)
{
    return (uint64_t)(((__uint128_t)a * b) % p);
}

static uint64_t pow_mod(uint64_t a, uint64_t e, uint64_t p)
{
    uint64_t result = 1;
    while (e) {
        if (e & 1U) result = mul_mod(result, a, p);
        a = mul_mod(a, a, p);
        e >>= 1;
    }
    return result;
}

static uint64_t hash_keys(uint64_t a, uint64_t b)
{
    a ^= a >> 30; a *= UINT64_C(0xbf58476d1ce4e5b9);
    a ^= a >> 27; a *= UINT64_C(0x94d049bb133111eb);
    a ^= a >> 31;
    b ^= b >> 30; b *= UINT64_C(0xbf58476d1ce4e5b9);
    b ^= b >> 27; b *= UINT64_C(0x94d049bb133111eb);
    b ^= b >> 31;
    return a ^ (b + UINT64_C(0x9e3779b97f4a7c15) + (a << 6) + (a >> 2));
}

static uint64_t map_bytes(size_t capacity)
{
    if (capacity > UINT64_MAX / (sizeof(StateEntry) + sizeof(uint32_t)))
        return UINT64_MAX;
    return (uint64_t)capacity *
           (uint64_t)(sizeof(StateEntry) + sizeof(uint32_t));
}

static bool map_allocate(StateMap *map, size_t capacity,
                         size_t modulus_count, MemoryTracker *memory)
{
    if (capacity < 16 || (capacity & (capacity - 1U)) != 0 ||
        capacity > UINT32_MAX) return false;
    uint64_t bytes = map_bytes(capacity);
    if (bytes == UINT64_MAX || !tracked_reserve(memory, bytes)) return false;
    void *block = malloc((size_t)bytes);
    if (!block) { tracked_release(memory, bytes); return false; }
    map->block = block;
    map->entry = block;
    map->active = (uint32_t *)(map->entry + capacity);
    map->capacity = capacity;
    map->count = 0;
    map->modulus_count = modulus_count;
    for (size_t i = 0; i < capacity; ++i) {
        map->entry[i].row_key = EMPTY_KEY;
        map->entry[i].column_key = EMPTY_KEY;
    }
    return true;
}

static void map_destroy(StateMap *map, MemoryTracker *memory)
{
    if (map->block) {
        uint64_t bytes = map_bytes(map->capacity);
        free(map->block);
        tracked_release(memory, bytes);
    }
    memset(map, 0, sizeof(*map));
}

static bool vector_zero(const uint64_t *value, size_t count)
{
    for (size_t i = 0; i < count; ++i) if (value[i]) return false;
    return true;
}

static void map_insert_raw(StateMap *map, uint64_t row, uint64_t column,
                           const uint64_t *value)
{
    size_t slot = (size_t)hash_keys(row, column) & (map->capacity - 1U);
    while (map->entry[slot].row_key != EMPTY_KEY) {
        slot = (slot + 1U) & (map->capacity - 1U);
    }
    StateEntry *entry = &map->entry[slot];
    entry->row_key = row; entry->column_key = column;
    memset(entry->value, 0, sizeof(entry->value));
    memcpy(entry->value, value, map->modulus_count * sizeof(*value));
    map->active[map->count++] = (uint32_t)slot;
}

static bool map_grow(StateMap *map, const uint64_t *modulus,
                     MemoryTracker *memory)
{
    if (map->capacity > UINT32_MAX / 2U) return false;
    StateMap larger = {0};
    if (!map_allocate(&larger, map->capacity * 2U,
                      map->modulus_count, memory)) return false;
    for (size_t i = 0; i < map->count; ++i) {
        StateEntry *old = &map->entry[map->active[i]];
        if (!vector_zero(old->value, map->modulus_count))
            map_insert_raw(&larger, old->row_key, old->column_key,
                           old->value);
    }
    (void)modulus;
    map_destroy(map, memory);
    *map = larger;
    return true;
}

static bool map_add(StateMap *map, uint64_t row, uint64_t column,
                    const uint64_t *value, const uint64_t *modulus,
                    MemoryTracker *memory)
{
    if (vector_zero(value, map->modulus_count)) return true;
    if ((map->count + 1U) * 10U >= map->capacity * 9U) {
        if (!map_grow(map, modulus, memory)) return false;
    }
    size_t slot = (size_t)hash_keys(row, column) & (map->capacity - 1U);
    for (;;) {
        StateEntry *entry = &map->entry[slot];
        if (entry->row_key == EMPTY_KEY) {
            map_insert_raw(map, row, column, value);
            return true;
        }
        if (entry->row_key == row && entry->column_key == column) {
            for (size_t p = 0; p < map->modulus_count; ++p)
                entry->value[p] = add_mod(entry->value[p], value[p],
                                          modulus[p]);
            return true;
        }
        slot = (slot + 1U) & (map->capacity - 1U);
    }
}

static bool initial_map(StateMap *map, size_t expected, size_t moduli,
                        MemoryTracker *memory)
{
    size_t capacity = 16;
    if (expected > UINT32_MAX / 2U) return false;
    size_t wanted = expected + expected / 8U + 16U;
    while (capacity < wanted) {
        if (capacity > UINT32_MAX / 2U) return false;
        capacity *= 2U;
    }
    return map_allocate(map, capacity, moduli, memory);
}

static unsigned key_exponent(uint64_t key, unsigned position)
{
    return (unsigned)((key >> (COUNT_BITS * position)) & COUNT_MASK);
}

static uint64_t key_add(uint64_t key, unsigned position, unsigned amount)
{
    unsigned old = key_exponent(key, position);
    if (amount > MAX_N || old + amount > MAX_N) {
        die("polynomial exponent escaped 0..n");
    }
    return key + ((uint64_t)amount << (COUNT_BITS * position));
}

static void permanent_bound(mpz_t bound, int n)
{
    mpz_set_ui(bound, 1);
    for (int i = 1; i <= n; ++i) {
        unsigned long sum = 0;
        for (int j = 1; j <= n; ++j) sum += (unsigned long)gcd_int(i,j);
        mpz_mul_ui(bound, bound, sum);
    }
}

static void choose_moduli(CrtPlan *plan, const mpz_t bound)
{
    memset(plan, 0, sizeof(*plan));
    mpz_init_set_ui(plan->product, 1);
    mpz_t candidate, prime;
    mpz_inits(candidate, prime, NULL);
    mpz_set_ui(candidate, 1); mpz_mul_2exp(candidate, candidate, 60);
    while (mpz_cmp(plan->product, bound) <= 0) {
        if (plan->count == MAX_CRT_MODULI)
            die("four 61-bit CRT primes do not exceed the n<=35 bound");
        mpz_nextprime(prime, candidate);
        uint64_t p = (uint64_t)mpz_get_ui(prime);
        if (p >= (UINT64_C(1) << 61) || !mpz_probab_prime_p(prime,25))
            die("invalid CRT prime selection");
        plan->modulus[plan->count++] = p;
        mpz_mul_ui(plan->product, plan->product, (unsigned long)p);
        mpz_set(candidate, prime);
    }
    mpz_clears(candidate, prime, NULL);
    if (!plan->count || plan->modulus[0] != UINT64_C(1152921504606847009))
        die("reference CRT prime changed");
}

static void clear_moduli(CrtPlan *plan)
{
    mpz_clear(plan->product); memset(plan, 0, sizeof(*plan));
}

static unsigned lcm_unsigned(unsigned a, unsigned b)
{
    return a / (unsigned)gcd_int((int)a,(int)b) * b;
}

static bool exact_h_nonzero(unsigned i, unsigned j, int n)
{
    unsigned step = lcm_unsigned(i,j);
    mpq_t sum, term;
    mpq_inits(sum, term, NULL);
    mpq_set_ui(sum, 0, 1);
    for (unsigned k = step; k <= (unsigned)n; k += step) {
        int coefficient = mobius(k/i) * mobius(k/j);
        if (coefficient) {
            mpq_set_si(term, coefficient, (signed long)totient(k));
            mpq_add(sum, sum, term);
        }
    }
    bool nonzero = mpq_sgn(sum) != 0;
    mpq_clears(sum, term, NULL);
    return nonzero;
}

static Status build_decomposition(Node *node, int n,
                                  unsigned *width)
{
    uint64_t adjacency[MAX_N] = {0};
    for (int v = 0; v < n; ++v) {
        node[v].parent = -1;
        node[v].first_child = -1;
        node[v].next_sibling = -1;
    }
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (exact_h_nonzero((unsigned)i+1U,(unsigned)j+1U,n)) {
                adjacency[i] |= UINT64_C(1) << j;
                adjacency[j] |= UINT64_C(1) << i;
            }
        }
    }
    *width = 0;
    for (int v = n - 1; v >= 0; --v) {
        uint64_t alive = v == 0 ? 0 : ((UINT64_C(1) << v) - 1U);
        uint64_t separator = adjacency[v] & alive;
        unsigned size = (unsigned)__builtin_popcountll(separator);
        if (size > *width) *width = size;
        if (size > MAX_WIDTH) return STATUS_WIDTH;
        node[v].separator_mask = separator;
        node[v].separator_size = (uint8_t)size;
        unsigned q = 0;
        uint64_t scan = separator;
        while (scan) {
            unsigned u = (unsigned)__builtin_ctzll(scan);
            node[v].separator[q++] = (uint8_t)u;
            scan &= scan - 1U;
        }
        scan = separator;
        while (scan) {
            unsigned a = (unsigned)__builtin_ctzll(scan);
            scan &= scan - 1U;
            adjacency[a] |= separator & ~(UINT64_C(1) << a);
        }
        if (separator) {
            int parent = 63 - __builtin_clzll(separator);
            node[v].parent = parent;
            node[v].next_sibling = node[parent].first_child;
            node[parent].first_child = v;
        }
    }
    for (int v = 0; v < n; ++v) {
        if (node[v].parent >= 0) {
            uint64_t parent_bag = node[node[v].parent].separator_mask |
                                  (UINT64_C(1) << node[v].parent);
            if ((node[v].separator_mask & ~parent_bag) != 0)
                return STATUS_INTERNAL;
        }
    }
    return STATUS_OK;
}

static Status build_ldl(uint64_t beta[MAX_N][MAX_WIDTH][MAX_MODULI],
                        uint64_t inverse_pivot[MAX_N][MAX_MODULI],
                        const Node *node, int n,
                        const ModulusPlan *plan)
{
    static uint64_t h[MAX_MODULI][MAX_N][MAX_N];
    memset(h, 0, sizeof(h));
    for (size_t pindex = 0; pindex < plan->count; ++pindex) {
        uint64_t p = plan->modulus[pindex];
        uint64_t invphi[MAX_N+1] = {0};
        for (int k = 1; k <= n; ++k)
            invphi[k] = pow_mod(totient((unsigned)k), p-2U, p);
        for (int i = 1; i <= n; ++i) {
            for (int j = 1; j <= n; ++j) {
                unsigned step = lcm_unsigned((unsigned)i,(unsigned)j);
                uint64_t value = 0;
                for (unsigned k = step; k <= (unsigned)n; k += step) {
                    int coefficient = mobius(k/(unsigned)i) *
                                      mobius(k/(unsigned)j);
                    if (coefficient > 0) value = add_mod(value,invphi[k],p);
                    else if (coefficient < 0) value = sub_mod(value,invphi[k],p);
                }
                h[pindex][i-1][j-1] = value;
            }
        }
        for (int v = n - 1; v >= 0; --v) {
            uint64_t pivot = h[pindex][v][v];
            if (!pivot) return STATUS_PIVOT;
            uint64_t inverse = pow_mod(pivot,p-2U,p);
            inverse_pivot[v][pindex] = inverse;
            unsigned size = node[v].separator_size;
            for (unsigned a = 0; a < size; ++a) {
                unsigned u = node[v].separator[a];
                beta[v][a][pindex] = h[pindex][v][u] == 0 ? 0 :
                    p - mul_mod(h[pindex][v][u], inverse, p);
            }
            for (unsigned a = 0; a < size; ++a) {
                unsigned u = node[v].separator[a];
                for (unsigned b = 0; b < size; ++b) {
                    unsigned w = node[v].separator[b];
                    uint64_t correction = mul_mod(
                        mul_mod(h[pindex][u][v],h[pindex][v][w],p),
                        inverse,p);
                    h[pindex][u][w] = sub_mod(h[pindex][u][w],correction,p);
                }
            }
        }
    }
    return STATUS_OK;
}

static void update_peak(Statistics *stats, const StateMap *map)
{
    if (map->count > stats->peak_states) stats->peak_states = map->count;
}

static Status combine_message(StateMap *current, const StateMap *child,
                              const Node *child_node,
                              unsigned vertex, const Node *node,
                              const ModulusPlan *plan,
                              MemoryTracker *memory, Statistics *stats)
{
    unsigned bag[MAX_WIDTH+1];
    bag[0] = vertex;
    for (unsigned i=0;i<node[vertex].separator_size;++i)
        bag[i+1]=node[vertex].separator[i];
    unsigned map_position[MAX_WIDTH];
    for (unsigned i=0;i<child_node->separator_size;++i) {
        unsigned j=0;
        while (j<=node[vertex].separator_size &&
               bag[j]!=child_node->separator[i]) ++j;
        if (j>node[vertex].separator_size) return STATUS_INTERNAL;
        map_position[i]=j;
    }
    StateMap result={0};
    size_t expected=current->count>child->count?current->count:child->count;
    if (!initial_map(&result,expected,plan->count,memory)) return STATUS_MEMORY;
    for (size_t ai=0;ai<current->count;++ai) {
        const StateEntry *a=&current->entry[current->active[ai]];
        if (vector_zero(a->value,plan->count)) continue;
        for (size_t bi=0;bi<child->count;++bi) {
            const StateEntry *b=&child->entry[child->active[bi]];
            if (vector_zero(b->value,plan->count)) continue;
            uint64_t row=a->row_key,column=a->column_key;
            for (unsigned k=0;k<child_node->separator_size;++k) {
                row=key_add(row,map_position[k],key_exponent(b->row_key,k));
                column=key_add(column,map_position[k],key_exponent(b->column_key,k));
            }
            uint64_t value[MAX_MODULI]={0};
            for (size_t p=0;p<plan->count;++p)
                value[p]=mul_mod(a->value[p],b->value[p],plan->modulus[p]);
            if (!map_add(&result,row,column,value,plan->modulus,memory)) {
                map_destroy(&result,memory); return STATUS_MEMORY;
            }
            if (stats->products!=UINT64_MAX) ++stats->products;
        }
    }
    map_destroy(current,memory); *current=result; update_peak(stats,current);
    return STATUS_OK;
}

static Status multiply_by_linear(StateMap *map, bool row_side,
                                 unsigned separator_size,
                                 uint64_t beta[MAX_WIDTH][MAX_MODULI],
                                 const ModulusPlan *plan,
                                 MemoryTracker *memory,
                                 Statistics *stats)
{
    if (map->count == 0) return STATUS_OK;
    StateMap result = {0};
    if (!initial_map(&result, map->count, plan->count, memory))
        return STATUS_MEMORY;
    for (size_t i = 0; i < map->count; ++i) {
        const StateEntry *source = &map->entry[map->active[i]];
        if (vector_zero(source->value, plan->count)) continue;
        for (unsigned j = 0; j < separator_size; ++j) {
            uint64_t value[MAX_MODULI] = {0};
            for (size_t p = 0; p < plan->count; ++p)
                value[p] = mul_mod(source->value[p], beta[j][p],
                                   plan->modulus[p]);
            uint64_t row = source->row_key;
            uint64_t column = source->column_key;
            if (row_side) row = key_add(row, j, 1);
            else column = key_add(column, j, 1);
            if (!map_add(&result, row, column, value,
                         plan->modulus, memory)) {
                map_destroy(&result, memory);
                return STATUS_MEMORY;
            }
            if (stats->transforms != UINT64_MAX) ++stats->transforms;
        }
    }
    map_destroy(map, memory);
    *map = result;
    update_peak(stats, map);
    return STATUS_OK;
}

static Status eliminate_vertex(StateMap *message, StateMap *current,
                               unsigned vertex, const Node *node,
                               uint64_t beta[MAX_N][MAX_WIDTH][MAX_MODULI],
                               uint64_t inverse_pivot[MAX_N][MAX_MODULI],
                               const ModulusPlan *plan,
                               MemoryTracker *memory, Statistics *stats)
{
    uint64_t factorial[MAX_MODULI][MAX_N+1]={0};
    uint64_t inverse_factorial[MAX_MODULI][MAX_N+1]={0};
    uint64_t inverse_power[MAX_MODULI][MAX_N+1]={0};
    for (size_t p=0;p<plan->count;++p) {
        factorial[p][0]=inverse_factorial[p][0]=inverse_power[p][0]=1;
        for (int k=1;k<=MAX_N;++k) {
            factorial[p][k]=mul_mod(factorial[p][k-1],(unsigned)k,
                                     plan->modulus[p]);
            inverse_power[p][k]=mul_mod(inverse_power[p][k-1],
                                        inverse_pivot[vertex][p],
                                        plan->modulus[p]);
        }
        inverse_factorial[p][MAX_N]=pow_mod(factorial[p][MAX_N],
                                            plan->modulus[p]-2U,
                                            plan->modulus[p]);
        for (int k=MAX_N;k>0;--k)
            inverse_factorial[p][k-1]=mul_mod(inverse_factorial[p][k],
                                               (unsigned)k,plan->modulus[p]);
    }
    unsigned separator_size=node[vertex].separator_size;
    Status status=STATUS_OK;

    /* First apply exp(h^(-1) d_z d_zbar).  Variables remain in the
       [v, separator] layout, so equal keys combine before substitution. */
    StateMap contracted={0};
    if (!initial_map(&contracted,current->count,plan->count,memory)) {
        map_destroy(current,memory);
        return STATUS_MEMORY;
    }
    for (size_t index=0;index<current->count;++index) {
        const StateEntry *source=&current->entry[current->active[index]];
        if (vector_zero(source->value,plan->count)) continue;
        unsigned a=key_exponent(source->row_key,0);
        unsigned b=key_exponent(source->column_key,0);
        unsigned limit=a<b?a:b;
        for (unsigned t=0;t<=limit;++t) {
            uint64_t value[MAX_MODULI]={0};
            for (size_t p=0;p<plan->count;++p) {
                uint64_t choose_a=mul_mod(factorial[p][a],
                    mul_mod(inverse_factorial[p][t],
                            inverse_factorial[p][a-t],plan->modulus[p]),
                    plan->modulus[p]);
                uint64_t choose_b=mul_mod(factorial[p][b],
                    mul_mod(inverse_factorial[p][t],
                            inverse_factorial[p][b-t],plan->modulus[p]),
                    plan->modulus[p]);
                value[p]=mul_mod(source->value[p],choose_a,plan->modulus[p]);
                value[p]=mul_mod(value[p],choose_b,plan->modulus[p]);
                value[p]=mul_mod(value[p],factorial[p][t],plan->modulus[p]);
                value[p]=mul_mod(value[p],inverse_power[p][t],plan->modulus[p]);
            }
            uint64_t row=source->row_key-t;
            uint64_t column=source->column_key-t;
            if (!map_add(&contracted,row,column,value,
                         plan->modulus,memory)) {
                status=STATUS_MEMORY; goto finish;
            }
            if (stats->transforms!=UINT64_MAX) ++stats->transforms;
        }
    }

    /* Horner substitution z_v <- sum beta_j z_j.  The column key still
       uses [v,separator], while the row key now uses just separator. */
    unsigned maximum_row=0;
    for (size_t i=0;i<contracted.count;++i) {
        StateEntry *entry=&contracted.entry[contracted.active[i]];
        unsigned exponent=key_exponent(entry->row_key,0);
        if (exponent>maximum_row) maximum_row=exponent;
    }
    StateMap row_result={0};
    if (!map_allocate(&row_result,16,plan->count,memory)) {
        status=STATUS_MEMORY; goto finish;
    }
    for (int degree=(int)maximum_row;degree>=0;--degree) {
        status=multiply_by_linear(&row_result,true,separator_size,
                                  beta[vertex],plan,memory,stats);
        if (status!=STATUS_OK) goto finish;
        for (size_t i=0;i<contracted.count;++i) {
            StateEntry *entry=&contracted.entry[contracted.active[i]];
            if (key_exponent(entry->row_key,0)!=(unsigned)degree) continue;
            if (!map_add(&row_result,entry->row_key>>COUNT_BITS,
                         entry->column_key,entry->value,
                         plan->modulus,memory)) {
                status=STATUS_MEMORY; goto finish;
            }
        }
    }
    map_destroy(&contracted,memory);

    /* The conjugate substitution is the same Horner operation on columns. */
    unsigned maximum_column=0;
    for (size_t i=0;i<row_result.count;++i) {
        StateEntry *entry=&row_result.entry[row_result.active[i]];
        unsigned exponent=key_exponent(entry->column_key,0);
        if (exponent>maximum_column) maximum_column=exponent;
    }
    if (!map_allocate(message,16,plan->count,memory)) {
        status=STATUS_MEMORY; goto finish;
    }
    for (int degree=(int)maximum_column;degree>=0;--degree) {
        status=multiply_by_linear(message,false,separator_size,
                                  beta[vertex],plan,memory,stats);
        if (status!=STATUS_OK) goto finish;
        for (size_t i=0;i<row_result.count;++i) {
            StateEntry *entry=&row_result.entry[row_result.active[i]];
            if (key_exponent(entry->column_key,0)!=(unsigned)degree) continue;
            if (!map_add(message,entry->row_key,
                         entry->column_key>>COUNT_BITS,entry->value,
                         plan->modulus,memory)) {
                status=STATUS_MEMORY; goto finish;
            }
        }
    }
    map_destroy(&row_result,memory);
finish:
    map_destroy(&contracted,memory);
    map_destroy(&row_result,memory);
    map_destroy(current,memory);
    if (status!=STATUS_OK) map_destroy(message,memory);
    else update_peak(stats,message);
    return status;
}

static Status gaussian_permanent(uint64_t *residue, int n,
                                 const ModulusPlan *plan,
                                 MemoryTracker *memory,
                                 Statistics *stats, bool report)
{
    Node node[MAX_N]; memset(node,0,sizeof(node));
    Status status=build_decomposition(node,n,&stats->width);
    if (status!=STATUS_OK) return status;
    static uint64_t beta[MAX_N][MAX_WIDTH][MAX_MODULI];
    static uint64_t inverse_pivot[MAX_N][MAX_MODULI];
    memset(beta,0,sizeof(beta)); memset(inverse_pivot,0,sizeof(inverse_pivot));
    status=build_ldl(beta,inverse_pivot,node,n,plan);
    if (status!=STATUS_OK) return status;
    if (report) fprintf(stderr,
        "085244_03: n=%d inverse-precision width=%u\n",n,stats->width);
    uint64_t one[MAX_MODULI]={0};
    for (size_t p=0;p<plan->count;++p) one[p]=1;
    uint64_t root_value[MAX_MODULI];
    for (size_t p=0;p<plan->count;++p) root_value[p]=1;
    stats->peak_states=1;
    for (int v=n-1;v>=0;--v) {
        StateMap current={0};
        if (!map_allocate(&current,16,plan->count,memory)) {
            status=STATUS_MEMORY; goto cleanup;
        }
        map_insert_raw(&current,1,1,one);
        for (int child=node[v].first_child;child>=0;
             child=node[child].next_sibling) {
            status=combine_message(&current,&node[child].message,
                                   &node[child],(unsigned)v,node,plan,
                                   memory,stats);
            if (status!=STATUS_OK) { map_destroy(&current,memory); goto cleanup; }
            map_destroy(&node[child].message,memory);
        }
        status=eliminate_vertex(&node[v].message,&current,(unsigned)v,node,
                                beta,inverse_pivot,plan,memory,stats);
        if (status!=STATUS_OK) goto cleanup;
        if (node[v].parent<0) {
            if (node[v].message.count!=1) { status=STATUS_INTERNAL; goto cleanup; }
            StateEntry *entry=&node[v].message.entry[node[v].message.active[0]];
            if (entry->row_key||entry->column_key) {
                status=STATUS_INTERNAL; goto cleanup;
            }
            for (size_t p=0;p<plan->count;++p)
                root_value[p]=mul_mod(root_value[p],entry->value[p],
                                      plan->modulus[p]);
            map_destroy(&node[v].message,memory);
        }
        if (report && (v==0 || ((unsigned)(n-v)%5U)==0))
            fprintf(stderr,
                "085244_03: n=%d eliminated=%d/%d vertex=%d sep=%u "
                "states=%zu live=%.3f GiB peak=%.3f GiB\n",
                n,n-v,n,v+1,node[v].separator_size,
                node[v].parent<0?1:node[v].message.count,
                (double)memory->used/(double)(UINT64_C(1)<<30),
                (double)memory->peak/(double)(UINT64_C(1)<<30));
    }
    memcpy(residue,root_value,plan->count*sizeof(*residue));
    status=STATUS_OK;
cleanup:
    for (int v=0;v<n;++v) map_destroy(&node[v].message,memory);
    return status;
}

static void reconstruct_crt(mpz_t result, const uint64_t *residue,
                            const CrtPlan *plan)
{
    mpz_t product; mpz_init_set_ui(product,1); mpz_set_ui(result,0);
    for (size_t i=0;i<plan->count;++i) {
        uint64_t p=plan->modulus[i];
        uint64_t pm=(uint64_t)mpz_fdiv_ui(product,(unsigned long)p);
        uint64_t rm=(uint64_t)mpz_fdiv_ui(result,(unsigned long)p);
        uint64_t multiplier=mul_mod(sub_mod(residue[i],rm,p),
                                    pow_mod(pm,p-2U,p),p);
        mpz_addmul_ui(result,product,(unsigned long)multiplier);
        mpz_mul_ui(product,product,(unsigned long)p);
    }
    mpz_clear(product);
}

static void verify_crt(const mpz_t result, const mpz_t bound,
                       const uint64_t *residue, const CrtPlan *plan)
{
    if (mpz_sgn(result)<0 || mpz_cmp(result,bound)>0 ||
        mpz_cmp(plan->product,bound)<=0 || mpz_cmp(result,plan->product)>=0)
        die("CRT uniqueness/range check failed");
    for (size_t i=0;i<plan->count;++i)
        if ((uint64_t)mpz_fdiv_ui(result,(unsigned long)plan->modulus[i])
            !=residue[i]) die("CRT residue replay failed");
}

static bool direct_subset(uint64_t *answer, int n, uint64_t p,
                          uint64_t memory_limit)
{
    size_t states=(size_t)1U<<n;
    if (states>SIZE_MAX/sizeof(uint64_t)) return false;
    uint64_t bytes=(uint64_t)states*sizeof(uint64_t);
    if (bytes>memory_limit) return false;
    uint64_t *dp=calloc(states,sizeof(*dp)); if (!dp) return false;
    dp[0]=1;
    for (size_t mask=0;mask+1U<states;++mask) {
        if (!dp[mask]) continue;
        int row=__builtin_popcountll((uint64_t)mask)+1;
        size_t available=(states-1U)^mask;
        while (available) {
            size_t bit=available&(0U-available);
            int column=__builtin_ctzll((uint64_t)bit)+1;
            uint64_t term=mul_mod(dp[mask],(unsigned)gcd_int(row,column),p);
            dp[mask|bit]=add_mod(dp[mask|bit],term,p);
            available^=bit;
        }
    }
    *answer=dp[states-1U]; free(dp); return true;
}

static const char *status_name(Status status)
{
    switch(status) {
    case STATUS_MEMORY:return "memory limit/allocation";
    case STATUS_WIDTH:return "inverse-precision width limit";
    case STATUS_OVERFLOW:return "size overflow";
    case STATUS_PIVOT:return "zero modular LDL pivot";
    case STATUS_INTERNAL:return "internal consistency";
    default:return "ok";
    }
}

static bool compute_exact(mpz_t result, int n, uint64_t memory_limit,
                          bool report)
{
    mpz_t bound; mpz_init(bound); permanent_bound(bound,n);
    CrtPlan crt; choose_moduli(&crt,bound);
    size_t pass_count=(crt.count+MAX_MODULI-1U)/MAX_MODULI;
    if (report) fprintf(stderr,
        "085244_03: n=%d bound=%zu bits, %zu CRT prime%s in %zu pass%s, "
        "memory limit=%.2f GiB\n",n,mpz_sizeinbase(bound,2),crt.count,
        crt.count==1?"":"s",pass_count,pass_count==1?"":"es",
        (double)memory_limit/(double)(UINT64_C(1)<<30));
    Statistics stats={0}; uint64_t peak_memory=0;
    uint64_t residue[MAX_CRT_MODULI]={0}; double start=monotonic_seconds();
    for (size_t offset=0, pass_index=0;offset<crt.count;
         offset+=MAX_MODULI,++pass_index) {
        ModulusPlan plan={0};
        plan.count=crt.count-offset;
        if (plan.count>MAX_MODULI) plan.count=MAX_MODULI;
        for (size_t p=0;p<plan.count;++p)
            plan.modulus[p]=crt.modulus[offset+p];
        if (report) fprintf(stderr,
            "085244_03: n=%d pass=%zu/%zu, primes=%zu\n",
            n,pass_index+1U,pass_count,plan.count);
        MemoryTracker memory={.limit=memory_limit}; Statistics pass_stats={0};
        Status status=gaussian_permanent(residue+offset,n,&plan,&memory,
                                         &pass_stats,report);
        if (memory.peak>peak_memory) peak_memory=memory.peak;
        if (status!=STATUS_OK) {
            fprintf(stderr,"error: A085244 n=%d pass=%zu/%zu stopped: %s; "
                    "used=%.3f GiB, peak=%.3f GiB, failed=%.3f GiB, width=%u\n",
                    n,pass_index+1U,pass_count,status_name(status),
                    (double)memory.used/(double)(UINT64_C(1)<<30),
                    (double)memory.peak/(double)(UINT64_C(1)<<30),
                    (double)memory.required/(double)(UINT64_C(1)<<30),
                    pass_stats.width);
            clear_moduli(&crt); mpz_clear(bound); return false;
        }
        stats.width=pass_stats.width;
        if (pass_stats.peak_states>stats.peak_states)
            stats.peak_states=pass_stats.peak_states;
        stats.products=UINT64_MAX-stats.products<pass_stats.products?
            UINT64_MAX:stats.products+pass_stats.products;
        stats.transforms=UINT64_MAX-stats.transforms<pass_stats.transforms?
            UINT64_MAX:stats.transforms+pass_stats.transforms;
    }
    if (n<=KNOWN_RESIDUE_N && residue[0]!=known_residue[n]) {
        fprintf(stderr,"error: n=%d residue=%" PRIu64
                " differs from reference=%" PRIu64 "\n",
                n,residue[0],known_residue[n]);
        clear_moduli(&crt);mpz_clear(bound);return false;
    }
    if (n<=DIRECT_CHECK_N) {
        uint64_t direct=0;
        if (!direct_subset(&direct,n,crt.modulus[0],memory_limit) ||
            direct!=residue[0]) die("Gaussian DP differs from subset DP");
    }
    reconstruct_crt(result,residue,&crt); verify_crt(result,bound,residue,&crt);
    if (n<=KNOWN_EXACT_N) {
        mpz_t known;mpz_init_set_str(known,known_exact[n],10);
        if (mpz_cmp(known,result)!=0) die("result differs from built-in term");
        mpz_clear(known);
    }
    stats.seconds=monotonic_seconds()-start;
    if (report) fprintf(stderr,
        "085244_03: n=%d done, width=%u, peak states=%zu, products=%" PRIu64
        ", transforms=%" PRIu64 ", peak=%.3f GiB, %.3f s\n",n,
        stats.width,stats.peak_states,stats.products,stats.transforms,
        (double)peak_memory/(double)(UINT64_C(1)<<30),stats.seconds);
    clear_moduli(&crt);mpz_clear(bound);return true;
}

static char *beside_executable(const char *argv0,const char *filename)
{
    char executable[PATH_MAX],resolved[PATH_MAX];bool found=false;
#ifdef __APPLE__
    uint32_t size=(uint32_t)sizeof(executable);
    if (_NSGetExecutablePath(executable,&size)==0) found=true;
#elif defined(__linux__)
    ssize_t length=readlink("/proc/self/exe",executable,sizeof(executable)-1U);
    if (length>=0) { executable[length]='\0';found=true; }
#endif
    if (!found) {
        size_t length=strlen(argv0);if(length>=sizeof(executable))die("path too long");
        memcpy(executable,argv0,length+1U);
    }
    const char *base=realpath(executable,resolved);if(!base)base=executable;
    const char *slash=strrchr(base,'/');
    size_t directory_length=slash?(size_t)(slash-base):1U;
    const char *directory=slash?base:".";size_t flen=strlen(filename);
    char *path=malloc(directory_length+flen+2U);if(!path)die("path allocation");
    memcpy(path,directory,directory_length);path[directory_length]='/';
    memcpy(path+directory_length+1U,filename,flen+1U);return path;
}

static int run_check(int maximum,uint64_t memory_limit)
{
    if(maximum>KNOWN_EXACT_N)die("--check exceeds built-in exact prefix");
    mpz_t result;mpz_init(result);
    for(int n=1;n<=maximum;++n)
        if(!compute_exact(result,n,memory_limit,false)){
            mpz_clear(result);return EXIT_FAILURE;
        }
    mpz_clear(result);
    printf("ok: inverse-Gaussian/CRT agrees with A085244 for n=1..%d\n",maximum);
    return EXIT_SUCCESS;
}

static int produce_file(const char *argv0,int maximum,int start,
                        uint64_t memory_limit)
{
    if(start>KNOWN_EXACT_N+1)die("START_N exceeds built-in prefix boundary");
    char *part=beside_executable(argv0,"b085244_3_part.txt");
    char *final=beside_executable(argv0,"b085244_3.txt");
    FILE *output=fopen(part,"w");
    if(!output){fprintf(stderr,"error: cannot open %s: %s\n",part,strerror(errno));
        free(part);free(final);return EXIT_FAILURE;}
    int prefix=start-1;if(prefix>maximum)prefix=maximum;
    for(int n=1;n<=prefix;++n)
        if(fprintf(output,"%d %s\n",n,known_exact[n])<0)die("b-file write");
    mpz_t result;mpz_init(result);
    for(int n=start;n<=maximum;++n) {
        if(!compute_exact(result,n,memory_limit,true)){
            fclose(output);mpz_clear(result);free(part);free(final);
            return EXIT_FAILURE;
        }
        if(gmp_fprintf(output,"%d %Zd\n",n,result)<0 || fflush(output) ||
           fsync(fileno(output)))die("b-file flush");
        gmp_printf("%d %Zd\n",n,result);fflush(stdout);
    }
    mpz_clear(result);
    if(fclose(output)||rename(part,final)){
        fprintf(stderr,"error: cannot finalize %s: %s\n",final,strerror(errno));
        free(part);free(final);return EXIT_FAILURE;
    }
    fprintf(stderr,"wrote %s (n=1..%d)\n",final,maximum);
    free(part);free(final);return EXIT_SUCCESS;
}

static void usage(const char *program)
{
    fprintf(stderr,"usage: %s --term N\n       %s --check [N]\n"
        "       %s MAX_N [START_N]\nN is in 1..%d.\n",
        program,program,program,MAX_N);
}

int main(int argc,char **argv)
{
    uint64_t memory_limit=memory_limit_bytes();
    if(argc>=2 && strcmp(argv[1],"--term")==0) {
        if(argc!=3){usage(argv[0]);return EXIT_FAILURE;}
        int n=parse_int(argv[2],1,MAX_N,"N");mpz_t result;mpz_init(result);
        bool ok=compute_exact(result,n,memory_limit,true);
        if(ok)gmp_printf("%d %Zd\n",n,result);mpz_clear(result);
        return ok?EXIT_SUCCESS:EXIT_FAILURE;
    }
    if(argc>=2 && strcmp(argv[1],"--check")==0) {
        if(argc>3){usage(argv[0]);return EXIT_FAILURE;}
        int maximum=argc==3?parse_int(argv[2],1,KNOWN_EXACT_N,"N"):20;
        return run_check(maximum,memory_limit);
    }
    if(argc<2||argc>3){usage(argv[0]);return EXIT_FAILURE;}
    int maximum=parse_int(argv[1],1,MAX_N,"MAX_N");
    int start=argc==3?parse_int(argv[2],1,maximum+1,"START_N"):
        (maximum<=KNOWN_EXACT_N?1:KNOWN_EXACT_N+1);
    return produce_file(argv[0],maximum,start,memory_limit);
}
