/*
 * A067957, A093313, A093314, A093315 -- fixed-memory two-ended search.
 *
 * Count permutations p of [n], optionally with prescribed p(1), such that
 *
 *     p(j) divides Sum_{i=1..j} p(i)       for every j in [n].
 *
 * If the constructed prefix has sum a, its next value x must divide a.
 * If the constructed suffix has sum b and T=n(n+1)/2, its next value y,
 * working from right to left, must divide T-b.  At every state this program
 * branches from the end having fewer viable candidates.  A bounded-depth
 * feasibility probe rejects candidates that immediately lead to an empty
 * domain.  These are necessary conditions only, so the pruning is exact.
 * The deterministic choice of one end partitions the permutations and never
 * counts a permutation twice.
 *
 * Unlike 093313_02.c, this version has no memo table.  It stores only the
 * current recursion path and a fixed-size divisor-mask table.  There is no
 * heap allocation, and the working-memory upper bound is independent of n
 * over the supported range n<=63.  Reaching the same state by another path
 * causes recomputation, trading time for predictable small memory usage.
 *
 * Counts use unsigned 128-bit integers with checked addition.  The default
 * is no speculative lookahead because that is faster in the tested range.
 * --lookahead 1 enables arc-consistency at the two exposed ends; larger
 * values perform deeper necessary-condition probes and may help some terms.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       093313_03.c -o 093313_03
 *
 * Examples:
 *   ./093313_03 2 --upto 44
 *   ./093313_03 4 --term 43
 *   ./093313_03 4 --term 44 --lookahead 2
 *   ./093313_03 _ --upto 30
 *
 * Fixed p(1)=K is saved as b09331(K+1)_03.txt.  Thus K=2,3,4 give
 * b093313_03.txt, b093314_03.txt, b093315_03.txt.  The unrestricted "_"
 * mode is saved as b067957_03.txt.  Files are replaced atomically only after
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
#error "093313_03.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 63U
#define MAX_TOTAL_SUM (MAX_N * (MAX_N + 1U) / 2U)
#define DEFAULT_MAX_N 40U
#define DEFAULT_LOOKAHEAD 0U
#define MAX_LOOKAHEAD 4U
#define PROGRESS_INTERVAL UINT64_C(1000000000)

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} OutputMode;

typedef struct {
    unsigned n;
    unsigned total_sum;
    unsigned lookahead;
    uint64_t full_mask;
    uint64_t divisor_mask[MAX_TOTAL_SUM + 1U];
    uint64_t calls;
    uint64_t completed_states;
    uint64_t branches;
    uint64_t dead_ends;
    uint64_t probe_calls;
    uint64_t probe_prunes;
    uint64_t next_report;
    double started;
} SearchContext;

_Static_assert(sizeof(SearchContext) <= 32768U,
               "fixed search context unexpectedly exceeds 32 KiB");

static bool quiet;
static unsigned requested_lookahead = DEFAULT_LOOKAHEAD;

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

static void increment_saturated(uint64_t *value)
{
    if (*value != UINT64_MAX) {
        ++*value;
    }
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

static void candidate_domains(const SearchContext *context, uint64_t unused,
                              unsigned prefix_sum, unsigned suffix_sum,
                              uint64_t *left, uint64_t *right)
{
    *left = context->divisor_mask[prefix_sum] & unused;
    *right = context->divisor_mask[context->total_sum - suffix_sum] & unused;
}

/* Return true if the state survives DEPTH exact necessary-condition levels. */
static bool probe_viable(SearchContext *context, uint64_t unused,
                         unsigned prefix_sum, unsigned suffix_sum,
                         unsigned depth)
{
    increment_saturated(&context->probe_calls);
    if (unused == 0U) {
        return true;
    }

    uint64_t left;
    uint64_t right;
    candidate_domains(context, unused, prefix_sum, suffix_sum, &left, &right);
    if (left == 0U || right == 0U) {
        return false;
    }
    if (depth <= 1U) {
        return true;
    }

    if (__builtin_popcountll(left) <= __builtin_popcountll(right)) {
        while (left != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(left);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            left &= left - 1U;
            if (probe_viable(context, unused ^ bit_mask,
                             prefix_sum + bit + 1U, suffix_sum,
                             depth - 1U)) {
                return true;
            }
        }
    } else {
        while (right != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(right);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            right &= right - 1U;
            if (probe_viable(context, unused ^ bit_mask,
                             prefix_sum, suffix_sum + bit + 1U,
                             depth - 1U)) {
                return true;
            }
        }
    }
    return false;
}

