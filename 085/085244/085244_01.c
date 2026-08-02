/*
 * A085244 -- exact permanent of the n by n GCD matrix.
 *
 * M(i,j)=gcd(i,j), 1<=i,j<=n.  For an odd prime p, Glynn's identity is
 *
 * per(M) = 2^(1-n) sum_{delta_1=1, delta_i in {+1,-1}}
 *          (product_i delta_i) product_j(sum_i delta_i M(i,j))  (mod p).
 *
 * The signs delta_2,...,delta_n are visited in Gray-code order.  One sign
 * changes per state, so all column sums are updated by adding or subtracting
 * twice one GCD-matrix row.  This is the ordinary direct Glynn method: no
 * cutoff q and no low-rank/sparse decomposition are used.
 *
 * Several independently prime 61-bit moduli are processed until their product
 * exceeds the rigorous bound
 *
 *                    U(n) = product_i sum_j gcd(i,j).
 *
 * Every permanent term is nonnegative and is at most the product of the row
 * sums, hence 0<=a(n)<=U(n).  CRT therefore reconstructs a(n) uniquely once
 * the modulus product is greater than U(n).  For n<=24, a direct subset DP
 * independently checks the first modular residue.
 *
 * The program computes every term from 1 through N.  It writes and flushes
 * b085244_part.txt beside the executable after each completed term.  Complete
 * success atomically renames it to b085244.txt, replacing the previous file.
 * The sequence has OFFSET 1.
 *
 * Build on the configured Apple Silicon Mac:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -Xpreprocessor -fopenmp \
 *     -I/opt/homebrew/opt/libomp/include \
 *     -L/opt/homebrew/opt/libomp/lib \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     085244_01.c -lomp -lgmp -o 085244_01
 *
 * A serial build is also valid; omit the OpenMP flags and -lomp.
 *
 * Usage:
 *
 *   ./085244_01 N
 *
 * The O(n*2^(n-1)) time, rather than memory, is the practical limitation.
 * On the tested 8 GB Apple Silicon Mac, n=45 is not realistic.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <gmp.h>

#ifdef __APPLE__
#include <mach-o/dyld.h>
#endif

#ifdef _OPENMP
#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#endif
#include <omp.h>
#ifdef __clang__
#pragma clang diagnostic pop
#endif
#endif

#if ULONG_MAX < UINT64_MAX
#error "085244_01 requires a platform with 64-bit unsigned long"
#endif

#define SEQUENCE_OFFSET 1
#define MAX_N 35
#define DIRECT_CHECK_MAX_N 24
#define MAX_MODULUS_COUNT 8

typedef struct {
    uint64_t residue;
    uint64_t states;
    uint64_t row_updates;
    double seconds;
} GlynnStats;

static void die(const char *message)
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

static int parse_n(const char *text)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < SEQUENCE_OFFSET || value > MAX_N) {
        fprintf(stderr, "error: N must be in %d..%d: %s\n",
                SEQUENCE_OFFSET, MAX_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int gcd_int(int left, int right)
{
    while (right != 0) {
        int remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

static uint64_t add_mod(uint64_t left, uint64_t right,
                        uint64_t modulus)
{
    uint64_t sum = left + right;
    return sum >= modulus ? sum - modulus : sum;
}

static uint64_t subtract_mod(uint64_t left, uint64_t right,
                             uint64_t modulus)
{
    return left >= right ? left - right : modulus - (right - left);
}

static uint64_t multiply_mod(uint64_t left, uint64_t right,
                             uint64_t modulus)
{
    return (uint64_t)(((__uint128_t)left * right) % modulus);
}

static uint64_t power_mod(uint64_t base, uint64_t exponent,
                          uint64_t modulus)
{
    uint64_t result = 1;
    while (exponent != 0) {
        if ((exponent & 1U) != 0) {
            result = multiply_mod(result, base, modulus);
        }
        base = multiply_mod(base, base, modulus);
        exponent >>= 1;
    }
    return result;
}

static char *path_beside_executable(const char *argv0,
                                    const char *filename)
{
    char executable[PATH_MAX];
    char resolved[PATH_MAX];
    bool found = false;

#ifdef __APPLE__
    uint32_t size = (uint32_t)sizeof(executable);
    if (_NSGetExecutablePath(executable, &size) == 0) {
        found = true;
    }
#elif defined(__linux__)
    ssize_t length = readlink("/proc/self/exe", executable,
                              sizeof(executable) - 1);
    if (length >= 0) {
        executable[length] = '\0';
        found = true;
    }
#endif
    if (!found) {
        size_t length = strlen(argv0);
        if (length >= sizeof(executable)) {
            die("executable path is too long");
        }
        memcpy(executable, argv0, length + 1);
    }

    const char *base = realpath(executable, resolved);
    if (base == NULL) {
        base = executable;
    }
    const char *slash = strrchr(base, '/');
    size_t directory_length = slash == NULL ? 1 : (size_t)(slash - base);
    const char *directory = slash == NULL ? "." : base;
    size_t filename_length = strlen(filename);
    if (directory_length > SIZE_MAX - filename_length - 2) {
        die("output path length overflow");
    }
    char *path = malloc(directory_length + filename_length + 2);
    if (path == NULL) {
        die("cannot allocate output path");
    }
    memcpy(path, directory, directory_length);
    path[directory_length] = '/';
    memcpy(path + directory_length + 1, filename, filename_length + 1);
    return path;
}

static GlynnStats permanent_glynn_mod(int n, uint64_t modulus)
{
    uint64_t column_sum[MAX_N];
    for (int column = 1; column <= n; ++column) {
        uint64_t sum = 0;
        for (int row = 1; row <= n; ++row) {
            sum += (uint64_t)gcd_int(row, column);
        }
        column_sum[column - 1] = sum % modulus;
    }

    uint64_t state_count = UINT64_C(1) << (n - 1);
    uint64_t glynn_sum = 0;
    uint64_t previous_gray = 0;
    uint64_t row_updates = 0;
    double start = monotonic_seconds();

    for (uint64_t state = 0; state < state_count; ++state) {
        if (state != 0) {
            uint64_t gray = state ^ (state >> 1);
            uint64_t flipped = gray ^ previous_gray;
            int row = (int)__builtin_ctzll(flipped) + 2;
            bool became_negative = (gray & flipped) != 0;

            for (int column = 1; column <= n; ++column) {
                uint64_t twice =
                    UINT64_C(2) * (uint64_t)gcd_int(row, column);
                if (became_negative) {
                    column_sum[column - 1] = subtract_mod(
                        column_sum[column - 1], twice, modulus);
                } else {
                    column_sum[column - 1] = add_mod(
                        column_sum[column - 1], twice, modulus);
                }
                ++row_updates;
            }
            previous_gray = gray;
        }

        uint64_t product = 1;
        for (int column = 0; column < n; ++column) {
            product = multiply_mod(product, column_sum[column], modulus);
        }
        if ((__builtin_parityll(previous_gray) & 1) == 0) {
            glynn_sum = add_mod(glynn_sum, product, modulus);
        } else {
            glynn_sum = subtract_mod(glynn_sum, product, modulus);
        }
    }

    /* Since modulus is odd, (modulus+1)/2 is 2^(-1) modulo modulus. */
    uint64_t inverse_two = (modulus + 1) / 2;
    uint64_t inverse_power =
        power_mod(inverse_two, (uint64_t)(n - 1), modulus);
    GlynnStats stats = {
        .residue = multiply_mod(glynn_sum, inverse_power, modulus),
        .states = state_count,
        .row_updates = row_updates,
        .seconds = monotonic_seconds() - start
    };
    return stats;
}

