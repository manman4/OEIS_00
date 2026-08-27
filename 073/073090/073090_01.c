/*
 * A073090 -- number of permutations p of [n] for which
 *
 *                   sum_{k=1..n} k / p(k)
 *
 * is an integer.
 *
 * Put q=p^{-1} and L=lcm(1,...,n).  Since p -> q is a bijection, we count
 * inverse permutations satisfying
 *
 *                   sum_{j=1..n} q(j)*(L/j) == 0 (mod L).          (1)
 *
 * If P_r is the largest power of the prime r not exceeding n, the Chinese
 * remainder theorem turns (1) into the independent exact constraints
 *
 *          sum_{r divides j} q(j)*(L/j) == 0 (mod P_r).            (2)
 *
 * Terms with r not dividing j vanish modulo P_r.  Denominators are ordered
 * so that the smallest unfinished constraint is completed first.  A sparse
 * dynamic program then stores
 *
 *              (unused numerator mask, partial sum modulo L) -> count.
 *
 * Equal states produced by different partial permutations are merged.  Only
 * the current and next DP layers are retained.  If the gcd of a constraint's
 * remaining coefficients and its modulus is g, every future contribution is
 * divisible by g; a partial sum not divisible by g is therefore discarded
 * immediately.  This includes the full test (2) when no coefficient remains.
 *
 * If n is prime, (2) for r=n forces q(n)=n, and therefore
 *
 *                         a(n) = a(n-1).                           (3)
 *
 * Known OEIS terms are used only for verification, never as computed output.
 * All additions are checked; the program stops rather than wrap a count or
 * exceed its configured memory limit.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic 073090_01.c \
 *       -o 073090_01
 *
 * Usage:
 *   ./073090_01
 *   ./073090_01 --upto 26 --verbose
 *   ./073090_01 --term 26 --memory-mb 2048 --verbose
 *   ./073090_01 --check
 *
 * The default and --upto atomically replace b073090_01.txt while holding an
 * exclusive writer lock.  --term and --check do not modify the b-file.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_N 26
#define DEFAULT_MAX_N 25
#define KNOWN_MAX_N 17
#define DIRECT_CHECK_MAX_N 10
#define MAX_CONSTRAINTS 9
#define DEFAULT_MEMORY_MB 2048
#define MIN_MEMORY_MB 32
#define MAX_MEMORY_MB 65536
#define INITIAL_TABLE_CAPACITY 1024
#define BFILE_NAME "b073090_01.txt"
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
} Constraint;

/* A 16-byte entry keeps the n=25 peak memory practical. */
typedef struct {
    uint64_t key_plus_one;
    uint64_t count;
} StateEntry;

typedef struct {
    StateEntry *entry;
    size_t capacity;
    size_t used;
} StateTable;

typedef struct {
    int n;
    uint64_t lcm;
    uint64_t weight[MAX_N + 1];
    Constraint constraint[MAX_CONSTRAINTS];
    unsigned constraint_count;

    size_t memory_limit;
    size_t live_memory;
    size_t peak_memory;
    size_t peak_states;
    uint64_t transitions;
} Search;

