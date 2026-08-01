/*
 * A005326 -- independent exact verifier through n=74.
 *
 * This program deliberately does NOT use the forbidden-cell rook DP in
 * 005326_02.c.  It performs a direct permanent DP on allowed coprime cells.
 * Rows (even n) or columns (odd n) having identical allowed neighborhoods
 * are grouped into multiplicity types.  A mixed-radix state records how many
 * distinct members of every type have been used.  A transition into a type
 * is multiplied by the number of its still-unused labeled members.
 *
 * The common parity decomposition is
 *
 *        [ 0   B ]
 *        [ B^T C ].
 *
 * For n=2m, the grouped DP computes per(B), and a(n)=per(B)^2.
 * For n=2m+1, columns of the m by (m+1) matrix B are grouped and m rows are
 * injected into them.  A terminal state missing one member of type t sums
 * over all possible omitted labeled columns of that type, so division by the
 * type multiplicity yields every maximal minor r_j.  Then a(n)=r^T C r.
 *
 * Modular passes use 31-bit primes above 2^30, disjoint from the 61-bit prime
 * band used by 005326_02.c.  Keeping every DP value in uint32_t halves the
 * dominant state storage.  CRT reconstructs the half-size permanent/minors
 * only after the prime product exceeds a rigorous bound.  All residues are
 * replayed after reconstruction.
 *
 * Build on the configured Apple Silicon Mac:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -Xpreprocessor -fopenmp \
 *     -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     005326_03.c -lomp -lgmp -o 005326_03
 *
 * The sequence offset is 1, so normal output starts with "1 1" in
 * b005326_2.txt beside the executable.  The readable
 * b005326_2_part.txt is flushed after each completed term and is atomically
 * renamed only after complete success.
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
#error "005326_03 requires a platform with 64-bit unsigned long"
#endif

#define DEFAULT_MAX_N 25
#define MAX_SUPPORTED_N 74
#define SEQUENCE_OFFSET 1
#define KNOWN_MAX_N 60
#define DEFAULT_CHECK_N 25
#define DEFAULT_MEMORY_MIB UINT64_C(2048)
#define PARALLEL_RESERVE_MIB UINT64_C(1024)
#define MIN_MEMORY_MIB UINT64_C(16)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define MAX_MODULUS_COUNT 5
#define MAX_HALF_COUNT 37

static const char *const known_terms[KNOWN_MAX_N + 1] = {
    "1", "1", "1", "3", "4", "28", "16", "256", "324",
    "3600", "3600", "129744", "63504", "3521232", "3459600",
    "60891840", "91240704", "8048712960", "3554067456",
    "425476094976", "320265446400", "12474417291264",
    "16417666704384", "2778580249611264", "1142807773593600",
    "172593628397420544", "216448078947876864",
    "17730530614153986048", "18445871071806160896",
    "4988322633552214818816", "1254090246683310489600",
    "427259978841815654400000", "590395790032294787481600",
    "57266563000754880493977600", "76697487481974474881433600",
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
    int processed_count;
    int type_count;
    int capacities[MAX_HALF_COUNT];
    uint32_t strides[MAX_HALF_COUNT];
    uint64_t type_patterns[MAX_HALF_COUNT];
    uint64_t allowed_types[MAX_HALF_COUNT];
    int column_type[MAX_HALF_COUNT];
    uint32_t full_state;
    uint32_t state_count;
    uint32_t peak_layer_states;
    uint64_t workspace_bytes;
} GroupBoard;

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
    if (errno != 0 || end == text || *end != '\0' ||
        value < SEQUENCE_OFFSET ||
        value > MAX_SUPPORTED_N) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, SEQUENCE_OFFSET, MAX_SUPPORTED_N, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static uint64_t memory_budget_bytes(void)
{
    const char *text = getenv("A005326_03_MEMORY_MIB");
    uint64_t mib = DEFAULT_MEMORY_MIB;
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A005326_03_MEMORY_MIB must be in %" PRIu64
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
    size_t directory_length = slash == NULL ? 1 : (size_t)(slash - base);
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

static bool coprime_cross(int row, int column)
{
    return gcd_int(row + 1, 2 * column + 1) == 1;
}

static int find_or_add_type(GroupBoard *board, uint64_t pattern)
{
    for (int type = 0; type < board->type_count; ++type) {
        if (board->type_patterns[type] == pattern) {
            ++board->capacities[type];
            return type;
        }
    }
    if (board->type_count >= MAX_HALF_COUNT) {
        die("too many grouped neighborhood types");
    }
    int type = board->type_count++;
    board->type_patterns[type] = pattern;
    board->capacities[type] = 1;
    return type;
}

static void finish_group_board(GroupBoard *board)
{
    uint64_t states = 1;
    uint64_t layer[MAX_HALF_COUNT + 1] = { 0 };
    uint64_t next[MAX_HALF_COUNT + 1] = { 0 };
    layer[0] = 1;
    int maximum_sum = 0;

    for (int type = 0; type < board->type_count; ++type) {
        board->strides[type] = (uint32_t)states;
        uint64_t radix = (uint64_t)board->capacities[type] + 1;
        if (states > UINT32_MAX / radix) {
            die("mixed-radix grouped state space exceeds uint32_t");
        }
        states *= radix;

        memset(next, 0, sizeof(next));
        for (int used = 0; used <= maximum_sum; ++used) {
            for (int add = 0; add <= board->capacities[type]; ++add) {
                next[used + add] += layer[used];
            }
        }
        maximum_sum += board->capacities[type];
        memcpy(layer, next, sizeof(layer));
    }
    board->state_count = (uint32_t)states;
    board->full_state = board->state_count - 1;
    uint64_t peak = 1;
    for (int used = 0; used <= maximum_sum; ++used) {
        if (layer[used] > peak) {
            peak = layer[used];
        }
    }
    if (peak > UINT32_MAX) {
        die("grouped peak layer exceeds uint32_t active indexing");
    }
    board->peak_layer_states = (uint32_t)peak;

    uint64_t value_bytes = states * sizeof(uint32_t);
    uint64_t mark_bytes = states * sizeof(uint8_t);
    uint64_t active_bytes = peak * sizeof(uint32_t);
    if (value_bytes > UINT64_MAX / 2 || mark_bytes > UINT64_MAX / 2 ||
        active_bytes > UINT64_MAX / 2 ||
        value_bytes * 2 > UINT64_MAX - mark_bytes * 2 ||
        value_bytes * 2 + mark_bytes * 2 >
            UINT64_MAX - active_bytes * 2) {
        die("grouped DP workspace size overflow");
    }
    board->workspace_bytes =
        value_bytes * 2 + mark_bytes * 2 + active_bytes * 2;
}

static void build_group_board(GroupBoard *board, int n)
{
    memset(board, 0, sizeof(*board));
    board->n = n;
    board->m = n / 2;
    board->odd_count = n - board->m;
    for (int column = 0; column < MAX_HALF_COUNT; ++column) {
        board->column_type[column] = -1;
    }

    if ((n & 1) == 0) {
        /* Group identical rows; process the m labeled odd columns. */
        board->processed_count = board->odd_count;
        for (int row = 0; row < board->m; ++row) {
            uint64_t pattern = 0;
            for (int column = 0; column < board->odd_count; ++column) {
                if (coprime_cross(row, column)) {
                    pattern |= UINT64_C(1) << (unsigned)column;
                }
            }
            (void)find_or_add_type(board, pattern);
        }
        for (int column = 0; column < board->processed_count; ++column) {
            uint64_t types = 0;
            for (int type = 0; type < board->type_count; ++type) {
                if ((board->type_patterns[type] &
                     (UINT64_C(1) << (unsigned)column)) != 0) {
                    types |= UINT64_C(1) << (unsigned)type;
                }
            }
            board->allowed_types[column] = types;
        }
    } else {
        /* Group identical columns; process the m labeled even rows. */
        board->processed_count = board->m;
        for (int column = 0; column < board->odd_count; ++column) {
            uint64_t pattern = 0;
            for (int row = 0; row < board->m; ++row) {
                if (coprime_cross(row, column)) {
                    pattern |= UINT64_C(1) << (unsigned)row;
                }
            }
            board->column_type[column] =
                find_or_add_type(board, pattern);
        }
        for (int row = 0; row < board->processed_count; ++row) {
            uint64_t types = 0;
            for (int type = 0; type < board->type_count; ++type) {
                if ((board->type_patterns[type] &
                     (UINT64_C(1) << (unsigned)row)) != 0) {
                    types |= UINT64_C(1) << (unsigned)type;
                }
            }
            board->allowed_types[row] = types;
        }
    }
    finish_group_board(board);
}

