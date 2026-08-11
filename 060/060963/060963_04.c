/*
 * A060963 -- independent ordinary-prime CRT implementation.
 *
 * Put m=2*n and Q_d(x)=sum_(0<=i<m-d) x_i*x_(i+d).  Then
 *
 *   A060963(n) = [t^n*x_0*...*x_(m-1)]
 *                product_(d=1)^(m-1) (1+t*Q_d(x)).
 *
 * Boolean Fourier extraction over x_i in {-1,+1}, with epsilon_0 fixed to
 * +1, gives
 *
 *   2^(-(m-1)) sum_epsilon (product_i epsilon_i)
 *       [t^n] product_d (1+t*Q_d(epsilon)).
 *
 * This file deliberately uses one set of ordinary 37-bit primes, not the
 * Mersenne moduli and specialized CRT of 060963_03.c.  Their product is made
 * greater than the rigorous upper bound (2*n-1)!!, so ordinary incremental
 * CRT reconstructs the answer uniquely.  Comparing this result with
 * 060963_03 gives an independent modular implementation check.
 *
 * Reversal symmetry, zero-factor removal, degree-range pruning, shared prime
 * traversal, and dynamic thread scheduling reduce work without changing the
 * coefficient formula.  The multiplier |Q_d| is at most 50, so checked
 * base-2^48 Shoup constants keep every DP operation within uint64_t.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       060963_04.c -o 060963_04
 *
 * Usage:
 *   ./060963_04 --term 19 --threads 8
 *   ./060963_04 19 --threads 8
 *   ./060963_04 --check --threads 8 --no-bfile
 */

#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "060963_04.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;
__extension__ typedef __int128 I128;

#define MAX_N 25
#define DEFAULT_N 9
#define CHECK_N 10
#define MAX_THREADS 64
#define PRIME_SET_SIZE 3
#define MAX_LANES PRIME_SET_SIZE
#define CHUNK_BITS 18

typedef struct {
    uint64_t modulus;
    uint64_t shoup[2 * MAX_N + 1];
} Prime;

/* Three distinct ordinary primes immediately below 2^37. */
static const uint64_t prime_values[PRIME_SET_SIZE] = {
    UINT64_C(137438953447), UINT64_C(137438953441),
    UINT64_C(137438953427)
};

static const char *const known[] = {
    "1", "1", "1", "5", "29", "145", "957", "8397", "85169",
    "944221", "11639417", "160699437", "2430145085",
    "39776366397", "703161838717", "13369111112753",
    "271734091323897", "5876684246433485", "134794262542773569"
};

static const char *output_path = "b060963_04.txt";
static bool write_bfile = true;
static int requested_threads = 4;

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double now_seconds(void)
{
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) {
        die("clock_gettime failed");
    }
    return (double)value.tv_sec + (double)value.tv_nsec / 1e9;
}

