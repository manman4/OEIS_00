/*
 * A073090 -- independent exact-target counter.
 *
 * This implementation deliberately differs from 073090_01.c.  Version 01
 * carries every partial residue modulo L in a breadth-first sparse DP.  Here
 * we enumerate the finitely many possible integer answers t and solve
 *
 *                 sum_j q(j)*(L/j) = t*L                       (1)
 *
 * separately by a top-down exact-assignment recursion.  The counts for all t
 * are added.  Independent targets run in parallel.  A memo table is private
 * to one target t and caches both zero and nonzero subtree counts.
 *
 * The recursion rejects a state only by exact necessary conditions:
 *
 *   - rearrangement-inequality minimum and maximum completion sums;
 *   - gcd divisibility of the exact residual;
 *   - the permutation-lattice congruence obtained from pair exchanges;
 *   - prime-power residual gcd conditions.
 *
 * For a prime-power modulus P, if g is the gcd of P and all still available
 * coefficients, every remaining contribution is divisible by g.  Therefore
 * the exact residual must be divisible by g.  This is the same proved
 * mathematical pruning fact used by 073090_01.c, but the target-by-target
 * recursive counting algorithm and memoization are independent of 01's
 * layer DP.
 *
 * If n is prime, q(n)=n and a(n)=a(n-1).
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic 073090_02.c \
 *       -o 073090_02 -pthread
 *
 * Usage:
 *   ./073090_02
 *   ./073090_02 --upto 26 --verbose
 *   ./073090_02 --term 26 --threads 4 --memory-mb 4096 --verbose
 *   ./073090_02 --check
 *
 * --memory-mb is a total bound and is divided among workers.  For a difficult
 * term, reducing --threads gives each target a larger memo table.
 *
 * The default and --upto atomically replace b073090_02.txt.  --term and
 * --check do not modify it.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_N 26
#define DEFAULT_MAX_N 20
#define KNOWN_MAX_N 17
#define DIRECT_CHECK_MAX_N 10
#define MAX_CONSTRAINTS 9
#define DEFAULT_MEMORY_MB 4096
#define MIN_MEMORY_MB 32
#define MAX_MEMORY_MB 65536
#define DEFAULT_THREADS 4
#define MAX_THREADS 64
#define INITIAL_MEMO_CAPACITY 1024
#define BFILE_NAME "b073090_02.txt"
#define BFILE_TEMP_TEMPLATE BFILE_NAME ".tmp.XXXXXX"
#define BFILE_LOCK_NAME BFILE_NAME ".lock"

_Static_assert(MAX_N < 32, "32-bit masks require n<32");

typedef uint32_t mask_t;

static const uint64_t known[KNOWN_MAX_N + 1] = {
    UINT64_C(1),        UINT64_C(1),        UINT64_C(1),
    UINT64_C(1),        UINT64_C(2),        UINT64_C(2),
    UINT64_C(8),        UINT64_C(8),        UINT64_C(22),
    UINT64_C(104),      UINT64_C(1128),     UINT64_C(1128),
    UINT64_C(14520),    UINT64_C(14520),    UINT64_C(229734),
    UINT64_C(3217088),  UINT64_C(21157428), UINT64_C(21157428)
};

typedef struct {
    mask_t denominator_mask;
    uint32_t modulus;
    uint32_t coefficient[MAX_N + 1];
} Constraint;

typedef struct {
    uint64_t key_plus_one;
    uint64_t count;
} MemoEntry;

typedef struct {
    MemoEntry *entry;
    size_t capacity;
    size_t used;
} MemoTable;

typedef struct {
    int n;
    uint64_t lcm;
    uint64_t weight[MAX_N + 1];
    Constraint constraint[MAX_CONSTRAINTS];
    unsigned constraint_count;
    int denominator_order[MAX_N];
    mask_t remaining_denominators[MAX_N + 1];
    uint64_t remaining_gcd[MAX_N + 1];
    uint64_t prime_divisor[MAX_N + 1];
    uint64_t target;

    size_t memory_limit;
    size_t live_memory;
    size_t peak_memory;
    size_t peak_memo_states;
    uint64_t nodes;
    uint64_t memo_hits;
    MemoTable memo;
} Search;

typedef struct {
    uint64_t nodes;
    uint64_t memo_hits;
    size_t peak_memo_states;
    size_t peak_memory;
    unsigned targets;
    int searched_n;
    bool prime_step;
} Statistics;

typedef struct {
    int n;
    uint64_t last_target;
    size_t memory_per_worker;
    _Atomic uint64_t next_target;
} TargetPool;

typedef struct {
    TargetPool *pool;
    uint64_t answer;
    uint64_t nodes;
    uint64_t memo_hits;
    size_t peak_memo_states;
    size_t peak_memory;
    unsigned targets;
} TargetWorker;

typedef enum {
    MODE_UPTO,
    MODE_TERM
} OutputMode;

static char bfile_temp_name[] = BFILE_TEMP_TEMPLATE;
static int bfile_lock_descriptor = -1;
static bool bfile_temp_active = false;

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        die("clock_gettime failed");
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static uint64_t gcd_u64(uint64_t a, uint64_t b)
{
    while (b != 0U) {
        const uint64_t remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

static bool is_prime(unsigned value)
{
    if (value < 2U) return false;
    if ((value & 1U) == 0U) return value == 2U;
    for (unsigned divisor = 3; divisor <= value / divisor; divisor += 2U)
        if (value % divisor == 0U) return false;
    return true;
}

static uint64_t make_lcm(int n)
{
    uint64_t result = 1;
    for (uint64_t k = 2; k <= (uint64_t)n; ++k) {
        const uint64_t factor = k / gcd_u64(result, k);
        if (result > UINT64_MAX / factor) die("LCM overflow");
        result *= factor;
    }
    return result;
}

static unsigned bit_count(mask_t mask)
{
#if defined(__clang__) || defined(__GNUC__)
    return (unsigned)__builtin_popcount(mask);
#else
    unsigned result = 0;
    while (mask != 0U) {
        mask &= mask - 1U;
        ++result;
    }
    return result;
#endif
}

static int first_index(mask_t mask)
{
#if defined(__clang__) || defined(__GNUC__)
    return (int)__builtin_ctz(mask) + 1;
#else
    int result = 1;
    while ((mask & 1U) == 0U) {
        mask >>= 1U;
        ++result;
    }
    return result;
#endif
}

static mask_t full_mask(int n)
{
    return ((mask_t)1U << n) - 1U;
}

static int parse_integer(const char *text, const char *label,
                         int minimum, int maximum)
{
    errno = 0;
    char *end = NULL;
    const intmax_t value = strtoimax(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int parse_n(const char *text)
{
    return parse_integer(text, "n", 0, MAX_N);
}

static unsigned parse_memory_mb(const char *text)
{
    return (unsigned)parse_integer(text, "memory-mb",
                                   MIN_MEMORY_MB, MAX_MEMORY_MB);
}

static unsigned parse_threads(const char *text)
{
    return (unsigned)parse_integer(text, "threads", 1, MAX_THREADS);
}

static uint64_t hash_u64(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static void memo_memory_error(const Search *search, size_t requested)
{
    fprintf(stderr,
            "error: memo memory limit exceeded at n=%d, target=%" PRIu64
            " (live %.1f MiB + request %.1f MiB > limit %.1f MiB); "
            "states=%zu, nodes=%" PRIu64 "; increase --memory-mb\n",
            search->n, search->target / search->lcm,
            (double)search->live_memory / 1048576.0,
            (double)requested / 1048576.0,
            (double)search->memory_limit / 1048576.0,
            search->memo.used, search->nodes);
    exit(EXIT_FAILURE);
}

static MemoEntry *allocate_memo(Search *search, size_t capacity)
{
    if (capacity > SIZE_MAX / sizeof(MemoEntry))
        die("memo allocation size overflow");
    const size_t bytes = capacity * sizeof(MemoEntry);
    if (bytes > search->memory_limit - search->live_memory)
        memo_memory_error(search, bytes);
    MemoEntry *result = calloc(capacity, sizeof(*result));
    if (result == NULL) {
        fprintf(stderr,
                "error: could not allocate %.1f MiB for target memo\n",
                (double)bytes / 1048576.0);
        exit(EXIT_FAILURE);
    }
    search->live_memory += bytes;
    if (search->live_memory > search->peak_memory)
        search->peak_memory = search->live_memory;
    return result;
}

static void memo_init(Search *search, size_t capacity)
{
    size_t rounded = INITIAL_MEMO_CAPACITY;
    while (rounded < capacity) {
        if (rounded > SIZE_MAX / 2U) die("memo capacity overflow");
        rounded *= 2U;
    }
    search->memo.entry = allocate_memo(search, rounded);
    search->memo.capacity = rounded;
    search->memo.used = 0;
}

static void memo_clear(Search *search)
{
    if (search->memo.entry == NULL) return;
    const size_t bytes = search->memo.capacity * sizeof(*search->memo.entry);
    if (bytes > search->live_memory) die("memo accounting error");
    free(search->memo.entry);
    search->live_memory -= bytes;
    memset(&search->memo, 0, sizeof(search->memo));
}

static void memo_grow(Search *search)
{
    MemoTable old = search->memo;
    if (old.capacity > SIZE_MAX / 2U) die("memo capacity overflow");
    MemoTable larger = {
        .entry = allocate_memo(search, old.capacity * 2U),
        .capacity = old.capacity * 2U,
        .used = 0
    };
    for (size_t i = 0; i < old.capacity; ++i) {
        const MemoEntry item = old.entry[i];
        if (item.key_plus_one == 0U) continue;
        size_t slot = (size_t)hash_u64(item.key_plus_one - 1U) &
                      (larger.capacity - 1U);
        while (larger.entry[slot].key_plus_one != 0U)
            slot = (slot + 1U) & (larger.capacity - 1U);
        larger.entry[slot] = item;
        ++larger.used;
    }
    const size_t old_bytes = old.capacity * sizeof(*old.entry);
    free(old.entry);
    search->live_memory -= old_bytes;
    search->memo = larger;
}

static bool memo_get(Search *search, uint64_t key, uint64_t *answer)
{
    const uint64_t stored = key + 1U;
    size_t slot = (size_t)hash_u64(key) &
                  (search->memo.capacity - 1U);
    while (search->memo.entry[slot].key_plus_one != 0U) {
        const MemoEntry *entry = &search->memo.entry[slot];
        if (entry->key_plus_one == stored) {
            *answer = entry->count;
            ++search->memo_hits;
            return true;
        }
        slot = (slot + 1U) & (search->memo.capacity - 1U);
    }
    return false;
}

static void memo_put(Search *search, uint64_t key, uint64_t answer)
{
    if ((search->memo.used + 1U) * 10U >=
        search->memo.capacity * 7U)
        memo_grow(search);
    const uint64_t stored = key + 1U;
    size_t slot = (size_t)hash_u64(key) &
                  (search->memo.capacity - 1U);
    while (search->memo.entry[slot].key_plus_one != 0U)
        slot = (slot + 1U) & (search->memo.capacity - 1U);
    search->memo.entry[slot].key_plus_one = stored;
    search->memo.entry[slot].count = answer;
    ++search->memo.used;
    if (search->memo.used > search->peak_memo_states)
        search->peak_memo_states = search->memo.used;
}

static void search_init(Search *search, int n, size_t memory_limit)
{
    memset(search, 0, sizeof(*search));
    search->n = n;
    search->memory_limit = memory_limit;
    search->lcm = make_lcm(n);
    for (int j = 1; j <= n; ++j)
        search->weight[j] = search->lcm / (uint64_t)j;

    for (unsigned prime = 2; prime <= (unsigned)n; ++prime) {
        if (!is_prime(prime)) continue;
        if (search->constraint_count == MAX_CONSTRAINTS)
            die("too many prime constraints");
        Constraint *constraint =
            &search->constraint[search->constraint_count++];
        uint32_t modulus = prime;
        while (modulus <= (uint32_t)n / prime) modulus *= prime;
        constraint->modulus = modulus;
        for (int j = (int)prime; j <= n; j += (int)prime) {
            constraint->denominator_mask |=
                (mask_t)1U << (j - 1);
            constraint->coefficient[j] =
                (uint32_t)(search->weight[j] % modulus);
        }
    }
}

static void assignment_bounds(const Search *search, mask_t denominators,
                              mask_t numerators, uint64_t *minimum,
                              uint64_t *maximum)
{
    int values[MAX_N];
    int count = 0;
    for (int value = 1; value <= search->n; ++value)
        if ((numerators & ((mask_t)1U << (value - 1))) != 0U)
            values[count++] = value;
    if ((unsigned)count != bit_count(denominators))
        die("assignment masks have different sizes");

    uint64_t low = 0;
    uint64_t high = 0;
    int rank = 0;
    for (int j = 1; j <= search->n; ++j) {
        if ((denominators & ((mask_t)1U << (j - 1))) == 0U) continue;
        low += (uint64_t)values[rank] * search->weight[j];
        high += (uint64_t)values[count - 1 - rank] * search->weight[j];
        ++rank;
    }
    *minimum = low;
    *maximum = high;
}

static bool lattice_congruence(const Search *search, mask_t denominators,
                               mask_t numerators, uint64_t residual)
{
    if (bit_count(numerators) < 2U) return true;
    int values[MAX_N];
    int count = 0;
    int first_numerator = 0;
    uint64_t numerator_gcd = 0;
    for (int value = 1; value <= search->n; ++value) {
        if ((numerators & ((mask_t)1U << (value - 1))) == 0U) continue;
        values[count++] = value;
        if (first_numerator == 0)
            first_numerator = value;
        else
            numerator_gcd = gcd_u64(
                numerator_gcd, (uint64_t)(value - first_numerator));
    }

    uint64_t base = 0;
    uint64_t weight_gcd = 0;
    uint64_t first_weight = 0;
    int index = 0;
    for (int j = 1; j <= search->n; ++j) {
        if ((denominators & ((mask_t)1U << (j - 1))) == 0U) continue;
        const uint64_t weight = search->weight[j];
        base += (uint64_t)values[index++] * weight;
        if (first_weight == 0U)
            first_weight = weight;
        else
            weight_gcd = gcd_u64(
                weight_gcd,
                first_weight > weight ? first_weight - weight
                                      : weight - first_weight);
    }
    if (numerator_gcd != 0U && weight_gcd > UINT64_MAX / numerator_gcd)
        die("lattice modulus overflow");
    const uint64_t modulus = numerator_gcd * weight_gcd;
    return modulus == 0U || residual % modulus == base % modulus;
}

static uint64_t exact_gcd(const Search *search, mask_t denominators)
{
    uint64_t result = 0;
    while (denominators != 0U) {
        const int j = first_index(denominators);
        denominators &= denominators - 1U;
        result = gcd_u64(result, search->weight[j]);
    }
    return result;
}

static uint64_t prime_required_divisor(const Search *search,
                                       mask_t denominators)
{
    uint64_t required = 1;
    for (unsigned c = 0; c < search->constraint_count; ++c) {
        const Constraint *constraint = &search->constraint[c];
        mask_t scan = denominators & constraint->denominator_mask;
        uint64_t divisor = constraint->modulus;
        while (scan != 0U) {
            const int j = first_index(scan);
            scan &= scan - 1U;
            divisor = gcd_u64(divisor,
                              constraint->coefficient[j]);
        }
        if (required > UINT64_MAX / divisor)
            die("prime required divisor overflow");
        required *= divisor;
    }
    if (search->lcm % required != 0U)
        die("invalid prime required divisor");
    return required;
}

static int choose_denominator(const Search *search, mask_t denominators)
{
    int active = -1;
    unsigned smallest = MAX_N + 1U;
    for (unsigned c = 0; c < search->constraint_count; ++c) {
        const unsigned count = bit_count(
            denominators & search->constraint[c].denominator_mask);
        if (count == 0U) continue;
        if (count < smallest ||
            (count == smallest &&
             (active < 0 || search->constraint[c].modulus >
                                  search->constraint[active].modulus))) {
            active = (int)c;
            smallest = count;
        }
    }
    if (active < 0) return first_index(denominators);

    const Constraint *constraint = &search->constraint[active];
    mask_t scan = denominators & constraint->denominator_mask;
    int best = 0;
    uint64_t best_gcd = UINT64_MAX;
    while (scan != 0U) {
        const int j = first_index(scan);
        scan &= scan - 1U;
        const uint64_t divisor = gcd_u64(
            constraint->coefficient[j], constraint->modulus);
        if (best == 0 || divisor < best_gcd ||
            (divisor == best_gcd && j < best)) {
            best = j;
            best_gcd = divisor;
        }
    }
    return best;
}

static void build_denominator_order(Search *search)
{
    mask_t denominators = full_mask(search->n);
    for (int depth = 0; depth <= search->n; ++depth) {
        search->remaining_denominators[depth] = denominators;
        search->remaining_gcd[depth] = exact_gcd(search, denominators);
        search->prime_divisor[depth] =
            prime_required_divisor(search, denominators);
        if (depth == search->n) break;
        const int denominator =
            choose_denominator(search, denominators);
        search->denominator_order[depth] = denominator;
        denominators ^= (mask_t)1U << (denominator - 1);
    }
}

/* At a fixed depth, the denominator mask is common to every branch.  Divide
 * the residual by the gcd of its remaining weights and pack it with the
 * numerator mask.  If an early residual is too wide, simply skip memoization
 * for that state; correctness never depends on caching. */
