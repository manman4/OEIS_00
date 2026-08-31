/*
 * A085244 -- exact permanent of the n by n GCD matrix.
 *
 * M(i,j)=gcd(i,j) is split at q=1,2,3, or 4 as
 *
 *   M = L_q + B_q,
 *   L_q(i,j) = sum_{d<=q, d|i, d|j} phi(d),
 *   B_q(i,j) = sum_{d|gcd(i,j), d>q} phi(d).
 *
 * Thus L_q has rank at most q and B_q(i,j) is nonzero exactly when
 * gcd(i,j)>q.  Expanding each permanent edge as an L-edge or a B-edge
 * first chooses a (not necessarily perfect) matching in the sparse graph
 * B_q.  All vertices left unmatched by B_q must then be matched through
 * L_q.
 *
 * This program counts the sparse matching with a row-frontier DP.  Its key
 * consists of
 *
 *   (1) the B-matched columns whose last incident row has not passed, and
 *   (2) the number of selected B-edges and, for colors 2..q, the numbers
 *       of unmatched rows and columns assigned to each color.
 *
 * Why these counts give the exact low-rank completion is proved below in
 * emit_monomer() and in the terminal contraction.  A fixed row order is
 * selected greedily to reduce the
 * maximum live-column frontier.
 * A row path decomposition is a special case of a tree decomposition.
 *
 * The frontier calculation is repeated modulo independently selected 61-bit
 * primes.  A rigorous upper bound
 *
 *                 per(M) <= product_i sum_j gcd(i,j)
 *
 * determines how many primes are needed.  CRT therefore reconstructs the
 * unique exact nonnegative permanent.  The first residue is compared with
 * known A085244 residues through n=38, and n<=24 is independently checked by
 * the ordinary subset DP.
 *
 * Two implementation details are important for speed: every sparse correction
 * edge is precomputed, and transition weights are small (at most n), so their
 * modular products use binary doubling rather than a 128-bit division.
 *
 * Results 1..N are flushed to b085244_02_part.txt after every completed term.
 * Only complete success atomically replaces b085244.txt.
 *
 * Build:
 *
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *     -I/opt/homebrew/include -L/opt/homebrew/lib \
 *     085244_02.c -lgmp -o 085244_02
 *
 * Usage:
 *
 *   ./085244_02 N          # compute a(1)..a(N), q=3
 *   ./085244_02 N Q        # use cutoff Q in 1..4
 *
 * Memory limit (default 2048 MiB):
 *
 *   A085244_02_MEMORY_MIB=6144 ./085244_02 38
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

#if ULONG_MAX < UINT64_MAX
#error "085244_02 requires a platform with 64-bit unsigned long"
#endif

#define SEQUENCE_OFFSET 1
#define MAX_N 45
#define DIRECT_CHECK_MAX_N 24
#define DEFAULT_MEMORY_MIB UINT64_C(2048)
#define MIN_MEMORY_MIB UINT64_C(32)
#define MAX_MEMORY_MIB UINT64_C(65536)
#define FIRST_REFERENCE_MODULUS UINT64_C(1152921504606847009)
#define MAX_MODULUS_COUNT 16
#define EMPTY_CODE UINT64_MAX
#define COUNT_BITS 6
#define COUNT_MASK ((UINT64_C(1) << COUNT_BITS) - 1)
#define MAX_KNOWN_N 38

_Static_assert(MAX_N <= COUNT_MASK,
               "one DP count must fit in COUNT_BITS");
_Static_assert(MAX_N <= 63, "column masks must fit in uint64_t");
_Static_assert(MAX_MODULUS_COUNT >= 2,
               "CRT needs room for independently selected moduli");

static const uint64_t known_residues[MAX_KNOWN_N + 1] = {
    UINT64_C(0),
    UINT64_C(1),
    UINT64_C(3),
    UINT64_C(14),
    UINT64_C(112),
    UINT64_C(872),
    UINT64_C(14372),
    UINT64_C(154480),
    UINT64_C(3098480),
    UINT64_C(59710816),
    UINT64_C(1688186176),
    UINT64_C(27925409152),
    UINT64_C(1327833590272),
    UINT64_C(25675495200768),
    UINT64_C(1017195720916224),
    UINT64_C(47444016840290304),
    UINT64_C(1114109633706177503),
    UINT64_C(1140200724325347792),
    UINT64_C(605814141477190406),
    UINT64_C(779523081708593516),
    UINT64_C(872003706515250454),
    UINT64_C(737146661465650219),
    UINT64_C(1009727701421576126),
    UINT64_C(854537493059079376),
    UINT64_C(679821370357018645),
    UINT64_C(260526890240354531),
    UINT64_C(486942201144866324),
    UINT64_C(1067481194159174585),
    UINT64_C(591049534218902125),
    UINT64_C(990531435660320261),
    UINT64_C(907081723088034761),
    UINT64_C(279022517005155903),
    UINT64_C(630390056218630735),
    UINT64_C(1149930728939511561),
    UINT64_C(986412578526681865),
    UINT64_C(518400539548616480),
    UINT64_C(1147905397317651708),
    UINT64_C(334581868099078631),
    UINT64_C(796567356817866531)
};

typedef struct {
    uint64_t mask;
    uint64_t value;
    uint64_t code;
} StateEntry;

typedef struct {
    StateEntry *entries;
    size_t capacity;
    size_t count;
    uint64_t modulus;
} StateMap;

typedef struct {
    uint64_t limit;
    uint64_t used;
    uint64_t peak;
    uint64_t failed_request;
} MemoryTracker;

typedef struct {
    int n;
    int q;
    int order[MAX_N];
    int first[MAX_N];
    int last[MAX_N];
    int maximum_frontier;
    uint8_t phi[5];
    uint8_t correction[MAX_N][MAX_N];
    uint8_t degree[MAX_N];
    uint8_t neighbor_column[MAX_N][MAX_N];
    uint8_t neighbor_weight[MAX_N][MAX_N];
} Board;

typedef struct {
    size_t peak_states;
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

static int parse_int(const char *text, int minimum, int maximum,
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

static uint64_t memory_budget_bytes(void)
{
    const char *text = getenv("A085244_02_MEMORY_MIB");
    uint64_t mib = DEFAULT_MEMORY_MIB;
    if (text != NULL && *text != '\0') {
        char *end = NULL;
        errno = 0;
        unsigned long long parsed = strtoull(text, &end, 10);
        if (errno != 0 || end == text || *end != '\0' ||
            parsed < MIN_MEMORY_MIB || parsed > MAX_MEMORY_MIB) {
            fprintf(stderr,
                    "error: A085244_02_MEMORY_MIB must be in %" PRIu64
                    "..%" PRIu64 ": %s\n",
                    MIN_MEMORY_MIB, MAX_MEMORY_MIB, text);
            exit(EXIT_FAILURE);
        }
        mib = (uint64_t)parsed;
    }
    return mib << 20;
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

static uint64_t multiply_small_mod(uint64_t value, unsigned factor,
                                   uint64_t modulus)
{
    uint64_t result = 0;
    while (factor != 0) {
        if ((factor & 1U) != 0) {
            result = add_mod(result, value, modulus);
        }
        factor >>= 1;
        if (factor != 0) {
            value = add_mod(value, value, modulus);
        }
    }
    return result;
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

static uint64_t mix64(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    return value ^ (value >> 31);
}

static bool reserve_memory(MemoryTracker *memory, uint64_t bytes)
{
    if (bytes > memory->limit - memory->used) {
        memory->failed_request = bytes;
        return false;
    }
    memory->used += bytes;
    if (memory->used > memory->peak) {
        memory->peak = memory->used;
    }
    return true;
}

static void release_memory(MemoryTracker *memory, uint64_t bytes)
{
    if (bytes > memory->used) {
        die("internal memory accounting underflow");
    }
    memory->used -= bytes;
}

static bool map_allocate(StateMap *map, size_t capacity,
                         uint64_t modulus, MemoryTracker *memory)
{
    if (capacity < 16 || (capacity & (capacity - 1)) != 0 ||
        capacity > SIZE_MAX / sizeof(StateEntry) ||
        modulus <= 2 || modulus >= (UINT64_C(1) << 63)) {
        return false;
    }
    uint64_t bytes = (uint64_t)capacity * sizeof(StateEntry);
    if (!reserve_memory(memory, bytes)) {
        return false;
    }
    StateEntry *entries = malloc((size_t)bytes);
    if (entries == NULL) {
        release_memory(memory, bytes);
        memory->failed_request = bytes;
        return false;
    }
    memset(entries, 0xff, (size_t)bytes);
    map->entries = entries;
    map->capacity = capacity;
    map->count = 0;
    map->modulus = modulus;
    return true;
}

static void map_free(StateMap *map, MemoryTracker *memory)
{
    if (map->entries != NULL) {
        uint64_t bytes = (uint64_t)map->capacity * sizeof(StateEntry);
        free(map->entries);
        release_memory(memory, bytes);
    }
    memset(map, 0, sizeof(*map));
}

static size_t state_hash(uint64_t mask, uint64_t code, size_t capacity)
{
    uint64_t combined = mask ^ mix64(code + UINT64_C(0x9e3779b97f4a7c15));
    return (size_t)(mix64(combined) & (capacity - 1));
}

static void map_insert_without_growth(StateMap *map, uint64_t mask,
                                      uint64_t code, uint64_t value)
{
    size_t slot = state_hash(mask, code, map->capacity);
    while (map->entries[slot].code != EMPTY_CODE) {
        if (map->entries[slot].mask == mask &&
            map->entries[slot].code == code) {
            map->entries[slot].value =
                add_mod(map->entries[slot].value, value, map->modulus);
            return;
        }
        slot = (slot + 1) & (map->capacity - 1);
    }
    map->entries[slot].mask = mask;
    map->entries[slot].code = code;
    map->entries[slot].value = value;
    ++map->count;
}

static bool map_grow(StateMap *map, MemoryTracker *memory)
{
    if (map->capacity > SIZE_MAX / 2) {
        return false;
    }
    StateMap larger = { 0 };
    if (!map_allocate(&larger, map->capacity * 2,
                      map->modulus, memory)) {
        return false;
    }
    for (size_t index = 0; index < map->capacity; ++index) {
        const StateEntry *entry = &map->entries[index];
        if (entry->code != EMPTY_CODE && entry->value != 0) {
            map_insert_without_growth(&larger, entry->mask,
                                      entry->code, entry->value);
        }
    }
    map_free(map, memory);
    *map = larger;
    return true;
}

static bool map_add(StateMap *map, uint64_t mask, uint64_t code,
                    uint64_t value, MemoryTracker *memory)
{
    if (value == 0) {
        return true;
    }
    /* Check for an existing key before considering growth.  Transitions
       collide heavily; growing first would waste memory when the insertion
       only updates a state already in the table. */
    size_t slot = state_hash(mask, code, map->capacity);
    while (map->entries[slot].code != EMPTY_CODE) {
        if (map->entries[slot].mask == mask &&
            map->entries[slot].code == code) {
            map->entries[slot].value =
                add_mod(map->entries[slot].value, value, map->modulus);
            return true;
        }
        slot = (slot + 1) & (map->capacity - 1);
    }
    if ((map->count + 1) * 10 > map->capacity * 7) {
        if (!map_grow(map, memory)) {
            return false;
        }
        map_insert_without_growth(map, mask, code, value);
        return true;
    }
    map->entries[slot].mask = mask;
    map->entries[slot].code = code;
    map->entries[slot].value = value;
    ++map->count;
    return true;
}