static uint64_t filter_left(SearchContext *context, uint64_t unused,
                            unsigned prefix_sum, unsigned suffix_sum,
                            uint64_t candidates)
{
    if (context->lookahead == 0U) {
        return candidates;
    }
    uint64_t viable = 0U;
    while (candidates != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(candidates);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        candidates &= candidates - 1U;
        if (probe_viable(context, unused ^ bit_mask,
                         prefix_sum + bit + 1U, suffix_sum,
                         context->lookahead)) {
            viable |= bit_mask;
        } else {
            increment_saturated(&context->probe_prunes);
        }
    }
    return viable;
}

static uint64_t filter_right(SearchContext *context, uint64_t unused,
                             unsigned prefix_sum, unsigned suffix_sum,
                             uint64_t candidates)
{
    if (context->lookahead == 0U) {
        return candidates;
    }
    uint64_t viable = 0U;
    while (candidates != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(candidates);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        candidates &= candidates - 1U;
        if (probe_viable(context, unused ^ bit_mask,
                         prefix_sum, suffix_sum + bit + 1U,
                         context->lookahead)) {
            viable |= bit_mask;
        } else {
            increment_saturated(&context->probe_prunes);
        }
    }
    return viable;
}

static void maybe_report_progress(SearchContext *context)
{
    if (quiet || context->completed_states < context->next_report) {
        return;
    }
    fprintf(stderr,
            "093313_03: n=%u states=%" PRIu64 " calls=%" PRIu64
            " branches=%" PRIu64 " probes=%" PRIu64
            " prunes=%" PRIu64 " memory=fixed time=%.1fs\n",
            context->n, context->completed_states, context->calls,
            context->branches, context->probe_calls, context->probe_prunes,
            monotonic_seconds() - context->started);
    if (context->next_report <= UINT64_MAX - PROGRESS_INTERVAL) {
        context->next_report += PROGRESS_INTERVAL;
    } else {
        context->next_report = UINT64_MAX;
    }
}

static U128 search_completions(SearchContext *context, uint64_t unused,
                               unsigned prefix_sum, unsigned suffix_sum)
{
    increment_saturated(&context->calls);
    if (unused == 0U) {
        return 1U;
    }

    uint64_t left;
    uint64_t right;
    candidate_domains(context, unused, prefix_sum, suffix_sum, &left, &right);

    U128 answer = 0U;
    if (left != 0U && right != 0U) {
        left = filter_left(context, unused, prefix_sum, suffix_sum, left);
        right = filter_right(context, unused, prefix_sum, suffix_sum, right);
    }

    if (left == 0U || right == 0U) {
        increment_saturated(&context->dead_ends);
    } else if (__builtin_popcountll(left) <= __builtin_popcountll(right)) {
        while (left != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(left);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            left &= left - 1U;
            increment_saturated(&context->branches);
            checked_add(&answer, search_completions(
                context, unused ^ bit_mask, prefix_sum + bit + 1U,
                suffix_sum));
        }
    } else {
        while (right != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(right);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            right &= right - 1U;
            increment_saturated(&context->branches);
            checked_add(&answer, search_completions(
                context, unused ^ bit_mask, prefix_sum,
                suffix_sum + bit + 1U));
        }
    }

    increment_saturated(&context->completed_states);
    maybe_report_progress(context);
    return answer;
}

