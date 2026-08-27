/*
 * A073090 -- bidirectional sparse DP.
 *
 * This algorithm is independent of 073090_01.c's full forward traversal.
 * Put q=p^-1, L=lcm(1,...,n), and w_j=L/j.  Inversion is a bijection, and
 * the required integrality is exactly
 *
 *                    sum_j q(j)*w_j == 0 (mod L).
 *
 * Version 03 grows one frontier from the initial state
 *
 *              (unused numerators = [n], residue = 0 mod L)
 *
 * and a second frontier backward from the modular terminal state
 *
 *                (used numerators = [n], residue = 0 mod L).
 *
 * The two frontiers meet after m assignments.  A forward state and a
 * backward state match when their numerator masks are complementary and
 * their prefix residues are equal.  Their path counts are multiplied and
 * summed.  Thus neither search traverses all n layers as in version 01.
 *
 * Safe pruning proof.  For every prime r<=n, let P_r be its largest power not
 * exceeding n.  The P_r are coprime and their product is L.  Modulo P_r, a
 * term q(j)*L/j vanishes unless r divides j.  If D is the set of denominator
 * positions still unassigned, every future contribution is divisible by
 *
 *       g_r = gcd(P_r, { L/j : j in D and r divides j }).
 *
 * Hence a completable prefix residue must be divisible by every g_r, and by
 * their product.  The test is only a necessary condition, so it cannot remove
 * a valid permutation.  It applies equally to a predecessor constructed by
 * the backward search.  Denominator ordering and meeting depth affect only
 * speed.  If n is prime, reduction modulo n forces q(n)=n and gives
 * a(n)=a(n-1).
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic 073090_03.c -o 073090_03
 *
 * Usage:
 *   ./073090_03
 *   ./073090_03 --upto 26 --verbose
 *   ./073090_03 --term 26 --memory-mb 4096 --verbose
 *   ./073090_03 --term 26 --meet 20 --verbose
 *   ./073090_03 --check
 *
 * The default and --upto atomically replace b073090_03.txt while holding an
 * exclusive writer lock.  --term and --check do not modify the b-file.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

#define MAX_N 26
#define DEFAULT_MAX_N 25
#define KNOWN_MAX_N 17
#define DIRECT_CHECK_MAX_N 10
#define MAX_CONSTRAINTS 9
#define DEFAULT_MEMORY_MB 4096
#define MIN_MEMORY_MB 32
#define MAX_MEMORY_MB 65536
#define INITIAL_CAPACITY 1024
#define BFILE_NAME "b073090_03.txt"
#define BFILE_TEMP_TEMPLATE BFILE_NAME ".tmp.XXXXXX"
#define BFILE_LOCK_NAME BFILE_NAME ".lock"

_Static_assert(MAX_N < 32, "32-bit masks require n<32");

static const uint64_t known[KNOWN_MAX_N + 1] = {
    UINT64_C(1),        UINT64_C(1),        UINT64_C(1),
    UINT64_C(1),        UINT64_C(2),        UINT64_C(2),
    UINT64_C(8),        UINT64_C(8),        UINT64_C(22),
    UINT64_C(104),      UINT64_C(1128),     UINT64_C(1128),
    UINT64_C(14520),    UINT64_C(14520),    UINT64_C(229734),
    UINT64_C(3217088),  UINT64_C(21157428), UINT64_C(21157428)
};

typedef uint32_t mask_t;

typedef struct {
    mask_t mask;
    uint32_t modulus;
} Constraint;

typedef struct {
    uint64_t key_plus_one;
    uint64_t count;
} Entry;

typedef struct {
    Entry *entry;
    size_t capacity;
    size_t used;
} Table;

typedef struct {
    int n;
    uint64_t lcm;
    uint64_t weight[MAX_N + 1];
    Constraint constraint[MAX_CONSTRAINTS];
    unsigned constraint_count;
    int order[MAX_N];
    uint64_t prefix_required[MAX_N + 1];
    size_t memory_limit;
    size_t live_memory;
    size_t peak_memory;
    size_t peak_states;
    uint64_t transitions;
    bool verbose_layers;
} Search;

typedef struct {
    size_t peak_states;
    size_t peak_memory;
    uint64_t transitions;
    int searched_n;
    int meet;
    bool prime_step;
} Statistics;

typedef enum {
    MODE_UPTO,
    MODE_TERM
} OutputMode;

static char bfile_temp_name[] = BFILE_TEMP_TEMPLATE;
static int bfile_lock_descriptor = -1;
static bool bfile_temp_active = false;

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static double monotonic_seconds(void)
{
    struct timespec now;
    if (clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        die("clock_gettime failed");
    return (double)now.tv_sec + (double)now.tv_nsec / 1000000000.0;
}

static int parse_integer(const char *text, const char *label,
                         int minimum, int maximum)
{
    errno = 0;
    char *end = NULL;
    intmax_t value = strtoimax(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %d..%d: %s\n",
                label, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (int)value;
}

static int parse_n(const char *text)
{
    return parse_integer(text, "n", 0, MAX_N);
}

static unsigned parse_memory_mb(const char *text)
{
    return (unsigned)parse_integer(text, "memory-mb",
                                   MIN_MEMORY_MB, MAX_MEMORY_MB);
}

static uint64_t gcd64(uint64_t a, uint64_t b)
{
    while (b != 0U) {
        const uint64_t r = a % b;
        a = b;
        b = r;
    }
    return a;
}

static bool prime(unsigned n)
{
    if (n < 2U) return false;
    if ((n & 1U) == 0U) return n == 2U;
    for (unsigned d = 3; d <= n / d; d += 2U)
        if (n % d == 0U) return false;
    return true;
}

static uint64_t lcm_to(int n)
{
    uint64_t result = 1;
    for (uint64_t k = 2; k <= (uint64_t)n; ++k) {
        const uint64_t factor = k / gcd64(result, k);
        if (result > UINT64_MAX / factor) die("LCM overflow");
        result *= factor;
    }
    return result;
}

static unsigned popcount(mask_t x)
{
    return (unsigned)__builtin_popcount(x);
}

static int first(mask_t x)
{
    return __builtin_ctz(x) + 1;
}

static mask_t full_mask(int n)
{
    return ((mask_t)1U << n) - 1U;
}

static uint64_t hash64(uint64_t x)
{
    x ^= x >> 30;
    x *= UINT64_C(0xbf58476d1ce4e5b9);
    x ^= x >> 27;
    x *= UINT64_C(0x94d049bb133111eb);
    return x ^ (x >> 31);
}

static Entry *allocate_entries(Search *s, size_t capacity)
{
    if (capacity > SIZE_MAX / sizeof(Entry)) die("allocation overflow");
    size_t bytes = capacity * sizeof(Entry);
    if (bytes > s->memory_limit - s->live_memory) {
        fprintf(stderr,
                "error: bidirectional DP memory limit exceeded at n=%d "
                "(live %.1f MiB + request %.1f MiB > limit %.1f MiB); "
                "peak_states=%zu, transitions=%" PRIu64 "; "
                "increase --memory-mb\n",
                s->n, (double)s->live_memory / 1048576.0,
                (double)bytes / 1048576.0,
                (double)s->memory_limit / 1048576.0, s->peak_states,
                s->transitions);
        exit(EXIT_FAILURE);
    }
    Entry *p = calloc(capacity, sizeof(*p));
    if (p == NULL) die("could not allocate bidirectional DP table");
    s->live_memory += bytes;
    if (s->live_memory > s->peak_memory) s->peak_memory = s->live_memory;
    return p;
}

static void table_init(Search *s, Table *t, size_t hint)
{
    size_t capacity = INITIAL_CAPACITY;
    while (capacity < hint) {
        if (capacity > SIZE_MAX / 2U) die("table capacity overflow");
        capacity *= 2U;
    }
    t->entry = allocate_entries(s, capacity);
    t->capacity = capacity;
    t->used = 0;
}

static void table_free(Search *s, Table *t)
{
    if (t->entry == NULL) return;
    size_t bytes = t->capacity * sizeof(*t->entry);
    if (bytes > s->live_memory) die("memory accounting error");
    free(t->entry);
    s->live_memory -= bytes;
    memset(t, 0, sizeof(*t));
}

static void table_grow(Search *s, Table *t)
{
    Table bigger;
    table_init(s, &bigger, t->capacity * 2U);
    for (size_t i = 0; i < t->capacity; ++i) {
        Entry item = t->entry[i];
        if (item.key_plus_one == 0U) continue;
        size_t slot = (size_t)hash64(item.key_plus_one - 1U) &
                      (bigger.capacity - 1U);
        while (bigger.entry[slot].key_plus_one != 0U)
            slot = (slot + 1U) & (bigger.capacity - 1U);
        bigger.entry[slot] = item;
        ++bigger.used;
    }
    table_free(s, t);
    *t = bigger;
}

static void table_add(Search *s, Table *t, uint64_t key, uint64_t add)
{
    uint64_t stored = key + 1U;
    if (stored == 0U) die("key overflow");
    for (;;) {
        size_t slot = (size_t)hash64(key) & (t->capacity - 1U);
        while (t->entry[slot].key_plus_one != 0U &&
               t->entry[slot].key_plus_one != stored)
            slot = (slot + 1U) & (t->capacity - 1U);
        Entry *e = &t->entry[slot];
        if (e->key_plus_one == stored) {
            if (UINT64_MAX - e->count < add) die("count overflow");
            e->count += add;
            return;
        }
        if ((t->used + 1U) * 10U >= t->capacity * 7U) {
            table_grow(s, t);
            continue;
        }
        e->key_plus_one = stored;
        e->count = add;
        ++t->used;
        if (t->used > s->peak_states) s->peak_states = t->used;
        return;
    }
}

static bool table_get(const Table *t, uint64_t key, uint64_t *count)
{
    uint64_t stored = key + 1U;
    size_t slot = (size_t)hash64(key) & (t->capacity - 1U);
    while (t->entry[slot].key_plus_one != 0U) {
        if (t->entry[slot].key_plus_one == stored) {
            *count = t->entry[slot].count;
            return true;
        }
        slot = (slot + 1U) & (t->capacity - 1U);
    }
    return false;
}

static int choose_denominator(const Search *s, mask_t denominators)
{
    mask_t forced = 0;
    int active = -1;
    unsigned smallest = MAX_N + 1U;
    for (unsigned c = 0; c < s->constraint_count; ++c) {
        mask_t remaining = denominators & s->constraint[c].mask;
        unsigned count = popcount(remaining);
        if (count == 1U) {
            forced |= remaining;
        } else if (count > 1U &&
                   (count < smallest ||
                    (count == smallest &&
                     (active < 0 || s->constraint[c].modulus >
                                          s->constraint[active].modulus)))) {
            active = (int)c;
            smallest = count;
        }
    }
    if (forced != 0U) return first(forced);
    if (active < 0) return first(denominators);

    const Constraint *constraint = &s->constraint[active];
    mask_t scan = denominators & constraint->mask;
    int best = 0;
    uint64_t best_gcd = UINT64_MAX;
    while (scan != 0U) {
        int j = first(scan);
        scan &= scan - 1U;
        uint64_t divisor = gcd64(s->weight[j] % constraint->modulus,
                                 constraint->modulus);
        if (best == 0 || divisor < best_gcd ||
            (divisor == best_gcd && j < best)) {
            best = j;
            best_gcd = divisor;
        }
    }
    return best;
}

static uint64_t required_for(const Search *s, mask_t remaining_denominators)
{
    /* Product of the pairwise-coprime g_r described in the file header. */
    uint64_t required = 1;
    for (unsigned c = 0; c < s->constraint_count; ++c) {
        const Constraint *constraint = &s->constraint[c];
        mask_t scan = remaining_denominators & constraint->mask;
        uint64_t divisor = constraint->modulus;
        while (scan != 0U) {
            int j = first(scan);
            scan &= scan - 1U;
            divisor = gcd64(divisor,
                            s->weight[j] % constraint->modulus);
        }
        required *= divisor;
    }
    if (s->lcm % required != 0U) die("invalid required modulus");
    return required;
}

