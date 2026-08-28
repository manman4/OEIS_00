/*
 * A022825 -- direct count of remainder-increasing permutations.
 *
 * A022825(N) is computed here as the number of permutations sigma of
 * [N-1] such that
 *
 *   sigma(k) mod sigma(k+1) < sigma(k+1) mod sigma(k+2)
 *
 * for every applicable k.  Thus sequence index N corresponds to permutation
 * size n=N-1.
 *
 * This implementation does not use the defining recurrence of A022825, the
 * divisor-chain bijection, or stored sequence terms as answers.  It constructs
 * permutations from left to right.  A memoized state consists exactly of
 *
 *   (unused values, last value, previous remainder).
 *
 * A transition appends y and is allowed precisely when
 *
 *   previous remainder < last mod y.
 *
 * If m values remain, their m future remainders must be distinct integers in
 * {previous+1,...,n-1}.  The necessary test m <= n-1-previous is therefore a
 * safe direct pruning rule.  It changes only running time, not the count.
 *
 * Known terms are verification data only and are never returned as computed
 * answers.  Count additions, diagnostic counters, packed keys, shifts,
 * allocation sizes, and the configured memory limit are checked.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -Werror \
 *       022825_01.c -o 022825_01
 *
 * Usage:
 *   ./022825_01
 *   ./022825_01 --upto 25 --verbose
 *   ./022825_01 --term 25 --memory-mb 1024 --verbose
 *   ./022825_01 --check
 *
 * The default and --upto modes print completed terms and atomically replace
 * b022825_01.txt.  --term and --check do not modify the b-file.
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

#define MAX_PERM_N 32
#define MAX_TERM_N (MAX_PERM_N + 1)
#define DEFAULT_UPTO 25
#define KNOWN_MAX_N 33
#define DIRECT_CHECK_MAX_PERM_N 10
#define DEFAULT_MEMORY_MB 1024
#define MIN_MEMORY_MB 16
#define MAX_MEMORY_MB 65536
#define INITIAL_CAPACITY 1024
#define LOAD_NUMERATOR 7
#define LOAD_DENOMINATOR 10
#define KEY_AUX_BITS 10
#define BFILE_NAME "b022825_01.txt"
#define BFILE_TEMP_TEMPLATE BFILE_NAME ".tmp.XXXXXX"
#define BFILE_LOCK_NAME BFILE_NAME ".lock"

typedef uint64_t mask_t;

_Static_assert(MAX_PERM_N == 32,
               "the packed key reserves 32 bits for the unused mask");
_Static_assert(KEY_AUX_BITS == 2 * 5,
               "the packed key reserves five bits per auxiliary value");
_Static_assert(MAX_PERM_N + KEY_AUX_BITS < 63,
               "packed keys and their nonzero sentinel must fit uint64_t");

static const uint64_t known[KNOWN_MAX_N + 1] = {
    UINT64_C(0),  UINT64_C(1),  UINT64_C(1),  UINT64_C(2),
    UINT64_C(3),  UINT64_C(4),  UINT64_C(6),  UINT64_C(7),
    UINT64_C(9),  UINT64_C(11), UINT64_C(13), UINT64_C(14),
    UINT64_C(19), UINT64_C(20), UINT64_C(22), UINT64_C(25),
    UINT64_C(29), UINT64_C(30), UINT64_C(36), UINT64_C(37),
    UINT64_C(42), UINT64_C(45), UINT64_C(47), UINT64_C(48),
    UINT64_C(60), UINT64_C(62), UINT64_C(64), UINT64_C(68),
    UINT64_C(73), UINT64_C(74), UINT64_C(84), UINT64_C(85),
    UINT64_C(93), UINT64_C(96)
};

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
    uint8_t remainder[MAX_PERM_N + 1][MAX_PERM_N + 1];
    MemoTable memo;
    size_t memory_limit;
    size_t live_memory;
    size_t peak_memory;
    size_t peak_states;
    uint64_t calls;
    uint64_t memo_hits;
    uint64_t transitions;
} Search;

typedef struct {
    size_t states;
    size_t peak_memory;
    uint64_t calls;
    uint64_t memo_hits;
    uint64_t transitions;
    double seconds;
} Statistics;

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} OutputMode;

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

static int parse_integer(const char *text, const char *label,
                         int minimum, int maximum)
{
    errno = 0;
    char *end = NULL;
    intmax_t value = strtoimax(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static void checked_increment(uint64_t *value, const char *label)
{
    if (*value == UINT64_MAX) {
        fprintf(stderr, "error: %s counter overflow\n", label);
        exit(EXIT_FAILURE);
    }
    ++*value;
}

static uint64_t checked_add(uint64_t left, uint64_t right)
{
    if (right > UINT64_MAX - left)
        die("count exceeds uint64_t");
    return left + right;
}

static uint64_t hash_u64(uint64_t value)
{
    /* Shift/XOR mixing avoids even intentional unsigned wraparound. */
    value ^= value >> 31;
    value ^= value << 11;
    value ^= value >> 17;
    value ^= value << 23;
    return value ^ (value >> 13);
}

