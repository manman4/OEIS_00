/*
 * A093313, A093314, A093315 -- common sparse subset DP.
 *
 * Count permutations s_1,s_2,...,s_n of 1,2,...,n, optionally with a
 * prescribed first value s_1, such that
 *
 *     s_j divides s_1 + s_2 + ... + s_j       (1 <= j <= n).
 *
 * The prescribed first values 2, 3, and 4 give A093313, A093314, and
 * A093315, respectively.  Passing "_" instead counts all possible first
 * values and gives A067957.
 *
 * If a valid prefix has used the set S and has sum t, a new value x can
 * be appended exactly when
 *
 *     x | (t + x)  <=>  x | t.
 *
 * Thus the condition depends only on S, not on the order of the elements
 * in S.  The program uses sparse subset DP from both ends.  The forward
 * map starts at {s_1}; the backward map starts at {1,...,n}.  At each step
 * the frontier with fewer candidate transitions is expanded.  The search
 * stops with a small gap between the frontier cardinalities; valid paths
 * crossing that gap are enumerated directly.  This avoids constructing the
 * usually largest middle layers as hash tables.
 *
 * Counts use unsigned 128-bit integers and every addition and multiplication
 * is checked.  Subsets use uint64_t, so the supported range is n <= 63.
 * The actual running-time and memory limit is normally reached much sooner.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic \
 *       093313_01.c -o 093313_01
 *
 * -march=native is an optional machine-specific optimization and may be
 * added only when the compiler and target support it.
 *
 * Examples:
 *   ./093313_01 _ --upto 41    # A067957
 *   ./093313_01 2 --upto 44    # A093313
 *   ./093313_01 3 --upto 40    # A093314
 *   ./093313_01 4 --term 40    # A093315(40)
 *   ./093313_01 2 --check 44
 *
 * Standard output is in OEIS b-file format.  A successful non-check run also
 * saves the same output atomically as b09331(K+1)_01.txt, where K is s_1.
 * Thus K=2, 3, 4 use b093313_01.txt, b093314_01.txt, b093315_01.txt.
 * The unrestricted "_" mode uses b067957_01.txt.
 * Progress is written to stderr.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#if !defined(__SIZEOF_INT128__)
#error "093313_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 63U
#define DEFAULT_MAX_N 40U
#define MAX_BRIDGE_STEPS 8U
#define AUTO_BRIDGE_MAX_STEPS 4U
#define AUTO_BRIDGE_MIN_STATES ((size_t)10000000)
#define INITIAL_CAPACITY ((size_t)16)
#define LOAD_NUMERATOR ((size_t)7)
#define LOAD_DENOMINATOR ((size_t)10)

typedef struct {
    uint64_t *key;
    uint64_t *count_low;
    uint64_t *count_high;
    uint16_t *sum;
    size_t capacity;
    size_t size;
} StateMap;

typedef struct {
    uint64_t expanded_states;
    uint64_t transitions;
    size_t peak_slots;
} Statistics;

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} OutputMode;

static bool quiet;
static bool verbose;
static unsigned bridge_steps;

static const char *const known_s2[] = {
    NULL,
    "0", "1", "1", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "6", "6", "1", "11", "9", "15", "14", "14", "23"
};

static const char *const known_s3[] = {
    NULL,
    "0", "0", "1", "1", "1", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "0", "0", "0", "0", "0", "0", "0",
    "0", "0", "0", "0", "1", "0", "0", "0", "0", "0", "0",
    "0", "12", "20", "20", "1", "163", "55"
};

static const char *const known_s4[] = {
    NULL,
    "0", "0", "0", "1", "2", "2", "2", "0", "0", "0", "0",
    "1", "1", "0", "1", "1", "1", "0", "0", "0", "0", "0",
    "0", "6", "8", "3", "14", "12", "18", "13", "14", "6", "26",
    "13", "198", "152", "220", "118", "1033", "807"
};

static const char *const known_all[] = {
    "1", "1", "1", "2", "2", "4", "5", "7", "7", "24",
    "22", "29", "39", "67", "55", "386", "235", "312", "347",
    "451", "1319", "5320", "3220", "4489", "20237", "36580",
    "52875", "197103", "216562", "289478", "567396", "659647",
    "1111153", "3131774", "2200426", "29523302", "34214028",
    "48161995", "32616148", "242860900", "293579041", "363415618"
};

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static void *xcalloc(size_t count, size_t size)
{
    if (size != 0U && count > SIZE_MAX / size) {
        die("allocation size overflow");
    }
    void *pointer = calloc(count, size);
    if (pointer == NULL) {
        die("out of memory");
    }
    return pointer;
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

static void u128_to_text(U128 value, char text[40])
{
    char reverse[40];
    size_t length = 0;
    do {
        reverse[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0);
    for (size_t i = 0; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0;
    if (text == NULL || *text == '\0') {
        return false;
    }
    while (*text != '\0') {
        if (*text < '0' || *text > '9') {
            return false;
        }
        const unsigned digit = (unsigned)(*text++ - '0');
        if (value > (maximum - digit) / 10U) {
            return false;
        }
        value = value * 10U + digit;
    }
    *result = value;
    return true;
}

static uint64_t hash_mask(uint64_t value)
{
    value ^= value >> 30;
    value *= UINT64_C(0xbf58476d1ce4e5b9);
    value ^= value >> 27;
    value *= UINT64_C(0x94d049bb133111eb);
    value ^= value >> 31;
    return value;
}

static StateMap map_create(size_t capacity)
{
    StateMap map;
    map.capacity = capacity;
    map.size = 0;
    map.key = xcalloc(capacity, sizeof(*map.key));
    map.count_low = xcalloc(capacity, sizeof(*map.count_low));
    map.count_high = NULL;
    map.sum = xcalloc(capacity, sizeof(*map.sum));
    return map;
}

static void map_destroy(StateMap *map)
{
    free(map->key);
    free(map->count_low);
    free(map->count_high);
    free(map->sum);
    memset(map, 0, sizeof(*map));
}

static U128 map_count_at(const StateMap *map, size_t slot)
{
    const U128 high = map->count_high == NULL ? 0U : map->count_high[slot];
    return (high << 64) | map->count_low[slot];
}

static void map_store_count(StateMap *map, size_t slot, U128 count)
{
    const uint64_t high = (uint64_t)(count >> 64);
    if (high != 0U && map->count_high == NULL) {
        map->count_high = xcalloc(map->capacity, sizeof(*map->count_high));
    }
    map->count_low[slot] = (uint64_t)count;
    if (map->count_high != NULL) {
        map->count_high[slot] = high;
    }
}

static void map_insert_without_growth(StateMap *map, uint64_t key,
                                      uint16_t sum, U128 count)
{
    size_t slot = (size_t)hash_mask(key) & (map->capacity - 1U);
    while (map->key[slot] != 0U) {
        slot = (slot + 1U) & (map->capacity - 1U);
    }
    map->key[slot] = key;
    map->sum[slot] = sum;
    map_store_count(map, slot, count);
    ++map->size;
}

static void map_grow(StateMap *map)
{
    if (map->capacity > SIZE_MAX / 2U) {
        die("hash-table capacity overflow");
    }
    StateMap grown = map_create(map->capacity * 2U);
    if (map->count_high != NULL) {
        grown.count_high = xcalloc(grown.capacity, sizeof(*grown.count_high));
    }
    for (size_t i = 0; i < map->capacity; ++i) {
        if (map->key[i] != 0U) {
            map_insert_without_growth(&grown, map->key[i], map->sum[i],
                                      map_count_at(map, i));
        }
    }
    map_destroy(map);
    *map = grown;
}

static void checked_add(U128 *target, U128 value)
{
    const U128 maximum = ~(U128)0;
    if (*target > maximum - value) {
        die("unsigned 128-bit count overflow");
    }
    *target += value;
}

static void map_add(StateMap *map, uint64_t key, uint16_t sum, U128 count)
{
    if ((map->size + 1U) * LOAD_DENOMINATOR >=
        map->capacity * LOAD_NUMERATOR) {
        map_grow(map);
    }

    size_t slot = (size_t)hash_mask(key) & (map->capacity - 1U);
    while (map->key[slot] != 0U && map->key[slot] != key) {
        slot = (slot + 1U) & (map->capacity - 1U);
    }
    if (map->key[slot] == 0U) {
        map->key[slot] = key;
        map->sum[slot] = sum;
        map_store_count(map, slot, count);
        ++map->size;
    } else {
        if (map->sum[slot] != sum) {
            die("internal subset-sum mismatch");
        }
        U128 updated = map_count_at(map, slot);
        checked_add(&updated, count);
        map_store_count(map, slot, updated);
    }
}

static bool map_find(const StateMap *map, uint64_t key, U128 *count)
{
    size_t slot = (size_t)hash_mask(key) & (map->capacity - 1U);
    while (map->key[slot] != 0U) {
        if (map->key[slot] == key) {
            *count = map_count_at(map, slot);
            return true;
        }
        slot = (slot + 1U) & (map->capacity - 1U);
    }
    return false;
}

static uint64_t saturated_add_u64(uint64_t left, uint64_t right)
{
    return left > UINT64_MAX - right ? UINT64_MAX : left + right;
}

static uint64_t forward_transition_count(
    const StateMap *map, const uint64_t *divisor_mask, uint64_t full_mask)
{
    uint64_t total = 0;
    for (size_t i = 0; i < map->capacity; ++i) {
        if (map->key[i] == 0U) {
            continue;
        }
        const uint64_t candidates = divisor_mask[map->sum[i]] &
                                    full_mask & ~map->key[i];
        total = saturated_add_u64(total,
                                  (uint64_t)__builtin_popcountll(candidates));
    }
    return total;
}

static uint64_t backward_candidates(uint64_t key, uint16_t sum,
                                    const uint64_t *divisor_mask,
                                    uint64_t allowed_first_mask)
{
    const uint64_t present_first = key & allowed_first_mask;
    if (present_first == 0U) {
        die("internal state has no allowed first value");
    }
    uint64_t candidates = divisor_mask[sum] & key;
    if ((present_first & (present_first - 1U)) == 0U) {
        candidates &= ~present_first;
    }
    return candidates;
}

static uint64_t backward_transition_count(
    const StateMap *map, const uint64_t *divisor_mask,
    uint64_t allowed_first_mask)
{
    uint64_t total = 0;
    for (size_t i = 0; i < map->capacity; ++i) {
        if (map->key[i] == 0U) {
            continue;
        }
        const uint64_t candidates = backward_candidates(
            map->key[i], map->sum[i], divisor_mask, allowed_first_mask);
        total = saturated_add_u64(total,
                                  (uint64_t)__builtin_popcountll(candidates));
    }
    return total;
}

static StateMap expand_forward(const StateMap *source,
                               const uint64_t *divisor_mask,
                               uint64_t full_mask, Statistics *statistics)
{
    StateMap destination = map_create(INITIAL_CAPACITY);
    for (size_t i = 0; i < source->capacity; ++i) {
        if (source->key[i] == 0U) {
            continue;
        }
        ++statistics->expanded_states;
        uint64_t candidates = divisor_mask[source->sum[i]] &
                              full_mask & ~source->key[i];
        while (candidates != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            candidates &= candidates - 1U;
            ++statistics->transitions;
            map_add(&destination, source->key[i] | bit_mask,
                    (uint16_t)(source->sum[i] + bit + 1U),
                    map_count_at(source, i));
        }
    }
    return destination;
}

static StateMap expand_backward(const StateMap *source,
                                const uint64_t *divisor_mask,
                                uint64_t allowed_first_mask,
                                Statistics *statistics)
{
    StateMap destination = map_create(INITIAL_CAPACITY);
    for (size_t i = 0; i < source->capacity; ++i) {
        if (source->key[i] == 0U) {
            continue;
        }
        ++statistics->expanded_states;
        uint64_t candidates = backward_candidates(
            source->key[i], source->sum[i], divisor_mask,
            allowed_first_mask);
        while (candidates != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            candidates &= candidates - 1U;
            ++statistics->transitions;
            map_add(&destination, source->key[i] ^ bit_mask,
                    (uint16_t)(source->sum[i] - bit - 1U),
                    map_count_at(source, i));
        }
    }
    return destination;
}

static U128 join_maps(const StateMap *left, const StateMap *right)
{
    if (left->size > right->size) {
        const StateMap *temporary = left;
        left = right;
        right = temporary;
    }

    U128 answer = 0;
    const U128 maximum = ~(U128)0;
    for (size_t i = 0; i < left->capacity; ++i) {
        if (left->key[i] == 0U) {
            continue;
        }
        U128 other;
        if (!map_find(right, left->key[i], &other)) {
            continue;
        }
        const U128 left_count = map_count_at(left, i);
        if (left_count != 0U && other > maximum / left_count) {
            die("unsigned 128-bit count overflow at frontier join");
        }
        checked_add(&answer, left_count * other);
    }
    return answer;
}

static void checked_add_product(U128 *answer, U128 left, U128 right)
{
    const U128 maximum = ~(U128)0;
    if (left != 0U && right > maximum / left) {
        die("unsigned 128-bit count overflow at frontier join");
    }
    checked_add(answer, left * right);
}

static void bridge_forward(uint64_t mask, uint16_t sum, unsigned steps,
                           U128 prefix_count, const StateMap *backward,
                           const uint64_t *divisor_mask, uint64_t full_mask,
                           Statistics *statistics, U128 *answer)
{
    if (steps == 0U) {
        U128 suffix_count;
        if (map_find(backward, mask, &suffix_count)) {
            checked_add_product(answer, prefix_count, suffix_count);
        }
        return;
    }

    uint64_t candidates = divisor_mask[sum] & full_mask & ~mask;
    while (candidates != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(candidates);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        candidates &= candidates - 1U;
        ++statistics->transitions;
        bridge_forward(mask | bit_mask, (uint16_t)(sum + bit + 1U),
                       steps - 1U, prefix_count, backward, divisor_mask,
                       full_mask, statistics, answer);
    }
}

static void bridge_backward(uint64_t mask, uint16_t sum, unsigned steps,
                            U128 suffix_count, const StateMap *forward,
                            const uint64_t *divisor_mask,
                            uint64_t allowed_first_mask,
                            Statistics *statistics, U128 *answer)
{
    if (steps == 0U) {
        U128 prefix_count;
        if (map_find(forward, mask, &prefix_count)) {
            checked_add_product(answer, prefix_count, suffix_count);
        }
        return;
    }

    uint64_t candidates = backward_candidates(
        mask, sum, divisor_mask, allowed_first_mask);
    while (candidates != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(candidates);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        candidates &= candidates - 1U;
        ++statistics->transitions;
        bridge_backward(mask ^ bit_mask, (uint16_t)(sum - bit - 1U),
                        steps - 1U, suffix_count, forward, divisor_mask,
                        allowed_first_mask, statistics, answer);
    }
}

static U128 join_across_gap(const StateMap *forward, const StateMap *backward,
                            unsigned gap, const uint64_t *divisor_mask,
                            uint64_t full_mask, uint64_t allowed_first_mask,
                            Statistics *statistics)
{
    const uint64_t forward_work = forward_transition_count(
        forward, divisor_mask, full_mask);
    const uint64_t backward_work = backward_transition_count(
        backward, divisor_mask, allowed_first_mask);
    U128 answer = 0;

    if (forward_work <= backward_work) {
        for (size_t i = 0; i < forward->capacity; ++i) {
            if (forward->key[i] != 0U) {
                bridge_forward(forward->key[i], forward->sum[i], gap,
                               map_count_at(forward, i), backward,
                               divisor_mask, full_mask, statistics, &answer);
            }
        }
    } else {
        for (size_t i = 0; i < backward->capacity; ++i) {
            if (backward->key[i] != 0U) {
                bridge_backward(backward->key[i], backward->sum[i], gap,
                                map_count_at(backward, i), forward,
                                divisor_mask, allowed_first_mask,
                                statistics, &answer);
            }
        }
    }
    return answer;
}

static void update_peak_slots(Statistics *statistics,
                              const StateMap *forward,
                              const StateMap *backward)
{
    if (forward->capacity > SIZE_MAX - backward->capacity) {
        die("slot count overflow");
    }
    const size_t slots = forward->capacity + backward->capacity;
    if (slots > statistics->peak_slots) {
        statistics->peak_slots = slots;
    }
}

static U128 count_first_group(unsigned n, uint64_t allowed_first_mask,
                              const uint64_t *divisor_mask,
                              uint64_t full_mask, Statistics *statistics,
                              unsigned *forward_depth_result,
                              unsigned *backward_depth_result,
                              size_t *forward_size, size_t *backward_size)
{
    memset(statistics, 0, sizeof(*statistics));
    *forward_depth_result = 0;
    *backward_depth_result = 0;
    *forward_size = 0;
    *backward_size = 0;

    const unsigned total_sum = n * (n + 1U) / 2U;
    allowed_first_mask &= full_mask;
    if (allowed_first_mask == 0U) {
        return 0U;
    }

    StateMap forward = map_create(INITIAL_CAPACITY);
    StateMap backward = map_create(INITIAL_CAPACITY);
    uint64_t first_values = allowed_first_mask;
    while (first_values != 0U) {
        const unsigned bit = (unsigned)__builtin_ctzll(first_values);
        const uint64_t bit_mask = UINT64_C(1) << bit;
        first_values &= first_values - 1U;
        map_add(&forward, bit_mask, (uint16_t)(bit + 1U), 1U);
    }
    map_add(&backward, full_mask, (uint16_t)total_sum, 1U);

    unsigned forward_depth = 1U;
    unsigned backward_depth = n;
    size_t previous_forward_size = forward.size;
    size_t previous_backward_size = backward.size;
    update_peak_slots(statistics, &forward, &backward);

    while (forward_depth + 1U < backward_depth &&
           forward.size != 0U && backward.size != 0U) {
        const unsigned gap = backward_depth - forward_depth;
        const bool forced_bridge = bridge_steps != 0U && gap <= bridge_steps;
        const bool growing_large_frontier =
            (forward.size >= AUTO_BRIDGE_MIN_STATES &&
             forward.size >= previous_forward_size) ||
            (backward.size >= AUTO_BRIDGE_MIN_STATES &&
             backward.size >= previous_backward_size);
        const bool automatic_bridge = bridge_steps == 0U &&
            gap <= AUTO_BRIDGE_MAX_STEPS && growing_large_frontier;
        if (forced_bridge || automatic_bridge) {
            break;
        }

        const uint64_t forward_work = forward_transition_count(
            &forward, divisor_mask, full_mask);
        const uint64_t backward_work = backward_transition_count(
            &backward, divisor_mask, allowed_first_mask);

        if (verbose) {
            fprintf(stderr,
                    "layer: depths=%u/%u states=%zu/%zu work=%" PRIu64
                    "/%" PRIu64 "\n",
                    forward_depth, backward_depth,
                    forward.size, backward.size,
                    forward_work, backward_work);
        }

        if (forward_work <= backward_work) {
            StateMap next = expand_forward(&forward, divisor_mask, full_mask,
                                           statistics);
            previous_forward_size = forward.size;
            map_destroy(&forward);
            forward = next;
            ++forward_depth;
        } else {
            StateMap next = expand_backward(&backward, divisor_mask,
                                            allowed_first_mask, statistics);
            previous_backward_size = backward.size;
            map_destroy(&backward);
            backward = next;
            --backward_depth;
        }
        update_peak_slots(statistics, &forward, &backward);
    }

    U128 answer = 0;
    if (forward_depth == backward_depth &&
        forward.size != 0U && backward.size != 0U) {
        answer = join_maps(&forward, &backward);
    } else if (forward_depth < backward_depth &&
               forward.size != 0U && backward.size != 0U) {
        answer = join_across_gap(
            &forward, &backward, backward_depth - forward_depth,
            divisor_mask, full_mask, allowed_first_mask, statistics);
    }

    *forward_depth_result = forward_depth;
    *backward_depth_result = backward_depth;
    *forward_size = forward.size;
    *backward_size = backward.size;
    map_destroy(&forward);
    map_destroy(&backward);
    return answer;
}

static U128 count_term(unsigned n, unsigned first, Statistics *statistics,
                       unsigned *forward_depth_result,
                       unsigned *backward_depth_result,
                       size_t *forward_size, size_t *backward_size)
{
    memset(statistics, 0, sizeof(*statistics));
    *forward_depth_result = 0U;
    *backward_depth_result = 0U;
    *forward_size = 0U;
    *backward_size = 0U;

    if (n == 0U) {
        return first == 0U ? 1U : 0U;
    }
    if (first > n) {
        return 0U;
    }

    const unsigned total_sum = n * (n + 1U) / 2U;
    uint64_t *divisor_mask = xcalloc(total_sum + 1U,
                                     sizeof(*divisor_mask));
    for (unsigned value = 1U; value <= n; ++value) {
        const uint64_t bit = UINT64_C(1) << (value - 1U);
        for (unsigned sum = value; sum <= total_sum; sum += value) {
            divisor_mask[sum] |= bit;
        }
    }
    const uint64_t full_mask = (UINT64_C(1) << n) - 1U;

    const uint64_t allowed_first_mask = first == 0U ? full_mask :
        UINT64_C(1) << (first - 1U);
    const U128 answer = count_first_group(
        n, allowed_first_mask, divisor_mask, full_mask, statistics,
        forward_depth_result, backward_depth_result,
        forward_size, backward_size);

    free(divisor_mask);
    return answer;
}

static const char *known_term(unsigned first, unsigned n)
{
    if (first == 0U && n < sizeof(known_all) / sizeof(known_all[0])) {
        return known_all[n];
    }
    if (first == 2U && n < sizeof(known_s2) / sizeof(known_s2[0])) {
        return known_s2[n];
    }
    if (first == 3U && n < sizeof(known_s3) / sizeof(known_s3[0])) {
        return known_s3[n];
    }
    if (first == 4U && n < sizeof(known_s4) / sizeof(known_s4[0])) {
        return known_s4[n];
    }
    return NULL;
}

static unsigned known_maximum(unsigned first)
{
    if (first == 0U) {
        return (unsigned)(sizeof(known_all) / sizeof(known_all[0]) - 1U);
    }
    if (first == 2U) {
        return (unsigned)(sizeof(known_s2) / sizeof(known_s2[0]) - 1U);
    }
    if (first == 3U) {
        return (unsigned)(sizeof(known_s3) / sizeof(known_s3[0]) - 1U);
    }
    if (first == 4U) {
        return (unsigned)(sizeof(known_s4) / sizeof(known_s4[0]) - 1U);
    }
    return 0U;
}

static FILE *open_output_file(unsigned first, char path[64], char part_path[72])
{
    const int path_length = first == 0U ?
        snprintf(path, 64, "b067957_01.txt") :
        snprintf(path, 64, "b09331%u_01.txt", first + 1U);
    if (path_length < 0 || path_length >= 64) {
        die("output path is too long");
    }
    const int part_length = snprintf(part_path, 72, "%s.part", path);
    if (part_length < 0 || part_length >= 72) {
        die("temporary output path is too long");
    }

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
            "  %s S1 [MAX_N]\n"
            "  %s S1 --upto MAX_N\n"
            "  %s S1 --term N\n"
            "  %s S1 --check [MAX_N]\n"
            "\n"
            "S1=2, 3, 4 gives A093313, A093314, A093315.\n"
            "S1=_ allows any first value and gives A067957.\n"
            "Options: --quiet, --verbose, --bridge-steps N\n"
            "The default bridge depth is selected automatically.\n",
            program, program, program, program);
    exit(status);
}

int main(int argc, char **argv)
{
    if (argc == 2 &&
        (strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0)) {
        usage(argv[0], EXIT_SUCCESS);
    }
    if (argc < 2) {
        usage(argv[0], EXIT_FAILURE);
    }

    const bool unrestricted = strcmp(argv[1], "_") == 0;
    const unsigned first = unrestricted ? 0U :
        parse_unsigned(argv[1], 1U, MAX_N, "S1");
    const unsigned minimum_n = unrestricted ? 0U : 1U;
    char first_text[16];
    if (unrestricted) {
        strcpy(first_text, "_");
    } else {
        snprintf(first_text, sizeof(first_text), "%u", first);
    }
    unsigned maximum = DEFAULT_MAX_N;
    OutputMode mode = MODE_UPTO;
    bool have_n = false;

    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--bridge-steps") == 0) {
            if (i + 1 >= argc) {
                usage(argv[0], EXIT_FAILURE);
            }
            bridge_steps = parse_unsigned(argv[++i], 1U, MAX_BRIDGE_STEPS,
                                          "BRIDGE_STEPS");
        } else if (strcmp(argv[i], "--term") == 0 ||
                   strcmp(argv[i], "--upto") == 0) {
            if (have_n || i + 1 >= argc) {
                usage(argv[0], EXIT_FAILURE);
            }
            mode = strcmp(argv[i], "--term") == 0 ? MODE_TERM : MODE_UPTO;
            maximum = parse_unsigned(argv[++i], minimum_n, MAX_N, "N");
            have_n = true;
        } else if (strcmp(argv[i], "--check") == 0) {
            if (mode != MODE_UPTO || have_n) {
                usage(argv[0], EXIT_FAILURE);
            }
            mode = MODE_CHECK;
            maximum = known_maximum(first);
            if (maximum == 0U) {
                die("--check has no known terms for this S1");
            }
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                maximum = parse_unsigned(argv[++i], minimum_n, maximum,
                                         "MAX_N");
            }
            have_n = true;
        } else {
            if (have_n || argv[i][0] == '-') {
                usage(argv[0], EXIT_FAILURE);
            }
            maximum = parse_unsigned(argv[i], minimum_n, MAX_N, "MAX_N");
            have_n = true;
        }
    }

    char output_path[64] = {0};
    char part_path[72] = {0};
    FILE *output_file = NULL;
    if (mode != MODE_CHECK) {
        output_file = open_output_file(first, output_path, part_path);
    }

    const unsigned begin = mode == MODE_TERM ? maximum : minimum_n;
    for (unsigned n = begin; n <= maximum; ++n) {
        Statistics statistics;
        unsigned forward_depth;
        unsigned backward_depth;
        size_t forward_size;
        size_t backward_size;
        const double started = monotonic_seconds();
        const U128 answer = count_term(n, first, &statistics,
                                       &forward_depth, &backward_depth,
                                       &forward_size, &backward_size);
        const double elapsed = monotonic_seconds() - started;

        char answer_text[40];
        u128_to_text(answer, answer_text);

        if (mode == MODE_CHECK) {
            const char *expected_text = known_term(first, n);
            U128 expected;
            if (!parse_u128(expected_text, &expected)) {
                die("invalid built-in known term");
            }
            if (answer != expected) {
                fprintf(stderr,
                        "error: mismatch at s_1=%s, n=%u: got %s, expected %s\n",
                        first_text, n, answer_text, expected_text);
                return EXIT_FAILURE;
            }
        } else {
            printf("%u %s\n", n, answer_text);
            if (fprintf(output_file, "%u %s\n", n, answer_text) < 0 ||
                fflush(output_file) != 0) {
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
                    "093313_01: s_1=%s n=%u answer=%s join=%u/%u "
                    "frontiers=%zu/%zu transitions=%" PRIu64
                    " peak-slots=%zu time=%.3fs%s\n",
                    first_text, n, answer_text, forward_depth, backward_depth,
                    forward_size, backward_size, statistics.transitions,
                    statistics.peak_slots, elapsed,
                    mode == MODE_CHECK ? " [OK]" : "");
        }
    }

    if (output_file != NULL) {
        finish_output_file(output_file, part_path, output_path);
        if (!quiet) {
            fprintf(stderr, "saved: %s\n", output_path);
        }
    }

    if (mode == MODE_CHECK && quiet) {
        fprintf(stderr, "s_1=%s: terms %u..%u OK\n",
                first_text, minimum_n, maximum);
    }
    return EXIT_SUCCESS;
}