static void search_init(Search *s, int n, size_t memory_limit)
{
    memset(s, 0, sizeof(*s));
    s->n = n;
    s->memory_limit = memory_limit;
    s->lcm = lcm_to(n);
    if (s->lcm > (UINT64_MAX >> n)) die("packed key overflow");
    for (int j = 1; j <= n; ++j) s->weight[j] = s->lcm / (uint64_t)j;
    for (unsigned r = 2; r <= (unsigned)n; ++r) {
        if (!prime(r)) continue;
        if (s->constraint_count == MAX_CONSTRAINTS)
            die("too many constraints");
        Constraint *constraint = &s->constraint[s->constraint_count++];
        uint32_t modulus = r;
        while (modulus <= (unsigned)n / r) modulus *= r;
        constraint->modulus = modulus;
        for (int j = (int)r; j <= n; j += (int)r)
            constraint->mask |= (mask_t)1U << (j - 1);
    }

    mask_t remaining = full_mask(n);
    s->prefix_required[0] = required_for(s, remaining);
    for (int depth = 0; depth < n; ++depth) {
        int j = choose_denominator(s, remaining);
        s->order[depth] = j;
        remaining ^= (mask_t)1U << (j - 1);
        s->prefix_required[depth + 1] = required_for(s, remaining);
    }
    if (s->prefix_required[n] != s->lcm)
        die("terminal modulus is not L");
}