static unsigned popcount_mask(mask_t mask)
{
#if defined(__clang__) || defined(__GNUC__)
    return (unsigned)__builtin_popcountll(mask);
#else
    unsigned count = 0;
    while (mask != 0U) {
        mask &= mask - 1U;
        ++count;
    }
    return count;
#endif
}

static int first_value(mask_t mask)
{
    if (mask == 0U)
        die("attempted to extract a value from an empty mask");
#if defined(__clang__) || defined(__GNUC__)
    return (int)__builtin_ctzll(mask) + 1;
#else
    int value = 1;
    while ((mask & 1U) == 0U) {
        mask >>= 1U;
        ++value;
    }
    return value;
#endif
}

static mask_t value_bit(int value)
{
    if (value < 1 || value > MAX_PERM_N)
        die("value-bit index out of range");
    return UINT64_C(1) << (unsigned)(value - 1);
}

static mask_t full_mask(int n)
{
    if (n < 0 || n > MAX_PERM_N)
        die("full-mask size out of range");
    return n == 0 ? UINT64_C(0) :
        (UINT64_C(1) << (unsigned)n) - UINT64_C(1);
}

static uint64_t pack_key(mask_t unused, int last, unsigned previous)
{
    if (unused >= (UINT64_C(1) << MAX_PERM_N) ||
        last < 1 || last > MAX_PERM_N || previous >= MAX_PERM_N)
        die("memo key component out of range");
    return (unused << KEY_AUX_BITS) |
           ((uint64_t)(unsigned)(last - 1) << 5) |
           (uint64_t)previous;
}

static void memory_error(const Search *search, size_t requested)
{
    fprintf(stderr,
            "error: memo memory limit exceeded for permutation n=%d "
            "(live %.1f MiB + request %.1f MiB > limit %.1f MiB); "
            "states=%zu, transitions=%" PRIu64 "; increase --memory-mb\n",
            search->n, (double)search->live_memory / 1048576.0,
            (double)requested / 1048576.0,
            (double)search->memory_limit / 1048576.0,
            search->memo.used, search->transitions);
    exit(EXIT_FAILURE);
}

static MemoEntry *allocate_entries(Search *search, size_t capacity)
{
    if (capacity == 0 || capacity > SIZE_MAX / sizeof(MemoEntry))
        die("memo allocation size overflow");
    const size_t bytes = capacity * sizeof(MemoEntry);
    if (search->live_memory > search->memory_limit ||
        bytes > search->memory_limit - search->live_memory)
        memory_error(search, bytes);
    MemoEntry *entry = calloc(capacity, sizeof(*entry));
    if (entry == NULL)
        die("cannot allocate memo table");
    search->live_memory += bytes;
    if (search->live_memory > search->peak_memory)
        search->peak_memory = search->live_memory;
    return entry;
}

static void insert_without_growth(MemoEntry *entry, size_t capacity,
                                  uint64_t key_plus_one, uint64_t count)
{
    size_t slot = (size_t)hash_u64(key_plus_one - 1U) & (capacity - 1U);
    while (entry[slot].key_plus_one != 0U)
        slot = (slot + 1U) & (capacity - 1U);
    entry[slot].key_plus_one = key_plus_one;
    entry[slot].count = count;
}

