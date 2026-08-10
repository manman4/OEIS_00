/*
 * A060963 -- sparse memoized search in decreasing difference order.
 *
 * Count perfect matchings of {0,...,2*n-1} whose edge differences are all
 * distinct.  Every completed matching has a unique decreasing ordering of
 * its differences.  Therefore place edges in that order.  The state is
 *
 *     (occupied position mask, exclusive upper bound for the next difference).
 *
 * If r pairs remain, a candidate difference d must satisfy d>=r, since
 * r-1 still smaller distinct positive differences will be needed.  States
 * are memoized in a bounded open-addressed hash table.  Reflection and
 * translation past occupied boundary positions share a cache entry.  Only
 * values fitting uint32_t are cached, reducing a slot from 24 to 12 bytes;
 * omitted large values are recomputed without affecting correctness.  Gap
 * parity detects impossible states and sometimes forces the next edge.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       060963_02.c -o 060963_02
 *
 * Usage:
 *   ./060963_02 12
 *   ./060963_02 --term 16 --threads 4 --cache-mib 4096
 *   ./060963_02 --check
 * Results are atomically recorded in b060963_02.txt by default.  Use
 * --output FILE to select another b-file or --no-bfile to disable writing.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "060963_02.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 19
#define DEFAULT_N 9
#define DEFAULT_CHECK_N 10
#define DEFAULT_CACHE_MIB 512
#define MAX_CACHE_MIB 16384
#define MAX_THREADS 32
#define KEY_BITS_FOR_LIMIT 6

typedef struct {
    uint64_t *keys;
    uint32_t *values;
    uint64_t capacity;
    uint64_t mask;
    uint64_t count;
    uint64_t insertion_limit;
    int saturated;
    uint64_t large_values;
} Memo;

typedef struct {
    int n;
    unsigned positions;
    uint64_t full;
    Memo *memo;
    uint64_t calls;
    uint64_t hits;
    uint64_t additions;
    uint64_t pruned;
    uint64_t forced;
} Search;

static const char *output_path = "b060963_02.txt";
static bool write_bfile = true;
static unsigned cache_mib = DEFAULT_CACHE_MIB;
static int requested_threads = 4;

static const char *const known[] = {
    "1", "1", "1", "5", "29", "145", "957", "8397", "85169",
    "944221", "11639417", "160699437", "2430145085",
    "39776366397", "703161838717", "13369111112753",
    "271734091323897", "5876684246433485", "134794262542773569"
};

static void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static int parse_n(const char *text)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 0 || value > MAX_N) {
        fprintf(stderr, "error: N must be in 0..%d: %s\n", MAX_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static unsigned parse_cache_mib(const char *text)
{
    char *end = NULL;
    errno = 0;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 1 || value > MAX_CACHE_MIB) {
        fprintf(stderr, "error: cache MiB must be in 1..%d: %s\n",
                MAX_CACHE_MIB, text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static int parse_threads(const char *text)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < 1 || value > MAX_THREADS) {
        fprintf(stderr, "error: threads must be in 1..%d: %s\n",
                MAX_THREADS, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static double now_seconds(void)
{
    struct timespec time;
    if (clock_gettime(CLOCK_MONOTONIC, &time) != 0) {
        die("clock_gettime failed");
    }
    return (double)time.tv_sec + (double)time.tv_nsec / 1e9;
}

static int print_u128(FILE *stream, U128 value)
{
    char digits[40];
    size_t length = 0;
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10));
        value /= 10;
    } while (value != 0);
    while (length != 0) {
        if (fputc(digits[--length], stream) == EOF) {
            return -1;
        }
    }
    return 0;
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0;
    if (*text == '\0') {
        return false;
    }
    for (; *text != '\0'; ++text) {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text - '0');
        if (value > (maximum - digit) / 10) {
            return false;
        }
        value = value * 10 + digit;
    }
    *result = value;
    return true;
}

static bool add_u128(U128 *destination, U128 addend)
{
    const U128 old = *destination;
    *destination += addend;
    return *destination >= old;
}

static void store_bfile_term(int n, U128 value)
{
    U128 values[MAX_N + 1] = {0};
    bool present[MAX_N + 1] = {false};
    int previous = -1;
    mode_t output_mode = 0644;
    struct stat metadata;
    if (stat(output_path, &metadata) == 0) {
        output_mode = metadata.st_mode & 0777;
    } else if (errno != ENOENT) {
        die("cannot inspect b-file");
    }

    FILE *input = fopen(output_path, "r");
    if (input == NULL && errno != ENOENT) {
        die("cannot open existing b-file");
    }
    if (input != NULL) {
        char line[128];
        while (fgets(line, sizeof(line), input) != NULL) {
            int index;
            char number[64];
            char extra;
            if (sscanf(line, "%d %63s %c", &index, number, &extra) != 2 ||
                index < 0 || index > MAX_N || index <= previous ||
                !parse_u128(number, &values[index])) {
                fclose(input);
                die("existing b-file is malformed or not strictly ordered");
            }
            present[index] = true;
            previous = index;
        }
        if (ferror(input) || fclose(input) != 0) {
            die("cannot read existing b-file");
        }
    }

    if (present[n]) {
        if (values[n] != value) {
            die("computed term disagrees with existing b-file");
        }
        return;
    }
    values[n] = value;
    present[n] = true;

    const char suffix[] = ".tmp.XXXXXX";
    const size_t path_length = strlen(output_path);
    if (path_length > SIZE_MAX - sizeof(suffix)) {
        die("b-file path is too long");
    }
    char *temporary = malloc(path_length + sizeof(suffix));
    if (temporary == NULL) {
        die("cannot allocate b-file temporary path");
    }
    memcpy(temporary, output_path, path_length);
    memcpy(temporary + path_length, suffix, sizeof(suffix));
    const int fd = mkstemp(temporary);
    if (fd < 0) {
        free(temporary);
        die("cannot create temporary b-file");
    }
    if (fchmod(fd, output_mode) != 0) {
        close(fd);
        unlink(temporary);
        free(temporary);
        die("cannot set temporary b-file permissions");
    }
    FILE *output = fdopen(fd, "w");
    if (output == NULL) {
        close(fd);
        unlink(temporary);
        free(temporary);
        die("cannot open temporary b-file stream");
    }
    bool failed = false;
    for (int index = 0; index <= MAX_N; ++index) {
        if (!present[index]) {
            continue;
        }
        if (fprintf(output, "%d ", index) < 0 ||
            print_u128(output, values[index]) != 0 ||
            fputc('\n', output) == EOF) {
            failed = true;
            break;
        }
    }
    if (!failed && fflush(output) != 0) {
        failed = true;
    }
    if (!failed && fsync(fd) != 0) {
        failed = true;
    }
    if (fclose(output) != 0) {
        failed = true;
    }
    if (failed) {
        unlink(temporary);
        free(temporary);
        die("cannot write temporary b-file");
    }
    if (rename(temporary, output_path) != 0) {
        unlink(temporary);
        free(temporary);
        die("cannot atomically replace b-file");
    }
    free(temporary);
    fprintf(stderr, "060963_02: recorded computed term n=%d in %s\n",
            n, output_path);
}

static uint64_t reverse_bits(uint64_t value, unsigned width)
{
    value = ((value >> 1) & UINT64_C(0x5555555555555555)) |
            ((value & UINT64_C(0x5555555555555555)) << 1);
    value = ((value >> 2) & UINT64_C(0x3333333333333333)) |
            ((value & UINT64_C(0x3333333333333333)) << 2);
    value = ((value >> 4) & UINT64_C(0x0f0f0f0f0f0f0f0f)) |
            ((value & UINT64_C(0x0f0f0f0f0f0f0f0f)) << 4);
    value = ((value >> 8) & UINT64_C(0x00ff00ff00ff00ff)) |
            ((value & UINT64_C(0x00ff00ff00ff00ff)) << 8);
    value = ((value >> 16) & UINT64_C(0x0000ffff0000ffff)) |
            ((value & UINT64_C(0x0000ffff0000ffff)) << 16);
    value = (value >> 32) | (value << 32);
    return value >> (64 - width);
}

static uint64_t hash_key(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static Memo make_memo(unsigned mib)
{
    Memo memo;
    memset(&memo, 0, sizeof(memo));
    const uint64_t bytes = (uint64_t)mib * 1024 * 1024;
    const uint64_t bytes_per_slot =
        sizeof(*memo.keys) + sizeof(*memo.values);
    uint64_t capacity = 1;
    while (capacity <= bytes / bytes_per_slot / 2) {
        capacity *= 2;
    }
    if (capacity < 1024) {
        capacity = 1024;
    }
    if (capacity > SIZE_MAX / sizeof(*memo.keys) ||
        capacity > SIZE_MAX / sizeof(*memo.values)) {
        die("cache size overflow");
    }
    memo.keys = calloc((size_t)capacity, sizeof(*memo.keys));
    memo.values = calloc((size_t)capacity, sizeof(*memo.values));
    if (memo.keys == NULL || memo.values == NULL) {
        free(memo.keys);
        free(memo.values);
        die("cannot allocate memoization cache; reduce --cache-mib");
    }
    memo.capacity = capacity;
    memo.mask = capacity - 1;
    memo.insertion_limit = capacity * 3 / 4;
    return memo;
}

static void destroy_memo(Memo *memo)
{
    free(memo->keys);
    free(memo->values);
    memset(memo, 0, sizeof(*memo));
}

static bool memo_get(const Memo *memo, uint64_t key, U128 *value)
{
    const uint64_t stored_key = key + 1;
    uint64_t slot = hash_key(key) & memo->mask;
    for (;;) {
        const uint64_t present = __atomic_load_n(
            &memo->keys[slot], __ATOMIC_ACQUIRE);
        if (present == 0) {
            return false;
        }
        const uint64_t actual = present & ~(UINT64_C(1) << 63);
        if (actual == stored_key) {
            if ((present >> 63) != 0) {
                return false;
            }
            *value = (U128)memo->values[slot];
            return true;
        }
        slot = (slot + 1) & memo->mask;
    }
}

static void memo_put(Memo *memo, uint64_t key, U128 value)
{
    if (value > UINT32_MAX) {
        __atomic_fetch_add(&memo->large_values, 1, __ATOMIC_RELAXED);
        return;
    }
    if (__atomic_load_n(&memo->count, __ATOMIC_RELAXED) >=
        memo->insertion_limit) {
        __atomic_store_n(&memo->saturated, 1, __ATOMIC_RELAXED);
        return;
    }
    const uint64_t stored_key = key + 1;
    const uint64_t busy_key = stored_key | (UINT64_C(1) << 63);
    uint64_t slot = hash_key(key) & memo->mask;
    for (;;) {
        uint64_t present = __atomic_load_n(
            &memo->keys[slot], __ATOMIC_ACQUIRE);
        if ((present & ~(UINT64_C(1) << 63)) == stored_key) {
            return;
        }
        if (present == 0 && __atomic_compare_exchange_n(
                &memo->keys[slot], &present, busy_key, false,
                __ATOMIC_ACQ_REL, __ATOMIC_ACQUIRE)) {
            memo->values[slot] = (uint32_t)value;
            __atomic_store_n(&memo->keys[slot], stored_key,
                             __ATOMIC_RELEASE);
            __atomic_fetch_add(&memo->count, 1, __ATOMIC_RELAXED);
            return;
        }
        slot = (slot + 1) & memo->mask;
    }
}

static uint64_t normalize_boundary(uint64_t occupied, unsigned positions,
                                   uint64_t full)
{
    const uint64_t top = UINT64_C(1) << (positions - 1);
    while ((occupied & top) != 0) {
        occupied = ((occupied ^ top) << 1) | 1;
    }
    return occupied & full;
}

static uint64_t canonical_mask(const Search *search, uint64_t occupied)
{
    const uint64_t direct = normalize_boundary(
        occupied, search->positions, search->full);
    const uint64_t reflected = normalize_boundary(
        reverse_bits(occupied, search->positions),
        search->positions, search->full);
    return direct < reflected ? direct : reflected;
}

/* Return -1 for impossible, 0 for no forced edge, and 1 for a forced edge. */
static int gap_constraint(const Search *search, uint64_t occupied,
                          unsigned limit, uint64_t *forced_pair)
{
    if (limit < 2) {
        return -1;
    }
    const unsigned maximum_gap = limit - 2;
    unsigned empty_seen = 0;
    unsigned filled = 0;
    unsigned forced_right = UINT32_MAX;

    for (unsigned position = 0; position < search->positions; ++position) {
        if ((occupied & (UINT64_C(1) << position)) == 0) {
            if (filled > maximum_gap && (empty_seen & 1U) != 0) {
                return -1;
            }
            if (filled == maximum_gap && (empty_seen & 1U) != 0) {
                if (forced_right != UINT32_MAX) {
                    return -1;
                }
                forced_right = position;
            }
            ++empty_seen;
            filled = 0;
        } else {
            ++filled;
        }
    }

    if (forced_right == UINT32_MAX) {
        return 0;
    }
    const unsigned difference = limit - 1;
    if (forced_right < difference) {
        return -1;
    }
    const unsigned forced_left = forced_right - difference;
    const uint64_t pair = (UINT64_C(1) << forced_left) |
                          (UINT64_C(1) << forced_right);
    if ((occupied & pair) != 0) {
        return -1;
    }
    *forced_pair = pair;
    return 1;
}