static size_t forward_hint(size_t states, uint64_t gain)
{
    if (states > (SIZE_MAX - INITIAL_CAPACITY) / 2U)
        die("table hint overflow");
    return states * 2U / gain + INITIAL_CAPACITY;
}

static Table make_forward_frontier(Search *s, int meet)
{
    const mask_t full = full_mask(s->n);
    Table current;
    table_init(s, &current, INITIAL_CAPACITY);
    table_add(s, &current, full, 1U);

    for (int depth = 0; depth < meet; ++depth) {
        Table next;
        uint64_t gain = s->prefix_required[depth + 1] /
                        s->prefix_required[depth];
        table_init(s, &next, forward_hint(current.used, gain));
        const int j = s->order[depth];
        const uint64_t mask64 = (UINT64_C(1) << s->n) - 1U;
        for (size_t slot = 0; slot < current.capacity; ++slot) {
            Entry item = current.entry[slot];
            if (item.key_plus_one == 0U) continue;
            uint64_t key = item.key_plus_one - 1U;
            mask_t unused = (mask_t)(key & mask64);
            uint64_t residue = key >> s->n;
            mask_t scan = unused;
            while (scan != 0U) {
                int value = first(scan);
                mask_t bit = (mask_t)1U << (value - 1);
                scan ^= bit;
                if (s->transitions == UINT64_MAX)
                    die("transition counter overflow");
                ++s->transitions;
                uint64_t contribution = (uint64_t)value * s->weight[j];
                uint64_t next_residue = (residue + contribution) % s->lcm;
                if (next_residue % s->prefix_required[depth + 1] != 0U)
                    continue;
                uint64_t next_key =
                    (next_residue << s->n) | (unused ^ bit);
                table_add(s, &next, next_key, item.count);
            }
        }
        table_free(s, &current);
        current = next;
        if (s->verbose_layers)
            fprintf(stderr, "  forward layer %d: %zu states\n",
                    depth + 1, current.used);
    }
    return current;
}

