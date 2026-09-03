/*
 * A093323 -- column-major parallel computation, row-major b-file output.
 *
 * Row r, column k is the number of divisor chains of length r beginning
 * with k.  A divisor chain is a permutation p of [r] such that p(j) divides
 * the sum of the preceding terms.  This is equivalent to p(j) dividing the
 * sum through p(j), because p(j) divides itself.
 *
 * Unlike the row-major 093323_01.c, this program fixes a column k and
 * computes rows k..N consecutively with the cache-free two-ended search from
 * 093313_03.c.  Columns are dynamically distributed among POSIX threads.
 * Results are kept in a fixed 2016-entry array and written in row-major order
 * only after all columns finish.  Entry (r,k) has b-file index
 * r(r-1)/2+k.
 *
 * The divisor masks for every r<=63 are represented by one read-only table;
 * bits above the current r are removed by the unused-value mask.  Explicit
 * data storage and worker stacks have fixed compile-time bounds and do not
 * grow with the number of searched states.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       093323_02.c -o 093323_02
 *
 * Examples:
 *   ./093323_02 26
 *   ./093323_02 35 --threads 8 --quiet
 *   ./093323_02 --check 26
 *
 * A successful normal run saves b093323_02.txt atomically.
 */

#if defined(__APPLE__)
#define _DARWIN_C_SOURCE 1
#endif
#define _POSIX_C_SOURCE 200809L

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if !defined(__SIZEOF_INT128__)
#error "093323_02.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 63U
#define MAX_TOTAL_SUM (MAX_N * (MAX_N + 1U) / 2U)
#define MAX_TERMS (MAX_N * (MAX_N + 1U) / 2U)
#define KNOWN_MAX_ROW 26U
#define MAX_THREADS 64U
#define WORKER_STACK_BYTES (256U * 1024U)
#define PROGRESS_INTERVAL UINT64_C(1000000000)

typedef struct {
    unsigned row;
    unsigned column;
    unsigned total_sum;
    uint64_t full_mask;
    uint64_t calls;
    uint64_t completed_states;
    uint64_t branches;
    uint64_t dead_ends;
    uint64_t next_report;
    double started;
} SearchContext;

typedef struct {
    unsigned id;
} Worker;

static uint64_t divisor_mask[MAX_TOTAL_SUM + 1U];
static U128 triangle[MAX_TERMS];
static unsigned column_order[MAX_N];
static Worker workers[MAX_THREADS];
static pthread_t worker_threads[MAX_THREADS];
static atomic_uint next_column;
static pthread_mutex_t report_mutex;
static unsigned maximum_row;
static unsigned thread_count;
static bool quiet;

_Static_assert(sizeof(triangle) <= 32768U,
               "fixed triangle storage unexpectedly exceeds 32 KiB");

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