static unsigned code_count(uint64_t code, int field)
{
    return (unsigned)((code >> (COUNT_BITS * field)) & COUNT_MASK);
}

static uint64_t increment_code_count(uint64_t code, int field, int n)
{
    if (code_count(code, field) >= (unsigned)n) {
        die("compressed DP count escaped its proven range");
    }
    return code + (UINT64_C(1) << (COUNT_BITS * field));
}

static int color_count_field(int color, bool row_side)
{
    /* Field 0 is the number of B-edges.  There are two fields per color
       2..q: row count first, then column count. */
    return 1 + 2 * (color - 2) + (row_side ? 0 : 1);
}

static bool counts_can_still_balance(uint64_t code, int q,
                                     const int *future_rows,
                                     const int *future_columns)
{
    /* Every future eligible endpoint is optimistically allowed to choose the
       tested color.  If even that cannot repair the current row/column count
       difference, the state can never contribute at the terminal contraction. */
    for (int color = 2; color <= q; ++color) {
        int row_count = (int)code_count(
            code, color_count_field(color, true));
        int column_count = (int)code_count(
            code, color_count_field(color, false));
        if (row_count > column_count + future_columns[color] ||
            column_count > row_count + future_rows[color]) {
            return false;
        }
    }
    return true;
}

static bool support_entry(const Board *board, int row, int column)
{
    return board->correction[row - 1][column - 1] != 0;
}

