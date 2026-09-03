/*
 * A093323 -- triangle of divisor chains, read by rows.
 *
 * Row r, column k is the number of permutations p of [r] with p(1)=k
 * such that every term divides the sum of all preceding terms.  Equivalently,
 *
 *     p(j) divides Sum_{i=1..j} p(i)       for every j in [r],
 *
 * because p(j) divides itself.  This program applies the fixed-memory,
 * cache-free two-ended search from 093313_03.c separately to every k=1..r.
 *
 * The triangle is flattened in row-major order.  Entry (r,k) has b-file
 * index r(r-1)/2+k.  Thus an input N computes all rows 1 through N and saves
 * them atomically as b093323_01.txt.
 *
 * Search storage is a fixed divisor-mask table plus a recursion stack of at
 * most 62 levels.  It does not grow with the number of visited states.
 * Subsets use uint64_t, so N<=63.  Counts use unsigned 128-bit integers and
 * every count addition is checked.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       093323_01.c -o 093323_01
 *
 * Examples:
 *   ./093323_01 26
 *   ./093323_01 30 --quiet
 *   ./093323_01 --check 26
 */

#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(__SIZEOF_INT128__)
#error "093323_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 63U
#define MAX_TOTAL_SUM (MAX_N * (MAX_N + 1U) / 2U)
#define KNOWN_MAX_ROW 26U
#define PROGRESS_INTERVAL UINT64_C(1000000000)

typedef struct {
    unsigned n;
    unsigned first;
    unsigned total_sum;
    uint64_t full_mask;
    uint64_t divisor_mask[MAX_TOTAL_SUM + 1U];
    uint64_t calls;
    uint64_t completed_states;
    uint64_t branches;
    uint64_t dead_ends;
    uint64_t next_report;
    double started;
} SearchContext;

_Static_assert(sizeof(SearchContext) <= 32768U,
               "fixed search context unexpectedly exceeds 32 KiB");

static bool quiet;

/* Rows 1..26 supplied in the A093323 entry. */
static const char known_triangle[] =
    "1\n"
    "0 1\n"
    "0 1 1\n"
    "0 0 1 1\n"
    "0 0 1 2 1\n"
    "0 0 0 2 2 1\n"
    "0 0 0 2 3 1 1\n"
    "0 0 0 0 1 0 1 5\n"
    "0 0 0 0 1 3 4 12 4\n"
    "0 0 0 0 0 4 5 7 3 3\n"
    "0 0 0 0 0 4 7 9 3 4 2\n"
    "0 0 0 1 0 0 2 5 4 8 11 8\n"
    "0 0 0 1 0 0 2 7 11 12 19 11 4\n"
    "0 0 0 0 0 0 0 4 12 4 14 7 8 6\n"
    "0 0 0 1 0 2 3 14 32 42 64 41 77 63 47\n"
    "0 0 0 1 0 0 0 0 16 34 39 26 20 24 31 44\n"
    "0 0 0 1 0 0 0 0 16 44 55 27 34 31 42 56 6\n"
    "0 0 0 0 0 2 3 2 2 21 13 20 19 31 51 70 76 37\n"
    "0 0 0 0 0 4 3 3 7 21 17 24 25 34 54 91 113 49 6\n"
    "0 0 0 0 0 2 0 8 17 12 31 41 43 91 60 121 223 144 360 166\n"
    "0 0 0 0 0 7 0 20 31 26 57 197 314 383 283 706 938 473 969 454 462\n"
    "0 0 0 0 0 6 0 17 18 21 0 124 131 220 148 445 538 232 443 222 423 232\n"
    "0 0 0 0 0 6 0 17 22 29 9 138 164 279 188 520 640 309 616 302 521 357 372\n"
    "0 0 0 6 0 6 0 44 76 219 86 155 314 545 389 1354 1296 819 727 1246 1959 2619 6247 2130\n"
    "0 0 0 8 0 11 7 60 112 257 102 273 323 1519 579 2388 2828 1600 2193 2535 3532 3955 9554 3155 1589\n"
    "0 0 0 3 5 14 15 80 53 139 34 556 453 3063 1160 3194 1739 1756 2015 3648 5311 2903 7496 4084 6061 9093\n";

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

static bool next_known_value(const char **cursor, U128 *result)
{
    const U128 maximum = ~(U128)0;
    const unsigned char *text = (const unsigned char *)*cursor;
    while (*text != '\0' && isspace(*text)) {
        ++text;
    }
    if (!isdigit(*text)) {
        return false;
    }

    U128 value = 0U;
    while (isdigit(*text)) {
        const unsigned digit = (unsigned)(*text - (unsigned char)'0');
        if (value > (maximum - digit) / 10U) {
            die("known value exceeds unsigned 128-bit range");
        }
        value = value * 10U + digit;
        ++text;
    }
    *cursor = (const char *)text;
    *result = value;
    return true;
}