static U128 search_state(Search *search, uint64_t occupied, unsigned limit)
{
    ++search->calls;
    if (occupied == search->full) {
        return 1;
    }

    const unsigned remaining =
        (search->positions - (unsigned)__builtin_popcountll(occupied)) / 2;
    if (remaining == 1) {
        const uint64_t available = search->full ^ occupied;
        const unsigned left = (unsigned)__builtin_ctzll(available);
        const uint64_t rest = available ^ (UINT64_C(1) << left);
        const unsigned right = (unsigned)__builtin_ctzll(rest);
        return right - left < limit ? 1 : 0;
    }

    const uint64_t canonical = canonical_mask(search, occupied);
    const uint64_t key = (canonical << KEY_BITS_FOR_LIMIT) | limit;
    U128 cached;
    if (memo_get(search->memo, key, &cached)) {
        ++search->hits;
        return cached;
    }

    uint64_t forced_pair = 0;
    const int constraint =
        gap_constraint(search, occupied, limit, &forced_pair);
    if (constraint < 0) {
        ++search->pruned;
        memo_put(search->memo, key, 0);
        return 0;
    }
    if (constraint > 0) {
        ++search->forced;
        const U128 answer = search_state(
            search, occupied | forced_pair, limit - 1);
        memo_put(search->memo, key, answer);
        return answer;
    }

    const uint64_t available = search->full ^ occupied;
    U128 answer = 0;

    for (unsigned difference = limit - 1;
         difference >= remaining; --difference) {
        uint64_t starts = available & (available >> difference);
        while (starts != 0) {
            const uint64_t first = starts & (UINT64_C(0) - starts);
            starts ^= first;
            const uint64_t pair = first | (first << difference);
            const U128 addend =
                search_state(search, occupied | pair, difference);
            if (!add_u128(&answer, addend)) {
                die("answer overflowed unsigned __int128");
            }
            ++search->additions;
        }
        if (difference == remaining) {
            break;
        }
    }
    memo_put(search->memo, key, answer);
    return answer;
}

