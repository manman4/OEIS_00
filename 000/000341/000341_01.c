/*
 * A000341 -- prime-sum pairings by a permanent and exact CRT.
 *
 * Every prime sum of two distinct positive integers is odd.  Hence every
 * admissible pair in {1,...,2*n} contains one odd and one even element.  Put
 *
 *     A[i,j] = 1 iff (2*i-1) + 2*j = 2*i+2*j-1 is prime,
 *
 * for 1<=i,j<=n.  Choosing all pairs is then exactly choosing a bijection
 * from the odd elements to the even elements, so
 *
 *                         A000341(n) = per(A).
 *
 * Each modular permanent is evaluated with Glynn's formula.  Gray-code sign
 * changes update only the 1-cells in one matrix row.  The product of signed
 * column sums is maintained using precomputed ratios, including explicit
 * zero-factor accounting.  Independent chunks and CRT moduli are OpenMP
 * jobs, so one-modulus terms can still use all requested workers.
 *
 * Pairwise distinct 61-bit primes are selected until their product exceeds
 * a rigorous integerized Bregman-Minc bound
 *
 *              per(A) <= product_i (r_i!)^(1/r_i)
 *                     <= product_i ceil((r_i!)^(1/r_i)),
 *
 * where r_i is row i's number of 1-cells.  Ordinary incremental CRT then
 * reconstructs the unique exact nonnegative answer.  For n<=24, a standard
 * row/subset DP independently checks the first modular residue.
 *
 * Serial build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       000341_01.c -lgmp -o 000341_01
 *
 * OpenMP build on the configured Apple Silicon Mac:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       -Xpreprocessor -fopenmp \
 *       -I/opt/homebrew/opt/libomp/include \
 *       -L/opt/homebrew/opt/libomp/lib -I/opt/homebrew/include \
 *       -L/opt/homebrew/lib 000341_01.c -lomp -lgmp -o 000341_01
 *
 * Usage:
 *   ./000341_01 --term 28 --threads 8
 *   ./000341_01 --upto 28 --start 25 --threads 8
 *   ./000341_01 --check 20 --threads 8
 *
 * A positional N is shorthand for --upto N.  --upto writes b000341.txt
 * beside the executable through an interruption-safe b000341_part.txt.
 * --start S copies the built-in verified prefix n<S, then computes S..N.
 * --term only prints one term and does not touch the b-file.
 * --no-direct-check disables the subset-DP residue check.
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
#error "000341_01 requires a platform with 64-bit unsigned long"
#endif

#define MIN_N 1
#define MAX_N 45
#define KNOWN_MAX_N 36
#define DEFAULT_CHECK_N 20
#define DIRECT_CHECK_MAX_N 24
#define MAX_THREADS 64
#define DEFAULT_THREADS 4
#define MAX_MODULUS_COUNT 8
#define TABLE_SIZE (2 * MAX_N + 1)
#define CHUNKS_PER_WORKER 4

typedef struct {
    int n;
    uint64_t row_mask[MAX_N];
    int row_degree[MAX_N];
    int column_degree[MAX_N];
} PrimeMatrix;

typedef struct {
    uint64_t modulus;
    uint64_t value[TABLE_SIZE];
    uint64_t inverse[TABLE_SIZE];
    uint64_t down_ratio[TABLE_SIZE];
    uint64_t up_ratio[TABLE_SIZE];
} ModTable;

typedef enum {
    MODE_TERM,
    MODE_UPTO,
    MODE_CHECK
} RunMode;

static const char *const known_terms[KNOWN_MAX_N + 1] = {
    NULL,
    "1",
    "2",
    "3",
    "6",
    "26",
    "96",
    "210",
    "1106",
    "3759",
    "12577",
    "74072",
    "423884",
    "2333828",
    "16736611",
    "99838851",
    "630091746",
    "4525325020",
    "38848875650",
    "342245714017",
    "3335164762941",
    "31315463942337",
    "241353231085002",
    "2350106537365732",
    "17903852593938447",
    "158065352670318614",
    "1815064841856534244",
    "20577063085601738871",
    "276081763499377227299",
    "4130939088868088745150",
    "53044513159810367967676",
    "541002401104002626813519",
    "5950265378777850823237485",
    "74951345418716657342827879",
    "826277073509694225690745840",
    "11574574930926612506322729198",
    "145547930475061998320782572612"
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

static int parse_integer(const char *text, int minimum, int maximum,
                         const char *label)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
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
    size_t directory_length =
        slash == NULL ? 1 : (size_t)(slash - base);
    if (slash != NULL && directory_length == 0) {
        directory_length = 1;
    }
    size_t filename_length = strlen(filename);
    if (directory_length > SIZE_MAX - filename_length - 2) {
        die("b-file output path length overflow");
    }
    char *path = malloc(directory_length + filename_length + 2);
    if (path == NULL) {
        die("could not allocate a b-file output path");
    }
    if (slash == NULL) {
        path[0] = '.';
    } else if (slash == base) {
        path[0] = '/';
    } else {
        memcpy(path, base, directory_length);
    }
    path[directory_length] = '/';
    memcpy(path + directory_length + 1, filename, filename_length + 1);
    return path;
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
    if (modulus == 0) {
        die("zero modular-arithmetic modulus");
    }
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

static bool *prime_sieve(int limit)
{
    bool *prime = calloc((size_t)limit + 1, sizeof(*prime));
    if (prime == NULL) {
        die("could not allocate the prime sieve");
    }
    for (int value = 2; value <= limit; ++value) {
        prime[value] = true;
    }
    for (int divisor = 2; divisor <= limit / divisor; ++divisor) {
        if (!prime[divisor]) {
            continue;
        }
        for (int value = divisor * divisor; value <= limit;
             value += divisor) {
            prime[value] = false;
        }
    }
    return prime;
}

static void build_matrix(PrimeMatrix *matrix, int n)
{
    memset(matrix, 0, sizeof(*matrix));
    matrix->n = n;
    bool *prime = prime_sieve(4 * n - 1);
    for (int row = 0; row < n; ++row) {
        uint64_t mask = 0;
        for (int column = 0; column < n; ++column) {
            int sum = 2 * (row + 1) + 2 * (column + 1) - 1;
            if (prime[sum]) {
                mask |= UINT64_C(1) << column;
                ++matrix->row_degree[row];
                ++matrix->column_degree[column];
            }
        }
        matrix->row_mask[row] = mask;
    }
    free(prime);
}

static void permanent_upper_bound(mpz_t bound,
                                  const PrimeMatrix *matrix)
{
    mpz_t factorial;
    mpz_t root;
    mpz_inits(factorial, root, NULL);
    mpz_set_ui(bound, 1);
    for (int row = 0; row < matrix->n; ++row) {
        unsigned long degree = (unsigned long)matrix->row_degree[row];
        if (degree == 0) {
            mpz_set_ui(bound, 0);
            break;
        }
        mpz_fac_ui(factorial, degree);
        int exact = mpz_root(root, factorial, degree);
        if (!exact) {
            mpz_add_ui(root, root, 1);
        }
        mpz_mul_ui(bound, bound, mpz_get_ui(root));
    }
    mpz_clears(root, factorial, NULL);
}

static size_t choose_moduli(uint64_t *moduli, mpz_t product,
                            const mpz_t bound)
{
    mpz_t candidate;
    mpz_t prime;
    mpz_inits(candidate, prime, NULL);
    mpz_set_ui(candidate, 1);
    mpz_mul_2exp(candidate, candidate, 61);
    mpz_sub_ui(candidate, candidate, UINT64_C(50000000));
    mpz_set_ui(product, 1);

    size_t count = 0;
    while (mpz_cmp(product, bound) <= 0) {
        if (count >= MAX_MODULUS_COUNT) {
            mpz_clears(prime, candidate, NULL);
            die("too many CRT moduli for the permanent bound");
        }
        mpz_nextprime(prime, candidate);
        if (mpz_sizeinbase(prime, 2) > 61) {
            mpz_clears(prime, candidate, NULL);
            die("failed to generate a 61-bit CRT prime");
        }
        moduli[count] = (uint64_t)mpz_get_ui(prime);
        mpz_mul_ui(product, product, (unsigned long)moduli[count]);
        ++count;
        mpz_set(candidate, prime);
    }
    mpz_clears(prime, candidate, NULL);
    return count;
}

static uint64_t signed_residue(int value, uint64_t modulus)
{
    if (value >= 0) {
        return (uint64_t)value;
    }
    return modulus - (uint64_t)(-value);
}

static void build_mod_table(ModTable *table, int n, uint64_t modulus)
{
    memset(table, 0, sizeof(*table));
    table->modulus = modulus;
    for (int value = -n; value <= n; ++value) {
        int index = value + MAX_N;
        table->value[index] = signed_residue(value, modulus);
        if (value != 0) {
            table->inverse[index] = power_mod(
                table->value[index], modulus - 2, modulus);
        }
    }
    for (int old = -n; old <= n; ++old) {
        int index = old + MAX_N;
        if (old != 0 && old - 2 >= -n && old - 2 != 0) {
            table->down_ratio[index] = multiply_mod(
                table->value[old - 2 + MAX_N],
                table->inverse[index], modulus);
        }
        if (old != 0 && old + 2 <= n && old + 2 != 0) {
            table->up_ratio[index] = multiply_mod(
                table->value[old + 2 + MAX_N],
                table->inverse[index], modulus);
        }
    }
}

static uint64_t glynn_chunk(const PrimeMatrix *matrix,
                            const ModTable *table,
                            uint64_t begin, uint64_t end)
{
    int n = matrix->n;
    uint64_t modulus = table->modulus;
    uint64_t gray = begin ^ (begin >> 1);
    int column_sum[MAX_N];
    for (int column = 0; column < n; ++column) {
        column_sum[column] = matrix->column_degree[column];
    }
    for (int row = 1; row < n; ++row) {
        if ((gray & (UINT64_C(1) << (row - 1))) == 0) {
            continue;
        }
        uint64_t columns = matrix->row_mask[row];
        while (columns != 0) {
            uint64_t bit = columns & (UINT64_C(0) - columns);
            int column = __builtin_ctzll(bit);
            column_sum[column] -= 2;
            columns ^= bit;
        }
    }

    uint64_t nonzero_product = 1;
    int zero_count = 0;
    for (int column = 0; column < n; ++column) {
        int value = column_sum[column];
        if (value == 0) {
            ++zero_count;
        } else {
            nonzero_product = multiply_mod(
                nonzero_product, table->value[value + MAX_N], modulus);
        }
    }

    uint64_t sum = 0;
    for (uint64_t state = begin; state < end; ++state) {
        if (zero_count == 0) {
            if ((__builtin_parityll(gray) & 1) == 0) {
                sum = add_mod(sum, nonzero_product, modulus);
            } else {
                sum = subtract_mod(sum, nonzero_product, modulus);
            }
        }
        if (state + 1 == end) {
            break;
        }

        uint64_t next_state = state + 1;
        uint64_t next_gray = next_state ^ (next_state >> 1);
        uint64_t flipped = gray ^ next_gray;
        int row = __builtin_ctzll(flipped) + 1;
        bool became_negative = (next_gray & flipped) != 0;
        int delta = became_negative ? -2 : 2;
        uint64_t columns = matrix->row_mask[row];
        while (columns != 0) {
            uint64_t bit = columns & (UINT64_C(0) - columns);
            int column = __builtin_ctzll(bit);
            int old = column_sum[column];
            int updated = old + delta;
            if (old == 0) {
                --zero_count;
                nonzero_product = multiply_mod(
                    nonzero_product,
                    table->value[updated + MAX_N], modulus);
            } else if (updated == 0) {
                ++zero_count;
                nonzero_product = multiply_mod(
                    nonzero_product,
                    table->inverse[old + MAX_N], modulus);
            } else {
                uint64_t ratio = became_negative
                                     ? table->down_ratio[old + MAX_N]
                                     : table->up_ratio[old + MAX_N];
                nonzero_product = multiply_mod(
                    nonzero_product, ratio, modulus);
            }
            column_sum[column] = updated;
            columns ^= bit;
        }
        gray = next_gray;
    }
    return sum;
}

static void permanent_residues(uint64_t *residues,
                               const PrimeMatrix *matrix,
                               const uint64_t *moduli,
                               size_t modulus_count,
                               int requested_threads,
                               bool report)
{
    if (matrix->n < MIN_N || matrix->n > MAX_N) {
        die("invalid matrix dimension in Glynn evaluation");
    }
    if (modulus_count == 0 || modulus_count > MAX_MODULUS_COUNT) {
        die("invalid CRT modulus count in Glynn evaluation");
    }
    if (requested_threads < 1 || requested_threads > MAX_THREADS) {
        die("invalid worker count in Glynn evaluation");
    }
    ModTable tables[MAX_MODULUS_COUNT];
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        build_mod_table(&tables[pass], matrix->n, moduli[pass]);
    }

    uint64_t state_count = UINT64_C(1) << (matrix->n - 1);
    int chunks = requested_threads * CHUNKS_PER_WORKER /
                 (int)modulus_count;
    if (chunks < 1) {
        chunks = 1;
    }
    if ((uint64_t)chunks > state_count) {
        chunks = (int)state_count;
    }
    size_t job_count = modulus_count * (size_t)chunks;
    uint64_t *partial = calloc(job_count, sizeof(*partial));
    if (partial == NULL) {
        die("could not allocate modular partial sums");
    }

    int workers = requested_threads;
#ifdef _OPENMP
    if (workers > omp_get_max_threads()) {
        workers = omp_get_max_threads();
    }
    if (workers > (int)job_count) {
        workers = (int)job_count;
    }
#else
    workers = 1;
#endif
    if (report) {
        fprintf(stderr,
                "000341_01: n=%d, Glynn states=%" PRIu64
                ", %zu CRT prime%s, %zu jobs, %d worker%s\n",
                matrix->n, state_count, modulus_count,
                modulus_count == 1 ? "" : "s", job_count, workers,
                workers == 1 ? "" : "s");
    }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1) num_threads(workers)
#endif
    for (long job = 0; job < (long)job_count; ++job) {
        size_t pass = (size_t)job / (size_t)chunks;
        int chunk = (int)((size_t)job % (size_t)chunks);
        uint64_t begin = (uint64_t)(((__uint128_t)state_count *
                                     (unsigned)chunk) /
                                    (unsigned)chunks);
        uint64_t end = (uint64_t)(((__uint128_t)state_count *
                                   (unsigned)(chunk + 1)) /
                                  (unsigned)chunks);
        partial[job] = glynn_chunk(matrix, &tables[pass], begin, end);
    }

    for (size_t pass = 0; pass < modulus_count; ++pass) {
        uint64_t sum = 0;
        for (int chunk = 0; chunk < chunks; ++chunk) {
            sum = add_mod(sum,
                          partial[pass * (size_t)chunks + (size_t)chunk],
                          moduli[pass]);
        }
        uint64_t inverse_two = (moduli[pass] + 1) / 2;
        uint64_t inverse_power = power_mod(
            inverse_two, (uint64_t)(matrix->n - 1), moduli[pass]);
        residues[pass] = multiply_mod(sum, inverse_power, moduli[pass]);
    }
    free(partial);
}

static uint64_t permanent_subset_mod(const PrimeMatrix *matrix,
                                     uint64_t modulus)
{
    size_t state_count = (size_t)1 << matrix->n;
    if (state_count > SIZE_MAX / sizeof(uint64_t)) {
        die("subset-DP allocation size overflow");
    }
    uint64_t *dp = calloc(state_count, sizeof(*dp));
    if (dp == NULL) {
        die("could not allocate the subset-DP workspace");
    }
    dp[0] = 1;
    for (size_t mask = 1; mask < state_count; ++mask) {
        int row = __builtin_popcountll((uint64_t)mask) - 1;
        uint64_t columns = (uint64_t)mask & matrix->row_mask[row];
        uint64_t value = 0;
        while (columns != 0) {
            uint64_t bit = columns & (UINT64_C(0) - columns);
            value = add_mod(value, dp[mask ^ (size_t)bit], modulus);
            columns ^= bit;
        }
        dp[mask] = value;
    }
    uint64_t result = dp[state_count - 1];
    free(dp);
    return result;
}

static void reconstruct_crt(mpz_t result, const uint64_t *residues,
                            const uint64_t *moduli,
                            size_t modulus_count)
{
    mpz_t product;
    mpz_init_set_ui(product, 1);
    mpz_set_ui(result, 0);
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        uint64_t modulus = moduli[pass];
        uint64_t product_mod = (uint64_t)mpz_fdiv_ui(
            product, (unsigned long)modulus);
        uint64_t inverse = power_mod(
            product_mod, modulus - 2, modulus);
        uint64_t result_mod = (uint64_t)mpz_fdiv_ui(
            result, (unsigned long)modulus);
        uint64_t difference = subtract_mod(
            residues[pass], result_mod, modulus);
        uint64_t multiplier = multiply_mod(
            difference, inverse, modulus);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)modulus);
    }
    mpz_clear(product);
}

static void verify_known(const mpz_t value, int n)
{
    if (n > KNOWN_MAX_N) {
        return;
    }
    mpz_t expected;
    mpz_init(expected);
    if (mpz_set_str(expected, known_terms[n], 10) != 0) {
        mpz_clear(expected);
        die("invalid built-in A000341 term");
    }
    if (mpz_cmp(value, expected) != 0) {
        gmp_fprintf(stderr,
                    "error: A000341 mismatch at n=%d: got %Zd, "
                    "expected %Zd\n",
                    n, value, expected);
        mpz_clear(expected);
        exit(EXIT_FAILURE);
    }
    mpz_clear(expected);
}

static void compute_exact(mpz_t result, int n, int requested_threads,
                          bool direct_check, bool report)
{
    double start = monotonic_seconds();
    PrimeMatrix matrix;
    build_matrix(&matrix, n);

    mpz_t bound;
    mpz_t modulus_product;
    mpz_inits(bound, modulus_product, NULL);
    permanent_upper_bound(bound, &matrix);
    if (mpz_sgn(bound) == 0) {
        mpz_set_ui(result, 0);
        verify_known(result, n);
        mpz_clears(modulus_product, bound, NULL);
        return;
    }

    uint64_t moduli[MAX_MODULUS_COUNT] = { 0 };
    size_t modulus_count = choose_moduli(
        moduli, modulus_product, bound);
    uint64_t residues[MAX_MODULUS_COUNT] = { 0 };
    if (report) {
        fprintf(stderr,
                "000341_01: n=%d, integer Bregman bound=%zu bits\n",
                n, mpz_sizeinbase(bound, 2));
    }
    permanent_residues(residues, &matrix, moduli, modulus_count,
                       requested_threads, report);

    if (direct_check && n <= DIRECT_CHECK_MAX_N) {
        double check_start = monotonic_seconds();
        uint64_t check = permanent_subset_mod(&matrix, moduli[0]);
        if (check != residues[0]) {
            mpz_clears(modulus_product, bound, NULL);
            die("Glynn residue differs from direct subset DP");
        }
        if (report) {
            fprintf(stderr,
                    "000341_01: n=%d subset-DP residue check ok, "
                    "%.3f s\n",
                    n, monotonic_seconds() - check_start);
        }
    }

    reconstruct_crt(result, residues, moduli, modulus_count);
    if (mpz_sgn(result) < 0 || mpz_cmp(result, bound) > 0) {
        mpz_clears(modulus_product, bound, NULL);
        die("CRT result exceeds the rigorous permanent bound");
    }
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        if ((uint64_t)mpz_fdiv_ui(result,
                                  (unsigned long)moduli[pass]) !=
            residues[pass]) {
            mpz_clears(modulus_product, bound, NULL);
            die("CRT reconstruction residue replay failed");
        }
    }
    verify_known(result, n);
    if (report) {
        fprintf(stderr, "000341_01: n=%d exact CRT done, %.3f s\n",
                n, monotonic_seconds() - start);
    }
    mpz_clears(modulus_product, bound, NULL);
}

static void flush_b_file(FILE *stream, const char *path)
{
    if (fflush(stream) != 0 || fsync(fileno(stream)) != 0) {
        fprintf(stderr, "error: cannot flush %s: %s\n",
                path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void write_known_prefix_term(FILE *stream, int n,
                                    const char *path)
{
    if (n < MIN_N || n > KNOWN_MAX_N || known_terms[n] == NULL) {
        die("requested built-in A000341 prefix term is unavailable");
    }
    if (fprintf(stream, "%d %s\n", n, known_terms[n]) < 0) {
        fprintf(stderr, "error: cannot write %s: %s\n",
                path, strerror(errno));
        exit(EXIT_FAILURE);
    }
}

static void produce_b_file(const char *argv0, int maximum_n, int start_n,
                           int requested_threads, bool direct_check)
{
    if (start_n < MIN_N || start_n > maximum_n + 1 ||
        start_n > KNOWN_MAX_N + 1) {
        fprintf(stderr,
                "error: start N must be in %d..min(MAX_N+1,%d)\n",
                MIN_N, KNOWN_MAX_N + 1);
        exit(EXIT_FAILURE);
    }

    char *final_path = path_beside_executable(argv0, "b000341.txt");
    char *part_path =
        path_beside_executable(argv0, "b000341_part.txt");
    FILE *output = fopen(part_path, "w");
    if (output == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part_path, strerror(errno));
        free(part_path);
        free(final_path);
        exit(EXIT_FAILURE);
    }

    for (int n = MIN_N; n < start_n && n <= maximum_n; ++n) {
        write_known_prefix_term(output, n, part_path);
    }
    flush_b_file(output, part_path);
    if (start_n > MIN_N) {
        fprintf(stderr,
                "000341_01: using built-in verified prefix n=%d..%d\n",
                MIN_N, start_n - 1);
    }

    mpz_t value;
    mpz_init(value);
    for (int n = start_n; n <= maximum_n; ++n) {
        compute_exact(value, n, requested_threads, direct_check, true);
        if (gmp_fprintf(output, "%d %Zd\n", n, value) < 0) {
            fprintf(stderr, "error: cannot write %s: %s\n",
                    part_path, strerror(errno));
            mpz_clear(value);
            fclose(output);
            free(part_path);
            free(final_path);
            exit(EXIT_FAILURE);
        }
        flush_b_file(output, part_path);
        gmp_printf("%d %Zd\n", n, value);
    }
    mpz_clear(value);

    if (fclose(output) != 0) {
        fprintf(stderr, "error: cannot close %s: %s\n",
                part_path, strerror(errno));
        free(part_path);
        free(final_path);
        exit(EXIT_FAILURE);
    }
    if (rename(part_path, final_path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part_path, final_path, strerror(errno));
        free(part_path);
        free(final_path);
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "wrote %s (n=%d..%d)\n",
            final_path, MIN_N, maximum_n);
    free(part_path);
    free(final_path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s N [--start S] [--threads T] "
            "[--no-direct-check]\n"
            "       %s --term N [--threads T] [--no-direct-check]\n"
            "       %s --upto N [--start S] [--threads T] "
            "[--no-direct-check]\n"
            "       %s --check [N] [--threads T] "
            "[--no-direct-check]\n",
            program, program, program, program);
}

int main(int argc, char **argv)
{
    RunMode mode = MODE_TERM;
    bool mode_set = false;
    bool direct_check = true;
    int target = -1;
    int start_n = MIN_N;
    bool start_set = false;
    int requested_threads = DEFAULT_THREADS;

    for (int argument = 1; argument < argc; ++argument) {
        const char *text = argv[argument];
        if (strcmp(text, "--term") == 0 ||
            strcmp(text, "--upto") == 0) {
            if (mode_set || target >= 0 || argument + 1 == argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(text, "--term") == 0 ? MODE_TERM : MODE_UPTO;
            mode_set = true;
            target = parse_integer(argv[++argument], MIN_N, MAX_N, "N");
        } else if (strcmp(text, "--check") == 0) {
            if (mode_set || target >= 0) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            mode = MODE_CHECK;
            mode_set = true;
            target = DEFAULT_CHECK_N;
            if (argument + 1 < argc && argv[argument + 1][0] != '-') {
                target = parse_integer(argv[++argument], MIN_N,
                                       KNOWN_MAX_N, "check N");
            }
        } else if (strcmp(text, "--threads") == 0) {
            if (argument + 1 == argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            requested_threads = parse_integer(
                argv[++argument], 1, MAX_THREADS, "threads");
        } else if (strcmp(text, "--start") == 0) {
            if (start_set || argument + 1 == argc) {
                usage(argv[0]);
                return EXIT_FAILURE;
            }
            start_n = parse_integer(argv[++argument], MIN_N,
                                    KNOWN_MAX_N + 1, "start N");
            start_set = true;
        } else if (strcmp(text, "--no-direct-check") == 0) {
            direct_check = false;
        } else if (text[0] != '-' && !mode_set && target < 0) {
            mode = MODE_UPTO;
            mode_set = true;
            target = parse_integer(text, MIN_N, MAX_N, "N");
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (target < 0) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    if (start_set && mode != MODE_UPTO) {
        fprintf(stderr, "error: --start is valid only with --upto or "
                "a positional upper bound N\n");
        return EXIT_FAILURE;
    }

    if (mode == MODE_TERM) {
        mpz_t value;
        mpz_init(value);
        compute_exact(value, target, requested_threads,
                      direct_check, true);
        gmp_printf("%d %Zd\n", target, value);
        mpz_clear(value);
    } else if (mode == MODE_UPTO) {
        produce_b_file(argv[0], target, start_n, requested_threads,
                       direct_check);
    } else {
        mpz_t value;
        mpz_init(value);
        for (int n = MIN_N; n <= target; ++n) {
            compute_exact(value, n, requested_threads,
                          direct_check, false);
        }
        mpz_clear(value);
        printf("ok: permanent/Glynn/CRT agrees with A000341 for "
               "n=%d..%d\n", MIN_N, target);
    }
    return EXIT_SUCCESS;
}