static void grow_memo(Search *search)
{
    const size_t old_capacity = search->memo.capacity;
    const size_t new_capacity = old_capacity == 0 ? INITIAL_CAPACITY :
        old_capacity * 2U;
    if (new_capacity < old_capacity ||
        (new_capacity & (new_capacity - 1U)) != 0U)
        die("memo capacity overflow");

    MemoEntry *new_entry = allocate_entries(search, new_capacity);
    for (size_t i = 0; i < old_capacity; ++i) {
        const MemoEntry old = search->memo.entry[i];
        if (old.key_plus_one != 0U)
            insert_without_growth(new_entry, new_capacity,
                                  old.key_plus_one, old.count);
    }

    if (old_capacity != 0) {
        const size_t old_bytes = old_capacity * sizeof(MemoEntry);
        free(search->memo.entry);
        search->live_memory -= old_bytes;
    }
    search->memo.entry = new_entry;
    search->memo.capacity = new_capacity;
}

static bool memo_lookup(Search *search, uint64_t key, uint64_t *count)
{
    if (search->memo.capacity == 0)
        return false;
    const uint64_t stored_key = key + 1U;
    size_t slot = (size_t)hash_u64(key) & (search->memo.capacity - 1U);
    for (;;) {
        const MemoEntry entry = search->memo.entry[slot];
        if (entry.key_plus_one == 0U)
            return false;
        if (entry.key_plus_one == stored_key) {
            *count = entry.count;
            checked_increment(&search->memo_hits, "memo-hit");
            return true;
        }
        slot = (slot + 1U) & (search->memo.capacity - 1U);
    }
}

static void memo_insert(Search *search, uint64_t key, uint64_t count)
{
    if (search->memo.capacity == 0 ||
        search->memo.used + 1U >
            search->memo.capacity * LOAD_NUMERATOR / LOAD_DENOMINATOR)
        grow_memo(search);

    insert_without_growth(search->memo.entry, search->memo.capacity,
                          key + 1U, count);
    ++search->memo.used;
    if (search->memo.used > search->peak_states)
        search->peak_states = search->memo.used;
}

static uint64_t count_suffix(Search *search, mask_t unused,
                             int last, unsigned previous)
{
    checked_increment(&search->calls, "recursive-call");
    if (unused == 0U)
        return UINT64_C(1);
    if (previous >= (unsigned)search->n)
        die("remainder invariant violated");

    const unsigned remaining = popcount_mask(unused);
    if (remaining > (unsigned)(search->n - 1) - previous)
        return UINT64_C(0);

    const uint64_t key = pack_key(unused, last, previous);
    uint64_t cached = 0;
    if (memo_lookup(search, key, &cached))
        return cached;

    uint64_t total = 0;
    mask_t candidates = unused;
    while (candidates != 0U) {
        const mask_t bit = candidates & (~candidates + UINT64_C(1));
        candidates ^= bit;
        const int next = first_value(bit);
        checked_increment(&search->transitions, "transition");
        const unsigned next_remainder = search->remainder[last][next];
        if (next_remainder <= previous)
            continue;
        if (remaining - 1U >
            (unsigned)(search->n - 1) - next_remainder)
            continue;
        const uint64_t child = count_suffix(search, unused ^ bit,
                                            next, next_remainder);
        total = checked_add(total, child);
    }

    memo_insert(search, key, total);
    return total;
}

static uint64_t count_permutations(int n, size_t memory_limit,
                                   Statistics *statistics)
{
    memset(statistics, 0, sizeof(*statistics));
    const double start = monotonic_seconds();
    if (n <= 1) {
        statistics->seconds = monotonic_seconds() - start;
        return UINT64_C(1);
    }

    Search search;
    memset(&search, 0, sizeof(search));
    search.n = n;
    search.memory_limit = memory_limit;
    for (int left = 1; left <= n; ++left)
        for (int right = 1; right <= n; ++right)
            search.remainder[left][right] =
                (uint8_t)((unsigned)left % (unsigned)right);

    const mask_t all = full_mask(n);
    const unsigned after_second = (unsigned)(n - 2);
    uint64_t total = 0;
    for (int first = 1; first <= n; ++first) {
        const mask_t without_first = all ^ value_bit(first);
        mask_t seconds = without_first;
        while (seconds != 0U) {
            const mask_t bit = seconds & (~seconds + UINT64_C(1));
            seconds ^= bit;
            const int second = first_value(bit);
            checked_increment(&search.transitions, "root-transition");
            const unsigned first_remainder = search.remainder[first][second];
            if (after_second >
                (unsigned)(n - 1) - first_remainder)
                continue;
            const uint64_t child = count_suffix(
                &search, without_first ^ bit, second, first_remainder);
            total = checked_add(total, child);
        }
    }

    statistics->states = search.peak_states;
    statistics->peak_memory = search.peak_memory;
    statistics->calls = search.calls;
    statistics->memo_hits = search.memo_hits;
    statistics->transitions = search.transitions;
    statistics->seconds = monotonic_seconds() - start;
    free(search.memo.entry);
    return total;
}