static bool make_memo_key(const Search *search, int depth,
                          mask_t numerators, uint64_t residual,
                          uint64_t *key)
{
    const uint64_t divisor = search->remaining_gcd[depth];
    if (divisor == 0U || residual % divisor != 0U) return false;
    const uint64_t quotient = residual / divisor;
    if (quotient > (UINT64_MAX >> search->n)) return false;
    *key = (quotient << search->n) | numerators;
    return *key != UINT64_MAX;
}

static uint64_t count_target(Search *search, int depth,
                             mask_t numerators, uint64_t residual)
{
    ++search->nodes;
    if (depth == search->n) return residual == 0U ? 1U : 0U;

    const mask_t denominators = search->remaining_denominators[depth];
    const uint64_t remaining_gcd = search->remaining_gcd[depth];
    if (residual % remaining_gcd != 0U ||
        residual % search->prime_divisor[depth] != 0U)
        return 0;

    uint64_t memo_key = 0;
    const bool memoizable =
        make_memo_key(search, depth, numerators, residual, &memo_key);
    uint64_t cached;
    if (memoizable && memo_get(search, memo_key, &cached)) return cached;

    uint64_t minimum, maximum;
    assignment_bounds(search, denominators, numerators, &minimum, &maximum);
    if (residual < minimum || residual > maximum ||
        !lattice_congruence(search, denominators, numerators, residual)) {
        if (memoizable) memo_put(search, memo_key, 0);
        return 0;
    }

    const int denominator = search->denominator_order[depth];
    const uint64_t next_gcd = search->remaining_gcd[depth + 1];
    const uint64_t next_prime_divisor = search->prime_divisor[depth + 1];
    uint64_t answer = 0;

    mask_t scan = numerators;
    while (scan != 0U) {
        const int value = first_index(scan);
        const mask_t value_bit = (mask_t)1U << (value - 1);
        scan &= scan - 1U;
        const uint64_t term =
            (uint64_t)value * search->weight[denominator];
        if (term > residual) continue;
        const uint64_t new_residual = residual - term;
        if ((next_gcd == 0U ? new_residual != 0U
                            : new_residual % next_gcd != 0U) ||
            new_residual % next_prime_divisor != 0U)
            continue;
        const uint64_t addend = count_target(
            search, depth + 1, numerators ^ value_bit, new_residual);
        if (UINT64_MAX - answer < addend)
            die("target count exceeds uint64_t");
        answer += addend;
    }

    if (memoizable) memo_put(search, memo_key, answer);
    return answer;
}