static Table make_backward_frontier(Search *s, int meet)
{
    /*
     * Here a mask records the numerators used by the prefix.  Starting from
     * a complete integral path, remove suffix assignments in reverse order.
     * Subtracting v*w_j modulo L gives the unique predecessor residue.
     */
    const mask_t full = full_mask(s->n);
    Table current;
    table_init(s, &current, INITIAL_CAPACITY);
    table_add(s, &current, full, 1U);

    for (int depth = s->n - 1; depth >= meet; --depth) {
        Table previous;
        size_t hint = current.used > (SIZE_MAX - INITIAL_CAPACITY) / 2U
                          ? SIZE_MAX
                          : current.used * 2U + INITIAL_CAPACITY;
        table_init(s, &previous, hint);
        const int j = s->order[depth];
        const uint64_t mask64 = (UINT64_C(1) << s->n) - 1U;
        for (size_t slot = 0; slot < current.capacity; ++slot) {
            Entry item = current.entry[slot];
            if (item.key_plus_one == 0U) continue;
            uint64_t key = item.key_plus_one - 1U;
            mask_t used = (mask_t)(key & mask64);
            uint64_t residue = key >> s->n;
            mask_t scan = used;
            while (scan != 0U) {
                int value = first(scan);
                mask_t bit = (mask_t)1U << (value - 1);
                scan ^= bit;
                if (s->transitions == UINT64_MAX)
                    die("transition counter overflow");
                ++s->transitions;
                uint64_t contribution =
                    ((uint64_t)value * s->weight[j]) % s->lcm;
                uint64_t predecessor = residue >= contribution
                                           ? residue - contribution
                                           : residue + s->lcm - contribution;
                if (predecessor % s->prefix_required[depth] != 0U)
                    continue;
                uint64_t previous_key =
                    (predecessor << s->n) | (used ^ bit);
                table_add(s, &previous, previous_key, item.count);
            }
        }
        table_free(s, &current);
        current = previous;
        if (s->verbose_layers)
            fprintf(stderr, "  backward layer %d: %zu states\n",
                    depth, current.used);
    }
    return current;
}

