/*
 * A067957, A093313, A093314, A093315 -- bounded parallel split search.
 *
 * Count permutations p of [n], optionally with prescribed p(1), such that
 *
 *     p(j) divides Sum_{i=1..j} p(i)       for every j in [n].
 *
 * Let a be the sum of a constructed prefix, b the sum of a constructed
 * suffix, U the unused values, and T=n(n+1)/2.  The next value on the left
 * must divide a; the next value on the right must divide T-b.  Choosing the
 * end with fewer candidates partitions all completions without duplication.
 *
 * This version differs from the path-at-a-time recursion in 093313_03.c.
 * It first expands a bounded breadth-first frontier, then distributes those
 * independent subtrees dynamically among POSIX worker threads.  Forced moves
 * are propagated iteratively inside each worker.  The task frontier has a
 * fixed compile-time capacity and no state cache is used, so memory cannot
 * grow with the number of visited states.  The default frontier is at most
 * 4096 tasks per worker, capped at 65536 tasks in total.
 *
 * Explicit search storage is about 1 MiB for the task ring plus 256 KiB of
 * limited stack per worker.  Counts use unsigned 128-bit integers with
 * checked addition.  Masks support n<=50 in this program.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       093313_04.c -o 093313_04
 *
 * Examples:
 *   ./093313_04 4 --term 45
 *   ./093313_04 4 --term 50 --threads 10
 *   ./093313_04 2 --upto 44
 *   ./093313_04 _ --upto 30
 *
 * Fixed p(1)=K is saved as b09331(K+1)_04.txt.  Thus K=2,3,4 give
 * b093313_04.txt, b093314_04.txt, b093315_04.txt.  The unrestricted "_"
 * mode is saved as b067957_04.txt.  Files are replaced atomically only after
 * all requested terms finish successfully.
 */

#if defined(__APPLE__)
#define _DARWIN_C_SOURCE 1
#endif
#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <inttypes.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if defined(__APPLE__)
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#if !defined(__SIZEOF_INT128__)
#error "093313_04.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MAX_N 50U
#define MAX_TOTAL_SUM (MAX_N * (MAX_N + 1U) / 2U)
#define DEFAULT_MAX_N 40U
#define MAX_THREADS 64U
#define TASK_CAPACITY 65536U
#ifndef TASKS_PER_THREAD
#define TASKS_PER_THREAD 4096U
#endif
#define WORKER_STACK_BYTES (256U * 1024U)
#define PROGRESS_INTERVAL UINT64_C(1000000000)

typedef enum {
    MODE_UPTO,
    MODE_TERM,
    MODE_CHECK
} OutputMode;

typedef struct {
    uint64_t unused;
    uint16_t prefix_sum;
    uint16_t suffix_sum;
} Task;

typedef struct {
    uint64_t states;
    uint64_t calls;
    uint64_t branches;
    uint64_t forced;
    uint64_t dead_ends;
} Statistics;

typedef struct {
    unsigned n;
    unsigned total_sum;
    unsigned thread_count;
    uint64_t full_mask;
    uint64_t divisor_mask[MAX_TOTAL_SUM + 1U];

    Task tasks[TASK_CAPACITY];
    size_t task_head;
    size_t task_count;
    atomic_size_t next_task;
    atomic_size_t finished_tasks;
    atomic_uint_fast64_t published_states;
    atomic_uint_fast64_t next_report;
    pthread_mutex_t report_mutex;

    Statistics split_statistics;
    U128 split_answer;
    double started;
} ParallelSearch;

typedef struct {
    ParallelSearch *search;
    unsigned id;
    Statistics statistics;
    U128 answer;
} Worker;

typedef struct {
    Statistics statistics;
    size_t tasks;
    unsigned threads;
} TermStatistics;

_Static_assert(sizeof(Task) <= 16U, "Task unexpectedly exceeds 16 bytes");
_Static_assert(TASK_CAPACITY > MAX_N, "task ring is too small");
_Static_assert(sizeof(ParallelSearch) <= 2U * 1024U * 1024U,
               "fixed parallel-search storage unexpectedly exceeds 2 MiB");

static ParallelSearch parallel_search;
static Worker workers[MAX_THREADS];
static pthread_t worker_threads[MAX_THREADS];
static bool quiet;
static unsigned requested_threads;

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
    "13", "198", "152", "220", "118", "1033", "807", "1026", "868",
    "1005", "2522"
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