typedef struct {
    uint64_t pair;
    unsigned difference;
    unsigned weight;
} RootBranch;

typedef struct {
    RootBranch *branches;
    uint64_t branch_count;
    uint64_t next_branch;
} RootQueue;

typedef struct {
    Search search;
    RootQueue *queue;
    U128 answer;
} RootWorker;

static void *root_worker_main(void *argument)
{
    RootWorker *worker = argument;
    for (;;) {
        const uint64_t index = __atomic_fetch_add(
            &worker->queue->next_branch, 1, __ATOMIC_RELAXED);
        if (index >= worker->queue->branch_count) {
            break;
        }
        const RootBranch branch = worker->queue->branches[index];
        U128 addend = search_state(&worker->search, branch.pair,
                                   branch.difference);
        if (branch.weight == 2) {
            const U128 maximum = ~(U128)0;
            if (addend > maximum / 2) {
                die("answer overflowed unsigned __int128");
            }
            addend *= 2;
        }
        if (!add_u128(&worker->answer, addend)) {
            die("answer overflowed unsigned __int128");
        }
    }
    return NULL;
}

static U128 a060963(int n)
{
    if (n == 0) {
        return 1;
    }
    const unsigned positions = 2U * (unsigned)n;
    const uint64_t full = (UINT64_C(1) << positions) - 1;
    RootBranch branches[MAX_N * MAX_N];
    uint64_t branch_count = 0;
    for (unsigned difference = positions - 1;
         difference >= (unsigned)n; --difference) {
        uint64_t starts = full & (full >> difference);
        while (starts != 0) {
            const uint64_t first = starts & (UINT64_C(0) - starts);
            starts ^= first;
            const uint64_t pair = first | (first << difference);
            const uint64_t reflected = reverse_bits(pair, positions);
            if (pair <= reflected) {
                RootBranch *branch = &branches[branch_count++];
                branch->pair = pair;
                branch->difference = difference;
                branch->weight = pair == reflected ? 1U : 2U;
            }
        }
        if (difference == (unsigned)n) {
            break;
        }
    }

    int threads = requested_threads;
    if ((uint64_t)threads > branch_count) {
        threads = (int)branch_count;
    }
    RootQueue queue = {branches, branch_count, 0};
    RootWorker *workers = calloc((size_t)threads, sizeof(*workers));
    pthread_t *thread_ids = calloc((size_t)threads, sizeof(*thread_ids));
    if (workers == NULL || thread_ids == NULL) {
        free(workers);
        free(thread_ids);
        die("cannot allocate root workers");
    }

    Memo memo = make_memo(cache_mib);
    for (int id = 0; id < threads; ++id) {
        workers[id].search.n = n;
        workers[id].search.positions = positions;
        workers[id].search.full = full;
        workers[id].search.memo = &memo;
        workers[id].queue = &queue;
    }
    fprintf(stderr,
            "060963_02: cache %.1f MiB total, slots=%" PRIu64
            ", threads=%d, root-branches=%" PRIu64 "\n",
            (double)(memo.capacity *
                (sizeof(uint64_t) + sizeof(uint32_t))) / 1048576.0,
            memo.capacity, threads, branch_count);

    const double started = now_seconds();
    for (int id = 0; id < threads; ++id) {
        if (threads == 1) {
            root_worker_main(&workers[id]);
        } else {
            const int error = pthread_create(&thread_ids[id], NULL,
                                             root_worker_main, &workers[id]);
            if (error != 0) {
                fprintf(stderr, "error: pthread_create: %s\n",
                        strerror(error));
                exit(EXIT_FAILURE);
            }
        }
    }

    U128 answer = 0;
    uint64_t hits = 0, transitions = 0;
    uint64_t pruned = 0, forced = 0;
    for (int id = 0; id < threads; ++id) {
        if (threads > 1) {
            const int error = pthread_join(thread_ids[id], NULL);
            if (error != 0) {
                fprintf(stderr, "error: pthread_join: %s\n",
                        strerror(error));
                exit(EXIT_FAILURE);
            }
        }
        if (!add_u128(&answer, workers[id].answer)) {
            die("answer overflowed unsigned __int128");
        }
        hits += workers[id].search.hits;
        transitions += workers[id].search.additions;
        pruned += workers[id].search.pruned;
        forced += workers[id].search.forced;
    }
    fprintf(stderr,
            "060963_02: n=%d, states=%" PRIu64 ", hits=%" PRIu64
            ", transitions=%" PRIu64 ", pruned=%" PRIu64
            ", forced=%" PRIu64 ", large=%" PRIu64 ", %.3f s%s\n",
            n, memo.count, hits, transitions, pruned, forced,
            memo.large_values,
            now_seconds() - started,
            memo.saturated ? ", cache saturated" : "");
    destroy_memo(&memo);
    free(workers);
    free(thread_ids);
    return answer;
}