static int compute_correction_weight(const Board *board,
                                     int row, int column)
{
    int common = gcd_int(row, column);
    int low_rank = 0;
    for (int divisor = 1; divisor <= board->q; ++divisor) {
        if (row % divisor == 0 && column % divisor == 0) {
            low_rank += board->phi[divisor];
        }
    }
    int correction = common - low_rank;
    if ((correction != 0) != (common > board->q) || correction < 0) {
        die("invalid sparse/low-rank decomposition");
    }
    return correction;
}

static void choose_row_order(Board *board)
{
    bool chosen[MAX_N] = { false };
    for (int position = 0; position < board->n; ++position) {
        int best_row = -1;
        int best_active = INT_MAX;
        int best_introduced = INT_MAX;

        for (int candidate = 1; candidate <= board->n; ++candidate) {
            if (chosen[candidate - 1]) {
                continue;
            }
            chosen[candidate - 1] = true;
            int active = 0;
            int introduced = 0;
            for (int column = 1; column <= board->n; ++column) {
                bool past = false;
                bool future = false;
                for (int row = 1; row <= board->n; ++row) {
                    if (!support_entry(board, row, column)) {
                        continue;
                    }
                    if (chosen[row - 1]) {
                        past = true;
                    } else {
                        future = true;
                    }
                }
                if (past) {
                    ++introduced;
                }
                if (past && future) {
                    ++active;
                }
            }
            chosen[candidate - 1] = false;

            if (active < best_active ||
                (active == best_active && introduced < best_introduced) ||
                (active == best_active && introduced == best_introduced &&
                 candidate < best_row)) {
                best_row = candidate;
                best_active = active;
                best_introduced = introduced;
            }
        }
        if (best_row < 1) {
            die("failed to choose a row order");
        }
        board->order[position] = best_row;
        chosen[best_row - 1] = true;
    }
}