static _Noreturn void die_pthread(const char *operation, int error)
{
    fprintf(stderr, "error: %s: %s\n", operation, strerror(error));
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

static void increment_saturated(uint64_t *value)
{
    if (*value != UINT64_MAX) {
        ++*value;
    }
}

static void add_saturated(uint64_t *target, uint64_t value)
{
    if (*target > UINT64_MAX - value) {
        *target = UINT64_MAX;
    } else {
        *target += value;
    }
}

static void add_statistics(Statistics *target, const Statistics *source)
{
    add_saturated(&target->states, source->states);
    add_saturated(&target->calls, source->calls);
    add_saturated(&target->branches, source->branches);
    add_saturated(&target->forced, source->forced);
    add_saturated(&target->dead_ends, source->dead_ends);
}

static void checked_add(U128 *target, U128 value)
{
    const U128 maximum = ~(U128)0;
    if (*target > maximum - value) {
        die("unsigned 128-bit count overflow");
    }
    *target += value;
}

static void u128_to_text(U128 value, char text[40])
{
    char reverse[40];
    size_t length = 0U;
    do {
        reverse[length++] = (char)('0' + (unsigned)(value % 10U));
        value /= 10U;
    } while (value != 0U);
    for (size_t i = 0U; i < length; ++i) {
        text[i] = reverse[length - 1U - i];
    }
    text[length] = '\0';
}

static bool parse_u128(const char *text, U128 *result)
{
    const U128 maximum = ~(U128)0;
    U128 value = 0U;
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

static unsigned default_thread_count(void)
{
#if defined(__APPLE__)
    unsigned online = 0U;
    size_t size = sizeof(online);
    if (sysctlbyname("hw.logicalcpu", &online, &size, NULL, 0U) != 0 ||
        online == 0U) {
        return 1U;
    }
    return online > MAX_THREADS ? MAX_THREADS : online;
#elif defined(_SC_NPROCESSORS_ONLN)
    const long online = sysconf(_SC_NPROCESSORS_ONLN);
    if (online <= 0L) {
        return 1U;
    }
    if ((unsigned long)online > MAX_THREADS) {
        return MAX_THREADS;
    }
    return (unsigned)online;
#else
    return 1U;
#endif
}

static void candidate_domains(const ParallelSearch *search, uint64_t unused,
                              unsigned prefix_sum, unsigned suffix_sum,
                              uint64_t *left, uint64_t *right)
{
    *left = search->divisor_mask[prefix_sum] & unused;
    *right = search->divisor_mask[search->total_sum - suffix_sum] & unused;
}

static void queue_push(ParallelSearch *search, Task task)
{
    if (search->task_count >= TASK_CAPACITY) {
        die("internal task-ring overflow");
    }
    const size_t slot = (search->task_head + search->task_count) %
                        TASK_CAPACITY;
    search->tasks[slot] = task;
    ++search->task_count;
}

static Task queue_pop(ParallelSearch *search)
{
    if (search->task_count == 0U) {
        die("internal empty task ring");
    }
    const Task task = search->tasks[search->task_head];
    search->task_head = (search->task_head + 1U) % TASK_CAPACITY;
    --search->task_count;
    return task;
}

/* Expand one queued state to its next genuine branch, folding forced moves. */
static void split_one_task(ParallelSearch *search, Task task)
{
    for (;;) {
        if (task.unused == 0U) {
            checked_add(&search->split_answer, 1U);
            return;
        }

        increment_saturated(&search->split_statistics.states);
        uint64_t left;
        uint64_t right;
        candidate_domains(search, task.unused, task.prefix_sum,
                          task.suffix_sum, &left, &right);
        if (left == 0U || right == 0U) {
            increment_saturated(&search->split_statistics.dead_ends);
            return;
        }

        const bool use_left =
            __builtin_popcountll(left) <= __builtin_popcountll(right);
        uint64_t candidates = use_left ? left : right;
        if ((candidates & (candidates - 1U)) == 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            task.unused ^= bit_mask;
            if (use_left) {
                task.prefix_sum = (uint16_t)(task.prefix_sum + bit + 1U);
            } else {
                task.suffix_sum = (uint16_t)(task.suffix_sum + bit + 1U);
            }
            increment_saturated(&search->split_statistics.branches);
            increment_saturated(&search->split_statistics.forced);
            continue;
        }

        while (candidates != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            candidates &= candidates - 1U;
            Task child = task;
            child.unused ^= bit_mask;
            if (use_left) {
                child.prefix_sum =
                    (uint16_t)(child.prefix_sum + bit + 1U);
            } else {
                child.suffix_sum =
                    (uint16_t)(child.suffix_sum + bit + 1U);
            }
            queue_push(search, child);
            increment_saturated(&search->split_statistics.branches);
        }
        return;
    }
}

static void build_task_frontier(ParallelSearch *search, unsigned first)
{
    search->task_head = 0U;
    search->task_count = 0U;
    memset(&search->split_statistics, 0, sizeof(search->split_statistics));
    search->split_answer = 0U;

    if (search->n == 0U) {
        search->split_answer = first == 0U ? 1U : 0U;
        return;
    }
    if (first > search->n) {
        return;
    }

    if (first != 0U) {
        const uint64_t bit = UINT64_C(1) << (first - 1U);
        const Task root = {
            search->full_mask ^ bit, (uint16_t)first, 0U
        };
        queue_push(search, root);
    } else {
        for (unsigned value = 1U; value <= search->n; ++value) {
            const uint64_t bit = UINT64_C(1) << (value - 1U);
            const Task root = {
                search->full_mask ^ bit, (uint16_t)value, 0U
            };
            queue_push(search, root);
        }
    }

    size_t target = (size_t)search->thread_count * TASKS_PER_THREAD;
    const size_t safe_maximum = TASK_CAPACITY - MAX_N;
    if (target > safe_maximum) {
        target = safe_maximum;
    }
    while (search->task_count != 0U && search->task_count < target) {
        const Task task = queue_pop(search);
        split_one_task(search, task);
    }
}

static U128 worker_search(Worker *worker, uint64_t unused,
                          unsigned prefix_sum, unsigned suffix_sum)
{
    increment_saturated(&worker->statistics.calls);

    for (;;) {
        if (unused == 0U) {
            return 1U;
        }

        increment_saturated(&worker->statistics.states);
        uint64_t left;
        uint64_t right;
        candidate_domains(worker->search, unused, prefix_sum, suffix_sum,
                          &left, &right);
        if (left == 0U || right == 0U) {
            increment_saturated(&worker->statistics.dead_ends);
            return 0U;
        }

        const bool use_left =
            __builtin_popcountll(left) <= __builtin_popcountll(right);
        uint64_t candidates = use_left ? left : right;
        if ((candidates & (candidates - 1U)) == 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            unused ^= bit_mask;
            if (use_left) {
                prefix_sum += bit + 1U;
            } else {
                suffix_sum += bit + 1U;
            }
            increment_saturated(&worker->statistics.branches);
            increment_saturated(&worker->statistics.forced);
            continue;
        }

        U128 answer = 0U;
        while (candidates != 0U) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            const uint64_t bit_mask = UINT64_C(1) << bit;
            candidates &= candidates - 1U;
            increment_saturated(&worker->statistics.branches);
            const U128 child = use_left ?
                worker_search(worker, unused ^ bit_mask,
                              prefix_sum + bit + 1U, suffix_sum) :
                worker_search(worker, unused ^ bit_mask,
                              prefix_sum, suffix_sum + bit + 1U);
            checked_add(&answer, child);
        }
        return answer;
    }
}

static void maybe_report_parallel_progress(ParallelSearch *search,
                                           uint64_t state_delta)
{
    if (quiet) {
        return;
    }
    const uint_fast64_t current = atomic_fetch_add_explicit(
        &search->published_states, state_delta, memory_order_relaxed) +
        state_delta;
    const size_t finished = atomic_fetch_add_explicit(
        &search->finished_tasks, 1U, memory_order_relaxed) + 1U;

    uint_fast64_t expected = atomic_load_explicit(
        &search->next_report, memory_order_relaxed);
    if (current < expected) {
        return;
    }
    uint_fast64_t next = current - current % PROGRESS_INTERVAL;
    if (next <= UINT_FAST64_MAX - PROGRESS_INTERVAL) {
        next += PROGRESS_INTERVAL;
    } else {
        next = UINT_FAST64_MAX;
    }
    if (!atomic_compare_exchange_strong_explicit(
            &search->next_report, &expected, next,
            memory_order_relaxed, memory_order_relaxed)) {
        return;
    }

    const int lock_error = pthread_mutex_lock(&search->report_mutex);
    if (lock_error != 0) {
        return;
    }
    fprintf(stderr,
            "093313_04: n=%u states~=%" PRIuFAST64
            " tasks=%zu/%zu threads=%u memory=bounded time=%.1fs\n",
            search->n, current, finished, search->task_count,
            search->thread_count, monotonic_seconds() - search->started);
    (void)pthread_mutex_unlock(&search->report_mutex);
}

static void *worker_main(void *argument)
{
    Worker *worker = argument;
    ParallelSearch *search = worker->search;

    for (;;) {
        const size_t task_number = atomic_fetch_add_explicit(
            &search->next_task, 1U, memory_order_relaxed);
        if (task_number >= search->task_count) {
            break;
        }
        const size_t slot = (search->task_head + task_number) % TASK_CAPACITY;
        const Task task = search->tasks[slot];
        const uint64_t states_before = worker->statistics.states;
        checked_add(&worker->answer,
                    worker_search(worker, task.unused,
                                  task.prefix_sum, task.suffix_sum));
        maybe_report_parallel_progress(
            search, worker->statistics.states - states_before);
    }
    return NULL;
}

static U128 run_workers(ParallelSearch *search, TermStatistics *result)
{
    memset(workers, 0, sizeof(workers));
    atomic_init(&search->next_task, 0U);
    atomic_init(&search->finished_tasks, 0U);
    atomic_init(&search->published_states, search->split_statistics.states);
    atomic_init(&search->next_report, PROGRESS_INTERVAL);

    if (search->task_count == 0U) {
        result->statistics = search->split_statistics;
        result->tasks = 0U;
        result->threads = 0U;
        return search->split_answer;
    }

    pthread_attr_t attributes;
    int error = pthread_attr_init(&attributes);
    if (error != 0) {
        die_pthread("pthread_attr_init", error);
    }
    error = pthread_attr_setstacksize(&attributes, WORKER_STACK_BYTES);
    if (error != 0) {
        (void)pthread_attr_destroy(&attributes);
        die_pthread("pthread_attr_setstacksize", error);
    }

    unsigned created = 0U;
    for (; created < search->thread_count; ++created) {
        workers[created].search = search;
        workers[created].id = created;
        error = pthread_create(&worker_threads[created], &attributes,
                               worker_main, &workers[created]);
        if (error != 0) {
            break;
        }
    }
    const int destroy_error = pthread_attr_destroy(&attributes);
    if (destroy_error != 0 && error == 0) {
        error = destroy_error;
    }

    for (unsigned id = 0U; id < created; ++id) {
        const int join_error = pthread_join(worker_threads[id], NULL);
        if (join_error != 0 && error == 0) {
            error = join_error;
        }
    }
    if (error != 0) {
        die_pthread("worker thread operation", error);
    }

    U128 answer = search->split_answer;
    result->statistics = search->split_statistics;
    for (unsigned id = 0U; id < created; ++id) {
        checked_add(&answer, workers[id].answer);
        add_statistics(&result->statistics, &workers[id].statistics);
    }
    result->tasks = search->task_count;
    result->threads = created;
    return answer;
}

static U128 count_term(unsigned n, unsigned first, TermStatistics *result)
{
    ParallelSearch *search = &parallel_search;
    memset(search, 0, sizeof(*search));
    search->n = n;
    search->thread_count = requested_threads;
    search->started = monotonic_seconds();

    int error = pthread_mutex_init(&search->report_mutex, NULL);
    if (error != 0) {
        die_pthread("pthread_mutex_init", error);
    }

    if (n != 0U && first <= n) {
        search->total_sum = n * (n + 1U) / 2U;
        search->full_mask = (UINT64_C(1) << n) - 1U;
        for (unsigned value = 1U; value <= n; ++value) {
            const uint64_t bit = UINT64_C(1) << (value - 1U);
            for (unsigned sum = value; sum <= search->total_sum;
                 sum += value) {
                search->divisor_mask[sum] |= bit;
            }
        }
    }

    build_task_frontier(search, first);
    const U128 answer = run_workers(search, result);
    error = pthread_mutex_destroy(&search->report_mutex);
    if (error != 0) {
        die_pthread("pthread_mutex_destroy", error);
    }
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
        snprintf(path, 64, "b067957_04.txt") :
        snprintf(path, 64, "b09331%u_04.txt", first + 1U);
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
            "Options: --threads 1..%u, --quiet\n"
            "Default thread count: online logical CPUs, capped at %u.\n",
            program, program, program, program, MAX_THREADS, MAX_THREADS);
    exit(status);
}