static void *target_worker_main(void *argument)
{
    TargetWorker *worker = argument;
    TargetPool *pool = worker->pool;
    Search search;
    search_init(&search, pool->n, pool->memory_per_worker);
    build_denominator_order(&search);

    for (;;) {
        const uint64_t target = atomic_fetch_add_explicit(
            &pool->next_target, 1U, memory_order_relaxed);
        if (target > pool->last_target) break;
        search.target = target * search.lcm;
        search.nodes = 0;
        search.memo_hits = 0;
        memo_init(&search, INITIAL_MEMO_CAPACITY);
        const uint64_t addend =
            count_target(&search, 0, full_mask(pool->n), search.target);
        if (UINT64_MAX - worker->answer < addend)
            die("worker target count exceeds uint64_t");
        worker->answer += addend;
        if (UINT64_MAX - worker->nodes < search.nodes ||
            UINT64_MAX - worker->memo_hits < search.memo_hits)
            die("worker statistics overflow");
        worker->nodes += search.nodes;
        worker->memo_hits += search.memo_hits;
        ++worker->targets;
        memo_clear(&search);
    }

    worker->peak_memo_states = search.peak_memo_states;
    worker->peak_memory = search.peak_memory;
    if (search.live_memory != 0U) die("memo memory accounting leak");
    return NULL;
}