static uint64_t compute_exact_term(int n, int requested_meet,
                                   size_t memory_limit, bool verbose_layers,
                                   Statistics *statistics)
{
    memset(statistics, 0, sizeof(*statistics));
    statistics->searched_n = n;
    int meet = requested_meet >= 0 ? requested_meet : (3 * n + 3) / 4;
    statistics->meet = meet;
    if (n == 0) {
        return 1;
    }
    if (meet < 0 || meet > n) die("internal meet depth out of range");
    Search s;
    search_init(&s, n, memory_limit);
    s.verbose_layers = verbose_layers;
    const mask_t full = full_mask(n);
    Table forward = make_forward_frontier(&s, meet);
    Table backward = make_backward_frontier(&s, meet);
    uint64_t answer = 0;
    const uint64_t mask64 = (UINT64_C(1) << n) - 1U;
    for (size_t slot = 0; slot < forward.capacity; ++slot) {
        Entry item = forward.entry[slot];
        if (item.key_plus_one == 0U) continue;
        uint64_t key = item.key_plus_one - 1U;
        mask_t unused = (mask_t)(key & mask64);
        uint64_t residue = key >> n;
        /* Complementary masks use every numerator exactly once. */
        uint64_t backward_key = (residue << n) | (full ^ unused);
        uint64_t suffix_count;
        if (!table_get(&backward, backward_key, &suffix_count)) continue;
        if (item.count != 0U && suffix_count > UINT64_MAX / item.count)
            die("joined count product overflow");
        /* Every complete path has one and only one state at the meet layer. */
        uint64_t product = item.count * suffix_count;
        if (UINT64_MAX - answer < product) die("answer overflow");
        answer += product;
    }
    table_free(&s, &forward);
    table_free(&s, &backward);
    if (s.live_memory != 0U) die("internal DP memory accounting leak");
    statistics->peak_states = s.peak_states;
    statistics->peak_memory = s.peak_memory;
    statistics->transitions = s.transitions;
    return answer;
}

static uint64_t compute_output_term(int n, int requested_meet,
                                    size_t memory_limit, bool verbose_layers,
                                    bool have_previous,
                                    uint64_t previous_value,
                                    Statistics *statistics)
{
    if (!prime((unsigned)n))
        return compute_exact_term(n, requested_meet, memory_limit,
                                  verbose_layers, statistics);

    uint64_t answer;
    if (have_previous) {
        memset(statistics, 0, sizeof(*statistics));
        statistics->searched_n = n - 1;
        answer = previous_value;
    } else {
        int meet = requested_meet;
        if (meet > n - 1) meet = n - 1;
        answer = compute_exact_term(n - 1, meet, memory_limit,
                                    verbose_layers, statistics);
    }
    statistics->prime_step = true;
    return answer;
}

static void verify_known(uint64_t value, int n)
{
    if (n <= KNOWN_MAX_N && value != known[n]) {
        fprintf(stderr,
                "error: A073090 mismatch at n=%d: got %" PRIu64
                ", expected %" PRIu64 "\n",
                n, value, known[n]);
        exit(EXIT_FAILURE);
    }
}

static bool next_permutation(int values[MAX_N], int n)
{
    int i = n - 2;
    while (i >= 0 && values[i] >= values[i + 1]) --i;
    if (i < 0) return false;

    int j = n - 1;
    while (values[j] <= values[i]) --j;
    int temporary = values[i];
    values[i] = values[j];
    values[j] = temporary;
    for (int left = i + 1, right = n - 1; left < right; ++left, --right) {
        int swap = values[left];
        values[left] = values[right];
        values[right] = swap;
    }
    return true;
}

/* Definition-level reference, used only by --check. */
static uint64_t direct_term(int n)
{
    if (n == 0) return 1;
    uint64_t lcm = lcm_to(n);
    int inverse[MAX_N];
    for (int i = 0; i < n; ++i) inverse[i] = i + 1;
    uint64_t answer = 0;
    do {
        uint64_t scaled_sum = 0;
        for (int j = 1; j <= n; ++j)
            scaled_sum += (uint64_t)inverse[j - 1] *
                          (lcm / (uint64_t)j);
        if (scaled_sum % lcm == 0U) {
            if (answer == UINT64_MAX) die("direct count overflow");
            ++answer;
        }
    } while (next_permutation(inverse, n));
    return answer;
}