static int parse_integer(const char *text, int minimum, int maximum,
                         const char *name)
{
    char *end = NULL;
    errno = 0;
    const long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                name, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
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

static U128 total_pairings(int n)
{
    U128 result = 1;
    for (unsigned odd = 1; odd < 2U * (unsigned)n; odd += 2) {
        const U128 old = result;
        result *= odd;
        if (result / odd != old) {
            die("pairing upper bound overflow");
        }
    }
    return result;
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
    mode_t mode = 0644;
    struct stat metadata;
    if (stat(output_path, &metadata) == 0) {
        mode = metadata.st_mode & 0777;
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
                die("invalid or unsorted b-file");
            }
            present[index] = true;
            previous = index;
        }
        if (ferror(input) || fclose(input) != 0) {
            die("cannot read existing b-file");
        }
    }
    if (present[n] && values[n] != value) {
        die("computed term disagrees with existing b-file");
    }
    if (present[n]) {
        release_bfile_lock(lock_fd);
        return;
    }
    values[n] = value;
    present[n] = true;

    const size_t path_length = strlen(output_path);
    char *temporary = malloc(path_length + 16);
    if (temporary == NULL) {
        die("cannot allocate temporary path");
    }
    snprintf(temporary, path_length + 16, "%s.tmp.XXXXXX", output_path);
    const int descriptor = mkstemp(temporary);
    if (descriptor < 0 || fchmod(descriptor, mode) != 0) {
        if (descriptor >= 0) {
            close(descriptor);
            unlink(temporary);
        }
        free(temporary);
        die("cannot create temporary b-file");
    }
    FILE *output = fdopen(descriptor, "w");
    if (output == NULL) {
        close(descriptor);
        unlink(temporary);
        free(temporary);
        die("cannot open temporary b-file stream");
    }
    bool failed = false;
    for (int index = 0; index <= MAX_N; ++index) {
        if (present[index]) {
            if (fprintf(output, "%d ", index) < 0 ||
                print_u128(output, values[index]) != 0 ||
                fputc('\n', output) == EOF) {
                failed = true;
                break;
            }
        }
    }
    if (!failed && fflush(output) != 0) {
        failed = true;
    }
    if (!failed && fsync(descriptor) != 0) {
        failed = true;
    }
    if (fclose(output) != 0) {
        failed = true;
    }
    if (failed || rename(temporary, output_path) != 0) {
        unlink(temporary);
        free(temporary);
        die("cannot atomically replace b-file");
    }
    sync_parent_directory(output_path);
    free(temporary);
    release_bfile_lock(lock_fd);
    fprintf(stderr, "060963_04: recorded computed ordinary-CRT term n=%d in %s\n",
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
    if ((reflected & 1U) != 0) {
        reflected ^= (UINT64_C(1) << positions) - 1;
    }
    return reflected;
}