static _Noreturn void die_pthread(const char *operation, int error)
{
    fprintf(stderr, "error: %s: %s\n", operation, strerror(error));
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

static unsigned default_thread_count(void)
{
#if defined(__APPLE__)
    unsigned online = 0U;
    size_t size = sizeof(online);
    if (sysctlbyname("hw.logicalcpu", &online, &size, NULL, 0U) != 0 ||
        online == 0U) {
        return 1U;
    }
    return online > MAX_THREADS ? MAX_THREADS : online;
#elif defined(_SC_NPROCESSORS_ONLN)
    const long online = sysconf(_SC_NPROCESSORS_ONLN);
    if (online <= 0L) {
        return 1U;
    }
    if ((unsigned long)online > MAX_THREADS) {
        return MAX_THREADS;
    }
    return (unsigned)online;
#else
    return 1U;
#endif
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

static size_t triangle_slot(unsigned row, unsigned column)
{
    return (size_t)row * (row - 1U) / 2U + column - 1U;
}

static void initialize_divisor_masks(void)
{
    memset(divisor_mask, 0, sizeof(divisor_mask));
    for (unsigned value = 1U; value <= MAX_N; ++value) {
        const uint64_t bit = UINT64_C(1) << (value - 1U);
        for (unsigned sum = value; sum <= MAX_TOTAL_SUM; sum += value) {
            divisor_mask[sum] |= bit;
        }
    }
}

static void report_progress(SearchContext *context)
{
    if (quiet || context->completed_states < context->next_report) {
        return;
    }
    const int error = pthread_mutex_lock(&report_mutex);
    if (error != 0) {
        return;
    }
    fprintf(stderr,
            "093323_02: row=%u column=%u states=%" PRIu64
            " calls=%" PRIu64 " branches=%" PRIu64 " time=%.1fs\n",
            context->row, context->column, context->completed_states,
            context->calls, context->branches,
            monotonic_seconds() - context->started);
    (void)pthread_mutex_unlock(&report_mutex);
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

    uint64_t left = divisor_mask[prefix_sum] & unused;
    uint64_t right = divisor_mask[context->total_sum - suffix_sum] & unused;
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
    report_progress(context);
    return answer;
}

static U128 count_cell(unsigned row, unsigned column, SearchContext *context)
{
    memset(context, 0, sizeof(*context));
    context->row = row;
    context->column = column;
    context->total_sum = row * (row + 1U) / 2U;
    context->full_mask = (UINT64_C(1) << row) - 1U;
    context->next_report = PROGRESS_INTERVAL;
    context->started = monotonic_seconds();
    const uint64_t first_bit = UINT64_C(1) << (column - 1U);
    return search_completions(context, context->full_mask ^ first_bit,
                              column, 0U);
}

static void report_cell(const SearchContext *context, U128 answer)
{
    if (quiet) {
        return;
    }
    char answer_text[40];
    u128_to_text(answer, answer_text);
    const int error = pthread_mutex_lock(&report_mutex);
    if (error != 0) {
        return;
    }
    fprintf(stderr,
            "093323_02: row=%u column=%u answer=%s states=%" PRIu64
            " calls=%" PRIu64 " branches=%" PRIu64 " dead=%" PRIu64
            " time=%.3fs\n",
            context->row, context->column, answer_text,
            context->completed_states, context->calls,
            context->branches, context->dead_ends,
            monotonic_seconds() - context->started);
    (void)pthread_mutex_unlock(&report_mutex);
}

static void *worker_main(void *argument)
{
    const Worker *worker = argument;
    (void)worker;
    for (;;) {
        const unsigned order_index = atomic_fetch_add_explicit(
            &next_column, 1U, memory_order_relaxed);
        if (order_index >= maximum_row) {
            break;
        }
        const unsigned column = column_order[order_index];
        for (unsigned row = column; row <= maximum_row; ++row) {
            SearchContext context;
            const U128 answer = count_cell(row, column, &context);
            triangle[triangle_slot(row, column)] = answer;
            report_cell(&context, answer);
        }
    }
    return NULL;
}

static void initialize_column_order(unsigned maximum)
{
    bool present[MAX_N + 1U] = {false};
    size_t length = 0U;
    static const unsigned priority[] = {4U, 2U, 3U};
    for (size_t i = 0U; i < sizeof(priority) / sizeof(priority[0]); ++i) {
        const unsigned column = priority[i];
        if (column <= maximum) {
            column_order[length++] = column;
            present[column] = true;
        }
    }
    for (unsigned column = 5U; column <= maximum; ++column) {
        column_order[length++] = column;
        present[column] = true;
    }
    for (unsigned column = 1U; column <= maximum; ++column) {
        if (!present[column]) {
            column_order[length++] = column;
        }
    }
    if (length != maximum) {
        die("internal column-order error");
    }
}

static void compute_columns(void)
{
    memset(triangle, 0, sizeof(triangle));
    initialize_column_order(maximum_row);
    atomic_init(&next_column, 0U);

    const unsigned active_threads =
        thread_count < maximum_row ? thread_count : maximum_row;
    pthread_attr_t attributes;
    int error = pthread_attr_init(&attributes);
    if (error != 0) {
        die_pthread("pthread_attr_init", error);
    }
    error = pthread_attr_setstacksize(&attributes, WORKER_STACK_BYTES);
    if (error != 0) {
        (void)pthread_attr_destroy(&attributes);
        die_pthread("pthread_attr_setstacksize", error);
    }

    unsigned created = 0U;
    for (; created < active_threads; ++created) {
        workers[created].id = created;
        error = pthread_create(&worker_threads[created], &attributes,
                               worker_main, &workers[created]);
        if (error != 0) {
            break;
        }
    }
    const int destroy_error = pthread_attr_destroy(&attributes);
    if (destroy_error != 0 && error == 0) {
        error = destroy_error;
    }
    for (unsigned id = 0U; id < created; ++id) {
        const int join_error = pthread_join(worker_threads[id], NULL);
        if (join_error != 0 && error == 0) {
            error = join_error;
        }
    }
    if (error != 0) {
        die_pthread("worker thread operation", error);
    }
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
            "  %s MAX_ROW [--threads N] [--quiet]\n"
            "  %s --check [MAX_ROW] [--threads N] [--quiet]\n"
            "\n"
            "A normal run saves rows 1..MAX_ROW as b093323_02.txt.\n"
            "--check compares rows with the built-in rows 1..%u.\n"
            "Thread range: 1..%u.\n",
            program, program, KNOWN_MAX_ROW, MAX_THREADS);
    exit(status);
}

int main(int argc, char **argv)
{
    thread_count = default_thread_count();
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(argv[0], EXIT_SUCCESS);
    }

    bool check = false;
    bool have_maximum = false;
    bool have_threads = false;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--check") == 0) {
            if (check) {
                usage(argv[0], EXIT_FAILURE);
            }
            check = true;
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (have_threads || i + 1 >= argc) {
                usage(argv[0], EXIT_FAILURE);
            }
            thread_count = parse_unsigned(
                argv[++i], 1U, MAX_THREADS, "threads");
            have_threads = true;
        } else if (strcmp(argv[i], "--quiet") == 0 ||
                   strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else {
            if (have_maximum || argv[i][0] == '-') {
                usage(argv[0], EXIT_FAILURE);
            }
            maximum_row = parse_unsigned(argv[i], 1U, MAX_N, "MAX_ROW");
            have_maximum = true;
        }
    }
    if (!have_maximum) {
        if (!check) {
            usage(argv[0], EXIT_FAILURE);
        }
        maximum_row = KNOWN_MAX_ROW;
    }
    if (check && maximum_row > KNOWN_MAX_ROW) {
        die("--check exceeds the built-in known rows");
    }

    initialize_divisor_masks();
    int error = pthread_mutex_init(&report_mutex, NULL);
    if (error != 0) {
        die_pthread("pthread_mutex_init", error);
    }
    const double started = monotonic_seconds();
    compute_columns();
    const double elapsed = monotonic_seconds() - started;
    error = pthread_mutex_destroy(&report_mutex);
    if (error != 0) {
        die_pthread("pthread_mutex_destroy", error);
    }

    if (check) {
        const char *cursor = known_triangle;
        for (unsigned row = 1U; row <= maximum_row; ++row) {
            for (unsigned column = 1U; column <= row; ++column) {
                U128 expected;
                if (!next_known_value(&cursor, &expected)) {
                    die("built-in triangle ended early");
                }
                const U128 answer = triangle[triangle_slot(row, column)];
                if (answer != expected) {
                    char answer_text[40];
                    char expected_text[40];
                    u128_to_text(answer, answer_text);
                    u128_to_text(expected, expected_text);
                    fprintf(stderr,
                            "error: mismatch at row=%u, column=%u: "
                            "got %s, expected %s\n",
                            row, column, answer_text, expected_text);
                    return EXIT_FAILURE;
                }
            }
        }
        fprintf(stderr,
                "A093323: rows 1..%u (%u terms) OK; "
                "columns=%u threads=%u time=%.3fs\n",
                maximum_row, maximum_row * (maximum_row + 1U) / 2U,
                maximum_row, thread_count, elapsed);
        return EXIT_SUCCESS;
    }

    static const char output_path[] = "b093323_02.txt";
    static const char part_path[] = "b093323_02.txt.part";
    FILE *output = open_output_file(part_path);
    uint64_t index = 1U;
    for (unsigned row = 1U; row <= maximum_row; ++row) {
        for (unsigned column = 1U; column <= row; ++column, ++index) {
            char answer_text[40];
            u128_to_text(triangle[triangle_slot(row, column)], answer_text);
            printf("%" PRIu64 " %s\n", index, answer_text);
            if (fprintf(output, "%" PRIu64 " %s\n",
                        index, answer_text) < 0) {
                fprintf(stderr, "error: cannot write %s: %s\n",
                        part_path, strerror(errno));
                return EXIT_FAILURE;
            }
        }
    }
    if (fflush(stdout) != 0 || fflush(output) != 0) {
        die("cannot flush output");
    }
    finish_output_file(output, part_path, output_path);
    if (!quiet) {
        fprintf(stderr,
                "saved: %s; rows=1..%u terms=%" PRIu64
                " columns=%u threads=%u time=%.3fs\n",
                output_path, maximum_row, index - 1U,
                maximum_row, thread_count, elapsed);
    }
    return EXIT_SUCCESS;
}