static uint64_t compute_exact_term(int n, size_t memory_limit,
                                   unsigned requested_threads,
                                   Statistics *statistics)
{
    memset(statistics, 0, sizeof(*statistics));
    statistics->searched_n = n;
    if (n == 0) return 1;

    Search bounds_search;
    search_init(&bounds_search, n, memory_limit);
    const mask_t all = full_mask(n);
    uint64_t minimum, maximum;
    assignment_bounds(&bounds_search, all, all, &minimum, &maximum);
    const uint64_t first_target =
        minimum / bounds_search.lcm + (minimum % bounds_search.lcm != 0U);
    const uint64_t last_target = maximum / bounds_search.lcm;
    const uint64_t target_count = last_target - first_target + 1U;
    const unsigned thread_count = target_count < requested_threads
                                      ? (unsigned)target_count
                                      : requested_threads;
    TargetPool pool = {
        .n = n,
        .last_target = last_target,
        .memory_per_worker = memory_limit / thread_count
    };
    atomic_init(&pool.next_target, first_target);
    TargetWorker worker[MAX_THREADS];
    pthread_t thread[MAX_THREADS];
    memset(worker, 0, sizeof(worker));

    for (unsigned i = 0; i < thread_count; ++i) {
        worker[i].pool = &pool;
        if (pthread_create(&thread[i], NULL, target_worker_main,
                           &worker[i]) != 0)
            die("pthread_create failed");
    }
    for (unsigned i = 0; i < thread_count; ++i)
        if (pthread_join(thread[i], NULL) != 0)
            die("pthread_join failed");

    uint64_t answer = 0;
    for (unsigned i = 0; i < thread_count; ++i) {
        if (UINT64_MAX - answer < worker[i].answer ||
            UINT64_MAX - statistics->nodes < worker[i].nodes ||
            UINT64_MAX - statistics->memo_hits < worker[i].memo_hits)
            die("combined target statistics overflow");
        answer += worker[i].answer;
        statistics->nodes += worker[i].nodes;
        statistics->memo_hits += worker[i].memo_hits;
        statistics->targets += worker[i].targets;
        if (worker[i].peak_memo_states > statistics->peak_memo_states)
            statistics->peak_memo_states = worker[i].peak_memo_states;
        if (SIZE_MAX - statistics->peak_memory < worker[i].peak_memory)
            die("combined memory statistic overflow");
        statistics->peak_memory += worker[i].peak_memory;
    }
    return answer;
}

