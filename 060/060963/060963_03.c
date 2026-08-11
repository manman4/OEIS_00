/*
 * A060963 -- algebraic coefficient extraction.
 *
 * Put m=2*n.  For each distance d define
 *
 *   Q_d(x) = sum_(0 <= i < m-d) x_i*x_(i+d).
 *
 * Choosing t*x_i*x_(i+d) from the d-th factor chooses the pair {i,i+d}.
 * There is one factor for each d, so no distance can be used twice.  Thus
 *
 *   A060963(n) = [t^n*x_0*...*x_(m-1)]
 *                product_(d=1)^(m-1) (1+t*Q_d(x)).
 *
 * The t exponent selects n pairs, and the x coefficient requires every
 * position exactly once.  Each pair has the unique representation i<i+d,
 * and the factors are indexed by d, so no extra factors n! or 2^n occur.
 *
 * For a polynomial whose relevant monomials have total x-degree m, Boolean
 * Fourier extraction gives the required multilinear coefficient as
 *
 *   2^(-m) sum_(epsilon in {-1,+1}^m)
 *       (product_i epsilon_i) [t^n] product_d (1+t*Q_d(epsilon)).
 *
 * Indeed, the sign sum keeps monomials having every x exponent odd.  Their
 * total degree is m in m variables, so every exponent must be exactly one.
 * Negating all epsilon_i leaves every Q_d and their product unchanged; the
 * product of the m=2*n signs is also unchanged.  We may therefore fix
 * epsilon_0=+1 and divide the remaining sum by 2^(m-1).
 *
 * The exact path updates all Q_d in Gray order.  The modular path uses the
 * additional reversal symmetry and directly recomputes Q_d only for the
 * chosen orbit representatives.  The coefficient of t^n is obtained by
 * elementary-symmetric polynomial DP.  Terms through n=14 use checked exact
 * signed-128 arithmetic.  Larger n use a 61-bit modulus, the wider 63-bit
 * modulus when that avoids CRT (n=17), or the coprime 61/59-bit pair as
 * required by the upper bound (2*n-1)!!.  Both CRT residues share one sign
 * traversal and one correlation calculation.  Since |Q_d|<=50, precomputed
 * 64-bit Shoup constants replace general modular products in the DP.  The
 * program refuses to start if the available modulus product does not cover
 * the bound.  Modular work is dynamically distributed among threads; no
 * large memo table is used.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       060963_03.c -o 060963_03
 *
 * Usage:
 *   ./060963_03 12
 *   ./060963_03 --term 14 --threads 8
 *   ./060963_03 --check
 * Results are atomically recorded in b060963_03.txt by default.  Use
 * --output FILE to select another b-file or --no-bfile to disable writing.
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdatomic.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "060963_03.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;
__extension__ typedef __int128 I128;

#define MAX_N 25
#define DEFAULT_N 9
#define DEFAULT_CHECK_N 10
#define MAX_THREADS 64
#define EXACT_MAX_N 14
#define MODULUS63 ((UINT64_C(1) << 63) - 1)
#define MODULUS1 ((UINT64_C(1) << 61) - 1)
#define MODULUS2 ((UINT64_C(1) << 59) - 1)
#ifndef MODULAR_CHUNK_BITS
#define MODULAR_CHUNK_BITS 18
#endif

static const char *output_path = "b060963_03.txt";
static bool write_bfile = true;
static int requested_threads = 4;
static bool force_modular = false;

static const char *const known[] = {
    "1", "1", "1", "5", "29", "145", "957", "8397", "85169",
    "944221", "11639417", "160699437", "2430145085",
    "39776366397", "703161838717", "13369111112753",
    "271734091323897", "5876684246433485", "134794262542773569"
};

static _Noreturn void die(const char *message)
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

static int acquire_bfile_lock(void)
{
    const char suffix[] = ".lock";
    const size_t path_length = strlen(output_path);
    if (path_length > SIZE_MAX - sizeof(suffix)) {
        die("b-file lock path is too long");
    }
    char *lock_path = malloc(path_length + sizeof(suffix));
    if (lock_path == NULL) {
        die("cannot allocate b-file lock path");
    }
    memcpy(lock_path, output_path, path_length);
    memcpy(lock_path + path_length, suffix, sizeof(suffix));
    const int fd = open(lock_path, O_RDWR | O_CREAT, 0666);
    free(lock_path);
    if (fd < 0) {
        die("cannot open b-file lock");
    }
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    int result;
    do {
        result = fcntl(fd, F_SETLKW, &lock);
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        close(fd);
        die("cannot lock b-file");
    }
    return fd;
}

static void release_bfile_lock(int fd)
{
    struct flock lock = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    const bool failed = fcntl(fd, F_SETLK, &lock) != 0;
    const bool close_failed = close(fd) != 0;
    if (failed || close_failed) {
        die("cannot release b-file lock");
    }
}

static void sync_parent_directory(const char *path)
{
    const char *slash = strrchr(path, '/');
    char *allocated = NULL;
    const char *directory;
    if (slash == NULL) {
        directory = ".";
    } else if (slash == path) {
        directory = "/";
    } else {
        const size_t length = (size_t)(slash - path);
        allocated = malloc(length + 1);
        if (allocated == NULL) {
            die("cannot allocate b-file directory path");
        }
        memcpy(allocated, path, length);
        allocated[length] = '\0';
        directory = allocated;
    }
    const int fd = open(directory, O_RDONLY);
    free(allocated);
    if (fd < 0) {
        die("cannot open b-file parent directory");
    }
    const int result = fsync(fd);
    const int saved_errno = errno;
    const bool close_failed = close(fd) != 0;
    if ((result != 0 && saved_errno != EINVAL && saved_errno != ENOTSUP) ||
        close_failed) {
        die("cannot synchronize b-file parent directory");
    }
}

static void store_bfile_term(int n, U128 value)
{
    const int lock_fd = acquire_bfile_lock();
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
        release_bfile_lock(lock_fd);
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
    sync_parent_directory(output_path);
    free(temporary);
    release_bfile_lock(lock_fd);
    fprintf(stderr, "060963_03: recorded computed term n=%d in %s\n",
            n, output_path);
}

static uint64_t gray_code(uint64_t index)
{
    return index ^ (index >> 1);
}

static uint64_t normalized_reflection(uint64_t negative,
                                      unsigned positions)
{
    uint64_t reflected = 0;
    for (unsigned position = 0; position < positions; ++position) {
        reflected = (reflected << 1) | ((negative >> position) & 1U);
    }
    /* Restore the epsilon_0=+1 convention by a global sign change. */
    if ((reflected & 1U) != 0) {
        reflected ^= (UINT64_C(1) << positions) - 1;
    }
    return reflected;
}