static void build_board(Board *board, int n, int q)
{
    memset(board, 0, sizeof(*board));
    board->n = n;
    board->q = q;
    board->phi[1] = 1;
    board->phi[2] = 1;
    board->phi[3] = 2;
    board->phi[4] = 2;

    for (int row = 1; row <= n; ++row) {
        for (int column = 1; column <= n; ++column) {
            int weight = compute_correction_weight(board, row, column);
            if (weight > UINT8_MAX) {
                die("sparse correction weight does not fit in uint8_t");
            }
            board->correction[row - 1][column - 1] =
                (uint8_t)weight;
            if (weight != 0) {
                unsigned degree = board->degree[row - 1];
                if (degree >= MAX_N) {
                    die("sparse correction degree overflow");
                }
                board->neighbor_column[row - 1][degree] =
                    (uint8_t)column;
                board->neighbor_weight[row - 1][degree] =
                    (uint8_t)weight;
                board->degree[row - 1] = (uint8_t)(degree + 1);
            }
        }
    }
    choose_row_order(board);

    for (int column = 0; column < n; ++column) {
        board->first[column] = n;
        board->last[column] = -1;
    }
    for (int position = 0; position < n; ++position) {
        int row = board->order[position];
        for (unsigned edge = 0; edge < board->degree[row - 1]; ++edge) {
            int column = board->neighbor_column[row - 1][edge];
            if (board->first[column - 1] == n) {
                board->first[column - 1] = position;
            }
            board->last[column - 1] = position;
        }
    }

    for (int position = 0; position < n; ++position) {
        int frontier = 0;
        for (int column = 0; column < n; ++column) {
            if (board->first[column] <= position &&
                position <= board->last[column]) {
                ++frontier;
            }
        }
        if (frontier > board->maximum_frontier) {
            board->maximum_frontier = frontier;
        }
    }
}

static bool emit_monomer(StateMap *next, const Board *board,
                         const StateEntry *state, bool is_row, int vertex,
                         const int *future_rows,
                         const int *future_columns,
                         MemoryTracker *memory, DpStats *stats)
{
    /*
     * An L_q edge is assigned a color d with d|row,d|column and weight
     * phi(d).  Here an endpoint not used by B_q merely chooses its color.
     * For color 1 no counter is needed; its count is derived at the end from
     * the total n-k endpoints, where k is the number of selected B-edges.
     * For colors 2..q, row and column endpoint counts are stored separately.
     * phi(d) is charged on the column endpoint, exactly once per eventual
     * color-d edge.
     */
    for (int color = 1; color <= board->q; ++color) {
        if (vertex % color != 0) {
            continue;
        }
        uint64_t code = state->code;
        if (color >= 2) {
            code = increment_code_count(
                code, color_count_field(color, is_row), board->n);
        }
        if (!counts_can_still_balance(code, board->q,
                                      future_rows, future_columns)) {
            continue;
        }
        unsigned endpoint_weight = is_row ? 1U : board->phi[color];
        uint64_t value = multiply_small_mod(
            state->value, endpoint_weight, next->modulus);
        if (!map_add(next, state->mask, code, value, memory)) {
            return false;
        }
        if (stats->transitions != UINT64_MAX) {
            ++stats->transitions;
        }
    }
    return true;
}

static bool start_next_map(StateMap *next, const StateMap *current,
                           MemoryTracker *memory)
{
    /* Size from the number of live keys, not from the old capacity.  Column
       forgetting can collapse tens of millions of intermediate states to a
       small table; carrying the old capacity forward caused repeated 1+ GiB
       memset calls.  A 3/2 estimate starts below the 70% load threshold and
       map_add grows it if the next event genuinely expands. */
    if (current->count > (SIZE_MAX - 16) / 3 * 2) {
        return false;
    }
    size_t needed = current->count + current->count / 2 + 16;
    size_t capacity = 16;
    while (capacity < needed) {
        if (capacity > SIZE_MAX / 2) {
            return false;
        }
        capacity *= 2;
    }
    return map_allocate(next, capacity, current->modulus, memory);
}