static uint64_t permanent_direct_subset_dp(int n, uint64_t modulus,
                                           double *seconds)
{
    size_t state_count = (size_t)1 << n;
    if (state_count > SIZE_MAX / sizeof(uint64_t)) {
        die("direct subset-DP allocation overflow");
    }
    uint64_t *dp = calloc(state_count, sizeof(*dp));
    if (dp == NULL) {
        die("cannot allocate direct subset-DP workspace");
    }

    double start = monotonic_seconds();
    dp[0] = 1;
    /* If k=popcount(mask), dp[mask] is the total weight of all bijections
       from rows 1..k onto exactly the columns in mask. */
    for (size_t mask = 0; mask + 1 < state_count; ++mask) {
        uint64_t value = dp[mask];
        if (value == 0) {
            continue;
        }
        int row = __builtin_popcountll((uint64_t)mask) + 1;
        size_t available = (state_count - 1) ^ mask;
        while (available != 0) {
            size_t bit = available & (0 - available);
            int column = __builtin_ctzll((uint64_t)bit) + 1;
            uint64_t term = multiply_mod(
                value, (uint64_t)gcd_int(row, column), modulus);
            dp[mask | bit] = add_mod(dp[mask | bit], term, modulus);
            available ^= bit;
        }
    }
    uint64_t result = dp[state_count - 1];
    *seconds = monotonic_seconds() - start;
    free(dp);
    return result;
}