static uint64_t compute_output_term(int n, size_t memory_limit,
                                    unsigned threads,
                                    bool have_previous,
                                    uint64_t previous_value,
                                    Statistics *statistics)
{
    if (!is_prime((unsigned)n))
        return compute_exact_term(n, memory_limit, threads, statistics);
    uint64_t answer;
    if (have_previous) {
        memset(statistics, 0, sizeof(*statistics));
        statistics->searched_n = n - 1;
        answer = previous_value;
    } else {
        answer = compute_exact_term(n - 1, memory_limit, threads, statistics);
    }
    statistics->prime_step = true;
    return answer;
}

static void verify_known(uint64_t value, int n)
{
    if (n <= KNOWN_MAX_N && value != known[n]) {
        fprintf(stderr,
                "error: A073090 mismatch at n=%d: got %" PRIu64
                ", expected %" PRIu64 "\n",
                n, value, known[n]);
        exit(EXIT_FAILURE);
    }
}

static bool next_permutation(int values[MAX_N], int n)
{
    int i = n - 2;
    while (i >= 0 && values[i] >= values[i + 1]) --i;
    if (i < 0) return false;
    int j = n - 1;
    while (values[j] <= values[i]) --j;
    const int temporary = values[i];
    values[i] = values[j];
    values[j] = temporary;
    for (int left = i + 1, right = n - 1; left < right;
         ++left, --right) {
        const int swap = values[left];
        values[left] = values[right];
        values[right] = swap;
    }
    return true;
}