static bool process_row(StateMap *current, const Board *board, int row,
                        const int *future_rows,
                        const int *future_columns,
                        MemoryTracker *memory, DpStats *stats)
{
    StateMap next = { 0 };
    size_t forced_capacity = 0;
    uint64_t transition_checkpoint = stats->transitions;

retry_row:
    memset(&next, 0, sizeof(next));
    if (forced_capacity != 0) {
        if (!map_allocate(&next, forced_capacity,
                          current->modulus, memory)) {
            return false;
        }
    } else if (!start_next_map(&next, current, memory)) {
        return false;
    }

    for (size_t index = 0; index < current->capacity; ++index) {
        const StateEntry *state = &current->entries[index];
        if (state->code == EMPTY_CODE || state->value == 0) {
            continue;
        }
        if (!emit_monomer(&next, board, state, true, row,
                          future_rows, future_columns,
                          memory, stats)) {
            goto retry_larger_row;
        }

        for (unsigned edge = 0; edge < board->degree[row - 1]; ++edge) {
            int column = board->neighbor_column[row - 1][edge];
            unsigned weight = board->neighbor_weight[row - 1][edge];
            uint64_t bit = UINT64_C(1) << (column - 1);
            if ((state->mask & bit) != 0) {
                continue;
            }
            uint64_t value = multiply_small_mod(
                state->value, weight, next.modulus);
            uint64_t code = increment_code_count(
                state->code, 0, board->n);
            if (!counts_can_still_balance(code, board->q,
                                          future_rows, future_columns)) {
                continue;
            }
            if (!map_add(&next, state->mask | bit, code,
                         value, memory)) {
                goto retry_larger_row;
            }
            if (stats->transitions != UINT64_MAX) {
                ++stats->transitions;
            }
        }
    }

    map_free(current, memory);
    *current = next;
    return true;

retry_larger_row:
    if (next.capacity > SIZE_MAX / 2) {
        map_free(&next, memory);
        return false;
    }
    forced_capacity = next.capacity * 2;
    map_free(&next, memory);
    stats->transitions = transition_checkpoint;
    uint64_t bytes = (uint64_t)forced_capacity * sizeof(StateEntry);
    if (bytes > memory->limit - memory->used) {
        memory->failed_request = bytes;
        return false;
    }
    memory->failed_request = 0;
    goto retry_row;
}

static bool process_column(StateMap *current, const Board *board, int column,
                           const int *future_rows,
                           const int *future_columns,
                           MemoryTracker *memory, DpStats *stats)
{
    StateMap next = { 0 };
    size_t forced_capacity = 0;
    uint64_t transition_checkpoint = stats->transitions;
    uint64_t bit = UINT64_C(1) << (column - 1);

retry_column:
    memset(&next, 0, sizeof(next));
    if (forced_capacity != 0) {
        if (!map_allocate(&next, forced_capacity,
                          current->modulus, memory)) {
            return false;
        }
    } else if (!start_next_map(&next, current, memory)) {
        return false;
    }

    for (size_t index = 0; index < current->capacity; ++index) {
        const StateEntry *state = &current->entries[index];
        if (state->code == EMPTY_CODE || state->value == 0) {
            continue;
        }
        if ((state->mask & bit) != 0) {
            if (!counts_can_still_balance(state->code, board->q,
                                          future_rows, future_columns)) {
                continue;
            }
            if (!map_add(&next, state->mask ^ bit, state->code,
                         state->value, memory)) {
                goto retry_larger_column;
            }
            if (stats->transitions != UINT64_MAX) {
                ++stats->transitions;
            }
        } else if (!emit_monomer(&next, board, state, false, column,
                                 future_rows, future_columns,
                                 memory, stats)) {
            goto retry_larger_column;
        }
    }

    map_free(current, memory);
    *current = next;
    return true;

retry_larger_column:
    if (next.capacity > SIZE_MAX / 2) {
        map_free(&next, memory);
        return false;
    }
    forced_capacity = next.capacity * 2;
    map_free(&next, memory);
    stats->transitions = transition_checkpoint;
    uint64_t bytes = (uint64_t)forced_capacity * sizeof(StateEntry);
    if (bytes > memory->limit - memory->used) {
        memory->failed_request = bytes;
        return false;
    }
    memory->failed_request = 0;
    goto retry_column;
}

static void report_memory_failure(int n, int q, const char *stage,
                                  const StateMap *map,
                                  const MemoryTracker *memory)
{
    fprintf(stderr,
            "error: A085244 n=%d q=%d: memory limit exceeded during %s\n"
            "current states=%zu, current capacity=%zu, used=%.3f GiB, "
            "failed allocation=%.3f GiB, limit=%.3f GiB\n"
            "Increase A085244_02_MEMORY_MIB only if physical RAM permits.\n",
            n, q, stage, map->count, map->capacity,
            (double)memory->used / (double)(UINT64_C(1) << 30),
            (double)memory->failed_request / (double)(UINT64_C(1) << 30),
            (double)memory->limit / (double)(UINT64_C(1) << 30));
}

static void count_future_rows(const Board *board, int first_position,
                              int *counts)
{
    memset(counts, 0, 5 * sizeof(*counts));
    for (int position = first_position; position < board->n; ++position) {
        int row = board->order[position];
        for (int color = 2; color <= board->q; ++color) {
            if (row % color == 0) {
                ++counts[color];
            }
        }
    }
}

