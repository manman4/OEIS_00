/*
 * A093323 -- use the bounded parallel search from 093313_04.c.
 *
 * Row r, column k counts divisor chains of length r beginning with k.
 * This driver computes every (r,k), 1<=k<=r<=N, with the exact parallel
 * split search already audited in 093313_04.c.  The triangle is written in
 * row-major order; (r,k) has b-file index r(r-1)/2+k.
 *
 * 093313_04.c must be in the same directory when this file is compiled.
 * Its main function is renamed below; its fixed-memory count_term function,
 * checked unsigned-128-bit arithmetic, bounded task ring, and worker pool are
 * reused without copying or changing the search algorithm.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       093323_03.c -o 093323_03
 *
 * Examples:
 *   ./093323_03 26
 *   ./093323_03 30 --threads 8 --quiet
 *   ./093323_03 --check 26 --threads 8
 *
 * A successful normal run saves b093323_03.txt atomically.
 */

#ifndef TASKS_PER_THREAD
#define TASKS_PER_THREAD 256U
#endif
#define main a093313_04_original_main
#include "093313_04.c"
#undef main

#include <ctype.h>

#define TRIANGLE_KNOWN_MAX_ROW 26U

static const char triangle_known[] =
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

static bool triangle_next_known(const char **cursor, U128 *result)
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
            die("known A093323 value exceeds unsigned 128-bit range");
        }
        value = value * 10U + digit;
        ++text;
    }
    *cursor = (const char *)text;
    *result = value;
    return true;
}

static FILE *triangle_open_output(const char *part_path)
{
    FILE *stream = fopen(part_path, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        exit(EXIT_FAILURE);
    }
    return stream;
}

static void triangle_finish_output(FILE *stream, const char *part_path,
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

static _Noreturn void triangle_usage(const char *program, int status)
{
    FILE *stream = status == EXIT_SUCCESS ? stdout : stderr;
    fprintf(stream,
            "Usage:\n"
            "  %s MAX_ROW [--threads N] [--quiet]\n"
            "  %s --check [MAX_ROW] [--threads N] [--quiet]\n"
            "\n"
            "A normal run saves rows 1..MAX_ROW as b093323_03.txt.\n"
            "--check compares rows with the built-in rows 1..%u.\n"
            "MAX_ROW range: 1..%u; thread range: 1..%u.\n",
            program, program, TRIANGLE_KNOWN_MAX_ROW, MAX_N, MAX_THREADS);
    exit(status);
}

int main(int argc, char **argv)
{
    requested_threads = default_thread_count();
    quiet = false;

    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        triangle_usage(argv[0], EXIT_SUCCESS);
    }

    bool check = false;
    bool have_maximum = false;
    bool have_threads = false;
    unsigned maximum = 0U;
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--check") == 0) {
            if (check) {
                triangle_usage(argv[0], EXIT_FAILURE);
            }
            check = true;
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (have_threads || i + 1 >= argc) {
                triangle_usage(argv[0], EXIT_FAILURE);
            }
            requested_threads = parse_unsigned(
                argv[++i], 1U, MAX_THREADS, "threads");
            have_threads = true;
        } else if (strcmp(argv[i], "--quiet") == 0 ||
                   strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else {
            if (have_maximum || argv[i][0] == '-') {
                triangle_usage(argv[0], EXIT_FAILURE);
            }
            maximum = parse_unsigned(argv[i], 1U, MAX_N, "MAX_ROW");
            have_maximum = true;
        }
    }

    if (!have_maximum) {
        if (!check) {
            triangle_usage(argv[0], EXIT_FAILURE);
        }
        maximum = TRIANGLE_KNOWN_MAX_ROW;
    }
    if (check && maximum > TRIANGLE_KNOWN_MAX_ROW) {
        die("--check exceeds the built-in known A093323 rows");
    }

    static const char output_path[] = "b093323_03.txt";
    static const char part_path[] = "b093323_03.txt.part";
    FILE *output = check ? NULL : triangle_open_output(part_path);
    const char *known_cursor = triangle_known;
    const double total_started = monotonic_seconds();

    uint64_t index = 1U;
    for (unsigned row = 1U; row <= maximum; ++row) {
        for (unsigned column = 1U; column <= row; ++column, ++index) {
            TermStatistics statistics;
            const double started = monotonic_seconds();
            const U128 answer = count_term(row, column, &statistics);
            const double elapsed = monotonic_seconds() - started;
            char answer_text[40];
            u128_to_text(answer, answer_text);

            if (check) {
                U128 expected;
                if (!triangle_next_known(&known_cursor, &expected)) {
                    die("built-in A093323 triangle ended early");
                }
                if (answer != expected) {
                    char expected_text[40];
                    u128_to_text(expected, expected_text);
                    fprintf(stderr,
                            "error: mismatch at row=%u, column=%u, "
                            "index=%" PRIu64 ": got %s, expected %s\n",
                            row, column, index,
                            answer_text, expected_text);
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
                        "093323_03: row=%u column=%u index=%" PRIu64
                        " answer=%s states=%" PRIu64
                        " branches=%" PRIu64 " tasks=%zu threads=%u"
                        " memory=bounded time=%.3fs%s\n",
                        row, column, index, answer_text,
                        statistics.statistics.states,
                        statistics.statistics.branches, statistics.tasks,
                        statistics.threads, elapsed,
                        check ? " [OK]" : "");
            }
        }
    }

    const double total_elapsed = monotonic_seconds() - total_started;
    if (output != NULL) {
        triangle_finish_output(output, part_path, output_path);
        if (!quiet) {
            fprintf(stderr,
                    "saved: %s; rows=1..%u terms=%" PRIu64
                    " threads=%u time=%.3fs\n",
                    output_path, maximum, index - 1U,
                    requested_threads, total_elapsed);
        }
    } else {
        fprintf(stderr,
                "A093323: rows 1..%u (%" PRIu64 ") OK; "
                "threads=%u time=%.3fs\n",
                maximum, index - 1U,
                requested_threads, total_elapsed);
    }
    return EXIT_SUCCESS;
}