typedef struct {
    int n;
    int permutation[MAX_PERM_N];
    mask_t used;
    uint64_t count;
} BruteSearch;

static void brute_visit(BruteSearch *search, int depth)
{
    if (depth == search->n) {
        search->count = checked_add(search->count, UINT64_C(1));
        return;
    }
    for (int next = 1; next <= search->n; ++next) {
        const mask_t bit = value_bit(next);
        if ((search->used & bit) != 0U)
            continue;
        if (depth >= 2) {
            const int previous = search->permutation[depth - 2] %
                                 search->permutation[depth - 1];
            const int current = search->permutation[depth - 1] % next;
            if (previous >= current)
                continue;
        }
        search->permutation[depth] = next;
        search->used |= bit;
        brute_visit(search, depth + 1);
        search->used ^= bit;
    }
}

static uint64_t brute_count(int n)
{
    BruteSearch search;
    memset(&search, 0, sizeof(search));
    search.n = n;
    brute_visit(&search, 0);
    return search.count;
}

static uint64_t compute_term(int term, size_t memory_limit,
                             Statistics *statistics)
{
    const int permutation_n = term - 1;
    const uint64_t value = count_permutations(
        permutation_n, memory_limit, statistics);
    if (term <= KNOWN_MAX_N && value != known[term]) {
        fprintf(stderr,
                "error: verification failed at A022825(%d): "
                "computed=%" PRIu64 ", expected=%" PRIu64 "\n",
                term, value, known[term]);
        exit(EXIT_FAILURE);
    }
    return value;
}

static void report_statistics(int term, const Statistics *statistics)
{
    const double hit_rate = statistics->calls == 0 ? 0.0 :
        100.0 * (double)statistics->memo_hits /
            (double)statistics->calls;
    fprintf(stderr,
            "022825_01: A022825(%d), permutation_n=%d, states=%zu, "
            "calls=%" PRIu64 ", transitions=%" PRIu64 ", "
            "cache_hit=%.1f%%, peak=%.1f MiB, elapsed=%.3f s\n",
            term, term - 1, statistics->states, statistics->calls,
            statistics->transitions, hit_rate,
            (double)statistics->peak_memory / 1048576.0,
            statistics->seconds);
}

static int acquire_bfile_lock(void)
{
    const int descriptor = open(BFILE_LOCK_NAME, O_CREAT | O_RDWR, 0666);
    if (descriptor < 0)
        die("cannot open b-file lock");
    struct flock lock;
    memset(&lock, 0, sizeof(lock));
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    if (fcntl(descriptor, F_SETLK, &lock) != 0) {
        close(descriptor);
        die("another writer holds the b-file lock");
    }
    return descriptor;
}

static void write_bfile(const uint64_t *terms, int upto)
{
    const int lock_descriptor = acquire_bfile_lock();
    char temporary[] = BFILE_TEMP_TEMPLATE;
    const int descriptor = mkstemp(temporary);
    if (descriptor < 0) {
        close(lock_descriptor);
        die("cannot create temporary b-file");
    }
    const mode_t old_mask = umask(0);
    (void)umask(old_mask);
    if (fchmod(descriptor, (mode_t)(0666 & ~old_mask)) != 0) {
        close(descriptor);
        unlink(temporary);
        close(lock_descriptor);
        die("cannot set temporary b-file permissions");
    }
    FILE *file = fdopen(descriptor, "w");
    if (file == NULL) {
        close(descriptor);
        unlink(temporary);
        close(lock_descriptor);
        die("cannot open temporary b-file stream");
    }

    bool failed = false;
    for (int term = 1; term <= upto; ++term)
        if (fprintf(file, "%d %" PRIu64 "\n", term, terms[term]) < 0)
            failed = true;
    if (fflush(file) != 0 || fsync(descriptor) != 0)
        failed = true;
    if (fclose(file) != 0)
        failed = true;
    if (failed) {
        unlink(temporary);
        close(lock_descriptor);
        die("cannot write temporary b-file");
    }
    if (rename(temporary, BFILE_NAME) != 0) {
        unlink(temporary);
        close(lock_descriptor);
        die("cannot replace b-file");
    }
    close(lock_descriptor);
}