static void prepare_row(SearchContext *context, unsigned n)
{
    memset(context, 0, sizeof(*context));
    context->n = n;
    context->total_sum = n * (n + 1U) / 2U;
    context->full_mask = (UINT64_C(1) << n) - 1U;
    for (unsigned value = 1U; value <= n; ++value) {
        const uint64_t bit = UINT64_C(1) << (value - 1U);
        for (unsigned sum = value; sum <= context->total_sum; sum += value) {
            context->divisor_mask[sum] |= bit;
        }
    }
}

static void maybe_report_progress(SearchContext *context)
{
    if (quiet || context->completed_states < context->next_report) {
        return;
    }
    fprintf(stderr,
            "093323_01: row=%u column=%u states=%" PRIu64
            " calls=%" PRIu64 " branches=%" PRIu64 " time=%.1fs\n",
            context->n, context->first, context->completed_states,
            context->calls, context->branches,
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

    uint64_t left = context->divisor_mask[prefix_sum] & unused;
    uint64_t right =
        context->divisor_mask[context->total_sum - suffix_sum] & unused;
    U128 answer = 0U;

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

static U128 count_column(SearchContext *context, unsigned first)
{
    context->first = first;
    context->calls = 0U;
    context->completed_states = 0U;
    context->branches = 0U;
    context->dead_ends = 0U;
    context->next_report = PROGRESS_INTERVAL;
    context->started = monotonic_seconds();

    const uint64_t first_bit = UINT64_C(1) << (first - 1U);
    return search_completions(context, context->full_mask ^ first_bit,
                              first, 0U);
}

static FILE *open_output_file(const char *part_path)
{
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
            "  %s MAX_ROW [--quiet]\n"
            "  %s --check [MAX_ROW] [--quiet]\n"
            "\n"
            "A normal run saves rows 1..MAX_ROW as b093323_01.txt.\n"
            "--check compares rows with the built-in rows 1..%u.\n",
            program, program, KNOWN_MAX_ROW);
    exit(status);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(argv[0], EXIT_SUCCESS);
    }

    bool check = false;
    bool have_maximum = false;
    unsigned maximum = 0U;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--check") == 0) {
            if (check) {
                usage(argv[0], EXIT_FAILURE);
            }
            check = true;
        } else if (strcmp(argv[i], "--quiet") == 0 ||
                   strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else {
            if (have_maximum || argv[i][0] == '-') {
                usage(argv[0], EXIT_FAILURE);
            }
            maximum = parse_unsigned(argv[i], 1U, MAX_N, "MAX_ROW");
            have_maximum = true;
        }
    }

    if (!have_maximum) {
        if (!check) {
            usage(argv[0], EXIT_FAILURE);
        }
        maximum = KNOWN_MAX_ROW;
    }
    if (check && maximum > KNOWN_MAX_ROW) {
        die("--check exceeds the built-in known rows");
    }

    static const char output_path[] = "b093323_01.txt";
    static const char part_path[] = "b093323_01.txt.part";
    FILE *output = check ? NULL : open_output_file(part_path);
    const char *known_cursor = known_triangle;

    uint64_t index = 1U;
    for (unsigned row = 1U; row <= maximum; ++row) {
        SearchContext context;
        prepare_row(&context, row);
        for (unsigned column = 1U; column <= row; ++column, ++index) {
            const double started = monotonic_seconds();
            const U128 answer = count_column(&context, column);
            const double elapsed = monotonic_seconds() - started;
            char answer_text[40];
            u128_to_text(answer, answer_text);

            if (check) {
                U128 expected;
                if (!next_known_value(&known_cursor, &expected)) {
                    die("built-in triangle ended early");
                }
                if (answer != expected) {
                    char expected_text[40];
                    u128_to_text(expected, expected_text);
                    fprintf(stderr,
                            "error: mismatch at row=%u, column=%u, index=%" PRIu64
                            ": got %s, expected %s\n",
                            row, column, index, answer_text, expected_text);
                    return EXIT_FAILURE;
                }
            } else {
                printf("%" PRIu64 " %s\n", index, answer_text);
                if (fprintf(output, "%" PRIu64 " %s\n",
                            index, answer_text) < 0 || fflush(output) != 0) {
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
                        "093323_01: row=%u column=%u index=%" PRIu64
                        " answer=%s states=%" PRIu64 " calls=%" PRIu64
                        " branches=%" PRIu64 " dead=%" PRIu64
                        " memory=fixed time=%.3fs%s\n",
                        row, column, index, answer_text,
                        context.completed_states, context.calls,
                        context.branches, context.dead_ends, elapsed,
                        check ? " [OK]" : "");
            }
        }
    }

    if (output != NULL) {
        finish_output_file(output, part_path, output_path);
        if (!quiet) {
            fprintf(stderr, "saved: %s\n", output_path);
        }
    } else if (quiet) {
        fprintf(stderr, "A093323: rows 1..%u (%" PRIu64 " terms) OK\n",
                maximum, index - 1U);
    }
    return EXIT_SUCCESS;
}