static void count_future_columns(const Board *board,
                                 const bool *column_done, int *counts)
{
    memset(counts, 0, 5 * sizeof(*counts));
    for (int column = 1; column <= board->n; ++column) {
        if (column_done[column - 1]) {
            continue;
        }
        for (int color = 2; color <= board->q; ++color) {
            if (column % color == 0) {
                ++counts[color];
            }
        }
    }
}

static bool permanent_frontier(uint64_t *result, const Board *board,
                               uint64_t modulus, uint64_t memory_limit,
                               size_t pass, size_t pass_count,
                               DpStats *stats)
{
    const int n = board->n;
    const int q = board->q;
    fprintf(stderr,
            "085244_02: n=%d pass=%zu/%zu p=%" PRIu64
            ", q=%d, row-frontier width=%d, memory limit=%.2f GiB\n",
            n, pass + 1, pass_count, modulus, q,
            board->maximum_frontier,
            (double)memory_limit / (double)(UINT64_C(1) << 30));

    MemoryTracker memory = { .limit = memory_limit };
    StateMap states = { 0 };
    if (!map_allocate(&states, 16, modulus, &memory)) {
        report_memory_failure(n, q, "initialization", &states, &memory);
        return false;
    }
    map_insert_without_growth(&states, 0, 0, 1);
    stats->peak_states = 1;
    stats->transitions = 0;
    double start = monotonic_seconds();
    int rows_processed = 0;
    int columns_processed = 0;
    bool column_done[MAX_N] = { false };

    for (int position = 0; position < n; ++position) {
        int row = board->order[position];
        int future_rows[5];
        int future_columns[5];
        count_future_rows(board, position + 1, future_rows);
        count_future_columns(board, column_done, future_columns);
        if (!process_row(&states, board, row,
                         future_rows, future_columns,
                         &memory, stats)) {
            report_memory_failure(n, q, "row transition", &states, &memory);
            map_free(&states, &memory);
            return false;
        }
        ++rows_processed;
        if (states.count > stats->peak_states) {
            stats->peak_states = states.count;
        }

        /* Forget every ordinary column at its final incident B-row. */
        for (int column = 1; column <= n; ++column) {
            if (board->last[column - 1] == position) {
                if (column_done[column - 1]) {
                    die("column scheduled more than once");
                }
                column_done[column - 1] = true;
                count_future_columns(board, column_done, future_columns);
                if (!process_column(&states, board, column,
                                    future_rows, future_columns,
                                    &memory, stats)) {
                    report_memory_failure(n, q, "column transition",
                                          &states, &memory);
                    map_free(&states, &memory);
                    return false;
                }
                ++columns_processed;
                if (states.count > stats->peak_states) {
                    stats->peak_states = states.count;
                }
            }
        }

        /* A zero-degree column has no frontier interval.  Processing it
           beside its identically numbered row keeps color balances small. */
        if (board->last[row - 1] < 0) {
            if (column_done[row - 1]) {
                die("zero-degree column scheduled more than once");
            }
            column_done[row - 1] = true;
            count_future_columns(board, column_done, future_columns);
            if (!process_column(&states, board, row,
                                future_rows, future_columns,
                                &memory, stats)) {
                report_memory_failure(n, q, "zero-column transition",
                                      &states, &memory);
                map_free(&states, &memory);
                return false;
            }
            ++columns_processed;
            if (states.count > stats->peak_states) {
                stats->peak_states = states.count;
            }
        }

        fprintf(stderr,
                "085244_02: n=%d pass=%zu/%zu row=%d/%d value=%d, "
                "states=%zu, allocated=%.3f GiB, elapsed=%.3f s\n",
                n, pass + 1, pass_count, position + 1, n, row,
                states.count,
                (double)memory.used / (double)(UINT64_C(1) << 30),
                monotonic_seconds() - start);
    }

    if (rows_processed != n || columns_processed != n) {
        die("frontier schedule did not process every vertex");
    }
    /*
     * Terminal contraction proof.  Fix a B-matching of size k.  For every
     * color d, suppose exactly k_d remaining rows and k_d remaining columns
     * chose d.  They can be bijected in k_d! ways, independently by color.
     * Every such edge has already received phi(d) from its column endpoint.
     * Hence multiplying by product_d k_d! gives exactly per(L_q) on the
     * vertices left by that B-matching.  Summing all terminal count states is
     * precisely the minor expansion per(L_q+B_q).
     */
    uint64_t factorial[MAX_N + 1] = { 0 };
    factorial[0] = 1;
    for (int index = 1; index <= n; ++index) {
        factorial[index] = multiply_small_mod(
            factorial[index - 1], (unsigned)index, modulus);
    }
    *result = 0;
    for (size_t index = 0; index < states.capacity; ++index) {
        const StateEntry *state = &states.entries[index];
        if (state->code == EMPTY_CODE || state->value == 0 ||
            state->mask != 0) {
            continue;
        }
        int b_edges = (int)code_count(state->code, 0);
        int row_nonone = 0;
        int column_nonone = 0;
        uint64_t completion = 1;
        bool balanced = true;
        for (int color = 2; color <= q; ++color) {
            int row_count = (int)code_count(
                state->code, color_count_field(color, true));
            int column_count = (int)code_count(
                state->code, color_count_field(color, false));
            row_nonone += row_count;
            column_nonone += column_count;
            if (row_count != column_count) {
                balanced = false;
                break;
            }
            completion = multiply_mod(
                completion, factorial[row_count], modulus);
        }
        int color_one_rows = n - b_edges - row_nonone;
        int color_one_columns = n - b_edges - column_nonone;
        if (!balanced || color_one_rows < 0 ||
            color_one_rows != color_one_columns) {
            continue;
        }
        completion = multiply_mod(
            completion, factorial[color_one_rows], modulus);
        uint64_t term = multiply_mod(state->value, completion, modulus);
        *result = add_mod(*result, term, modulus);
    }
    stats->seconds = monotonic_seconds() - start;
    fprintf(stderr,
            "085244_02: n=%d pass=%zu/%zu done, residue=%" PRIu64
            ", peak states=%zu, transitions=%" PRIu64
            ", peak allocation=%.3f GiB, %.3f s\n",
            n, pass + 1, pass_count, *result, stats->peak_states,
            stats->transitions,
            (double)memory.peak / (double)(UINT64_C(1) << 30),
            stats->seconds);
    map_free(&states, &memory);
    return true;
}