int main(int argc, char **argv)
{
    requested_threads = default_thread_count();

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
    bool have_threads = false;
    for (int i = 2; i < argc; ++i) {
        if (strcmp(argv[i], "--quiet") == 0 || strcmp(argv[i], "-q") == 0) {
            quiet = true;
        } else if (strcmp(argv[i], "--threads") == 0) {
            if (have_threads || i + 1 >= argc) {
                usage(argv[0], EXIT_FAILURE);
            }
            requested_threads = parse_unsigned(
                argv[++i], 1U, MAX_THREADS, "threads");
            have_threads = true;
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
        TermStatistics statistics;
        const double started = monotonic_seconds();
        const U128 answer = count_term(n, first, &statistics);
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
                    "093313_04: s_1=%s n=%u answer=%s states=%" PRIu64
                    " calls=%" PRIu64 " branches=%" PRIu64
                    " forced=%" PRIu64 " dead=%" PRIu64
                    " tasks=%zu threads=%u memory=bounded time=%.3fs%s\n",
                    first_text, n, answer_text, statistics.statistics.states,
                    statistics.statistics.calls, statistics.statistics.branches,
                    statistics.statistics.forced,
                    statistics.statistics.dead_ends, statistics.tasks,
                    statistics.threads, elapsed,
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