static void initial_correlations(unsigned positions, uint64_t negative,
                                 int edges[2 * MAX_N])
{
    /* edges[d] = Q_d(epsilon): agreements contribute +1, differences -1. */
    for (unsigned distance = 1; distance < positions; ++distance) {
        const uint64_t differing = negative ^ (negative >> distance);
        const uint64_t mask =
            (UINT64_C(1) << (positions - distance)) - 1;
        const unsigned disagreements =
            (unsigned)__builtin_popcountll(differing & mask);
        edges[distance] = (int)(positions - distance) -
                          2 * (int)disagreements;
    }
}

static void advance_correlations(unsigned positions, uint64_t *negative,
                                 unsigned changed,
                                 int edges[2 * MAX_N])
{
    const unsigned changed_sign =
        (unsigned)((*negative >> changed) & 1U);
    for (unsigned other = 0; other < positions; ++other) {
        if (other == changed) {
            continue;
        }
        const unsigned other_sign =
            (unsigned)((*negative >> other) & 1U);
        const unsigned distance = changed > other ?
            changed - other : other - changed;
        edges[distance] += changed_sign == other_sign ? -2 : 2;
    }
    *negative ^= UINT64_C(1) << changed;
}

typedef struct {
    int n;
    unsigned positions;
    uint64_t begin;
    uint64_t end;
    U128 positive;
    U128 negative;
    int failed;
} ExactWorker;