static void release_bfile_lock(void)
{
    if (bfile_lock_descriptor < 0) return;
    int saved_error = errno;
    const struct flock lock = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    (void)fcntl(bfile_lock_descriptor, F_SETLK, &lock);
    (void)close(bfile_lock_descriptor);
    bfile_lock_descriptor = -1;
    errno = saved_error;
}

static void cleanup_bfile(void)
{
    int saved_error = errno;
    if (bfile_temp_active) {
        (void)unlink(bfile_temp_name);
        bfile_temp_active = false;
    }
    release_bfile_lock();
    errno = saved_error;
}

static FILE *open_bfile(void)
{
    bfile_lock_descriptor = open(BFILE_LOCK_NAME, O_CREAT | O_RDWR, 0666);
    if (bfile_lock_descriptor < 0) {
        fprintf(stderr, "error: cannot open %s: %s\n",
                BFILE_LOCK_NAME, strerror(errno));
        exit(EXIT_FAILURE);
    }
    const struct flock lock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    if (fcntl(bfile_lock_descriptor, F_SETLK, &lock) != 0) {
        int saved_error = errno;
        cleanup_bfile();
        if (saved_error == EACCES || saved_error == EAGAIN)
            fprintf(stderr, "error: another writer holds %s\n",
                    BFILE_LOCK_NAME);
        else
            fprintf(stderr, "error: cannot lock %s: %s\n",
                    BFILE_LOCK_NAME, strerror(saved_error));
        exit(EXIT_FAILURE);
    }

    int descriptor = mkstemp(bfile_temp_name);
    if (descriptor < 0) {
        int saved_error = errno;
        cleanup_bfile();
        fprintf(stderr, "error: cannot create %s: %s\n",
                BFILE_TEMP_TEMPLATE, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    bfile_temp_active = true;

    struct stat existing;
    mode_t mode;
    if (stat(BFILE_NAME, &existing) == 0) {
        mode = existing.st_mode & 0777;
    } else if (errno == ENOENT) {
        mode_t mask = umask(0);
        (void)umask(mask);
        mode = 0666 & ~mask;
    } else {
        int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot inspect %s: %s\n",
                BFILE_NAME, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    if (fchmod(descriptor, mode) != 0) {
        int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot set permissions on %s: %s\n",
                bfile_temp_name, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    FILE *stream = fdopen(descriptor, "w");
    if (stream == NULL) {
        int saved_error = errno;
        (void)close(descriptor);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot open %s: %s\n",
                bfile_temp_name, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    return stream;
}

static void write_bfile_term(FILE *stream, int n, uint64_t value)
{
    if (fprintf(stream, "%d %" PRIu64 "\n", n, value) < 0 ||
        fflush(stream) != 0) {
        int saved_error = errno == 0 ? EIO : errno;
        (void)fclose(stream);
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot write %s: %s\n",
                bfile_temp_name, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
}

static void finish_bfile(FILE *stream)
{
    bool failed = false;
    int saved_error = 0;
    if (fflush(stream) != 0) {
        failed = true;
        saved_error = errno;
    }
    int descriptor = fileno(stream);
    if (descriptor < 0) {
        failed = true;
        if (saved_error == 0) saved_error = errno;
    } else if (!failed && fsync(descriptor) != 0) {
        failed = true;
        saved_error = errno;
    }
    if (fclose(stream) != 0) {
        failed = true;
        if (saved_error == 0) saved_error = errno;
    }
    if (failed) {
        if (saved_error == 0) saved_error = EIO;
        errno = saved_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot finalize %s: %s\n",
                bfile_temp_name, strerror(saved_error));
        exit(EXIT_FAILURE);
    }
    if (rename(bfile_temp_name, BFILE_NAME) != 0) {
        int rename_error = errno;
        errno = rename_error;
        cleanup_bfile();
        fprintf(stderr, "error: cannot replace %s: %s\n",
                BFILE_NAME, strerror(rename_error));
        exit(EXIT_FAILURE);
    }
    bfile_temp_active = false;
    release_bfile_lock();
}

static void usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "Usage: %s [--upto N | --term N | --check] "
            "[--memory-mb MiB] [--meet depth] [--verbose]\n",
            program);
}

int main(int argc, char **argv)
{
    OutputMode mode = MODE_UPTO;
    int limit = DEFAULT_MAX_N;
    unsigned memory_mb = DEFAULT_MEMORY_MB;
    int requested_meet = -1;
    bool check = false;
    bool verbose = false;
    bool mode_seen = false;

    if (atexit(cleanup_bfile) != 0) die("could not register cleanup");
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--upto") == 0 ||
            strcmp(argv[i], "--term") == 0) {
            if (mode_seen || i + 1 == argc) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            mode = strcmp(argv[i], "--term") == 0 ? MODE_TERM : MODE_UPTO;
            limit = parse_n(argv[++i]);
            mode_seen = true;
        } else if (strcmp(argv[i], "--check") == 0) {
            if (mode_seen) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            check = true;
            limit = KNOWN_MAX_N;
            mode_seen = true;
        } else if (strcmp(argv[i], "--memory-mb") == 0) {
            if (i + 1 == argc) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            memory_mb = parse_memory_mb(argv[++i]);
        } else if (strcmp(argv[i], "--meet") == 0) {
            if (i + 1 == argc) {
                usage(stderr, argv[0]);
                return EXIT_FAILURE;
            }
            requested_meet = parse_integer(argv[++i], "meet", 0, MAX_N);
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = true;
        } else if (strcmp(argv[i], "--help") == 0 ||
                   strcmp(argv[i], "-h") == 0) {
            usage(stdout, argv[0]);
            return EXIT_SUCCESS;
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argv[i]);
            usage(stderr, argv[0]);
            return EXIT_FAILURE;
        }
    }
    if (requested_meet > limit) {
        fprintf(stderr, "error: meet must be in 0..n: %d\n", requested_meet);
        return EXIT_FAILURE;
    }

    size_t memory_limit = (size_t)memory_mb * 1024U * 1024U;
    FILE *bfile = mode == MODE_UPTO && !check ? open_bfile() : NULL;
    int first_n = mode == MODE_TERM ? limit : 0;
    bool have_previous = false;
    uint64_t previous_value = 0;

    for (int n = first_n; n <= limit; ++n) {
        Statistics statistics;
        double started = monotonic_seconds();
        int term_meet = requested_meet;
        if (term_meet > n) term_meet = n;
        uint64_t value = compute_output_term(
            n, term_meet, memory_limit, verbose,
            mode == MODE_UPTO && have_previous, previous_value, &statistics);
        verify_known(value, n);

        if (check && n <= DIRECT_CHECK_MAX_N) {
            uint64_t direct = direct_term(n);
            if (direct != value) {
                fprintf(stderr,
                        "error: direct permutation mismatch at n=%d: "
                        "DP=%" PRIu64 ", direct=%" PRIu64 "\n",
                        n, value, direct);
                return EXIT_FAILURE;
            }
        }
        if (bfile != NULL) write_bfile_term(bfile, n, value);
        previous_value = value;
        have_previous = true;

        if (mode == MODE_TERM)
            printf("%" PRIu64 "\n", value);
        else
            printf("%s%" PRIu64, n == 0 ? "" : ", ", value);
        if (fflush(stdout) != 0) die("could not flush stdout");

        if (verbose) {
            const char *method = statistics.prime_step
                                     ? "prime-recurrence"
                                     : "bidirectional-dp";
            fprintf(stderr,
                    "073090_03: n=%d, a(n)=%" PRIu64
                    ", method=%s, searched_n=%d, meet=%d, "
                    "peak_states=%zu, transitions=%" PRIu64
                    ", peak_memory=%.1f MiB, %.3f s\n",
                    n, value, method, statistics.searched_n,
                    statistics.meet, statistics.peak_states,
                    statistics.transitions,
                    (double)statistics.peak_memory / 1048576.0,
                    monotonic_seconds() - started);
        }
    }

    if (mode == MODE_UPTO) {
        if (putchar('\n') == EOF || fflush(stdout) != 0)
            die("could not finish stdout");
    }
    if (bfile != NULL) finish_bfile(bfile);
    if (check)
        fprintf(stderr,
                "check passed: A073090(0..%d), direct permutations "
                "through n=%d\n",
                KNOWN_MAX_N, DIRECT_CHECK_MAX_N);
    return EXIT_SUCCESS;
}