typedef struct {
    size_t peak_states;
    size_t peak_memory;
    uint64_t transitions;
    int searched_n;
    bool prime_step;
} Statistics;

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
    while (b != 0) {
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

static uint64_t hash_u64(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static void memory_error(const Search *search, size_t requested)
{
    const double live_mib = (double)search->live_memory / 1048576.0;
    const double request_mib = (double)requested / 1048576.0;
    const double limit_mib = (double)search->memory_limit / 1048576.0;
    fprintf(stderr,
            "error: DP memory limit exceeded at n=%d "
            "(live %.1f MiB + request %.1f MiB > limit %.1f MiB); "
            "peak_states=%zu, transitions=%" PRIu64 "; "
            "increase --memory-mb\n",
            search->n, live_mib, request_mib, limit_mib,
            search->peak_states, search->transitions);
    exit(EXIT_FAILURE);
}

static StateEntry *allocate_entries(Search *search, size_t capacity)
{
    if (capacity > SIZE_MAX / sizeof(StateEntry))
        die("state-table allocation size overflow");
    const size_t bytes = capacity * sizeof(StateEntry);
    if (bytes > search->memory_limit - search->live_memory)
        memory_error(search, bytes);
    StateEntry *result = calloc(capacity, sizeof(*result));
    if (result == NULL) {
        fprintf(stderr,
                "error: could not allocate %.1f MiB for the DP at n=%d\n",
                (double)bytes / 1048576.0, search->n);
        exit(EXIT_FAILURE);
    }
    search->live_memory += bytes;
    if (search->live_memory > search->peak_memory)
        search->peak_memory = search->live_memory;
    return result;
}

static void free_table(Search *search, StateTable *table)
{
    if (table->entry == NULL) return;
    const size_t bytes = table->capacity * sizeof(*table->entry);
    if (bytes > search->live_memory) die("internal memory accounting error");
    free(table->entry);
    search->live_memory -= bytes;
    memset(table, 0, sizeof(*table));
}

static void table_init(Search *search, StateTable *table, size_t hint)
{
    size_t capacity = INITIAL_TABLE_CAPACITY;
    while (capacity < hint) {
        if (capacity > SIZE_MAX / 2U)
            die("state-table capacity overflow");
        capacity *= 2U;
    }
    table->entry = allocate_entries(search, capacity);
    table->capacity = capacity;
    table->used = 0;
}

static void table_grow(Search *search, StateTable *table)
{
    if (table->capacity > SIZE_MAX / 2U)
        die("state-table capacity overflow");
    StateTable larger;
    table_init(search, &larger, table->capacity * 2U);

    for (size_t i = 0; i < table->capacity; ++i) {
        const StateEntry item = table->entry[i];
        if (item.key_plus_one == 0U) continue;
        size_t slot = (size_t)hash_u64(item.key_plus_one - 1U) &
                      (larger.capacity - 1U);
        while (larger.entry[slot].key_plus_one != 0U)
            slot = (slot + 1U) & (larger.capacity - 1U);
        larger.entry[slot] = item;
        ++larger.used;
    }

    free_table(search, table);
    *table = larger;
}

static void table_add(Search *search, StateTable *table,
                      uint64_t key, uint64_t addend)
{
    const uint64_t stored_key = key + 1U;
    if (stored_key == 0U) die("state key overflow");

    for (;;) {
        size_t slot = (size_t)hash_u64(key) & (table->capacity - 1U);
        while (table->entry[slot].key_plus_one != 0U &&
               table->entry[slot].key_plus_one != stored_key)
            slot = (slot + 1U) & (table->capacity - 1U);

        StateEntry *entry = &table->entry[slot];
        if (entry->key_plus_one == stored_key) {
            if (UINT64_MAX - entry->count < addend)
                die("per-state count exceeds uint64_t");
            entry->count += addend;
            return;
        }

        /* Grow only for a genuinely new state. */
        if ((table->used + 1U) * 10U >= table->capacity * 7U) {
            table_grow(search, table);
            continue;
        }
        entry->key_plus_one = stored_key;
        entry->count = addend;
        ++table->used;
        if (table->used > search->peak_states)
            search->peak_states = table->used;
        return;
    }
}

static bool table_get(const StateTable *table, uint64_t key,
                      uint64_t *count)
{
    const uint64_t stored_key = key + 1U;
    size_t slot = (size_t)hash_u64(key) & (table->capacity - 1U);
    while (table->entry[slot].key_plus_one != 0U) {
        if (table->entry[slot].key_plus_one == stored_key) {
            *count = table->entry[slot].count;
            return true;
        }
        slot = (slot + 1U) & (table->capacity - 1U);
    }
    return false;
}

static void search_init(Search *search, int n, size_t memory_limit)
{
    memset(search, 0, sizeof(*search));
    search->n = n;
    search->memory_limit = memory_limit;
    search->lcm = make_lcm(n);

    if (search->lcm > (UINT64_MAX >> n))
        die("packed DP state would overflow uint64_t");
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
        for (int j = (int)prime; j <= n; j += (int)prime)
            constraint->denominator_mask |=
                (mask_t)1U << (j - 1);
    }
}

/*
 * Pick an order depending only on the unassigned denominator set.  A last
 * variable is preferred.  Otherwise work on the smallest constraint.  A
 * coefficient having a small gcd with the modulus is assigned first: removing
 * it can increase the gcd of all remaining coefficients and expose a
 * necessary congruence earlier.
 */
static int choose_denominator(const Search *search, mask_t denominators)
{
    mask_t forced = 0;
    int active_constraint = -1;
    unsigned smallest_group = MAX_N + 1U;

    for (unsigned c = 0; c < search->constraint_count; ++c) {
        const mask_t remaining =
            denominators & search->constraint[c].denominator_mask;
        const unsigned count = bit_count(remaining);
        if (count == 1U) {
            forced |= remaining;
        } else if (count > 1U &&
                   (count < smallest_group ||
                    (count == smallest_group &&
                     (active_constraint < 0 ||
                      search->constraint[c].modulus >
                          search->constraint[active_constraint].modulus)))) {
            smallest_group = count;
            active_constraint = (int)c;
        }
    }

    if (forced != 0U) return first_index(forced);
    if (active_constraint >= 0) {
        const Constraint *constraint =
            &search->constraint[active_constraint];
        mask_t scan = denominators & constraint->denominator_mask;
        int best_denominator = 0;
        uint64_t best_gcd = UINT64_MAX;
        while (scan != 0U) {
            const int denominator = first_index(scan);
            scan &= scan - 1U;
            const uint64_t divisor =
                gcd_u64(search->weight[denominator] % constraint->modulus,
                        constraint->modulus);
            if (best_denominator == 0 || divisor < best_gcd ||
                (divisor == best_gcd && denominator < best_denominator)) {
                best_denominator = denominator;
                best_gcd = divisor;
            }
        }
        return best_denominator;
    }
    return first_index(denominators);
}

static void make_denominator_order(const Search *search,
                                   int order[MAX_N],
                                   uint64_t required_modulus[MAX_N],
                                   uint64_t filter_gain[MAX_N])
{
    mask_t denominators = full_mask(search->n);
    uint64_t previous_required = 1;
    for (int depth = 0; depth < search->n; ++depth) {
        const int denominator =
            choose_denominator(search, denominators);
        order[depth] = denominator;
        const mask_t bit = (mask_t)1U << (denominator - 1);
        const mask_t remaining_denominators = denominators ^ bit;
        required_modulus[depth] = 1;

        for (unsigned c = 0; c < search->constraint_count; ++c) {
            const Constraint *constraint = &search->constraint[c];
            mask_t scan = remaining_denominators &
                          constraint->denominator_mask;
            uint64_t divisor = constraint->modulus;
            while (scan != 0U) {
                const int j = first_index(scan);
                scan &= scan - 1U;
                divisor = gcd_u64(
                    divisor,
                    search->weight[j] % constraint->modulus);
            }
            if (required_modulus[depth] >
                UINT64_MAX / divisor)
                die("required modulus overflow");
            required_modulus[depth] *= divisor;
        }
        if (search->lcm % required_modulus[depth] != 0U)
            die("internal nondividing constraint modulus");
        if (required_modulus[depth] % previous_required != 0U)
            die("internal nonmonotone constraint modulus");
        filter_gain[depth] =
            required_modulus[depth] / previous_required;
        previous_required = required_modulus[depth];
        denominators = remaining_denominators;
    }
}

static size_t next_table_hint(size_t states, uint64_t required_modulus)
{
    if (states > (SIZE_MAX - INITIAL_TABLE_CAPACITY) / 2U)
        die("state-table hint overflow");
    return states * 2U / required_modulus + INITIAL_TABLE_CAPACITY;
}

static uint64_t compute_exact_term(int n, size_t memory_limit,
                                   Statistics *statistics)
{
    memset(statistics, 0, sizeof(*statistics));
    statistics->searched_n = n;
    if (n == 0) return 1;

    Search search;
    search_init(&search, n, memory_limit);
    int order[MAX_N];
    uint64_t required_modulus[MAX_N];
    uint64_t filter_gain[MAX_N];
    make_denominator_order(&search, order, required_modulus, filter_gain);

    const mask_t all = full_mask(n);
    StateTable current;
    table_init(&search, &current, INITIAL_TABLE_CAPACITY);
    table_add(&search, &current, all, 1);

    for (int depth = 0; depth < n; ++depth) {
        StateTable next;
        table_init(&search, &next,
                   next_table_hint(current.used,
                                   filter_gain[depth]));
        const int denominator = order[depth];
        const uint64_t mask64 = (UINT64_C(1) << n) - 1U;

        for (size_t slot = 0; slot < current.capacity; ++slot) {
            const StateEntry item = current.entry[slot];
            if (item.key_plus_one == 0U) continue;
            const uint64_t key = item.key_plus_one - 1U;
            const mask_t numerators = (mask_t)(key & mask64);
            const uint64_t residue = key >> n;
            mask_t scan = numerators;

            while (scan != 0U) {
                const int value = first_index(scan);
                const mask_t value_bit = (mask_t)1U << (value - 1);
                scan ^= value_bit;
                if (search.transitions == UINT64_MAX)
                    die("transition counter overflow");
                ++search.transitions;

                const uint64_t contribution =
                    (uint64_t)value * search.weight[denominator];
                const uint64_t new_residue =
                    (residue + contribution) % search.lcm;
                if (new_residue % required_modulus[depth] != 0U)
                    continue;
                const uint64_t new_key =
                    (new_residue << n) | (numerators ^ value_bit);
                table_add(&search, &next, new_key, item.count);
            }
        }

        free_table(&search, &current);
        current = next;
    }

    uint64_t answer = 0;
    if (current.used != 1U || !table_get(&current, 0, &answer))
        die("final DP state is inconsistent");
    free_table(&search, &current);
    if (search.live_memory != 0U)
        die("internal DP memory leak accounting error");

    statistics->peak_states = search.peak_states;
    statistics->peak_memory = search.peak_memory;
    statistics->transitions = search.transitions;
    return answer;
}

static uint64_t compute_output_term(int n, size_t memory_limit,
                                    bool have_previous,
                                    uint64_t previous_value,
                                    Statistics *statistics)
{
    if (!is_prime((unsigned)n))
        return compute_exact_term(n, memory_limit, statistics);

    uint64_t answer;
    if (have_previous) {
        memset(statistics, 0, sizeof(*statistics));
        statistics->searched_n = n - 1;
        answer = previous_value;
    } else {
        answer = compute_exact_term(n - 1, memory_limit, statistics);
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

/* Definition-level reference used only by --check. */
static uint64_t direct_term(int n)
{
    if (n == 0) return 1;
    const uint64_t lcm = make_lcm(n);
    int inverse[MAX_N];
    for (int i = 0; i < n; ++i) inverse[i] = i + 1;

    uint64_t answer = 0;
    do {
        uint64_t scaled_sum = 0;
        for (int j = 1; j <= n; ++j)
            scaled_sum +=
                (uint64_t)inverse[j - 1] * (lcm / (uint64_t)j);
        if (scaled_sum % lcm == 0U) {
            if (answer == UINT64_MAX) die("direct count overflow");
            ++answer;
        }
    } while (next_permutation(inverse, n));
    return answer;
}

static void release_bfile_lock(void)
{
    if (bfile_lock_descriptor < 0) return;
    const int saved_error = errno;
    const struct flock lock = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
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
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
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
        fprintf(stderr, "error: cannot set permissions on %s: %s\n",
                bfile_temp_name, strerror(saved_error));
        exit(EXIT_FAILURE);
    }

    FILE *stream = fdopen(descriptor, "w");
    if (stream == NULL) {
        const int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot open %s: %s\n",
                bfile_temp_name, strerror(saved_error));
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
        fprintf(stderr, "error: cannot write %s: %s\n",
                bfile_temp_name, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
}

static void finish_bfile(FILE *stream)
{
    bool failed = false;
    int saved_error = 0;
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
        fprintf(stderr, "error: cannot finalize %s: %s\n",
                bfile_temp_name, strerror(saved_error));
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
            "[--memory-mb MiB] [--verbose]\n",
            program);
}

int main(int argc, char **argv)
{
    OutputMode mode = MODE_UPTO;
    int limit = DEFAULT_MAX_N;
    unsigned memory_mb = DEFAULT_MEMORY_MB;
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
            n, memory_limit, mode == MODE_UPTO && have_previous,
            previous_value, &statistics);
        verify_known(value, n);

        if (check && n <= DIRECT_CHECK_MAX_N) {
            const uint64_t direct = direct_term(n);
            if (direct != value) {
                fprintf(stderr,
                        "error: direct permutation mismatch at n=%d: "
                        "DP=%" PRIu64 ", direct=%" PRIu64 "\n",
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

        if (verbose) {
            const char *method = statistics.prime_step
                                     ? "prime-recurrence"
                                     : "sparse-dp";
            fprintf(stderr,
                    "073090_01: n=%d, a(n)=%" PRIu64
                    ", method=%s, searched_n=%d, peak_states=%zu, "
                    "transitions=%" PRIu64 ", peak_memory=%.1f MiB, "
                    "%.3f s\n",
                    n, value, method, statistics.searched_n,
                    statistics.peak_states, statistics.transitions,
                    (double)statistics.peak_memory / 1048576.0,
                    monotonic_seconds() - started);
        }
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