static void permanent_upper_bound(mpz_t bound, int n)
{
    mpz_set_ui(bound, 1);
    for (int row = 1; row <= n; ++row) {
        unsigned long row_sum = 0;
        for (int column = 1; column <= n; ++column) {
            row_sum += (unsigned long)gcd_int(row, column);
        }
        mpz_mul_ui(bound, bound, row_sum);
    }
}

static size_t choose_moduli(uint64_t *moduli, mpz_t product,
                            const mpz_t bound)
{
    mpz_t candidate;
    mpz_t prime;
    mpz_inits(candidate, prime, NULL);
    mpz_set_ui(candidate, 1);
    mpz_mul_2exp(candidate, candidate, 60);
    mpz_set_ui(product, 1);

    size_t count = 0;
    while (mpz_cmp(product, bound) <= 0) {
        if (count >= MAX_MODULUS_COUNT) {
            mpz_clears(candidate, prime, NULL);
            die("too many CRT moduli for the rigorous upper bound");
        }
        mpz_nextprime(prime, candidate);
        uint64_t modulus = (uint64_t)mpz_get_ui(prime);
        if (modulus >= (UINT64_C(1) << 63)) {
            mpz_clears(candidate, prime, NULL);
            die("selected modulus is too large for add_mod");
        }
        moduli[count++] = modulus;
        mpz_mul_ui(product, product, (unsigned long)modulus);
        mpz_set(candidate, prime);
    }
    mpz_clears(candidate, prime, NULL);
    return count;
}

static void reconstruct_crt(mpz_t result, const uint64_t *residues,
                            const uint64_t *moduli, size_t count)
{
    mpz_t product;
    mpz_init_set_ui(product, 1);
    mpz_set_ui(result, 0);

    for (size_t index = 0; index < count; ++index) {
        uint64_t modulus = moduli[index];
        uint64_t product_mod =
            (uint64_t)mpz_fdiv_ui(product, (unsigned long)modulus);
        uint64_t inverse = power_mod(product_mod, modulus - 2, modulus);
        uint64_t result_mod =
            (uint64_t)mpz_fdiv_ui(result, (unsigned long)modulus);
        uint64_t difference = subtract_mod(
            residues[index], result_mod, modulus);
        uint64_t multiplier = multiply_mod(difference, inverse, modulus);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)modulus);
    }
    mpz_clear(product);
}

static void verify_crt(const mpz_t result, const mpz_t bound,
                       const uint64_t *residues,
                       const uint64_t *moduli, size_t count)
{
    if (mpz_sgn(result) < 0 || mpz_cmp(result, bound) > 0) {
        die("CRT result lies outside the rigorous permanent bound");
    }
    for (size_t index = 0; index < count; ++index) {
        unsigned long actual = mpz_fdiv_ui(
            result, (unsigned long)moduli[index]);
        if ((uint64_t)actual != residues[index]) {
            die("CRT reconstruction residue check failed");
        }
    }
}