static bool permanent_direct_subset_dp(uint64_t *result, int n,
                                       uint64_t modulus,
                                       uint64_t memory_limit,
                                       double *seconds)
{
    if (n < 0 || n >= (int)(sizeof(size_t) * CHAR_BIT)) {
        return false;
    }
    size_t state_count = (size_t)1 << n;
    if (state_count > SIZE_MAX / sizeof(uint64_t)) {
        return false;
    }
    uint64_t bytes = (uint64_t)state_count * sizeof(uint64_t);
    if (bytes > memory_limit) {
        return false;
    }
    uint64_t *dp = calloc(state_count, sizeof(*dp));
    if (dp == NULL) {
        return false;
    }

    double start = monotonic_seconds();
    dp[0] = 1;
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
            uint64_t term = multiply_small_mod(
                value, (unsigned)gcd_int(row, column), modulus);
            dp[mask | bit] = add_mod(dp[mask | bit], term, modulus);
            available ^= bit;
        }
    }
    *result = dp[state_count - 1];
    *seconds = monotonic_seconds() - start;
    free(dp);
    return true;
}

static void permanent_upper_bound(mpz_t bound, int n)
{
    mpz_set_ui(bound, 1);
    for (int row = 1; row <= n; ++row) {
        unsigned long row_sum = 0;
        for (int column = 1; column <= n; ++column) {
            unsigned long entry = (unsigned long)gcd_int(row, column);
            if (row_sum > ULONG_MAX - entry) {
                die("row-sum upper bound overflow");
            }
            row_sum += entry;
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
        if (mpz_probab_prime_p(prime, 25) == 0 ||
            mpz_sizeinbase(prime, 2) > 61) {
            mpz_clears(candidate, prime, NULL);
            die("failed to select a verified 61-bit CRT prime");
        }
        uint64_t modulus = (uint64_t)mpz_get_ui(prime);
        if (modulus <= 2 || modulus >= (UINT64_C(1) << 61) ||
            (modulus & UINT64_C(1)) == 0) {
            mpz_clears(candidate, prime, NULL);
            die("selected CRT modulus is outside the safe range");
        }
        moduli[count++] = modulus;
        mpz_mul_ui(product, product, (unsigned long)modulus);
        mpz_set(candidate, prime);
    }
    mpz_clears(candidate, prime, NULL);
    if (count == 0 || moduli[0] != FIRST_REFERENCE_MODULUS) {
        die("unexpected first CRT prime; reference residues are invalid");
    }
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
        uint64_t inverse = power_mod(
            product_mod, modulus - 2, modulus);
        uint64_t result_mod =
            (uint64_t)mpz_fdiv_ui(result, (unsigned long)modulus);
        uint64_t difference = subtract_mod(
            residues[index], result_mod, modulus);
        uint64_t multiplier = multiply_mod(
            difference, inverse, modulus);
        mpz_addmul_ui(result, product, (unsigned long)multiplier);
        mpz_mul_ui(product, product, (unsigned long)modulus);
    }
    mpz_clear(product);
}

