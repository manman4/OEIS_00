/*
 * A067957, A093313, A093314, A093315 -- two-ended memoized search.
 *
 * Count permutations p of [n], optionally with prescribed p(1), such that
 *
 *     p(j) divides Sum_{i=1..j} p(i)       for every j in [n].
 *
 * This is algorithmically independent of 093313_01.c.  Suppose a valid
 * prefix uses A, a valid suffix (already fixed from right to left) uses B,
 * and U=[n]-(A union B) is still unplaced.  Put a=sum(A), b=sum(B), and
 * T=n(n+1)/2.  A value x can be put next on the left exactly when x|a.
 * A value y can be put next on the right exactly when y|(T-b).  Both are
 * necessary conditions for every completion, so an empty candidate set on
 * either side proves that the state has no completion.
 *
 * At every state the search branches on the end with fewer candidates.
 * This deterministic choice partitions all completions without duplication.
 * Results are memoized by the exact pair (A,B) in a bounded four-way cache.
 * A cache collision merely evicts an old result and causes safe recomputation;
 * it cannot change the answer.  The cache uses at most about 384 MiB while
 * counts fit 64 bits, or 512 MiB if its lazy upper-64-bit array is needed.
 * This tends to work well when the number of complete chains is small, while
 * giving an independent check on the frontier-intersection algorithm in
 * 093313_01.c.
 *
 * Counts use unsigned 128-bit integers with checked arithmetic.  The upper
 * 64-bit memo array is allocated only if a count actually needs it.  Subsets
 * use uint64_t, so n<=63; practical time and memory limits are normally met
 * much sooner.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       093313_02.c -o 093313_02
 *
 * Examples:
 *   ./093313_02 2 --upto 44
 *   ./093313_02 3 --term 40
 *   ./093313_02 4 --check 40
 *   ./093313_02 _ --upto 30
 *
 * Fixed p(1)=K is saved as b09331(K+1)_02.txt.  Thus K=2,3,4 give
 * b093313_02.txt, b093314_02.txt, b093315_02.txt.  The unrestricted "_"
 * mode is saved as b067957_02.txt.  Files are replaced atomically only after
 * all requested terms finish successfully.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(__SIZEOF_INT128__)
#error "093313_02.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 63U
#define DEFAULT_MAX_N 30U
#define CACHE_WAYS 4U
#define MIN_CACHE_EXPONENT 10U
#define MAX_CACHE_EXPONENT 24U
#define PROGRESS_INTERVAL UINT64_C(10000000)

_Static_assert((CACHE_WAYS & (CACHE_WAYS - 1U)) == 0U,
               "CACHE_WAYS must be a power of two");

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} OutputMode;

typedef struct {
    uint64_t *prefix;
    uint64_t *suffix;
    uint64_t *count_low;
    uint64_t *count_high;
    size_t capacity;
    size_t size;
} MemoMap;

typedef struct {
    unsigned n;
    unsigned total_sum;
    uint64_t full_mask;
    uint64_t *divisor_mask;
    MemoMap memo;
    uint64_t calls;
    uint64_t computed_states;
    uint64_t memo_hits;
    uint64_t cache_replacements;
    uint64_t branches;
    uint64_t dead_ends;
    uint64_t next_report;
    double started;
} SearchContext;

static bool quiet;

static const char *const known_s2[] = {
    NULL,
    "0", "1", "1", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "6", "6", "1", "11", "9", "15", "14", "14", "23"
};

static const char *const known_s3[] = {
    NULL,
    "0", "0", "1", "1", "1", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "1", "0", "0", "0", "0", "0", "0",
    "0", "12", "20", "20", "1", "163", "55"
};

static const char *const known_s4[] = {
    NULL,
    "0", "0", "0", "1", "2", "2", "2", "0", "0", "0", "0",
    "1", "1", "0", "1", "1", "1", "0", "0", "0", "0", "0",
    "0", "6", "8", "3", "14", "12", "18", "13", "14", "6", "26",
    "13", "198", "152", "220", "118", "1033", "807"
};

static const char *const known_all[] = {
    "1", "1", "1", "2", "2", "4", "5", "7", "7", "24",
    "22", "29", "39", "67", "55", "386", "235", "312", "347",
    "451", "1319", "5320", "3220", "4489", "20237", "36580",
    "52875", "197103", "216562", "289478", "567396", "659647",
    "1111153", "3131774", "2200426", "29523302", "34214028",
    "48161995", "32616148", "242860900", "293579041", "363415618"
};

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void *xcalloc(size_t count, size_t size)
{
    if (size != 0U && count > SIZE_MAX / size) {
        die("allocation size overflow");
    }
    void *pointer = calloc(count, size);
    if (pointer == NULL) {
        die("out of memory");
    }
    return pointer;
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
        die("clock_gettime failed");
    }
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static unsigned parse_unsigned(const char *text, unsigned low, unsigned high,
                               const char *label)
{
    char *end = NULL;
    errno = 0;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < low || value > high) {
        fprintf(stderr, "error: %s must be in %u..%u: %s\n",
                label, low, high, text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static void checked_add(U128 *target, U128 value)
{
    const U128 maximum = ~(U128)0;
    if (*target > maximum - value) {
        die("unsigned 128-bit count overflow");
    }
    *target += value;
}

static void u128_to_text(U128 value, char text[40])
{
    char reverse[40];
    size_t length = 0U;
    do {
        reverse[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0U);
    for (size_t i = 0U; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0U;
    if (text == NULL || *text == '\0') {
        return false;
    }
    while (*text != '\0') {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text++ - '0');
        if (value > (maximum - digit) / 10U) {
            return false;
        }
        value = value * 10U + digit;
    }
    *result = value;
    return true;
}

static uint64_t mix64(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31;
    return value;
}

static uint64_t hash_pair(uint64_t prefix, uint64_t suffix)
{
    return mix64(prefix ^ (mix64(suffix) + UINT64_C(0x9e3779b97f4a7c15)));
}

static MemoMap memo_create(size_t capacity)
{
    MemoMap map;
    map.prefix = xcalloc(capacity, sizeof(*map.prefix));
    map.suffix = xcalloc(capacity, sizeof(*map.suffix));
    map.count_low = xcalloc(capacity, sizeof(*map.count_low));
    map.count_high = NULL;
    map.capacity = capacity;
    map.size = 0U;
    return map;
}

static void memo_destroy(MemoMap *map)
{
    free(map->prefix);
    free(map->suffix);
    free(map->count_low);
    free(map->count_high);
    memset(map, 0, sizeof(*map));
}

static U128 memo_count_at(const MemoMap *map, size_t slot)
{
    const U128 high = map->count_high == NULL ? 0U : map->count_high[slot];
    return (high << 64) | map->count_low[slot];
}

static void memo_store_count(MemoMap *map, size_t slot, U128 count)
{
    const uint64_t high = (uint64_t)(count >> 64);
    if (high != 0U && map->count_high == NULL) {
        map->count_high = xcalloc(map->capacity, sizeof(*map->count_high));
    }
    map->count_low[slot] = (uint64_t)count;
    if (map->count_high != NULL) {
        map->count_high[slot] = high;
    }
}

static size_t memo_set_base(const MemoMap *map, uint64_t hash)
{
    const size_t set_count = map->capacity / CACHE_WAYS;
    return ((size_t)hash & (set_count - 1U)) * CACHE_WAYS;
}

static bool memo_find(const MemoMap *map, uint64_t prefix, uint64_t suffix,
                      U128 *count)
{
    const uint64_t hash = hash_pair(prefix, suffix);
    const size_t base = memo_set_base(map, hash);
    for (size_t way = 0U; way < CACHE_WAYS; ++way) {
        const size_t slot = base + way;
        if (map->prefix[slot] == prefix && map->suffix[slot] == suffix) {
            *count = memo_count_at(map, slot);
            return true;
        }
    }
    return false;
}

static bool memo_insert(MemoMap *map, uint64_t prefix, uint64_t suffix,
                        U128 count)
{
    const uint64_t hash = hash_pair(prefix, suffix);
    const size_t base = memo_set_base(map, hash);
    size_t destination = SIZE_MAX;
    for (size_t way = 0U; way < CACHE_WAYS; ++way) {
        const size_t slot = base + way;
        if (map->prefix[slot] == prefix && map->suffix[slot] == suffix) {
            destination = slot;
            break;
        }
        if (destination == SIZE_MAX && map->prefix[slot] == 0U) {
            destination = slot;
        }
    }
    bool replaced = false;
    if (destination == SIZE_MAX) {
        destination = base + (size_t)((hash >> 32) & (CACHE_WAYS - 1U));
        replaced = true;
    } else if (map->prefix[destination] == 0U) {
        ++map->size;
    }
    map->prefix[destination] = prefix;
    map->suffix[destination] = suffix;
    memo_store_count(map, destination, count);
    return replaced;
}

static size_t memo_capacity_for_n(unsigned n)
{
    unsigned exponent = n > 11U ? n - 11U : MIN_CACHE_EXPONENT;
    if (exponent < MIN_CACHE_EXPONENT) {
        exponent = MIN_CACHE_EXPONENT;
    }
    if (exponent > MAX_CACHE_EXPONENT) {
        exponent = MAX_CACHE_EXPONENT;
    }
    return (size_t)1U << exponent;
}

static void increment_saturated(uint64_t *value)
{
    if (*value != UINT64_MAX) {
        ++*value;
    }
}

static void maybe_report_progress(SearchContext *context)
{
    if (quiet || context->computed_states < context->next_report) {
        return;
    }
    fprintf(stderr,
            "093313_02: n=%u states=%" PRIu64 " cache=%zu calls=%" PRIu64
            " hits=%" PRIu64 " branches=%" PRIu64 " time=%.1fs\n",
            context->n, context->computed_states, context->memo.size,
            context->calls, context->memo_hits, context->branches,
            monotonic_seconds() - context->started);
    if (context->next_report <= UINT64_MAX - PROGRESS_INTERVAL) {
        context->next_report += PROGRESS_INTERVAL;
    } else {
        context->next_report = UINT64_MAX;
    }
}

static uint64_t viable_left_candidates(const SearchContext *context,
                                       uint64_t unused, uint16_t prefix_sum,
                                       uint16_t suffix_sum,
                                       uint64_t candidates)
{
    uint64_t viable = 0U;
    while (candidates != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(candidates);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        candidates &= candidates - 1U;
        const uint64_t next_unused = unused ^ bit_mask;
        if (next_unused == 0U ||
            ((context->divisor_mask[prefix_sum + bit + 1U] & next_unused) != 0U &&
             (context->divisor_mask[context->total_sum - suffix_sum] &
              next_unused) != 0U)) {
            viable |= bit_mask;
        }
    }
    return viable;
}

static uint64_t viable_right_candidates(const SearchContext *context,
                                        uint64_t unused, uint16_t prefix_sum,
                                        uint16_t suffix_sum,
                                        uint64_t candidates)
{
    uint64_t viable = 0U;
    while (candidates != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(candidates);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        candidates &= candidates - 1U;
        const uint64_t next_unused = unused ^ bit_mask;
        if (next_unused == 0U ||
            ((context->divisor_mask[prefix_sum] & next_unused) != 0U &&
             (context->divisor_mask[
                  context->total_sum - suffix_sum - bit - 1U] &
              next_unused) != 0U)) {
            viable |= bit_mask;
        }
    }
    return viable;
}

static U128 search_completions(SearchContext *context,
                               uint64_t prefix_mask, uint64_t suffix_mask,
                               uint16_t prefix_sum, uint16_t suffix_sum)
{
    increment_saturated(&context->calls);
    const uint64_t unused = context->full_mask &
                            ~(prefix_mask | suffix_mask);
    if (unused == 0U) {
        return 1U;
    }

    U128 cached;
    if (memo_find(&context->memo, prefix_mask, suffix_mask, &cached)) {
        increment_saturated(&context->memo_hits);
        return cached;
    }

    uint64_t left_candidates =
        context->divisor_mask[prefix_sum] & unused;
    uint64_t right_candidates =
        context->divisor_mask[context->total_sum - suffix_sum] & unused;

    U128 answer = 0U;
    if (left_candidates != 0U && right_candidates != 0U) {
        left_candidates = viable_left_candidates(
            context, unused, prefix_sum, suffix_sum, left_candidates);
        right_candidates = viable_right_candidates(
            context, unused, prefix_sum, suffix_sum, right_candidates);
    }

    if (left_candidates == 0U || right_candidates == 0U) {
        increment_saturated(&context->dead_ends);
    } else if (__builtin_popcountll(left_candidates) <=
               __builtin_popcountll(right_candidates)) {
        uint64_t candidates = left_candidates;
        while (candidates != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            candidates &= candidates - 1U;
            increment_saturated(&context->branches);
            checked_add(&answer, search_completions(
                context, prefix_mask | bit_mask, suffix_mask,
                (uint16_t)(prefix_sum + bit + 1U), suffix_sum));
        }
    } else {
        uint64_t candidates = right_candidates;
        while (candidates != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            candidates &= candidates - 1U;
            increment_saturated(&context->branches);
            checked_add(&answer, search_completions(
                context, prefix_mask, suffix_mask | bit_mask,
                prefix_sum, (uint16_t)(suffix_sum + bit + 1U)));
        }
    }

    increment_saturated(&context->computed_states);
    if (memo_insert(&context->memo, prefix_mask, suffix_mask, answer)) {
        increment_saturated(&context->cache_replacements);
    }
    maybe_report_progress(context);
    return answer;
}

static U128 count_term(unsigned n, unsigned first, SearchContext *statistics)
{
    memset(statistics, 0, sizeof(*statistics));
    statistics->n = n;
    statistics->started = monotonic_seconds();
    statistics->next_report = PROGRESS_INTERVAL;

    if (n == 0U) {
        return first == 0U ? 1U : 0U;
    }
    if (first > n) {
        return 0U;
    }

    statistics->total_sum = n * (n + 1U) / 2U;
    statistics->full_mask = (UINT64_C(1) << n) - 1U;
    statistics->divisor_mask = xcalloc(statistics->total_sum + 1U,
                                       sizeof(*statistics->divisor_mask));
    for (unsigned value = 1U; value <= n; ++value) {
        const uint64_t bit = UINT64_C(1) << (value - 1U);
        for (unsigned sum = value; sum <= statistics->total_sum;
             sum += value) {
            statistics->divisor_mask[sum] |= bit;
        }
    }
    statistics->memo = memo_create(memo_capacity_for_n(n));

    U128 answer = 0U;
    if (first != 0U) {
        const uint64_t bit = UINT64_C(1) << (first - 1U);
        answer = search_completions(statistics, bit, 0U,
                                    (uint16_t)first, 0U);
    } else {
        for (unsigned value = 1U; value <= n; ++value) {
            const uint64_t bit = UINT64_C(1) << (value - 1U);
            checked_add(&answer, search_completions(
                statistics, bit, 0U, (uint16_t)value, 0U));
        }
    }
    return answer;
}

static void destroy_search_context(SearchContext *context)
{
    memo_destroy(&context->memo);
    free(context->divisor_mask);
    context->divisor_mask = NULL;
}

static const char *known_term(unsigned first, unsigned n)
{
    if (first == 0U && n < sizeof(known_all) / sizeof(known_all[0])) {
        return known_all[n];
    }
    if (first == 2U && n < sizeof(known_s2) / sizeof(known_s2[0])) {
        return known_s2[n];
    }
    if (first == 3U && n < sizeof(known_s3) / sizeof(known_s3[0])) {
        return known_s3[n];
    }
    if (first == 4U && n < sizeof(known_s4) / sizeof(known_s4[0])) {
        return known_s4[n];
    }
    return NULL;
}

static unsigned known_maximum(unsigned first)
{
    if (first == 0U) {
        return (unsigned)(sizeof(known_all) / sizeof(known_all[0]) - 1U);
    }
    if (first == 2U) {
        return (unsigned)(sizeof(known_s2) / sizeof(known_s2[0]) - 1U);
    }
    if (first == 3U) {
        return (unsigned)(sizeof(known_s3) / sizeof(known_s3[0]) - 1U);
    }
    if (first == 4U) {
        return (unsigned)(sizeof(known_s4) / sizeof(known_s4[0]) - 1U);
    }
    return 0U;
}

static FILE *open_output_file(unsigned first, char path[64], char part_path[72])
{
    const int path_length = first == 0U ?
        snprintf(path, 64, "b067957_02.txt") :
        snprintf(path, 64, "b09331%u_02.txt", first + 1U);
    if (path_length < 0 || path_length >= 64) {
        die("output path is too long");
    }
    const int part_length = snprintf(part_path, 72, "%s.part", path);
    if (part_length < 0 || part_length >= 72) {
        die("temporary output path is too long");
    }
    FILE *stream = fopen(part_path, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return stream;
}

static void finish_output_file(FILE *stream, const char *part_path,
                               const char *path)
{
    if (fclose(stream) != 0) {
        fprintf(stderr, "error: cannot close %s: %s\n",
                part_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (rename(part_path, path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static _Noreturn void usage(const char *program, int status)
{
    FILE *stream = status == EXIT_SUCCESS ? stdout : stderr;
    fprintf(stream,
            "Usage:\n"
            "  %s S1 [MAX_N]\n"
            "  %s S1 --upto MAX_N\n"
            "  %s S1 --term N\n"
            "  %s S1 --check [MAX_N]\n"
            "\n"
            "S1=2, 3, 4 gives A093313, A093314, A093315.\n"
            "S1=_ allows any first value and gives A067957.\n"
            "Option: --quiet\n",
            program, program, program, program);
    exit(status);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(argv[0], EXIT_SUCCESS);
    }
    if (argc < 2) {
        usage(argv[0], EXIT_FAILURE);
    }

    const bool unrestricted = strcmp(argv[1], "_") == 0;
    const unsigned first = unrestricted ? 0U :
        parse_unsigned(argv[1], 1U, MAX_N, "S1");
    const unsigned minimum_n = unrestricted ? 0U : 1U;
    char first_text[16];
    if (unrestricted) {
        strcpy(first_text, "_");
    } else {
        snprintf(first_text, sizeof(first_text), "%u", first);
    }

    unsigned maximum = DEFAULT_MAX_N;
    OutputMode mode = MODE_UPTO;
    bool have_n = false;
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "--term") == 0 ||
                   strcmp(argv[i], "--upto") == 0) {
            if (have_n || i + 1 >= argc) {
                usage(argv[0], EXIT_FAILURE);
            }
            mode = strcmp(argv[i], "--term") == 0 ? MODE_TERM : MODE_UPTO;
            maximum = parse_unsigned(argv[++i], minimum_n, MAX_N, "N");
            have_n = true;
        } else if (strcmp(argv[i], "--check") == 0) {
            if (mode != MODE_UPTO || have_n) {
                usage(argv[0], EXIT_FAILURE);
            }
            mode = MODE_CHECK;
            maximum = known_maximum(first);
            if (maximum == 0U) {
                die("--check has no known terms for this S1");
            }
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                maximum = parse_unsigned(argv[++i], minimum_n, maximum,
                                         "MAX_N");
            }
            have_n = true;
        } else {
            if (have_n || argv[i][0] == '-') {
                usage(argv[0], EXIT_FAILURE);
            }
            maximum = parse_unsigned(argv[i], minimum_n, MAX_N, "MAX_N");
            have_n = true;
        }
    }

    char output_path[64] = {0};
    char part_path[72] = {0};
    FILE *output_file = NULL;
    if (mode != MODE_CHECK) {
        output_file = open_output_file(first, output_path, part_path);
    }

    const unsigned begin = mode == MODE_TERM ? maximum : minimum_n;
    for (unsigned n = begin; n <= maximum; ++n) {
        SearchContext statistics;
        const double started = monotonic_seconds();
        const U128 answer = count_term(n, first, &statistics);
        const double elapsed = monotonic_seconds() - started;
        char answer_text[40];
        u128_to_text(answer, answer_text);

        if (mode == MODE_CHECK) {
            const char *expected_text = known_term(first, n);
            U128 expected;
            if (!parse_u128(expected_text, &expected)) {
                destroy_search_context(&statistics);
                die("invalid built-in known term");
            }
            if (answer != expected) {
                fprintf(stderr,
                        "error: mismatch at s_1=%s, n=%u: got %s, expected %s\n",
                        first_text, n, answer_text, expected_text);
                destroy_search_context(&statistics);
                return EXIT_FAILURE;
            }
        } else {
            printf("%u %s\n", n, answer_text);
            if (fprintf(output_file, "%u %s\n", n, answer_text) < 0 ||
                fflush(output_file) != 0) {
                fprintf(stderr, "error: cannot write %s: %s\n",
                        part_path, strerror(errno));
                destroy_search_context(&statistics);
                return EXIT_FAILURE;
            }
            if (fflush(stdout) != 0) {
                destroy_search_context(&statistics);
                die("cannot flush standard output");
            }
        }

        if (!quiet) {
            fprintf(stderr,
                    "093313_02: s_1=%s n=%u answer=%s states=%" PRIu64
                    " cache=%zu replacements=%" PRIu64 " "
                    "calls=%" PRIu64 " hits=%" PRIu64
                    " branches=%" PRIu64 " dead=%" PRIu64
                    " time=%.3fs%s\n",
                    first_text, n, answer_text, statistics.computed_states,
                    statistics.memo.size, statistics.cache_replacements,
                    statistics.calls, statistics.memo_hits,
                    statistics.branches, statistics.dead_ends, elapsed,
                    mode == MODE_CHECK ? " [OK]" : "");
        }
        destroy_search_context(&statistics);
    }

    if (output_file != NULL) {
        finish_output_file(output_file, part_path, output_path);
        if (!quiet) {
            fprintf(stderr, "saved: %s\n", output_path);
        }
    }
    if (mode == MODE_CHECK && quiet) {
        fprintf(stderr, "s_1=%s: terms %u..%u OK\n",
                first_text, minimum_n, maximum);
    }
    return EXIT_SUCCESS;
}