static void run_check(size_t memory_limit, bool verbose)
{
    for (int permutation_n = 0;
         permutation_n <= DIRECT_CHECK_MAX_PERM_N; ++permutation_n) {
        const int term = permutation_n + 1;
        Statistics statistics;
        const uint64_t dynamic = compute_term(
            term, memory_limit, &statistics);
        const uint64_t brute = brute_count(permutation_n);
        if (dynamic != brute) {
            fprintf(stderr,
                    "error: direct check failed for permutation n=%d: "
                    "memoized=%" PRIu64 ", brute=%" PRIu64 "\n",
                    permutation_n, dynamic, brute);
            exit(EXIT_FAILURE);
        }
        if (verbose)
            report_statistics(term, &statistics);
    }
    printf("check passed through permutation n=%d "
           "(A022825 indices 1..%d)\n",
           DIRECT_CHECK_MAX_PERM_N, DIRECT_CHECK_MAX_PERM_N + 1);
}

static void usage(const char *program)
{
    printf("Usage:\n"
           "  %s [--upto N] [--memory-mb M] [--verbose]\n"
           "  %s --term N [--memory-mb M] [--verbose]\n"
           "  %s --check [--memory-mb M] [--verbose]\n"
           "\nN is the A022825 index; permutation size is N-1.\n"
           "Valid N range: 1..%d. Default --upto: %d.\n",
           program, program, program, MAX_TERM_N, DEFAULT_UPTO);
}

int main(int argc, char **argv)
{
    OutputMode mode = MODE_UPTO;
    int requested = DEFAULT_UPTO;
    int memory_mb = DEFAULT_MEMORY_MB;
    bool mode_was_set = false;
    bool verbose = false;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--upto") == 0 ||
            strcmp(argv[i], "--term") == 0) {
            if (mode_was_set)
                die("specify only one of --upto, --term, and --check");
            if (++i >= argc)
                die("missing value after mode option");
            mode = strcmp(argv[i - 1], "--upto") == 0 ?
                MODE_UPTO : MODE_TERM;
            requested = parse_integer(argv[i], "N", 1, MAX_TERM_N);
            mode_was_set = true;
        } else if (strcmp(argv[i], "--check") == 0) {
            if (mode_was_set)
                die("specify only one of --upto, --term, and --check");
            mode = MODE_CHECK;
            mode_was_set = true;
        } else if (strcmp(argv[i], "--memory-mb") == 0) {
            if (++i >= argc)
                die("missing value after --memory-mb");
            memory_mb = parse_integer(argv[i], "memory-mb",
                                      MIN_MEMORY_MB, MAX_MEMORY_MB);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if ((size_t)memory_mb > SIZE_MAX / (size_t)1048576)
        die("memory limit size overflow");
    const size_t memory_limit = (size_t)memory_mb * (size_t)1048576;

    if (mode == MODE_CHECK) {
        run_check(memory_limit, verbose);
        return EXIT_SUCCESS;
    }

    if (mode == MODE_TERM) {
        Statistics statistics;
        const uint64_t value = compute_term(
            requested, memory_limit, &statistics);
        printf("%d %" PRIu64 "\n", requested, value);
        if (verbose)
            report_statistics(requested, &statistics);
        return EXIT_SUCCESS;
    }

    uint64_t terms[MAX_TERM_N + 1];
    memset(terms, 0, sizeof(terms));
    for (int term = 1; term <= requested; ++term) {
        Statistics statistics;
        terms[term] = compute_term(term, memory_limit, &statistics);
        printf("%d %" PRIu64 "\n", term, terms[term]);
        fflush(stdout);
        if (verbose)
            report_statistics(term, &statistics);
    }
    write_bfile(terms, requested);
    return EXIT_SUCCESS;
}