static uint64_t direct_term(int n)
{
    if (n == 0) return 1;
    const uint64_t lcm = make_lcm(n);
    int inverse[MAX_N];
    for (int i = 0; i < n; ++i) inverse[i] = i + 1;
    uint64_t answer = 0;
    do {
        uint64_t sum = 0;
        for (int j = 1; j <= n; ++j)
            sum += (uint64_t)inverse[j - 1] * (lcm / (uint64_t)j);
        if (sum % lcm == 0U) ++answer;
    } while (next_permutation(inverse, n));
    return answer;
}

static void release_bfile_lock(void)
{
    if (bfile_lock_descriptor < 0) return;
    const int saved_error = errno;
    const struct flock lock = {
        .l_type = F_UNLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    (void)fcntl(bfile_lock_descriptor, F_SETLK, &lock);
    (void)close(bfile_lock_descriptor);
    bfile_lock_descriptor = -1;
    errno = saved_error;
}

static void cleanup_bfile(void)
{
    const int saved_error = errno;
    if (bfile_temp_active) {
        (void)unlink(bfile_temp_name);
        bfile_temp_active = false;
    }
    release_bfile_lock();
    errno = saved_error;
}

static FILE *open_bfile(void)
{
    bfile_lock_descriptor = open(BFILE_LOCK_NAME, O_CREAT | O_RDWR, 0666);
    if (bfile_lock_descriptor < 0) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                BFILE_LOCK_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    const struct flock lock = {
        .l_type = F_WRLCK, .l_whence = SEEK_SET, .l_start = 0, .l_len = 0
    };
    if (fcntl(bfile_lock_descriptor, F_SETLK, &lock) != 0) {
        const int saved_error = errno;
        cleanup_bfile();
        if (saved_error == EACCES || saved_error == EAGAIN)
            fprintf(stderr, "error: another writer holds %s\n",
                    BFILE_LOCK_NAME);
        else
            fprintf(stderr, "error: cannot lock %s: %s\n",
                    BFILE_LOCK_NAME, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    const int descriptor = mkstemp(bfile_temp_name);
    if (descriptor < 0) {
        const int saved_error = errno;
        cleanup_bfile();
        fprintf(stderr, "error: cannot create %s: %s\n",
                BFILE_TEMP_TEMPLATE, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    bfile_temp_active = true;

    struct stat existing;
    mode_t mode;
    if (stat(BFILE_NAME, &existing) == 0) {
        mode = existing.st_mode & 0777;
    } else if (errno == ENOENT) {
        const mode_t mask = umask(0);
        (void)umask(mask);
        mode = 0666 & ~mask;
    } else {
        const int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot inspect %s: %s\n",
                BFILE_NAME, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    if (fchmod(descriptor, mode) != 0) {
        const int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot set permissions: %s\n",
                strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    FILE *stream = fdopen(descriptor, "w");
    if (stream == NULL) {
        const int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot open temporary b-file: %s\n",
                strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    return stream;
}

static void write_bfile_term(FILE *stream, int n, uint64_t value)
{
    if (fprintf(stream, "%d %" PRIu64 "\n", n, value) < 0 ||
        fflush(stream) != 0) {
        const int saved_error = errno == 0 ? EIO : errno;
        (void)fclose(stream);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot write b-file: %s\n",
                strerror(saved_error));
        exit(EXIT_FAILURE);
    }
}

static void finish_bfile(FILE *stream)
{
    int saved_error = 0;
    bool failed = false;
    if (fflush(stream) != 0) {
        failed = true;
        saved_error = errno;
    }
    const int descriptor = fileno(stream);
    if (descriptor < 0) {
        failed = true;
        if (saved_error == 0) saved_error = errno;
    } else if (!failed && fsync(descriptor) != 0) {
        failed = true;
        saved_error = errno;
    }
    if (fclose(stream) != 0) {
        failed = true;
        if (saved_error == 0) saved_error = errno;
    }
    if (failed) {
        if (saved_error == 0) saved_error = EIO;
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot finalize b-file: %s\n",
                strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    if (rename(bfile_temp_name, BFILE_NAME) != 0) {
        const int rename_error = errno;
        errno = rename_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot replace %s: %s\n",
                BFILE_NAME, strerror(rename_error));
        exit(EXIT_FAILURE);
    }
    bfile_temp_active = false;
    release_bfile_lock();
}

static void usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "Usage: %s [--upto N | --term N | --check] "
            "[--threads T] [--memory-mb MiB] [--verbose]\n",
            program);
}

int main(int argc, char **argv)
{
    OutputMode mode = MODE_UPTO;
    int limit = DEFAULT_MAX_N;
    unsigned memory_mb = DEFAULT_MEMORY_MB;
    unsigned threads = DEFAULT_THREADS;
    bool check = false;
    bool verbose = false;
    bool mode_seen = false;
    if (atexit(cleanup_bfile) != 0) die("could not register cleanup");

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--upto") == 0 ||
            strcmp(argv[i], "--term") == 0) {
            if (mode_seen || i + 1 == argc) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(argv[i], "--term") == 0 ? MODE_TERM : MODE_UPTO;
            limit = parse_n(argv[++i]);
            mode_seen = true;
        } else if (strcmp(argv[i], "--check") == 0) {
            if (mode_seen) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            check = true;
            mode = MODE_UPTO;
            limit = KNOWN_MAX_N;
            mode_seen = true;
        } else if (strcmp(argv[i], "--memory-mb") == 0) {
            if (i + 1 == argc) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            memory_mb = parse_memory_mb(argv[++i]);
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (i + 1 == argc) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            threads = parse_threads(argv[++i]);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }

    const size_t memory_limit = (size_t)memory_mb * 1024U * 1024U;
    FILE *bfile = mode == MODE_UPTO && !check ? open_bfile() : NULL;
    const int first = mode == MODE_TERM ? limit : 0;
    bool have_previous = false;
    uint64_t previous_value = 0;
    for (int n = first; n <= limit; ++n) {
        Statistics statistics;
        const double started = monotonic_seconds();
        const uint64_t value = compute_output_term(
            n, memory_limit, threads,
            mode == MODE_UPTO && have_previous,
            previous_value, &statistics);
        verify_known(value, n);
        if (check && n <= DIRECT_CHECK_MAX_N) {
            const uint64_t direct = direct_term(n);
            if (direct != value) {
                fprintf(stderr,
                        "error: direct mismatch at n=%d: exact-target=%" PRIu64
                        ", direct=%" PRIu64 "\n",
                        n, value, direct);
                return EXIT_FAILURE;
            }
        }
        if (bfile != NULL) write_bfile_term(bfile, n, value);
        previous_value = value;
        have_previous = true;

        if (mode == MODE_TERM)
            printf("%" PRIu64 "\n", value);
        else
            printf("%s%" PRIu64, n == 0 ? "" : ", ", value);
        if (fflush(stdout) != 0) die("could not flush stdout");
        if (verbose)
            fprintf(stderr,
                    "073090_02: n=%d, a(n)=%" PRIu64
                    ", method=%s, searched_n=%d, targets=%u, nodes=%" PRIu64
                    ", memo_hits=%" PRIu64 ", peak_memo=%zu, "
                    "threads=%u, peak_memory_bound=%.1f MiB, %.3f s\n",
                    n, value,
                    statistics.prime_step ? "prime-recurrence"
                                          : "exact-target-recursion",
                    statistics.searched_n, statistics.targets,
                    statistics.nodes, statistics.memo_hits,
                    statistics.peak_memo_states,
                    threads,
                    (double)statistics.peak_memory / 1048576.0,
                    monotonic_seconds() - started);
    }
    if (mode == MODE_UPTO) {
        if (putchar('\n') == EOF || fflush(stdout) != 0)
            die("could not finish stdout");
    }
    if (bfile != NULL) finish_bfile(bfile);
    if (check)
        fprintf(stderr,
                "check passed: A073090(0..%d), direct permutations "
                "through n=%d\n",
                KNOWN_MAX_N, DIRECT_CHECK_MAX_N);
    return EXIT_SUCCESS;
}