static void verify_crt(const mpz_t result, const mpz_t bound,
                       const mpz_t modulus_product,
                       const uint64_t *residues,
                       const uint64_t *moduli, size_t count)
{
    if (mpz_sgn(result) < 0 || mpz_cmp(result, bound) > 0) {
        die("CRT result lies outside the rigorous permanent bound");
    }
    if (mpz_cmp(modulus_product, bound) <= 0 ||
        mpz_cmp(result, modulus_product) >= 0) {
        die("CRT modulus product does not prove uniqueness");
    }
    for (size_t index = 0; index < count; ++index) {
        unsigned long actual = mpz_fdiv_ui(
            result, (unsigned long)moduli[index]);
        if ((uint64_t)actual != residues[index]) {
            die("CRT reconstruction residue check failed");
        }
    }
}

static bool compute_exact_term(mpz_t result, int n, int q,
                               uint64_t memory_limit)
{
    Board board;
    build_board(&board, n, q);

    mpz_t bound;
    mpz_t modulus_product;
    mpz_inits(bound, modulus_product, NULL);
    permanent_upper_bound(bound, n);
    uint64_t moduli[MAX_MODULUS_COUNT] = { 0 };
    uint64_t residues[MAX_MODULUS_COUNT] = { 0 };
    size_t modulus_count = choose_moduli(
        moduli, modulus_product, bound);
    size_t bound_bits = mpz_sizeinbase(bound, 2);

    fprintf(stderr,
            "085244_02: n=%d exact plan, q=%d, bound=%zu bits, "
            "%zu sequential CRT pass%s, frontier width=%d\n",
            n, q, bound_bits, modulus_count,
            modulus_count == 1 ? "" : "es", board.maximum_frontier);

    double start = monotonic_seconds();
    size_t peak_states = 0;
    uint64_t total_transitions = 0;
    for (size_t pass = 0; pass < modulus_count; ++pass) {
        DpStats stats = { 0 };
        if (!permanent_frontier(&residues[pass], &board, moduli[pass],
                                memory_limit, pass, modulus_count, &stats)) {
            mpz_clears(bound, modulus_product, NULL);
            return false;
        }
        if (stats.peak_states > peak_states) {
            peak_states = stats.peak_states;
        }
        if (UINT64_MAX - total_transitions < stats.transitions) {
            total_transitions = UINT64_MAX;
        } else {
            total_transitions += stats.transitions;
        }

        if (pass == 0 && n <= MAX_KNOWN_N &&
            residues[pass] != known_residues[n]) {
            fprintf(stderr,
                    "error: n=%d first residue=%" PRIu64
                    " differs from known residue=%" PRIu64 "\n",
                    n, residues[pass], known_residues[n]);
            mpz_clears(bound, modulus_product, NULL);
            return false;
        }
        if (pass == 0 && n <= MAX_KNOWN_N) {
            fprintf(stderr,
                    "085244_02: n=%d known first-residue check ok\n", n);
        }

        if (pass == 0 && n <= DIRECT_CHECK_MAX_N) {
            uint64_t direct = 0;
            double check_seconds = 0.0;
            if (permanent_direct_subset_dp(
                    &direct, n, moduli[pass], memory_limit,
                    &check_seconds)) {
                if (direct != residues[pass]) {
                    mpz_clears(bound, modulus_product, NULL);
                    die("frontier residue differs from direct subset DP");
                }
                fprintf(stderr,
                        "085244_02: n=%d independent subset-DP check ok, "
                        "%.3f s\n", n, check_seconds);
            } else {
                fprintf(stderr,
                        "085244_02: n=%d subset-DP check skipped "
                        "(memory unavailable)\n", n);
            }
        }
    }

    reconstruct_crt(result, residues, moduli, modulus_count);
    verify_crt(result, bound, modulus_product,
               residues, moduli, modulus_count);
    fprintf(stderr,
            "085244_02: n=%d exact CRT done, peak states=%zu, "
            "total transitions=%" PRIu64 ", %.3f s wall\n",
            n, peak_states, total_transitions,
            monotonic_seconds() - start);
    mpz_clears(bound, modulus_product, NULL);
    return true;
}

static void usage(const char *program)
{
    fprintf(stderr, "usage: %s N [1|2|3|4]\n", program);
}

int main(int argc, char **argv)
{
    if (argc < 2 || argc > 3) {
        usage(argv[0]);
        return EXIT_FAILURE;
    }
    int maximum_n = parse_int(
        argv[1], SEQUENCE_OFFSET, MAX_N, "N");
    int q = argc == 3 ? parse_int(argv[2], 1, 4, "Q") : 3;
    uint64_t memory_limit = memory_budget_bytes();

    char *part_path = path_beside_executable(
        argv[0], "b085244_02_part.txt");
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
        if (!compute_exact_term(value, n, q, memory_limit)) {
            fclose(output);
            mpz_clear(value);
            free(final_path);
            free(part_path);
            return EXIT_FAILURE;
        }
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
        fflush(stdout);
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