static void correlations(unsigned positions, uint64_t negative,
                         int edges[2 * MAX_N])
{
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

static uint64_t mod_add(uint64_t left, uint64_t right, uint64_t modulus)
{
    const uint64_t sum = left + right;
    return sum >= modulus ? sum - modulus : sum;
}

static uint64_t mod_sub(uint64_t left, uint64_t right, uint64_t modulus)
{
    return left >= right ? left - right : modulus - (right - left);
}

static uint64_t mod_multiply_small(uint64_t value, unsigned multiplier,
                                   const Prime *prime)
{
    /*
     * Shoup multiplication with base 2^48.  Since value<modulus<2^37, the
     * approximate quotient is at most one low.  Both products and the
     * remainder below 2*modulus fit in uint64_t.
     */
    const uint64_t quotient =
        (value * prime->shoup[multiplier]) >> 48;
    uint64_t remainder =
        value * multiplier - quotient * prime->modulus;
    if (remainder >= prime->modulus) {
        remainder -= prime->modulus;
    }
    return remainder;
}

typedef struct {
    _Atomic uint64_t next;
    uint64_t end;
    uint64_t chunk;
} Schedule;

typedef struct {
    int n;
    unsigned positions;
    unsigned lanes;
    Prime primes[MAX_LANES];
    Schedule *schedule;
    uint64_t sum[MAX_LANES];
} Worker;

static void coefficients_all(const Worker *worker,
                             const int edges[2 * MAX_N],
                             unsigned total_factors,
                             uint64_t result[MAX_LANES])
{
    /* Lanes are contiguous because the prime loop is the innermost loop. */
    uint64_t coefficient[MAX_N + 1][MAX_LANES] = {{0}};
    for (unsigned lane = 0; lane < worker->lanes; ++lane) {
        coefficient[0][lane] = 1;
    }
    unsigned factors = 0;
    for (unsigned distance = 1; distance < worker->positions; ++distance) {
        const int edge = edges[distance];
        if (edge == 0) {
            continue;
        }
        ++factors;
        unsigned degree = factors < (unsigned)worker->n ?
                          factors : (unsigned)worker->n;
        const unsigned remaining = total_factors - factors;
        const unsigned minimum_degree = (unsigned)worker->n > remaining ?
            (unsigned)worker->n - remaining : 1U;
        if (degree < minimum_degree) {
            continue;
        }
        const unsigned magnitude = (unsigned)(edge < 0 ? -edge : edge);
        for (;; --degree) {
            for (unsigned lane = 0; lane < worker->lanes; ++lane) {
                const Prime *prime = &worker->primes[lane];
                const uint64_t product = magnitude == 1 ?
                    coefficient[degree - 1][lane] : mod_multiply_small(
                        coefficient[degree - 1][lane], magnitude, prime);
                coefficient[degree][lane] = edge < 0 ?
                    mod_sub(coefficient[degree][lane], product,
                            prime->modulus) :
                    mod_add(coefficient[degree][lane], product,
                            prime->modulus);
            }
            if (degree == minimum_degree) {
                break;
            }
        }
    }
    for (unsigned lane = 0; lane < worker->lanes; ++lane) {
        result[lane] = coefficient[worker->n][lane];
    }
}

static void coefficients_two(const Worker *worker,
                             const int edges[2 * MAX_N],
                             unsigned total_factors,
                             uint64_t result[2])
{
    uint64_t coefficient[2][MAX_N + 1] = {{0}};
    coefficient[0][0] = 1;
    coefficient[1][0] = 1;
    unsigned factors = 0;
    for (unsigned distance = 1; distance < worker->positions; ++distance) {
        const int edge = edges[distance];
        if (edge == 0) {
            continue;
        }
        ++factors;
        unsigned degree = factors < (unsigned)worker->n ?
                          factors : (unsigned)worker->n;
        const unsigned remaining = total_factors - factors;
        const unsigned minimum_degree = (unsigned)worker->n > remaining ?
            (unsigned)worker->n - remaining : 1U;
        if (degree < minimum_degree) {
            continue;
        }
        const unsigned magnitude = (unsigned)(edge < 0 ? -edge : edge);
        for (;; --degree) {
            for (unsigned lane = 0; lane < 2; ++lane) {
                const Prime *prime = &worker->primes[lane];
                const uint64_t product = magnitude == 1 ?
                    coefficient[lane][degree - 1] : mod_multiply_small(
                        coefficient[lane][degree - 1], magnitude, prime);
                coefficient[lane][degree] = edge < 0 ?
                    mod_sub(coefficient[lane][degree], product,
                            prime->modulus) :
                    mod_add(coefficient[lane][degree], product,
                            prime->modulus);
            }
            if (degree == minimum_degree) {
                break;
            }
        }
    }
    result[0] = coefficient[0][worker->n];
    result[1] = coefficient[1][worker->n];
}

static void *worker_main(void *argument)
{
    Worker *worker = argument;
    if (worker->lanes < 1 || worker->lanes > MAX_LANES) {
        die("worker CRT lane count is outside implementation limits");
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
            const uint64_t reflected =
                normalized_reflection(signs, worker->positions);
            if (signs > reflected) {
                continue;
            }
            const unsigned orbit_weight = signs == reflected ? 1U : 2U;
            int edges[2 * MAX_N];
            correlations(worker->positions, signs, edges);
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
            uint64_t term[MAX_LANES] = {0};
            if (worker->lanes == 2) {
                coefficients_two(worker, edges, total_factors, term);
            } else {
                coefficients_all(worker, edges, total_factors, term);
            }
            for (unsigned lane = 0; lane < worker->lanes; ++lane) {
                const uint64_t modulus = worker->primes[lane].modulus;
                if (orbit_weight == 2U) {
                    term[lane] = mod_add(term[lane], term[lane], modulus);
                }
                worker->sum[lane] = (index & 1) != 0 ?
                    mod_sub(worker->sum[lane], term[lane], modulus) :
                    mod_add(worker->sum[lane], term[lane], modulus);
            }
        }
    }
    return NULL;
}