static void *exact_worker_main(void *argument)
{
    ExactWorker *worker = argument;
    uint64_t signs = gray_code(worker->begin) << 1;
    int edges[2 * MAX_N];
    initial_correlations(worker->positions, signs, edges);

    for (uint64_t index = worker->begin; index < worker->end; ++index) {
        /* Descending updates compute [t^k] product_d (1+t*edges[d]). */
        I128 coefficient[MAX_N + 1] = {0};
        coefficient[0] = 1;
        unsigned factors = 0;
        for (unsigned distance = 1;
             distance < worker->positions; ++distance) {
            ++factors;
            unsigned degree = factors < (unsigned)worker->n ?
                              factors : (unsigned)worker->n;
            const unsigned remaining =
                worker->positions - 1U - factors;
            const unsigned minimum_degree =
                (unsigned)worker->n > remaining ?
                (unsigned)worker->n - remaining : 1U;
            /* Lower degrees can no longer reach degree n with the factors left. */
            for (;; --degree) {
                I128 product;
                if (__builtin_mul_overflow(coefficient[degree - 1],
                                           (I128)edges[distance], &product) ||
                    __builtin_add_overflow(coefficient[degree], product,
                                           &coefficient[degree])) {
                    worker->failed = 1;
                    return NULL;
                }
                if (degree == minimum_degree) {
                    break;
                }
            }
        }

        /* parity(gray(index)) = parity(index), hence product epsilon_i=(-1)^index. */
        I128 signed_value = coefficient[worker->n];
        if ((index & 1) != 0) {
            signed_value = -signed_value;
        }
        U128 magnitude;
        U128 *destination;
        if (signed_value < 0) {
            magnitude = (U128)(-signed_value);
            destination = &worker->negative;
        } else {
            magnitude = (U128)signed_value;
            destination = &worker->positive;
        }
        if (!add_u128(destination, magnitude)) {
            worker->failed = 1;
            return NULL;
        }

        if (index + 1 < worker->end) {
            const unsigned changed =
                (unsigned)__builtin_ctzll(index + 1) + 1;
            advance_correlations(worker->positions, &signs,
                                 changed, edges);
        }
    }
    return NULL;
}