static void compute_half_bound(mpz_t bound, const GroupBoard *board)
{
    mpz_t factorial;
    mpz_t degree_product;
    mpz_inits(factorial, degree_product, NULL);
    mpz_fac_ui(factorial, (unsigned long)board->m);
    mpz_set_ui(degree_product, 1);
    for (int row = 0; row < board->m; ++row) {
        unsigned long degree = 0;
        for (int column = 0; column < board->odd_count; ++column) {
            if (coprime_cross(row, column)) {
                ++degree;
            }
        }
        mpz_mul_ui(degree_product, degree_product, degree);
    }
    mpz_set(bound, mpz_cmp(degree_product, factorial) < 0
                       ? degree_product : factorial);
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

static bool add_next(uint32_t *values, uint32_t *active,
                     uint8_t *marks, size_t *active_count,
                     size_t capacity, uint32_t state, uint32_t value,
                     uint32_t modulus)
{
    if (marks[state] == 0) {
        if (*active_count >= capacity) {
            return false;
        }
        marks[state] = 1;
        active[(*active_count)++] = state;
    }
    values[state] = (uint32_t)add_mod(values[state], value, modulus);
    return true;
}

static bool grouped_permanent_mod(uint64_t *outputs,
                                  const GroupBoard *board,
                                  uint64_t modulus, DpStats *stats)
{
    if (modulus == 0 || modulus > UINT32_MAX) {
        die("grouped-DP modulus is outside the uint32_t range");
    }
    uint32_t small_modulus = (uint32_t)modulus;
    memset(stats, 0, sizeof(*stats));
    double start = monotonic_seconds();
    size_t states = board->state_count;
    size_t active_capacity = board->peak_layer_states;

    uint32_t *values_a = calloc(states, sizeof(*values_a));
    uint32_t *values_b = calloc(states, sizeof(*values_b));
    uint8_t *marks_a = calloc(states, sizeof(*marks_a));
    uint8_t *marks_b = calloc(states, sizeof(*marks_b));
    uint32_t *active_a = malloc(active_capacity * sizeof(*active_a));
    uint32_t *active_b = malloc(active_capacity * sizeof(*active_b));
    if (values_a == NULL || values_b == NULL || marks_a == NULL ||
        marks_b == NULL || active_a == NULL || active_b == NULL) {
        free(active_b);
        free(active_a);
        free(marks_b);
        free(marks_a);
        free(values_b);
        free(values_a);
        return false;
    }

    uint32_t *current_values = values_a;
    uint32_t *next_values = values_b;
    uint8_t *current_marks = marks_a;
    uint8_t *next_marks = marks_b;
    uint32_t *current_active = active_a;
    uint32_t *next_active = active_b;
    size_t current_count = 1;
    current_values[0] = 1;
    current_marks[0] = 1;
    current_active[0] = 0;
    stats->peak_active = 1;

    for (int item = 0; item < board->processed_count; ++item) {
        size_t next_count = 0;
        uint64_t allowed = board->allowed_types[item];
        for (size_t entry = 0; entry < current_count; ++entry) {
            uint32_t state = current_active[entry];
            uint32_t value = current_values[state];
            if (value == 0) {
                continue;
            }
            uint64_t scan = allowed;
            while (scan != 0) {
                uint64_t bit = scan & (UINT64_C(0) - scan);
                unsigned type = (unsigned)__builtin_ctzll(bit);
                uint32_t stride = board->strides[type];
                unsigned radix = (unsigned)board->capacities[type] + 1U;
                unsigned used = (state / stride) % radix;
                if (used < (unsigned)board->capacities[type]) {
                    uint32_t target = state + stride;
                    uint32_t contribution = (uint32_t)(
                        ((uint64_t)value *
                         ((unsigned)board->capacities[type] - used)) %
                        small_modulus);
                    if (!add_next(next_values, next_active, next_marks,
                                  &next_count, active_capacity, target,
                                  contribution, small_modulus)) {
                        die("grouped active-state list overflow");
                    }
                    if (stats->transitions != UINT64_MAX) {
                        ++stats->transitions;
                    }
                }
                scan ^= bit;
            }
        }

        for (size_t entry = 0; entry < current_count; ++entry) {
            uint32_t state = current_active[entry];
            current_values[state] = 0;
            current_marks[state] = 0;
        }
        uint32_t *value_swap = current_values;
        current_values = next_values;
        next_values = value_swap;
        uint8_t *mark_swap = current_marks;
        current_marks = next_marks;
        next_marks = mark_swap;
        uint32_t *active_swap = current_active;
        current_active = next_active;
        next_active = active_swap;
        current_count = next_count;
        if (current_count > stats->peak_active) {
            stats->peak_active = current_count;
        }
    }

    if ((board->n & 1) == 0) {
        outputs[0] = current_values[board->full_state];
    } else {
        uint64_t type_minor[MAX_HALF_COUNT] = { 0 };
        for (int type = 0; type < board->type_count; ++type) {
            uint32_t state = board->full_state - board->strides[type];
            uint64_t aggregate = current_values[state];
            uint64_t inverse = power_mod(
                (uint64_t)board->capacities[type], modulus - 2, modulus);
            type_minor[type] = multiply_mod(aggregate, inverse, modulus);
        }
        for (int column = 0; column < board->odd_count; ++column) {
            outputs[column] = type_minor[board->column_type[column]];
        }
    }

    stats->seconds = monotonic_seconds() - start;
    free(active_b);
    free(active_a);
    free(marks_b);
    free(marks_a);
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
    mpz_mul_2exp(candidate, candidate, 30);
    mpz_set_ui(product, 1);
    size_t count = 0;
    while (mpz_cmp(product, bound) <= 0) {
        if (count >= MAX_MODULUS_COUNT) {
            mpz_clears(candidate, prime, NULL);
            die("too many CRT moduli for the grouped-DP bound");
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
                            size_t stride, size_t component,
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
        uint64_t residue = residues[index * stride + component];
        uint64_t difference = subtract_mod(residue, result_mod, modulus);
        uint64_t multiplier = multiply_mod(difference, inverse, modulus);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)modulus);
    }
    for (size_t index = 0; index < modulus_count; ++index) {
        if ((uint64_t)mpz_fdiv_ui(result,
                                  (unsigned long)moduli[index]) !=
            residues[index * stride + component]) {
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
        for (int column = 0; column < odd_count; ++column) {
            if (gcd_int(2 * row + 1, 2 * column + 1) == 1) {
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
    GroupBoard board;
    build_group_board(&board, n);
    if (board.workspace_bytes > memory_budget) {
        fprintf(stderr,
                "error: A005326 verifier n=%d needs %.3f GiB per worker; "
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
    uint64_t residues[MAX_MODULUS_COUNT * MAX_HALF_COUNT] = { 0 };
    DpStats pass_stats[MAX_MODULUS_COUNT];
    bool status[MAX_MODULUS_COUNT] = { false };

    int workers = 1;
#ifdef _OPENMP
    /*
     * Leave headroom for GMP, stdio, libomp, and the rest of macOS.  The
     * nominal memory limit applies to this process, but filling all of it
     * with parallel DP passes causes severe compression/swap on an 8 GiB Mac.
     */
    uint64_t parallel_reserve =
        PARALLEL_RESERVE_MIB * UINT64_C(1024) * UINT64_C(1024);
    uint64_t parallel_budget =
        memory_budget > parallel_reserve
            ? memory_budget - parallel_reserve
            : memory_budget;
    uint64_t memory_workers = parallel_budget / board.workspace_bytes;
    workers = omp_get_max_threads();
    if (workers > (int)modulus_count) {
        workers = (int)modulus_count;
    }
    if ((uint64_t)workers > memory_workers) {
        workers = (int)memory_workers;
    }
    /* Random access over about one hundred million mixed-radix states is
     * memory-bandwidth bound.  Multiple copies force macOS compression on
     * the target 8 GiB machine and are dramatically slower than one pass. */
    if (board.state_count >= UINT32_C(50000000)) {
        workers = 1;
    }
    if (workers < 1) {
        workers = 1;
    }
#endif
    if (report) {
        fprintf(stderr,
                "005326_03: n=%d, half=%d, %d multiplicity types, "
                "%" PRIu32 " mixed states, peak layer=%" PRIu32
                ", bound=%zu bits, %zu CRT pass%s, %d worker%s, "
                "memory limit=%.2f GiB\n",
                n, board.m, board.type_count, board.state_count,
                board.peak_layer_states, mpz_sizeinbase(half_bound, 2),
                modulus_count, modulus_count == 1 ? "" : "es",
                workers, workers == 1 ? "" : "s",
                (double)memory_budget /
                    (1024.0 * 1024.0 * 1024.0));
    }

#ifdef _OPENMP
#pragma omp parallel for num_threads(workers) schedule(static)
#endif
    for (long pass = 0; pass < (long)modulus_count; ++pass) {
        status[pass] = grouped_permanent_mod(
            &residues[(size_t)pass * MAX_HALF_COUNT], &board,
            moduli[pass], &pass_stats[pass]);
    }
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        if (!status[pass]) {
            mpz_clears(half_bound, modulus_product, NULL);
            die("could not allocate grouped-DP workspace");
        }
        if (report) {
            fprintf(stderr,
                    "005326_03: n=%d pass %zu/%zu ok, p=%" PRIu64
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
        reconstruct_crt(half_permanent, residues, MAX_HALF_COUNT, 0,
                        moduli, modulus_count);
        if (mpz_cmp(half_permanent, half_bound) > 0) {
            mpz_clear(half_permanent);
            mpz_clears(half_bound, modulus_product, NULL);
            die("grouped-DP half permanent exceeds its bound");
        }
        mpz_mul(result, half_permanent, half_permanent);
        for (size_t pass = 0; pass < modulus_count; ++pass) {
            uint64_t expected = multiply_mod(
                residues[pass * MAX_HALF_COUNT],
                residues[pass * MAX_HALF_COUNT], moduli[pass]);
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
        mpz_t minors[MAX_HALF_COUNT];
        for (size_t component = 0; component < components; ++component) {
            mpz_init(minors[component]);
            reconstruct_crt(minors[component], residues,
                            MAX_HALF_COUNT, component,
                            moduli, modulus_count);
            if (mpz_cmp(minors[component], half_bound) > 0) {
                for (size_t clear = 0; clear <= component; ++clear) {
                    mpz_clear(minors[clear]);
                }
                mpz_clears(half_bound, modulus_product, NULL);
                die("grouped-DP maximal minor exceeds its bound");
            }
        }
        mpz_set_ui(result, 0);
        for (int row = 0; row < board.odd_count; ++row) {
            for (int column = 0; column < board.odd_count; ++column) {
                if (gcd_int(2 * row + 1, 2 * column + 1) == 1) {
                    mpz_addmul(result, minors[row], minors[column]);
                }
            }
        }
        for (size_t pass = 0; pass < modulus_count; ++pass) {
            uint64_t expected = odd_quadratic_mod(
                &residues[pass * MAX_HALF_COUNT], board.odd_count,
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
                "005326_03: n=%d done, workspace/worker=%.3f GiB, "
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
    for (int n = SEQUENCE_OFFSET; n <= max_n; ++n) {
        compute_checked(value, n, memory_budget, false);
    }
    mpz_clear(value);
    printf("ok: grouped allowed-cell DP agrees with A005326 for n=%d..%d\n",
           SEQUENCE_OFFSET, max_n);
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
    if (start_n < SEQUENCE_OFFSET || start_n > max_n + 1 ||
        start_n > KNOWN_MAX_N + 1) {
        fprintf(stderr,
                "error: START_N must be in %d..min(MAX_N+1,%d)\n",
                SEQUENCE_OFFSET, KNOWN_MAX_N + 1);
        exit(EXIT_FAILURE);
    }
    char *path = path_beside_executable(argv0, "b005326_2.txt");
    char *part =
        path_beside_executable(argv0, "b005326_2_part.txt");
    FILE *stream = fopen(part, "w");
    if (stream == NULL) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                part, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }
    for (int n = SEQUENCE_OFFSET; n < start_n && n <= max_n; ++n) {
        write_known(stream, n);
    }
    if (fflush(stream) != 0) {
        fclose(stream);
        free(part);
        free(path);
        die("could not flush the built-in A005326 prefix");
    }
    if (start_n > SEQUENCE_OFFSET) {
        fprintf(stderr,
                "005326_03: using built-in verified prefix n=%d..%d\n",
                SEQUENCE_OFFSET, start_n - 1);
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
            die("could not write the A005326 verifier b-file");
        }
    }
    mpz_clear(value);
    if (fclose(stream) != 0) {
        free(part);
        free(path);
        die("could not close the A005326 verifier b-file");
    }
    if (rename(part, path) != 0) {
        fprintf(stderr, "error: cannot rename %s to %s: %s\n",
                part, path, strerror(errno));
        free(part);
        free(path);
        exit(EXIT_FAILURE);
    }
    printf("wrote %s (n=%d..%d)\n", path, SEQUENCE_OFFSET, max_n);
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
            "START_N defaults to %d; the built-in prefix ends at n=%d.\n"
            "The default memory limit is %" PRIu64 " MiB; override it with\n"
            "A005326_03_MEMORY_MIB.\n",
            program, program, program, DEFAULT_MAX_N, MAX_SUPPORTED_N,
            SEQUENCE_OFFSET, KNOWN_MAX_N, DEFAULT_MEMORY_MIB);
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
    int start_n = argc == 3
                      ? parse_n(argv[2], "START_N")
                      : SEQUENCE_OFFSET;
    produce_b_file(argv[0], max_n, start_n, memory_budget);
    return EXIT_SUCCESS;
}