static unsigned select_primes(U128 bound, Prime selected[PRIME_SET_SIZE],
                              U128 *product)
{
    *product = 1;
    unsigned count = 0;
    while (*product <= bound && count < PRIME_SET_SIZE) {
        const uint64_t modulus = prime_values[count];
        selected[count].modulus = modulus;
        for (unsigned multiplier = 0; multiplier <= 2 * MAX_N;
             ++multiplier) {
            selected[count].shoup[multiplier] =
                ((uint64_t)multiplier << 48) / modulus;
        }
        if (*product > (~(U128)0) / modulus) {
            die("prime product overflow");
        }
        *product *= modulus;
        ++count;
    }
    if (*product <= bound) {
        die("prime-set product does not exceed pairing upper bound");
    }
    return count;
}

static uint64_t inverse_mod(uint64_t value, uint64_t modulus)
{
    I128 old_r = value, r = modulus;
    I128 old_s = 1, s = 0;
    while (r != 0) {
        const I128 quotient = old_r / r;
        const I128 next_r = old_r - quotient * r;
        const I128 next_s = old_s - quotient * s;
        old_r = r;
        r = next_r;
        old_s = s;
        s = next_s;
    }
    if (old_r != 1) {
        die("CRT moduli are not coprime");
    }
    old_s %= modulus;
    if (old_s < 0) {
        old_s += modulus;
    }
    return (uint64_t)old_s;
}

static U128 reconstruct(const Prime primes[PRIME_SET_SIZE],
                        const uint64_t residues[PRIME_SET_SIZE],
                        unsigned count)
{
    U128 value = residues[0];
    U128 product = primes[0].modulus;
    for (unsigned index = 1; index < count; ++index) {
        const uint64_t modulus = primes[index].modulus;
        const uint64_t value_modulus = (uint64_t)(value % modulus);
        const uint64_t difference =
            mod_sub(residues[index], value_modulus, modulus);
        const uint64_t inverse =
            inverse_mod((uint64_t)(product % modulus), modulus);
        const uint64_t multiplier =
            (uint64_t)(((U128)difference * inverse) % modulus);
        value += product * multiplier;
        product *= modulus;
    }
    return value;
}