static void compute_exact_term(mpz_t result, int n)
{
    mpz_t bound;
    mpz_t modulus_product;
    mpz_inits(bound, modulus_product, NULL);
    permanent_upper_bound(bound, n);

    uint64_t moduli[MAX_MODULUS_COUNT] = { 0 };
    size_t modulus_count = choose_moduli(moduli, modulus_product, bound);
    uint64_t residues[MAX_MODULUS_COUNT] = { 0 };
    GlynnStats stats[MAX_MODULUS_COUNT];
    memset(stats, 0, sizeof(stats));

    size_t bound_bits = mpz_sizeinbase(bound, 2);
    int workers = (int)modulus_count;
#ifdef _OPENMP
    int available_workers = omp_get_max_threads();
    if (workers > available_workers) {
        workers = available_workers;
    }
#else
    workers = 1;
#endif
    fprintf(stderr,
            "085244_01: n=%d, direct Glynn, states=%" PRIu64
            ", bound=%zu bits, %zu CRT passes, %d worker%s\n",
            n, UINT64_C(1) << (n - 1), bound_bits, modulus_count,
            workers, workers == 1 ? "" : "s");

    double start = monotonic_seconds();
#ifdef _OPENMP
#pragma omp parallel for schedule(static) num_threads(workers)
#endif
    for (int pass = 0; pass < (int)modulus_count; ++pass) {
        stats[pass] = permanent_glynn_mod(n, moduli[pass]);
        residues[pass] = stats[pass].residue;
    }

    for (size_t pass = 0; pass < modulus_count; ++pass) {
        fprintf(stderr,
                "085244_01: n=%d pass %zu/%zu ok, p=%" PRIu64
                ", row updates=%" PRIu64 ", %.3f s\n",
                n, pass + 1, modulus_count, moduli[pass],
                stats[pass].row_updates, stats[pass].seconds);
    }

    if (n <= DIRECT_CHECK_MAX_N) {
        double check_seconds = 0.0;
        uint64_t check = permanent_direct_subset_dp(
            n, moduli[0], &check_seconds);
        if (check != residues[0]) {
            die("Glynn residue differs from direct subset DP");
        }
        fprintf(stderr,
                "085244_01: n=%d direct subset-DP check ok, %.3f s\n",
                n, check_seconds);
    }

    reconstruct_crt(result, residues, moduli, modulus_count);
    verify_crt(result, bound, residues, moduli, modulus_count);
    fprintf(stderr, "085244_01: n=%d exact CRT done, %.3f s wall\n",
            n, monotonic_seconds() - start);
    mpz_clears(bound, modulus_product, NULL);
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "usage: %s N\n", argv[0]);
        return EXIT_FAILURE;
    }
    int maximum_n = parse_n(argv[1]);

    char *part_path = path_beside_executable(argv[0],
                                             "b085244_part.txt");
    char *final_path = path_beside_executable(argv[0], "b085244.txt");
    FILE *output = fopen(part_path, "w");
    if (output == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        free(final_path);
        free(part_path);
        return EXIT_FAILURE;
    }

    mpz_t value;
    mpz_init(value);
    for (int n = SEQUENCE_OFFSET; n <= maximum_n; ++n) {
        compute_exact_term(value, n);
        if (gmp_fprintf(output, "%d %Zd\n", n, value) < 0 ||
            fflush(output) != 0 || fsync(fileno(output)) != 0) {
            fprintf(stderr, "error: cannot write %s: %s\n",
                    part_path, strerror(errno));
            fclose(output);
            mpz_clear(value);
            free(final_path);
            free(part_path);
            return EXIT_FAILURE;
        }
        gmp_printf("%d %Zd\n", n, value);
    }
    mpz_clear(value);

    if (fclose(output) != 0) {
        fprintf(stderr, "error: cannot close %s: %s\n",
                part_path, strerror(errno));
        free(final_path);
        free(part_path);
        return EXIT_FAILURE;
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        free(final_path);
        free(part_path);
        return EXIT_FAILURE;
    }
    fprintf(stderr, "wrote %s (n=%d..%d)\n",
            final_path, SEQUENCE_OFFSET, maximum_n);
    free(final_path);
    free(part_path);
    return EXIT_SUCCESS;
}
