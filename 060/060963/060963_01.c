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
 * are memoized in a bounded open-addressed hash table.  Reflection of all
 * positions preserves differences, so a mask and its bit reversal share a
 * cache entry.  Reaching the cache load limit only disables new insertions;
 * it never changes the answer.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic 060963_01.c -o 060963_01
 *
 * Usage:
 *   ./060963_01 12
 *   ./060963_01 --term 16 --cache-mib 1024
 *   ./060963_01 --check
 * Results are atomically recorded in b060963_01.txt by default.  Use
 * --output FILE to select another b-file or --no-bfile to disable writing.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "060963_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 18
#define DEFAULT_N 9
#define DEFAULT_CHECK_N 10
#define DEFAULT_CACHE_MIB 512
#define MAX_CACHE_MIB 16384
#define KEY_BITS_FOR_LIMIT 6

typedef struct {
    uint64_t *keys;
    U128 *values;
    uint64_t capacity;
    uint64_t mask;
    uint64_t count;
    uint64_t insertion_limit;
    bool saturated;
} Memo;

typedef struct {
    int n;
    unsigned positions;
    uint64_t full;
    Memo memo;
    uint64_t calls;
    uint64_t hits;
    uint64_t additions;
} Search;

static const char *output_path = "b060963_01.txt";
static bool write_bfile = true;
static unsigned cache_mib = DEFAULT_CACHE_MIB;

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
    fprintf(stderr, "060963_01: recorded computed term n=%d in %s\n",
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
    const uint64_t bytes_per_slot = sizeof(uint64_t) + sizeof(U128);
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
    fprintf(stderr, "060963_01: cache %.1f MiB, slots=%" PRIu64 "\n",
            (double)(capacity * bytes_per_slot) / 1048576.0, capacity);
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
        const uint64_t present = memo->keys[slot];
        if (present == 0) {
            return false;
        }
        if (present == stored_key) {
            *value = memo->values[slot];
            return true;
        }
        slot = (slot + 1) & memo->mask;
    }
}

static void memo_put(Memo *memo, uint64_t key, U128 value)
{
    if (memo->count >= memo->insertion_limit) {
        memo->saturated = true;
        return;
    }
    const uint64_t stored_key = key + 1;
    uint64_t slot = hash_key(key) & memo->mask;
    while (memo->keys[slot] != 0) {
        if (memo->keys[slot] == stored_key) {
            memo->values[slot] = value;
            return;
        }
        slot = (slot + 1) & memo->mask;
    }
    memo->values[slot] = value;
    memo->keys[slot] = stored_key;
    ++memo->count;
}

static U128 search_state(Search *search, uint64_t occupied, unsigned limit)
{
    ++search->calls;
    if (occupied == search->full) {
        return 1;
    }

    const uint64_t reflected = reverse_bits(occupied, search->positions);
    const uint64_t canonical = occupied < reflected ? occupied : reflected;
    const uint64_t key = (canonical << KEY_BITS_FOR_LIMIT) | limit;
    U128 cached;
    if (memo_get(&search->memo, key, &cached)) {
        ++search->hits;
        return cached;
    }

    const unsigned remaining =
        (search->positions - (unsigned)__builtin_popcountll(occupied)) / 2;
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
    memo_put(&search->memo, key, answer);
    return answer;
}

static U128 a060963(int n)
{
    if (n == 0) {
        return 1;
    }
    Search search;
    memset(&search, 0, sizeof(search));
    search.n = n;
    search.positions = 2U * (unsigned)n;
    search.full = (UINT64_C(1) << search.positions) - 1;
    search.memo = make_memo(cache_mib);
    const double started = now_seconds();
    const U128 answer = search_state(&search, 0, search.positions);
    fprintf(stderr,
            "060963_01: n=%d, states=%" PRIu64 ", hits=%" PRIu64
            ", transitions=%" PRIu64 ", %.3f s%s\n",
            n, search.memo.count, search.hits, search.additions,
            now_seconds() - started,
            search.memo.saturated ? ", cache saturated" : "");
    destroy_memo(&search.memo);
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
            "usage: %s [MAX_N] [--cache-mib M] [--output FILE]\n"
            "       %s --term N [--cache-mib M] [--output FILE]\n"
            "       %s --check [--cache-mib M] [--no-bfile]\n"
            "N must be in 0..%d; M must be in 1..%d.\n",
            program, program, program, MAX_N, MAX_CACHE_MIB);
}

int main(int argc, char **argv)
{
    enum { MODE_RANGE, MODE_TERM, MODE_CHECK } mode = MODE_RANGE;
    int maximum = -1;
    bool have_mode = false;
    bool have_cache = false;
    bool have_output = false;

    for (int i = 1; i < argc; ++i) {
        if (!strcmp(argv[i], "--help") || !strcmp(argv[i], "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        }
        if (!strcmp(argv[i], "--cache-mib")) {
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