static U128 exact_pass(int n, int threads)
{
    const unsigned positions = 2U * (unsigned)n;
    const uint64_t evaluations = UINT64_C(1) << (positions - 1);
    if ((uint64_t)threads > evaluations) {
        threads = (int)evaluations;
    }
    ExactWorker *workers = calloc((size_t)threads, sizeof(*workers));
    pthread_t *ids = calloc((size_t)threads, sizeof(*ids));
    if (workers == NULL || ids == NULL) {
        free(workers);
        free(ids);
        die("cannot allocate algebraic workers");
    }
    for (int id = 0; id < threads; ++id) {
        workers[id].n = n;
        workers[id].positions = positions;
        workers[id].begin = evaluations * (uint64_t)id /
                            (uint64_t)threads;
        workers[id].end = evaluations * (uint64_t)(id + 1) /
                          (uint64_t)threads;
        const int error = pthread_create(&ids[id], NULL,
                                         exact_worker_main, &workers[id]);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }
    U128 positive = 0, negative = 0;
    for (int id = 0; id < threads; ++id) {
        const int error = pthread_join(ids[id], NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
        if (workers[id].failed ||
            !add_u128(&positive, workers[id].positive) ||
            !add_u128(&negative, workers[id].negative)) {
            die("exact algebraic accumulator overflow");
        }
    }
    free(workers);
    free(ids);
    if (positive < negative) {
        die("negative algebraic result");
    }
    const U128 numerator = positive - negative;
    const U128 divisor = (U128)1 << (positions - 1);
    if (numerator % divisor != 0) {
        die("algebraic result is not exactly divisible");
    }
    return numerator / divisor;
}

static uint64_t mersenne_reduce(U128 value, unsigned bits,
                                uint64_t modulus)
{
    value = (value & modulus) + (value >> bits);
    value = (value & modulus) + (value >> bits);
    uint64_t result = (uint64_t)value;
    if (result >= modulus) {
        result -= modulus;
    }
    return result;
}

static uint64_t mod_add(uint64_t left, uint64_t right, uint64_t modulus)
{
    const uint64_t sum = left + right;
    return sum >= modulus ? sum - modulus : sum;
}

static uint64_t mod_sub(uint64_t left, uint64_t right, uint64_t modulus)
{
    return left >= right ? left - right : modulus - (right - left);
}

static uint64_t gcd_u64(uint64_t left, uint64_t right)
{
    while (right != 0) {
        const uint64_t remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

typedef struct {
    _Atomic uint64_t next;
    uint64_t end;
    uint64_t chunk;
} ModularSchedule;

typedef struct {
    int n;
    unsigned positions;
    unsigned passes;
    uint64_t modulus[2];
    uint64_t shoup[2][2 * MAX_N + 1];
    ModularSchedule *schedule;
    uint64_t sum[2];
} ModularWorker;

static uint64_t mod_multiply_small(uint64_t value, unsigned multiplier,
                                   uint64_t modulus,
                                   const uint64_t shoup[2 * MAX_N + 1])
{
    /*
     * q=floor(value*floor(multiplier*2^64/modulus)/2^64) is at most one
     * below the exact quotient.  Thus the remainder is below 2*modulus and,
     * because modulus<2^63, fits in uint64_t.  Unsigned wrap in the two low
     * products cancels in their subtraction and leaves that exact remainder.
     */
    const uint64_t quotient =
        (uint64_t)(((U128)value * shoup[multiplier]) >> 64);
    uint64_t remainder = value * multiplier - quotient * modulus;
    if (remainder >= modulus) {
        remainder -= modulus;
    }
    return remainder;
}

static uint64_t modular_coefficient(int n, unsigned positions,
                                    const int edges[2 * MAX_N],
                                    unsigned total_factors,
                                    uint64_t modulus,
                                    const uint64_t shoup[2 * MAX_N + 1])
{
    uint64_t coefficient[MAX_N + 1] = {0};
    coefficient[0] = 1;
    unsigned factors = 0;
    for (unsigned distance = 1; distance < positions; ++distance) {
        const int edge = edges[distance];
        if (edge == 0) {
            continue;           /* 1+t*0 is the identity factor. */
        }
        ++factors;
        unsigned degree = factors < (unsigned)n ? factors : (unsigned)n;
        const unsigned remaining = total_factors - factors;
        const unsigned minimum_degree = (unsigned)n > remaining ?
                                        (unsigned)n - remaining : 1U;
        if (degree < minimum_degree) {
            continue;           /* Fewer than n nonzero factors. */
        }
        /* Discard degrees unable to reach n with the factors left. */
        for (;; --degree) {
            const unsigned magnitude =
                (unsigned)(edge < 0 ? -edge : edge);
            const uint64_t product = magnitude == 1 ?
                coefficient[degree - 1] : mod_multiply_small(
                    coefficient[degree - 1], magnitude, modulus, shoup);
            coefficient[degree] = edge < 0 ?
                mod_sub(coefficient[degree], product, modulus) :
                mod_add(coefficient[degree], product, modulus);
            if (degree == minimum_degree) {
                break;
            }
        }
    }
    return coefficient[n];
}

static void modular_coefficient_pair(int n, unsigned positions,
                                     const int edges[2 * MAX_N],
                                     unsigned total_factors,
                                     const uint64_t modulus[2],
                                     const uint64_t shoup[2][2 * MAX_N + 1],
                                     uint64_t result[2])
{
    uint64_t coefficient[2][MAX_N + 1] = {{0}};
    coefficient[0][0] = 1;
    coefficient[1][0] = 1;
    unsigned factors = 0;
    for (unsigned distance = 1; distance < positions; ++distance) {
        const int edge = edges[distance];
        if (edge == 0) {
            continue;
        }
        ++factors;
        unsigned degree = factors < (unsigned)n ? factors : (unsigned)n;
        const unsigned remaining = total_factors - factors;
        const unsigned minimum_degree = (unsigned)n > remaining ?
                                        (unsigned)n - remaining : 1U;
        if (degree < minimum_degree) {
            continue;
        }
        for (;; --degree) {
            const unsigned magnitude =
                (unsigned)(edge < 0 ? -edge : edge);
            for (unsigned pass = 0; pass < 2; ++pass) {
                const uint64_t product = magnitude == 1 ?
                    coefficient[pass][degree - 1] : mod_multiply_small(
                        coefficient[pass][degree - 1], magnitude,
                        modulus[pass], shoup[pass]);
                coefficient[pass][degree] = edge < 0 ?
                    mod_sub(coefficient[pass][degree], product,
                            modulus[pass]) :
                    mod_add(coefficient[pass][degree], product,
                            modulus[pass]);
            }
            if (degree == minimum_degree) {
                break;
            }
        }
    }
    result[0] = coefficient[0][n];
    result[1] = coefficient[1][n];
}

static void *modular_worker_main(void *argument)
{
    ModularWorker *worker = argument;
    if (worker->passes < 1 || worker->passes > 2) {
        die("worker modular pass count must be one or two");
    }
    for (;;) {
        const uint64_t begin = atomic_fetch_add_explicit(
            &worker->schedule->next, worker->schedule->chunk,
            memory_order_relaxed);
        if (begin >= worker->schedule->end) {
            break;
        }
        uint64_t end = begin + worker->schedule->chunk;
        if (end > worker->schedule->end) {
            end = worker->schedule->end;
        }
        for (uint64_t index = begin; index < end; ++index) {
            const uint64_t signs = gray_code(index) << 1;
            /*
             * Reversal preserves every Q_d.  Normalize the reversed signs
             * back to epsilon_0=+1 by global negation, which also preserves
             * the summand because positions=2*n is even.  Evaluate one
             * representative of each two-element orbit with weight 2;
             * fixed points have weight 1.
             */
            const uint64_t reflected =
                normalized_reflection(signs, worker->positions);
            if (signs <= reflected) {
                const unsigned orbit_weight = signs == reflected ? 1U : 2U;
                int edges[2 * MAX_N];
                initial_correlations(worker->positions, signs, edges);
                unsigned total_factors = 0;
                for (unsigned distance = 1;
                     distance < worker->positions; ++distance) {
                    const int terms = (int)(worker->positions - distance);
                    if (edges[distance] < -terms ||
                        edges[distance] > terms ||
                        ((edges[distance] + terms) & 1) != 0) {
                        die("correlation invariant failed");
                    }
                    total_factors += edges[distance] != 0;
                }
                uint64_t term[2] = {0, 0};
                if (worker->passes == 1) {
                    term[0] = modular_coefficient(
                        worker->n, worker->positions, edges, total_factors,
                        worker->modulus[0], worker->shoup[0]);
                } else {
                    modular_coefficient_pair(
                        worker->n, worker->positions, edges, total_factors,
                        worker->modulus, worker->shoup, term);
                }
                for (unsigned pass = 0; pass < worker->passes; ++pass) {
                    if (orbit_weight == 2U) {
                        term[pass] = mod_add(term[pass], term[pass],
                                             worker->modulus[pass]);
                    }
                    /* parity(gray(index))=parity(index)=product of signs. */
                    worker->sum[pass] = (index & 1) != 0 ?
                        mod_sub(worker->sum[pass], term[pass],
                                worker->modulus[pass]) :
                        mod_add(worker->sum[pass], term[pass],
                                worker->modulus[pass]);
                }
            }
        }
    }
    return NULL;
}

static void modular_passes(int n, int threads, unsigned passes,
                           const uint64_t modulus[2],
                           uint64_t residues[2])
{
    if (passes < 1 || passes > 2) {
        die("modular pass count must be one or two");
    }
    const unsigned positions = 2U * (unsigned)n;
    if (positions == 0 || positions > 2U * MAX_N || positions >= 64) {
        die("position count is outside modular implementation limits");
    }
    for (unsigned pass = 0; pass < passes; ++pass) {
        if (modulus[pass] <= 1 || (modulus[pass] & 1) == 0 ||
            modulus[pass] >= (UINT64_C(1) << 63)) {
            die("modulus violates Shoup arithmetic preconditions");
        }
    }
    if (passes == 2 && gcd_u64(modulus[0], modulus[1]) != 1) {
        die("CRT moduli are not coprime");
    }
    const uint64_t evaluations = UINT64_C(1) << (positions - 1);
    ModularWorker *workers = calloc((size_t)threads, sizeof(*workers));
    pthread_t *ids = calloc((size_t)threads, sizeof(*ids));
    if (workers == NULL || ids == NULL) {
        free(workers);
        free(ids);
        die("cannot allocate modular workers");
    }
    ModularSchedule schedule = {
        .next = 0,
        .end = evaluations,
        .chunk = UINT64_C(1) << MODULAR_CHUNK_BITS
    };
    for (int id = 0; id < threads; ++id) {
        workers[id].n = n;
        workers[id].positions = positions;
        workers[id].passes = passes;
        for (unsigned pass = 0; pass < passes; ++pass) {
            workers[id].modulus[pass] = modulus[pass];
            for (unsigned multiplier = 0; multiplier <= 2 * MAX_N;
                 ++multiplier) {
                workers[id].shoup[pass][multiplier] = (uint64_t)(
                    ((U128)multiplier << 64) / modulus[pass]);
            }
        }
        workers[id].schedule = &schedule;
        const int error = pthread_create(&ids[id], NULL,
                                         modular_worker_main, &workers[id]);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }
    residues[0] = 0;
    residues[1] = 0;
    for (int id = 0; id < threads; ++id) {
        const int error = pthread_join(ids[id], NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
        for (unsigned pass = 0; pass < passes; ++pass) {
            residues[pass] = mod_add(residues[pass],
                                     workers[id].sum[pass], modulus[pass]);
        }
    }
    free(workers);
    free(ids);
    /*
     * Divide by 2^(m-1) in an odd modulus.  If a residue is odd, adding the
     * modulus gives the even representative; integer division then applies
     * the modular inverse of 2.
     */
    for (unsigned pass = 0; pass < passes; ++pass) {
        for (unsigned i = 0; i < positions - 1; ++i) {
            if ((residues[pass] & 1) != 0) {
                residues[pass] += modulus[pass];
            }
            residues[pass] /= 2;
        }
    }
}

static U128 total_pairings(int n)
{
    /* Every admissible object is one of all (2*n-1)!! pairings of 2*n points. */
    U128 result = 1;
    for (unsigned odd = 1; odd < 2U * (unsigned)n; odd += 2) {
        const U128 old = result;
        result *= odd;
        if (result / odd != old) {
            die("pairing bound overflow");
        }
    }
    return result;
}

static U128 crt_pair(uint64_t residue1, uint64_t residue2)
{
    /*
     * MODULUS1 = 2^61-1 is 3 modulo MODULUS2 = 2^59-1.  The moduli are
     * coprime because gcd(2^61-1,2^59-1)=2^gcd(61,59)-1=1.  Write the answer
     * as residue1 + MODULUS1*k and solve
     *
     *   3*k = residue2-residue1 (mod MODULUS2).
     *
     * Since 2*MODULUS2+1 is divisible by 3, it represents inverse(3).
     */
    const uint64_t residue1_mod2 = residue1 % MODULUS2;
    const uint64_t difference =
        mod_sub(residue2, residue1_mod2, MODULUS2);
    const uint64_t inverse3 = (uint64_t)(((U128)2 * MODULUS2 + 1) / 3);
    const uint64_t multiplier = mersenne_reduce(
        (U128)difference * inverse3, 59, MODULUS2);
    return (U128)residue1 + (U128)MODULUS1 * multiplier;
}

static U128 a060963(int n)
{
    if (n == 0) {
        return 1;
    }
    const unsigned positions = 2U * (unsigned)n;
    const uint64_t evaluations = UINT64_C(1) << (positions - 1);
    int threads = requested_threads;
    if ((uint64_t)threads > evaluations) {
        threads = (int)evaluations;
    }
    const double started = now_seconds();
    U128 answer;
    if (n <= EXACT_MAX_N && !force_modular) {
        answer = exact_pass(n, threads);
        fprintf(stderr,
                "060963_03: n=%d, exact algebraic, evaluations=%" PRIu64
                ", threads=%d, %.3f s\n",
                n, evaluations, threads, now_seconds() - started);
    } else {
        const U128 bound = total_pairings(n);
        const U128 modulus_product = (U128)MODULUS1 * MODULUS2;
        uint64_t modulus[2];
        unsigned passes;
        /*
         * A nonnegative integer below a modulus is determined by one residue;
         * below their product it is determined by the CRT pair.  Strict '<'
         * is essential: equality would have residue zero and be ambiguous.
         */
        if (bound < MODULUS1) {
            passes = 1;
            modulus[0] = MODULUS1;
        } else if (bound < MODULUS63) {
            /* 33!! fits in 2^63-1, so n=17 needs only one wider pass. */
            passes = 1;
            modulus[0] = MODULUS63;
        } else if (bound < modulus_product) {
            passes = 2;
            modulus[0] = MODULUS1;
            modulus[1] = MODULUS2;
        } else {
            die("available CRT moduli do not exceed the pairing upper bound");
        }

        uint64_t residues[2];
        modular_passes(n, threads, passes, modulus, residues);
        if (passes == 1) {
            answer = residues[0];
        } else {
            answer = crt_pair(residues[0], residues[1]);
        }
        if (answer > bound) {
            die("CRT result exceeds total number of pairings");
        }
        fprintf(stderr,
                "060963_03: n=%d, modular algebraic, evaluations=%" PRIu64
                " x%u, threads=%d, %.3f s\n",
                n, evaluations, passes, threads, now_seconds() - started);
    }
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

static void verify_modular_paths(void)
{
    if (!(total_pairings(16) < MODULUS1 &&
          total_pairings(17) >= MODULUS1 &&
          total_pairings(17) < MODULUS63 &&
          total_pairings(18) >= MODULUS63 &&
          total_pairings(MAX_N) < (U128)MODULUS1 * MODULUS2)) {
        die("unexpected modular-pass bound thresholds");
    }
    const uint64_t test_modulus[3] = {MODULUS63, MODULUS1, MODULUS2};
    const unsigned test_bits[3] = {63, 61, 59};
    for (unsigned which = 0; which < 3; ++which) {
        uint64_t shoup[2 * MAX_N + 1];
        for (unsigned multiplier = 0; multiplier <= 2 * MAX_N;
             ++multiplier) {
            shoup[multiplier] = (uint64_t)(
                ((U128)multiplier << 64) / test_modulus[which]);
        }
        uint64_t state = UINT64_C(0x9e3779b97f4a7c15) ^
                         test_modulus[which];
        for (unsigned multiplier = 0; multiplier <= 2 * MAX_N;
             ++multiplier) {
            for (unsigned test = 0; test < 1000; ++test) {
                state = state * UINT64_C(6364136223846793005) + 1;
                const uint64_t value = state % test_modulus[which];
                const uint64_t expected = mersenne_reduce(
                    (U128)value * multiplier, test_bits[which],
                    test_modulus[which]);
                if (mod_multiply_small(value, multiplier,
                                       test_modulus[which], shoup) !=
                    expected) {
                    die("64-bit Shoup multiplication self-test failed");
                }
            }
        }
    }
    const int n = DEFAULT_CHECK_N;
    const uint64_t wide_modulus[2] = {MODULUS63, 0};
    uint64_t residues[2];
    modular_passes(n, requested_threads, 1, wide_modulus, residues);
    verify_known(n, residues[0]);

    const uint64_t crt_modulus[2] = {MODULUS1, MODULUS2};
    modular_passes(n, requested_threads, 2, crt_modulus, residues);
    verify_known(n, crt_pair(residues[0], residues[1]));
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--threads T] [--output FILE] [--modular]\n"
            "       %s --term N [--threads T] [--output FILE] [--modular]\n"
            "       %s --check [--threads T] [--no-bfile] [--modular]\n"
            "N must be in 0..%d; T must be in 1..%d.\n",
            program, program, program, MAX_N, MAX_THREADS);
}

int main(int argc, char **argv)
{
    enum { MODE_RANGE, MODE_TERM, MODE_CHECK } mode = MODE_RANGE;
    int maximum = -1;
    bool have_mode = false;
    bool have_threads = false;
    bool have_modular = false;
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
        } else if (!strcmp(argv[i], "--modular")) {
            if (have_modular) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            force_modular = true;
            have_modular = true;
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
        verify_modular_paths();
        printf("ok: A060963 terms n=0..%d and all modular paths verified\n",
               DEFAULT_CHECK_N);
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