static U128 count_term(unsigned n, unsigned first, SearchContext *context)
{
    memset(context, 0, sizeof(*context));
    context->n = n;
    context->lookahead = requested_lookahead;
    context->started = monotonic_seconds();
    context->next_report = PROGRESS_INTERVAL;

    if (n == 0U) {
        return first == 0U ? 1U : 0U;
    }
    if (first > n) {
        return 0U;
    }

    context->total_sum = n * (n + 1U) / 2U;
    context->full_mask = (UINT64_C(1) << n) - 1U;
    for (unsigned value = 1U; value <= n; ++value) {
        const uint64_t bit = UINT64_C(1) << (value - 1U);
        for (unsigned sum = value; sum <= context->total_sum; sum += value) {
            context->divisor_mask[sum] |= bit;
        }
    }

    U128 answer = 0U;
    if (first != 0U) {
        const uint64_t first_bit = UINT64_C(1) << (first - 1U);
        answer = search_completions(context,
                                    context->full_mask ^ first_bit,
                                    first, 0U);
    } else {
        for (unsigned value = 1U; value <= n; ++value) {
            const uint64_t first_bit = UINT64_C(1) << (value - 1U);
            checked_add(&answer, search_completions(
                context, context->full_mask ^ first_bit, value, 0U));
        }
    }
    return answer;
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
        snprintf(path, 64, "b067957_03.txt") :
        snprintf(path, 64, "b09331%u_03.txt", first + 1U);
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
            "Options: --lookahead 0..%u, --quiet\n"
            "Memory use is fixed; the default lookahead is %u.\n",
            program, program, program, program,
            MAX_LOOKAHEAD, DEFAULT_LOOKAHEAD);
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
    bool have_lookahead = false;
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "--lookahead") == 0) {
            if (have_lookahead || i + 1 >= argc) {
                usage(argv[0], EXIT_FAILURE);
            }
            requested_lookahead = parse_unsigned(
                argv[++i], 0U, MAX_LOOKAHEAD, "lookahead");
            have_lookahead = true;
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
        SearchContext context;
        const double started = monotonic_seconds();
        const U128 answer = count_term(n, first, &context);
        const double elapsed = monotonic_seconds() - started;
        char answer_text[40];
        u128_to_text(answer, answer_text);

        if (mode == MODE_CHECK) {
            const char *expected_text = known_term(first, n);
            U128 expected;
            if (!parse_u128(expected_text, &expected)) {
                die("invalid built-in known term");
            }
            if (answer != expected) {
                fprintf(stderr,
                        "error: mismatch at s_1=%s, n=%u: got %s, expected %s\n",
                        first_text, n, answer_text, expected_text);
                return EXIT_FAILURE;
            }
        } else {
            printf("%u %s\n", n, answer_text);
            if (fprintf(output_file, "%u %s\n", n, answer_text) < 0 ||
                fflush(output_file) != 0) {
                fprintf(stderr, "error: cannot write %s: %s\n",
                        part_path, strerror(errno));
                return EXIT_FAILURE;
            }
            if (fflush(stdout) != 0) {
                die("cannot flush standard output");
            }
        }

        if (!quiet) {
            fprintf(stderr,
                    "093313_03: s_1=%s n=%u answer=%s states=%" PRIu64
                    " calls=%" PRIu64 " branches=%" PRIu64
                    " dead=%" PRIu64 " probes=%" PRIu64
                    " prunes=%" PRIu64 " lookahead=%u memory=fixed"
                    " time=%.3fs%s\n",
                    first_text, n, answer_text, context.completed_states,
                    context.calls, context.branches, context.dead_ends,
                    context.probe_calls, context.probe_prunes,
                    context.lookahead, elapsed,
                    mode == MODE_CHECK ? " [OK]" : "");
        }
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
