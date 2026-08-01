/*
 * A005326 -- fast exact coprime-permutation counter.
 *
 * Count permutations p of {1,...,n} such that gcd(i,p(i)) = 1 for every i.
 * After even/odd row and column permutations, the coprime matrix is
 *
 *        [ 0   B ]
 *        [ B^T C ].
 *
 * For n=2m, a(n)=per(B)^2.  For n=2m+1, let r_j be the permanent of the
 * m by m matrix obtained by deleting column j from the m by (m+1) matrix B.
 * Then a(n)=r^T C r.
 *
 * B is dense, so this program works on its sparse forbidden board
 * F(i,j)=[gcd(2(i+1),2j+1)>1].  Columns without a forbidden cell are omitted
 * from the mask.  A rook DP counts every matching T in F.  Inclusion-
 * exclusion gives
 *
 *   per(B) = sum_T (-1)^|T| (m-|T|)!,
 *
 * and, in the rectangular odd case, the same pass gives every r_j by
 * retaining only matchings whose column set does not contain j.
 *
 * DP arithmetic is modular.  Independent 61-bit primes are processed (in
 * parallel when OpenMP is enabled) until their product exceeds a rigorous
 * bound for the half-size permanent/minors.  CRT reconstructs those smaller
 * exact integers; the final square or quadratic form is then evaluated by
 * GMP.  This needs fewer CRT passes than reconstructing a(n) directly.
 *
 * Build on the configured Apple Silicon Mac:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -Xpreprocessor -fopenmp \
 *     -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     005326_02.c -lomp -lgmp -o 005326_02
 *
 * Without OpenMP, omit the OpenMP flags and -lomp.
 *
 * The normal output is b005326_1.txt beside the executable.  During a run,
 * b005326_1_part.txt is flushed after every completed term and remains
 * readable.  It is atomically renamed only after complete success.
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
#error "005326_02 requires a platform with 64-bit unsigned long"
#endif

#define DEFAULT_MAX_N 25
#define MAX_SUPPORTED_N 74
#define KNOWN_MAX_N 60
#define DEFAULT_CHECK_N 25
#define DEFAULT_MEMORY_MIB UINT64_C(2048)
#define MIN_MEMORY_MIB UINT64_C(16)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define MAX_MODULUS_COUNT 4
#define MAX_ODD_COUNT 37

static const char *const known_terms[KNOWN_MAX_N + 1] = {
    "1",
    "1",
    "1",
    "3",
    "4",
    "28",
    "16",
    "256",
    "324",
    "3600",
    "3600",
    "129744",
    "63504",
    "3521232",
    "3459600",
    "60891840",
    "91240704",
    "8048712960",
    "3554067456",
    "425476094976",
    "320265446400",
    "12474417291264",
    "16417666704384",
    "2778580249611264",
    "1142807773593600",
    "172593628397420544",
    "216448078947876864",
    "17730530614153986048",
    "18445871071806160896",
    "4988322633552214818816",
    "1254090246683310489600",
    "427259978841815654400000",
    "590395790032294787481600",
    "57266563000754880493977600",
    "76697487481974474881433600",
    "14786097120330296843693260800",
    "6363701977541276937461760000",
    "3004050753199657126879764480000",
    "3947796913545845325997056000000",
    "536232134065318935894365552640000",
    "487642804417296524356603453440000",
    "274431790155416580402144584785920000",
    "78897178142661844486471090176000000",
    "51681608012142138983265921023262720000",
    "59040475246917094561184182234152960000",
    "7417723304411612192092096851178291200000",
    "10485777204333159390346163621310627840000",
    "7896338788322918879731318625512774041600000",
    "3149579571108996337369871470555305738240000",
    "1989208671980285257956064090726080876380160000",
    "1624208884300947165386503283648520781824000000",
    "393781445984533829598486699032039939679191040000",
    "497280366514981132254051404731803098142474240000",
    "480688393344538200323949514462321717685853880320000",
    "188639647519558209694637410209801926810271744000000",
    "111606649265979755819875823761454973510301935206400000",
    "118357167240773359691149864989231683712061538304000000",
    "36063751465145770083075423127525449114004289130332160000",
    "51337900703326246042874565785416797440854888871362560000",
    "60846434078194614664392837274810745590542444502701834240000",
    "14029373910111430910947522897348608654624755758462402560000"
};

typedef struct {
    int n;
    int m;
    int odd_count;
    int bad_count;
    int active_row_count;
    int bad_index[MAX_ODD_COUNT];
    uint64_t row_masks[MAX_ODD_COUNT];
    uint64_t state_count;
    uint64_t workspace_bytes;
} CrossBoard;

typedef struct {
    size_t peak_active;
    uint64_t transitions;
    double seconds;
} DpStats;

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

static int parse_n(const char *text, const char *label)
{
    char *end = NULL;
    errno = 0;
    long value = strtol(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || value < 0 ||
        value > MAX_SUPPORTED_N) {
        fprintf(stderr, "error: %s must be in 0..%d: %s\n",
                label, MAX_SUPPORTED_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static uint64_t memory_budget_bytes(void)
{
    const char *text = getenv("A005326_02_MEMORY_MIB");
    uint64_t mib = DEFAULT_MEMORY_MIB;
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A005326_02_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)parsed;
    }
    return mib << 20;
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
    char *path = malloc(directory_length + 1 + filename_length + 1);
    if (path == NULL) {
        die("could not allocate an output path");
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

static int gcd_int(int left, int right)
{
    while (right != 0) {
        int remainder = left % right;
        left = right;
        right = remainder;
    }
    return left;
}

static int compare_row_masks(const void *left_pointer,
                             const void *right_pointer)
{
    uint64_t left = *(const uint64_t *)left_pointer;
    uint64_t right = *(const uint64_t *)right_pointer;
    unsigned left_degree = (unsigned)__builtin_popcountll(left);
    unsigned right_degree = (unsigned)__builtin_popcountll(right);
    if (left_degree != right_degree) {
        return left_degree < right_degree ? -1 : 1;
    }
    return left < right ? -1 : left > right;
}

static void build_cross_board(CrossBoard *board, int n)
{
    memset(board, 0, sizeof(*board));
    board->n = n;
    board->m = n / 2;
    board->odd_count = n - board->m;
    for (int column = 0; column < MAX_ODD_COUNT; ++column) {
        board->bad_index[column] = -1;
    }

    bool bad_column[MAX_ODD_COUNT] = { false };
    for (int column = 0; column < board->odd_count; ++column) {
        int odd_value = 2 * column + 1;
        for (int row = 0; row < board->m; ++row) {
            if (gcd_int(row + 1, odd_value) > 1) {
                bad_column[column] = true;
                break;
            }
        }
        if (bad_column[column]) {
            board->bad_index[column] = board->bad_count++;
        }
    }
    if (board->bad_count >= 32) {
        die("compressed forbidden board exceeds the uint32_t state index");
    }

    uint64_t unsorted[MAX_ODD_COUNT] = { 0 };
    for (int row = 0; row < board->m; ++row) {
        uint64_t mask = 0;
        for (int column = 0; column < board->odd_count; ++column) {
            int index = board->bad_index[column];
            if (index >= 0 &&
                gcd_int(row + 1, 2 * column + 1) > 1) {
                mask |= UINT64_C(1) << (unsigned)index;
            }
        }
        if (mask != 0) {
            unsorted[board->active_row_count++] = mask;
        }
    }
    qsort(unsorted, (size_t)board->active_row_count,
          sizeof(*unsorted), compare_row_masks);
    memcpy(board->row_masks, unsorted,
           (size_t)board->active_row_count * sizeof(*unsorted));

    board->state_count = UINT64_C(1) << (unsigned)board->bad_count;
    uint64_t mark_bytes = board->state_count * sizeof(uint8_t);
    uint64_t value_bytes = board->state_count * sizeof(uint64_t);
    uint64_t list_bytes = board->state_count * sizeof(uint32_t);
    if (value_bytes > UINT64_MAX / 2 || list_bytes > UINT64_MAX / 2 ||
        mark_bytes > UINT64_MAX / 2 ||
        value_bytes * 2 > UINT64_MAX - list_bytes * 2 ||
        value_bytes * 2 + list_bytes * 2 > UINT64_MAX - mark_bytes * 2) {
        die("rook workspace size overflow");
    }
    board->workspace_bytes =
        value_bytes * 2 + list_bytes * 2 + mark_bytes * 2;
}

static void compute_half_bound(mpz_t bound, const CrossBoard *board)
{
    mpz_t factorial;
    mpz_t degree_product;
    mpz_inits(factorial, degree_product, NULL);
    mpz_fac_ui(factorial, (unsigned long)board->m);
    mpz_set_ui(degree_product, 1);

    for (int row = 0; row < board->m; ++row) {
        int forbidden = 0;
        int row_value = row + 1;
        for (int column = 0; column < board->odd_count; ++column) {
            if (gcd_int(row_value, 2 * column + 1) > 1) {
                ++forbidden;
            }
        }
        unsigned long degree =
            (unsigned long)(board->odd_count - forbidden);
        mpz_mul_ui(degree_product, degree_product, degree);
    }
    if (mpz_cmp(degree_product, factorial) < 0) {
        mpz_set(bound, degree_product);
    } else {
        mpz_set(bound, factorial);
    }
    mpz_clears(factorial, degree_product, NULL);
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
        die("zero modular arithmetic modulus");
    }
    return (uint64_t)(((__uint128_t)left * right) % modulus);
}

static uint64_t power_mod(uint64_t base, uint64_t exponent,
                          uint64_t modulus)
{
    uint64_t result = 1;
    while (exponent != 0) {
        if ((exponent & 1) != 0) {
            result = multiply_mod(result, base, modulus);
        }
        base = multiply_mod(base, base, modulus);
        exponent >>= 1;
    }
    return result;
}

static bool add_next(uint64_t *values, uint32_t *active,
                     uint8_t *marks, size_t *active_count,
                     size_t capacity, uint32_t mask, uint64_t value,
                     uint64_t modulus)
{
    if (marks[mask] == 0) {
        if (*active_count >= capacity) {
            return false;
        }
        marks[mask] = 1;
        active[(*active_count)++] = mask;
    }
    values[mask] = add_mod(values[mask], value, modulus);
    return true;
}

static bool rook_minors_mod(uint64_t *outputs,
                            const CrossBoard *board,
                            uint64_t modulus, DpStats *stats)
{
    memset(stats, 0, sizeof(*stats));
    double start = monotonic_seconds();
    size_t states = (size_t)board->state_count;

    uint64_t *values_a = calloc(states, sizeof(*values_a));
    uint64_t *values_b = calloc(states, sizeof(*values_b));
    uint32_t *active_a = malloc(states * sizeof(*active_a));
    uint32_t *active_b = malloc(states * sizeof(*active_b));
    uint8_t *marks_a = calloc(states, sizeof(*marks_a));
    uint8_t *marks_b = calloc(states, sizeof(*marks_b));
    if (values_a == NULL || values_b == NULL || active_a == NULL ||
        active_b == NULL || marks_a == NULL || marks_b == NULL) {
        free(marks_b);
        free(marks_a);
        free(active_b);
        free(active_a);
        free(values_b);
        free(values_a);
        return false;
    }

    uint64_t *current_values = values_a;
    uint64_t *next_values = values_b;
    uint32_t *current_active = active_a;
    uint32_t *next_active = active_b;
    uint8_t *current_marks = marks_a;
    uint8_t *next_marks = marks_b;
    size_t current_count = 1;
    current_values[0] = 1;
    current_active[0] = 0;
    current_marks[0] = 1;
    stats->peak_active = 1;

    for (int row = 0; row < board->active_row_count; ++row) {
        uint64_t row_mask = board->row_masks[row];
        size_t next_count = 0;
        for (size_t entry = 0; entry < current_count; ++entry) {
            uint32_t mask = current_active[entry];
            uint64_t value = current_values[mask];
            if (value == 0) {
                continue;
            }
            if (!add_next(next_values, next_active, next_marks,
                          &next_count, states, mask, value, modulus)) {
                die("rook active-state list overflow");
            }
            uint64_t available = row_mask & ~(uint64_t)mask;
            while (available != 0) {
                uint64_t bit = available & (UINT64_C(0) - available);
                uint32_t target = mask | (uint32_t)bit;
                if (!add_next(next_values, next_active, next_marks,
                              &next_count, states, target, value,
                              modulus)) {
                    die("rook active-state list overflow");
                }
                if (stats->transitions != UINT64_MAX) {
                    ++stats->transitions;
                }
                available ^= bit;
            }
        }

        for (size_t entry = 0; entry < current_count; ++entry) {
            uint32_t mask = current_active[entry];
            current_values[mask] = 0;
            current_marks[mask] = 0;
        }

        uint64_t *value_swap = current_values;
        current_values = next_values;
        next_values = value_swap;
        uint32_t *active_swap = current_active;
        current_active = next_active;
        next_active = active_swap;
        uint8_t *mark_swap = current_marks;
        current_marks = next_marks;
        next_marks = mark_swap;
        current_count = next_count;
        if (current_count > stats->peak_active) {
            stats->peak_active = current_count;
        }
    }

    uint64_t factorial[MAX_ODD_COUNT + 1] = { 0 };
    factorial[0] = 1;
    for (int index = 1; index <= board->m; ++index) {
        factorial[index] = multiply_mod(
            factorial[index - 1], (uint64_t)index, modulus);
    }

    uint64_t base = 0;
    uint64_t used_sum[MAX_ODD_COUNT] = { 0 };
    for (size_t entry = 0; entry < current_count; ++entry) {
        uint32_t mask = current_active[entry];
        uint64_t value = current_values[mask];
        if (value == 0) {
            continue;
        }
        unsigned rooks = (unsigned)__builtin_popcount(mask);
        uint64_t term = multiply_mod(
            value, factorial[board->m - (int)rooks], modulus);
        if ((rooks & 1U) != 0 && term != 0) {
            term = modulus - term;
        }
        base = add_mod(base, term, modulus);
        uint32_t scan = mask;
        while (scan != 0) {
            uint32_t bit = scan & (UINT32_C(0) - scan);
            unsigned index = (unsigned)__builtin_ctz(bit);
            used_sum[index] = add_mod(used_sum[index], term, modulus);
            scan ^= bit;
        }
    }

    if ((board->n & 1) == 0) {
        outputs[0] = base;
    } else {
        for (int column = 0; column < board->odd_count; ++column) {
            int index = board->bad_index[column];
            outputs[column] = index < 0
                                  ? base
                                  : subtract_mod(base, used_sum[index],
                                                 modulus);
        }
    }

    stats->seconds = monotonic_seconds() - start;
    free(marks_b);
    free(marks_a);
    free(active_b);
    free(active_a);
    free(values_b);
    free(values_a);
    return true;
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
            die("too many CRT moduli for the half-size bound");
        }
        mpz_nextprime(prime, candidate);
        moduli[count++] = (uint64_t)mpz_get_ui(prime);
        mpz_mul_ui(product, product,
                   (unsigned long)moduli[count - 1]);
        mpz_set(candidate, prime);
    }
    mpz_clears(candidate, prime, NULL);
    return count;
}

static void reconstruct_crt(mpz_t result, const uint64_t *residues,
                            size_t residue_stride, size_t component,
                            const uint64_t *moduli, size_t modulus_count)
{
    mpz_t product;
    mpz_init_set_ui(product, 1);
    mpz_set_ui(result, 0);

    for (size_t index = 0; index < modulus_count; ++index) {
        uint64_t modulus = moduli[index];
        uint64_t product_mod =
            (uint64_t)mpz_fdiv_ui(product, (unsigned long)modulus);
        uint64_t inverse = power_mod(product_mod, modulus - 2, modulus);
        uint64_t result_mod =
            (uint64_t)mpz_fdiv_ui(result, (unsigned long)modulus);
        uint64_t residue = residues[index * residue_stride + component];
        uint64_t difference = subtract_mod(residue, result_mod, modulus);
        uint64_t multiplier =
            multiply_mod(difference, inverse, modulus);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)modulus);
    }

    for (size_t index = 0; index < modulus_count; ++index) {
        if ((uint64_t)mpz_fdiv_ui(result,
                                  (unsigned long)moduli[index]) !=
            residues[index * residue_stride + component]) {
            mpz_clear(product);
            die("CRT residue replay check failed");
        }
    }
    mpz_clear(product);
}

static uint64_t odd_quadratic_mod(const uint64_t *minors,
                                  int odd_count, uint64_t modulus)
{
    uint64_t result = 0;
    for (int row = 0; row < odd_count; ++row) {
        int left = 2 * row + 1;
        for (int column = 0; column < odd_count; ++column) {
            int right = 2 * column + 1;
            if (gcd_int(left, right) == 1) {
                result = add_mod(
                    result,
                    multiply_mod(minors[row], minors[column], modulus),
                    modulus);
            }
        }
    }
    return result;
}

static void permanent_exact(mpz_t result, int n, uint64_t memory_budget,
                            bool report)
{
    double start = monotonic_seconds();
    if (n == 0) {
        mpz_set_ui(result, 1);
        return;
    }

    CrossBoard board;
    build_cross_board(&board, n);
    if (board.workspace_bytes > memory_budget) {
        fprintf(stderr,
                "error: A005326 n=%d needs %.3f GiB per rook-DP worker; "
                "configured limit is %.3f GiB\n",
                n,
                (double)board.workspace_bytes /
                    (1024.0 * 1024.0 * 1024.0),
                (double)memory_budget /
                    (1024.0 * 1024.0 * 1024.0));
        exit(EXIT_FAILURE);
    }

    mpz_t half_bound;
    mpz_t modulus_product;
    mpz_inits(half_bound, modulus_product, NULL);
    compute_half_bound(half_bound, &board);
    uint64_t moduli[MAX_MODULUS_COUNT] = { 0 };
    size_t modulus_count =
        choose_moduli(moduli, modulus_product, half_bound);

    size_t components = (n & 1) == 0 ? 1U : (size_t)board.odd_count;
    uint64_t residues[MAX_MODULUS_COUNT * MAX_ODD_COUNT] = { 0 };
    DpStats pass_stats[MAX_MODULUS_COUNT];
    bool status[MAX_MODULUS_COUNT] = { false };

    int workers = 1;
#ifdef _OPENMP
    uint64_t memory_workers = memory_budget / board.workspace_bytes;
    int maximum_workers = omp_get_max_threads();
    workers = maximum_workers;
    if (workers > (int)modulus_count) {
        workers = (int)modulus_count;
    }
    if ((uint64_t)workers > memory_workers) {
        workers = (int)memory_workers;
    }
    if (workers < 1) {
        workers = 1;
    }
#endif

    if (report) {
        fprintf(stderr,
                "005326_02: n=%d, half=%d, forbidden-mask=%d bits "
                "(%" PRIu64 " states), bound=%zu bits, %zu CRT pass%s, "
                "%d worker%s, memory limit=%.2f GiB\n",
                n, board.m, board.bad_count, board.state_count,
                mpz_sizeinbase(half_bound, 2), modulus_count,
                modulus_count == 1 ? "" : "es", workers,
                workers == 1 ? "" : "s",
                (double)memory_budget /
                    (1024.0 * 1024.0 * 1024.0));
    }

#ifdef _OPENMP
#pragma omp parallel for num_threads(workers) schedule(static)
#endif
    for (long pass = 0; pass < (long)modulus_count; ++pass) {
        status[pass] = rook_minors_mod(
            &residues[(size_t)pass * MAX_ODD_COUNT], &board,
            moduli[pass], &pass_stats[pass]);
    }

    for (size_t pass = 0; pass < modulus_count; ++pass) {
        if (!status[pass]) {
            mpz_clears(half_bound, modulus_product, NULL);
            die("could not allocate the rook-DP workspace");
        }
        if (report) {
            fprintf(stderr,
                    "005326_02: n=%d pass %zu/%zu ok, p=%" PRIu64
                    ", peak states=%zu, transitions=%" PRIu64
                    ", %.3f s\n",
                    n, pass + 1, modulus_count, moduli[pass],
                    pass_stats[pass].peak_active,
                    pass_stats[pass].transitions,
                    pass_stats[pass].seconds);
        }
    }

    if ((n & 1) == 0) {
        mpz_t half_permanent;
        mpz_init(half_permanent);
        reconstruct_crt(half_permanent, residues, MAX_ODD_COUNT, 0,
                        moduli, modulus_count);
        if (mpz_cmp(half_permanent, half_bound) > 0) {
            mpz_clear(half_permanent);
            mpz_clears(half_bound, modulus_product, NULL);
            die("reconstructed half permanent exceeds its bound");
        }
        mpz_mul(result, half_permanent, half_permanent);
        for (size_t pass = 0; pass < modulus_count; ++pass) {
            uint64_t expected = multiply_mod(
                residues[pass * MAX_ODD_COUNT],
                residues[pass * MAX_ODD_COUNT], moduli[pass]);
            if ((uint64_t)mpz_fdiv_ui(result,
                                      (unsigned long)moduli[pass]) !=
                expected) {
                mpz_clear(half_permanent);
                mpz_clears(half_bound, modulus_product, NULL);
                die("final even residue check failed");
            }
        }
        mpz_clear(half_permanent);
    } else {
        mpz_t minors[MAX_ODD_COUNT];
        for (size_t component = 0; component < components; ++component) {
            mpz_init(minors[component]);
            reconstruct_crt(minors[component], residues,
                            MAX_ODD_COUNT, component,
                            moduli, modulus_count);
            if (mpz_cmp(minors[component], half_bound) > 0) {
                for (size_t clear = 0; clear <= component; ++clear) {
                    mpz_clear(minors[clear]);
                }
                mpz_clears(half_bound, modulus_product, NULL);
                die("reconstructed maximal minor exceeds its bound");
            }
        }
        mpz_set_ui(result, 0);
        for (int row = 0; row < board.odd_count; ++row) {
            int left = 2 * row + 1;
            for (int column = 0; column < board.odd_count; ++column) {
                int right = 2 * column + 1;
                if (gcd_int(left, right) == 1) {
                    mpz_addmul(result, minors[row], minors[column]);
                }
            }
        }
        for (size_t pass = 0; pass < modulus_count; ++pass) {
            uint64_t expected = odd_quadratic_mod(
                &residues[pass * MAX_ODD_COUNT], board.odd_count,
                moduli[pass]);
            if ((uint64_t)mpz_fdiv_ui(result,
                                      (unsigned long)moduli[pass]) !=
                expected) {
                for (size_t component = 0; component < components;
                     ++component) {
                    mpz_clear(minors[component]);
                }
                mpz_clears(half_bound, modulus_product, NULL);
                die("final odd residue check failed");
            }
        }
        for (size_t component = 0; component < components; ++component) {
            mpz_clear(minors[component]);
        }
    }

    if (report) {
        fprintf(stderr,
                "005326_02: n=%d done, workspace/worker=%.3f GiB, "
                "total %.3f s\n",
                n,
                (double)board.workspace_bytes /
                    (1024.0 * 1024.0 * 1024.0),
                monotonic_seconds() - start);
    }
    mpz_clears(half_bound, modulus_product, NULL);
}

static void verify_known(const mpz_t value, int n)
{
    if (n < 0 || n > KNOWN_MAX_N) {
        return;
    }
    mpz_t expected;
    mpz_init(expected);
    if (mpz_set_str(expected, known_terms[n], 10) != 0) {
        mpz_clear(expected);
        die("invalid built-in A005326 term");
    }
    if (mpz_cmp(value, expected) != 0) {
        gmp_fprintf(stderr,
                    "error: A005326 mismatch at n=%d: got %Zd, "
                    "expected %Zd\n",
                    n, value, expected);
        mpz_clear(expected);
        exit(EXIT_FAILURE);
    }
    mpz_clear(expected);
}

static void compute_checked(mpz_t value, int n, uint64_t memory_budget,
                            bool report)
{
    permanent_exact(value, n, memory_budget, report);
    verify_known(value, n);
}

static int check_known_terms(int max_n, uint64_t memory_budget)
{
    if (max_n > KNOWN_MAX_N) {
        fprintf(stderr, "error: --check has known terms only through n=%d\n",
                KNOWN_MAX_N);
        return EXIT_FAILURE;
    }
    mpz_t value;
    mpz_init(value);
    for (int n = 0; n <= max_n; ++n) {
        compute_checked(value, n, memory_budget, false);
    }
    mpz_clear(value);
    printf("ok: parity/rook/CRT agrees with A005326 for n=0..%d\n",
           max_n);
    return EXIT_SUCCESS;
}

static void write_known(FILE *stream, int n)
{
    if (n < 0 || n > KNOWN_MAX_N) {
        die("requested built-in A005326 term is unavailable");
    }
    if (fprintf(stream, "%d %s\n", n, known_terms[n]) < 0) {
        die("could not write a built-in A005326 term");
    }
}

static void produce_b_file(const char *argv0, int max_n, int start_n,
                           uint64_t memory_budget)
{
    if (start_n > max_n + 1 || start_n > KNOWN_MAX_N + 1) {
        fprintf(stderr,
                "error: START_N must be at most min(MAX_N+1,%d)\n",
                KNOWN_MAX_N + 1);
        exit(EXIT_FAILURE);
    }

    char *path = path_beside_executable(argv0, "b005326_1.txt");
    char *part =
        path_beside_executable(argv0, "b005326_1_part.txt");
    FILE *stream = fopen(part, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }

    for (int n = 0; n < start_n && n <= max_n; ++n) {
        write_known(stream, n);
    }
    if (fflush(stream) != 0) {
        fclose(stream);
        free(part);
        free(path);
        die("could not flush the built-in A005326 prefix");
    }
    if (start_n > 0) {
        fprintf(stderr,
                "005326_02: using built-in verified prefix n=0..%d\n",
                start_n - 1);
    }

    mpz_t value;
    mpz_init(value);
    for (int n = start_n; n <= max_n; ++n) {
        compute_checked(value, n, memory_budget, true);
        if (gmp_fprintf(stream, "%d %Zd\n", n, value) < 0 ||
            fflush(stream) != 0) {
            mpz_clear(value);
            fclose(stream);
            free(part);
            free(path);
            die("could not write the A005326 b-file");
        }
    }
    mpz_clear(value);

    if (fclose(stream) != 0) {
        free(part);
        free(path);
        die("could not close the A005326 b-file");
    }
    if (rename(part, path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part, path, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=0..%d)\n", path, max_n);
    free(part);
    free(path);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [MAX_N [START_N]]\n"
            "       %s --term N\n"
            "       %s --check [MAX_N]\n"
            "\n"
            "MAX_N defaults to %d and may be at most %d.\n"
            "START_N defaults to 0; the built-in prefix ends at n=%d.\n"
            "The default memory limit is %" PRIu64 " MiB; override it with\n"
            "A005326_02_MEMORY_MIB.\n",
            program, program, program, DEFAULT_MAX_N, MAX_SUPPORTED_N,
            KNOWN_MAX_N, DEFAULT_MEMORY_MIB);
}

int main(int argc, char **argv)
{
    uint64_t memory_budget = memory_budget_bytes();

    if (argc >= 2 && strcmp(argv[1], "--term") == 0) {
        if (argc != 3) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        int n = parse_n(argv[2], "N");
        mpz_t value;
        mpz_init(value);
        compute_checked(value, n, memory_budget, true);
        gmp_printf("%d %Zd\n", n, value);
        mpz_clear(value);
        return EXIT_SUCCESS;
    }

    if (argc >= 2 && strcmp(argv[1], "--check") == 0) {
        if (argc > 3) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        int max_n = argc == 3
                        ? parse_n(argv[2], "MAX_N")
                        : DEFAULT_CHECK_N;
        return check_known_terms(max_n, memory_budget);
    }

    if (argc > 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int max_n = argc >= 2
                    ? parse_n(argv[1], "MAX_N")
                    : DEFAULT_MAX_N;
    int start_n = argc == 3 ? parse_n(argv[2], "START_N") : 0;
    produce_b_file(argv[0], max_n, start_n, memory_budget);
    return EXIT_SUCCESS;
}