static U128 a060963(int n)
{
    if (n == 0) {
        return 1;
    }
    const U128 bound = total_pairings(n);
    Prime primes[PRIME_SET_SIZE];
    U128 modulus_product;
    const unsigned lanes = select_primes(bound, primes, &modulus_product);
    const unsigned positions = 2U * (unsigned)n;
    if (lanes < 1 || lanes > MAX_LANES ||
        positions == 0 || positions > 2U * MAX_N || positions >= 64) {
        die("ordinary-prime CRT dimensions are outside implementation limits");
    }
    for (unsigned lane = 0; lane < lanes; ++lane) {
        if (primes[lane].modulus <= 1 ||
            (primes[lane].modulus & 1) == 0 ||
            primes[lane].modulus >= (UINT64_C(1) << 37)) {
            die("modulus violates base-2^48 Shoup preconditions");
        }
    }
    const uint64_t evaluations = UINT64_C(1) << (positions - 1);
    int threads = requested_threads;
    if ((uint64_t)threads > evaluations) {
        threads = (int)evaluations;
    }
    Worker *workers = calloc((size_t)threads, sizeof(*workers));
    pthread_t *ids = calloc((size_t)threads, sizeof(*ids));
    if (workers == NULL || ids == NULL) {
        free(workers);
        free(ids);
        die("cannot allocate CRT workers");
    }
    Schedule schedule = {
        .next = 0,
        .end = evaluations,
        .chunk = UINT64_C(1) << CHUNK_BITS
    };
    for (int id = 0; id < threads; ++id) {
        workers[id].n = n;
        workers[id].positions = positions;
        workers[id].lanes = lanes;
        workers[id].schedule = &schedule;
        for (unsigned lane = 0; lane < lanes; ++lane) {
            workers[id].primes[lane] = primes[lane];
        }
        const int error = pthread_create(&ids[id], NULL,
                                         worker_main, &workers[id]);
        if (error != 0) {
            fprintf(stderr, "error: pthread_create: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
    }
    uint64_t residues[MAX_LANES] = {0};
    for (int id = 0; id < threads; ++id) {
        const int error = pthread_join(ids[id], NULL);
        if (error != 0) {
            fprintf(stderr, "error: pthread_join: %s\n", strerror(error));
            exit(EXIT_FAILURE);
        }
        for (unsigned lane = 0; lane < lanes; ++lane) {
            residues[lane] = mod_add(residues[lane], workers[id].sum[lane],
                                     workers[id].primes[lane].modulus);
        }
    }
    free(workers);
    free(ids);
    for (unsigned lane = 0; lane < lanes; ++lane) {
        const uint64_t modulus = primes[lane].modulus;
        for (unsigned bit = 0; bit < positions - 1; ++bit) {
            uint64_t value = residues[lane];
            if ((value & 1U) != 0) {
                value += modulus;
            }
            residues[lane] = value / 2;
        }
    }
    const U128 answer = reconstruct(primes, residues, lanes);
    if (answer > bound) {
        die("CRT result exceeds total number of pairings");
    }
    fprintf(stderr,
            "060963_04: n=%d, ordinary-prime CRT, evaluations=%" PRIu64
            ", primes=%u, threads=%d\n",
            n, evaluations, lanes, threads);
    return answer;
}

static bool is_prime(uint64_t value)
{
    if (value < 2 || (value & 1U) == 0) {
        return value == 2;
    }
    for (uint64_t divisor = 3;
         divisor * divisor <= value; divisor += 2) {
        if (value % divisor == 0) {
            return false;
        }
    }
    return true;
}

static void verify_constants(void)
{
    U128 product = 1;
    Prime configured[PRIME_SET_SIZE];
    for (unsigned index = 0; index < PRIME_SET_SIZE; ++index) {
        const uint64_t modulus = prime_values[index];
        if (!is_prime(modulus)) {
            die("configured CRT modulus is not prime");
        }
        for (unsigned other = 0; other < index; ++other) {
            if (modulus == prime_values[other]) {
                die("configured CRT moduli are not distinct");
            }
        }
        Prime prime = {.modulus = modulus};
        for (unsigned multiplier = 0; multiplier <= 2 * MAX_N;
             ++multiplier) {
            prime.shoup[multiplier] =
                ((uint64_t)multiplier << 48) / modulus;
        }
        configured[index] = prime;
        uint64_t state = UINT64_C(0x9e3779b97f4a7c15) ^ modulus;
        for (unsigned multiplier = 0; multiplier <= 2 * MAX_N;
             ++multiplier) {
            for (unsigned test = 0; test < 1000; ++test) {
                state = state * UINT64_C(6364136223846793005) + 1;
                const uint64_t sample = state % modulus;
                const uint64_t expected =
                    (sample * multiplier) % modulus;
                if (mod_multiply_small(sample, multiplier, &prime) !=
                    expected) {
                    die("Shoup multiplication self-test failed");
                }
            }
        }
        if (product > (~(U128)0) / modulus) {
            die("configured prime product overflow");
        }
        product *= modulus;
    }
    if (product <= total_pairings(MAX_N)) {
        die("complete prime set does not cover MAX_N upper bound");
    }
    const U128 first_two =
        (U128)prime_values[0] * prime_values[1];
    if (!(first_two > total_pairings(19) &&
          first_two <= total_pairings(20))) {
        die("unexpected ordinary-prime pass threshold");
    }
    U128 prefix_product = 1;
    for (unsigned count = 1; count <= PRIME_SET_SIZE; ++count) {
        prefix_product *= configured[count - 1].modulus;
        const U128 sample = prefix_product - 12345;
        uint64_t residues[PRIME_SET_SIZE] = {0};
        for (unsigned index = 0; index < count; ++index) {
            residues[index] =
                (uint64_t)(sample % configured[index].modulus);
        }
        if (reconstruct(configured, residues, count) != sample) {
            die("ordinary CRT reconstruction self-test failed");
        }
    }
}

static void verify_known(int n, U128 value)
{
    const unsigned count = (unsigned)(sizeof(known) / sizeof(known[0]));
    if ((unsigned)n >= count) {
        return;
    }
    U128 expected;
    if (!parse_u128(known[n], &expected) || value != expected) {
        fprintf(stderr, "error: A060963 verification failed at n=%d\n", n);
        exit(EXIT_FAILURE);
    }
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N] [--threads T] [--output FILE]\n"
            "       %s --term N [--threads T] [--output FILE]\n"
            "       %s --check [--threads T] [--no-bfile]\n"
            "N must be in 0..%d; T must be in 1..%d.\n",
            program, program, program, MAX_N, MAX_THREADS);
}