static void verify_known(int n, U128 value)
{
    const int count = (int)(sizeof(known) / sizeof(known[0]));
    if (n >= count) {
        return;
    }
    U128 expected;
    if (!parse_u128(known[n], &expected)) {
        die("invalid built-in known term");
    }
    if (value != expected) {
        fprintf(stderr, "error: A060963 mismatch at n=%d: got ", n);
        print_u128(stderr, value);
        fprintf(stderr, ", expected %s\n", known[n]);
        exit(EXIT_FAILURE);
    }
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--threads T] [--cache-mib M] [--output FILE]\n"
            "       %s --term N [--threads T] [--cache-mib M] [--output FILE]\n"
            "       %s --check [--threads T] [--cache-mib M] [--no-bfile]\n"
            "N must be in 0..%d; T in 1..%d; M in 1..%d.\n",
            program, program, program, MAX_N, MAX_THREADS, MAX_CACHE_MIB);
}

int main(int argc, char **argv)
{
    enum { MODE_RANGE, MODE_TERM, MODE_CHECK } mode = MODE_RANGE;
    int maximum = -1;
    bool have_mode = false;
    bool have_cache = false;
    bool have_threads = false;
    bool have_output = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (!strcmp(argv[i], "--threads")) {
            if (have_threads || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            requested_threads = parse_threads(argv[i]);
            have_threads = true;
        } else if (!strcmp(argv[i], "--cache-mib")) {
            if (have_cache || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            cache_mib = parse_cache_mib(argv[i]);
            have_cache = true;
        } else if (!strcmp(argv[i], "--output")) {
            if (have_output || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            output_path = argv[i];
            write_bfile = true;
            have_output = true;
        } else if (!strcmp(argv[i], "--no-bfile")) {
            if (have_output) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            write_bfile = false;
            have_output = true;
        } else if (!strcmp(argv[i], "--term")) {
            if (have_mode || ++i >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_TERM;
            maximum = parse_n(argv[i]);
            have_mode = true;
        } else if (!strcmp(argv[i], "--check")) {
            if (have_mode) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_CHECK;
            have_mode = true;
        } else if (argv[i][0] != '-' && !have_mode) {
            maximum = parse_n(argv[i]);
            mode = MODE_RANGE;
            have_mode = true;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (maximum < 0) {
        maximum = DEFAULT_N;
    }
    if (mode == MODE_CHECK) {
        for (int n = 0; n <= DEFAULT_CHECK_N; ++n) {
            verify_known(n, a060963(n));
        }
        printf("ok: A060963 terms n=0..%d verified\n", DEFAULT_CHECK_N);
        return EXIT_SUCCESS;
    }
    if (mode == MODE_TERM) {
        const U128 value = a060963(maximum);
        verify_known(maximum, value);
        if (write_bfile) {
            store_bfile_term(maximum, value);
        }
        printf("%d ", maximum);
        print_u128(stdout, value);
        putchar('\n');
        return EXIT_SUCCESS;
    }

    for (int n = 0; n <= maximum; ++n) {
        const U128 value = a060963(n);
        verify_known(n, value);
        if (write_bfile) {
            store_bfile_term(n, value);
        }
        printf("%d ", n);
        print_u128(stdout, value);
        putchar('\n');
    }
    return EXIT_SUCCESS;
}