int main(int argc, char **argv)
{
    enum { MODE_RANGE, MODE_TERM, MODE_CHECK } mode = MODE_RANGE;
    int maximum = -1;
    bool have_mode = false, have_threads = false, have_output = false;
    for (int index = 1; index < argc; ++index) {
        if (!strcmp(argv[index], "--help") || !strcmp(argv[index], "-h")) {
            usage(argv[0]);
            return EXIT_SUCCESS;
        } else if (!strcmp(argv[index], "--threads")) {
            if (have_threads || ++index >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            requested_threads = parse_integer(argv[index], 1, MAX_THREADS,
                                              "threads");
            have_threads = true;
        } else if (!strcmp(argv[index], "--output")) {
            if (have_output || ++index >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            output_path = argv[index];
            have_output = true;
        } else if (!strcmp(argv[index], "--no-bfile")) {
            if (have_output) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            write_bfile = false;
            have_output = true;
        } else if (!strcmp(argv[index], "--term")) {
            if (have_mode || ++index >= argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_TERM;
            maximum = parse_integer(argv[index], 0, MAX_N, "N");
            have_mode = true;
        } else if (!strcmp(argv[index], "--check")) {
            if (have_mode) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_CHECK;
            have_mode = true;
        } else if (argv[index][0] != '-' && !have_mode) {
            maximum = parse_integer(argv[index], 0, MAX_N, "N");
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
    verify_constants();
    if (mode == MODE_CHECK) {
        for (int n = 0; n <= CHECK_N; ++n) {
            verify_known(n, a060963(n));
        }
        printf("ok: A060963 n=0..%d verified by ordinary-prime CRT\n",
               CHECK_N);
        return EXIT_SUCCESS;
    }
    if (mode == MODE_TERM) {
        const double started = now_seconds();
        const U128 value = a060963(maximum);
        verify_known(maximum, value);
        fprintf(stderr, "060963_04: n=%d completed in %.3f s\n",
                maximum, now_seconds() - started);
        if (write_bfile) {
            store_bfile_term(maximum, value);
        }
        printf("%d ", maximum);
        print_u128(stdout, value);
        putchar('\n');
        return EXIT_SUCCESS;
    }
    for (int n = 0; n <= maximum; ++n) {
        const double started = now_seconds();
        const U128 value = a060963(n);
        verify_known(n, value);
        if (n != 0) {
            fprintf(stderr, "060963_04: n=%d completed in %.3f s\n",
                    n, now_seconds() - started);
        }
        if (write_bfile) {
            store_bfile_term(n, value);
        }
        printf("%d ", n);
        print_u128(stdout, value);
        putchar('\n');
        fflush(stdout);
    }
    return EXIT_SUCCESS;
}
