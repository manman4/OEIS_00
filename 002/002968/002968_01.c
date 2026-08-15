/*
 * A002968 -- bitset exact-cover search with shared value constraints.
 *
 * Count pairings {b(i),c(i)} of {1,...,2*n}, b(i)<c(i), such that all 2*n
 * positive values
 *
 *     b(i)+c(i) and c(i)-b(i)
 *
 * are different.  A candidate pair {x,y}, x<y, covers four columns:
 *
 *     vertex x, vertex y, value x+y, value y-x.
 *
 * Vertex columns are primary (each must be covered exactly once).  All sums
 * and differences use one shared family of secondary value columns (each may
 * be covered at most once).  Thus two candidate edges conflict if they share
 * an endpoint or if either value of one edge equals either value of the
 * other.  Every edge's complete conflict set is precomputed as a bitset.
 * Selecting edge e during the search is then
 *
 *     active_edges &= ~conflicts[e].
 *
 * Algorithm X's minimum-column rule selects the uncovered vertex having the
 * fewest active incident edges.  Branching over that vertex's edges counts
 * each unordered pairing once, with no division by n! or 2^n.
 *
 * Root branches (the possible partners of vertex 1) are dynamically shared
 * by pthread workers.  Search state and answer accumulators are private to a
 * worker.  The edge/conflict tables are immutable after construction.
 *
 * Every answer addition is checked in unsigned __int128.  The unrestricted
 * pairing count (2*n-1)!! bounds the answer and fits unsigned __int128 for
 * n<=28, which is this program's hard limit.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -Wpedantic -pthread \
 *       002968_01.c -o 002968_01
 *
 * Usage:
 *   ./002968_01 --term 17 --threads 4
 *   ./002968_01 --upto 17 --threads 4
 *   ./002968_01 --upto 17 --start 17 --threads 4
 *   ./002968_01 --check 12 --threads 4
 *
 * A positional N is shorthand for --upto N.  --upto writes
 * b002968_01.txt through b002968_01_part.txt unless --no-bfile is given.
 * --start S copies the verified built-in prefix n<S to the b-file and only
 * calculates and prints S..N.  --term and --check do not change the b-file.
 * A file lock serializes simultaneous b-file writers.  Progress is written
 * to stderr every 30 seconds by default; --progress SEC changes the interval,
 * and zero disables progress reporting.
 */

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <pthread.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#if !defined(__SIZEOF_INT128__)
#error "002968_01.c requires unsigned __int128"
#endif

__extension__ typedef unsigned __int128 U128;

#define MIN_N 0
#define MAX_N 28
#define KNOWN_MAX_N 17
#define DEFAULT_THREADS 4
#define MAX_THREADS 64
#define DEFAULT_PROGRESS_SECONDS 30
#define MAX_PROGRESS_SECONDS 3600
#define MAX_VERTICES (2 * MAX_N)
#define MAX_EDGES (MAX_VERTICES * (MAX_VERTICES - 1) / 2)
#define MAX_EDGE_WORDS ((MAX_EDGES + 63) / 64)
#define NODE_FLUSH_INTERVAL UINT64_C(65536)

#define BFILE_NAME "b002968_01.txt"
#define PART_FILE_NAME "b002968_01_part.txt"
#define LOCK_FILE_NAME "b002968_01.txt.lock"

typedef enum {
    MODE_NONE,
    MODE_TERM,
    MODE_UPTO,
    MODE_CHECK
} RunMode;

typedef struct {
    uint64_t endpoints;
    unsigned sum;
    unsigned difference;
} Edge;

typedef struct {
    unsigned n;
    unsigned vertex_count;
    unsigned edge_count;
    unsigned word_count;
    uint64_t full_vertices;
    Edge *edges;
    uint64_t *incidence;
    uint64_t *conflicts;
} Problem;

typedef struct {
    const Problem *problem;
    const unsigned *root_edges;
    unsigned root_count;
    _Atomic unsigned next_root;
    _Atomic unsigned completed_roots;
    _Atomic uint64_t completed_nodes;
    bool done;
    pthread_mutex_t progress_mutex;
    pthread_cond_t progress_condition;
    double start_time;
    unsigned progress_seconds;
} TaskQueue;

typedef struct {
    TaskQueue *queue;
    U128 result;
    uint64_t pending_nodes;
} Worker;

typedef struct {
    RunMode mode;
    int n;
    int start;
    unsigned threads;
    unsigned progress_seconds;
    bool write_bfile;
} Options;

static const uint64_t known_terms[KNOWN_MAX_N + 1] = {
    UINT64_C(1), UINT64_C(1), UINT64_C(0), UINT64_C(1),
    UINT64_C(8), UINT64_C(22), UINT64_C(51), UINT64_C(342),
    UINT64_C(2609), UINT64_C(16896), UINT64_C(99114),
    UINT64_C(876579), UINT64_C(8551800), UINT64_C(79595269),
    UINT64_C(764804085), UINT64_C(8905825760),
    UINT64_C(112166089619), UINT64_C(1423772212734)
};

static _Noreturn void die(const char *message)
{
    fprintf(stderr, "error: %s\n", message);
    exit(EXIT_FAILURE);
}

static _Noreturn void die_errno(const char *message)
{
    fprintf(stderr, "error: %s: %s\n", message, strerror(errno));
    exit(EXIT_FAILURE);
}

static double monotonic_seconds(void)
{
    struct timespec value;
    if (clock_gettime(CLOCK_MONOTONIC, &value) != 0) {
        die_errno("clock_gettime failed");
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

static unsigned parse_unsigned(const char *text, unsigned minimum,
                               unsigned maximum, const char *name)
{
    char *end = NULL;
    errno = 0;
    const unsigned long value = strtoul(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' ||
        value < minimum || value > maximum) {
        fprintf(stderr, "error: %s must be in %u..%u: %s\n",
                name, minimum, maximum, text);
        exit(EXIT_FAILURE);
    }
    return (unsigned)value;
}

static void print_u128(FILE *stream, U128 value)
{
    char digits[40];
    size_t length = 0;
    do {
        digits[length++] = (char)('0' + (unsigned)(value % 10));
        value /= 10;
    } while (value != 0);
    while (length != 0) {
        if (fputc(digits[--length], stream) == EOF) {
            die("could not write an integer");
        }
    }
}

static void write_term(FILE *stream, int n, U128 value)
{
    if (fprintf(stream, "%d ", n) < 0) {
        die("could not write a term index");
    }
    print_u128(stream, value);
    if (fputc('\n', stream) == EOF || fflush(stream) != 0) {
        die_errno("could not flush a sequence term");
    }
}

static void add_u128(U128 *destination, U128 addend)
{
    const U128 maximum = ~(U128)0;
    if (*destination > maximum - addend) {
        die("answer overflow in unsigned __int128");
    }
    *destination += addend;
}

static U128 pairing_upper_bound(unsigned n)
{
    U128 result = 1;
    for (unsigned odd = 1; odd < 2 * n; odd += 2) {
        const U128 previous = result;
        result *= odd;
        if (result / odd != previous) {
            die("pairing upper bound overflow");
        }
    }
    return result;
}

static inline uint64_t *row(uint64_t *table, unsigned index,
                            unsigned words)
{
    return table + (size_t)index * words;
}

static inline const uint64_t *const_row(const uint64_t *table,
                                        unsigned index, unsigned words)
{
    return table + (size_t)index * words;
}

static inline void set_edge_bit(uint64_t *bits, unsigned edge)
{
    bits[edge >> 6] |= UINT64_C(1) << (edge & 63U);
}

static Problem *make_problem(unsigned n)
{
    Problem *problem = calloc(1, sizeof(*problem));
    if (problem == NULL) {
        die("could not allocate the problem descriptor");
    }
    problem->n = n;
    problem->vertex_count = 2 * n;
    problem->edge_count =
        problem->vertex_count * (problem->vertex_count - 1) / 2;
    problem->word_count = (problem->edge_count + 63) / 64;
    problem->full_vertices = problem->vertex_count == 0
        ? UINT64_C(0)
        : (UINT64_C(1) << problem->vertex_count) - 1;
    if (problem->edge_count == 0) {
        return problem;
    }
    if (problem->edge_count > MAX_EDGES || problem->word_count == 0 ||
        problem->word_count > MAX_EDGE_WORDS) {
        die("internal exact-cover table dimensions are out of range");
    }

    const size_t edge_words =
        (size_t)problem->edge_count * problem->word_count;
    problem->edges = calloc(problem->edge_count, sizeof(*problem->edges));
    problem->incidence = calloc(
        (size_t)problem->vertex_count * problem->word_count,
        sizeof(*problem->incidence));
    problem->conflicts = calloc(edge_words, sizeof(*problem->conflicts));
    if (problem->edges == NULL || problem->incidence == NULL ||
        problem->conflicts == NULL) {
        die("could not allocate the exact-cover tables");
    }

    const unsigned maximum_value = 2 * problem->vertex_count - 1;
    uint64_t *value_edges = calloc(
        (size_t)(maximum_value + 1) * problem->word_count,
        sizeof(*value_edges));
    if (value_edges == NULL) {
        die("could not allocate the secondary value-column table");
    }

    unsigned edge_index = 0;
    for (unsigned x = 0; x < problem->vertex_count; ++x) {
        for (unsigned y = x + 1; y < problem->vertex_count; ++y) {
            if (edge_index >= problem->edge_count) {
                die("internal edge-table overflow");
            }
            Edge *edge = &problem->edges[edge_index];
            edge->endpoints =
                (UINT64_C(1) << x) | (UINT64_C(1) << y);
            edge->sum = x + y + 2;
            edge->difference = y - x;
            if (edge->sum == edge->difference ||
                edge->sum > maximum_value ||
                edge->difference > maximum_value) {
                die("internal secondary value is out of range");
            }
            set_edge_bit(row(problem->incidence, x,
                             problem->word_count), edge_index);
            set_edge_bit(row(problem->incidence, y,
                             problem->word_count), edge_index);
            set_edge_bit(row(value_edges, edge->sum,
                             problem->word_count), edge_index);
            set_edge_bit(row(value_edges, edge->difference,
                             problem->word_count), edge_index);
            ++edge_index;
        }
    }
    if (edge_index != problem->edge_count) {
        die("internal edge-count mismatch");
    }

    for (unsigned e = 0; e < problem->edge_count; ++e) {
        const Edge *edge = &problem->edges[e];
        const unsigned x = (unsigned)__builtin_ctzll(edge->endpoints);
        const unsigned y = (unsigned)__builtin_ctzll(
            edge->endpoints & (edge->endpoints - 1));
        uint64_t *destination = row(problem->conflicts, e,
                                    problem->word_count);
        const uint64_t *incident_x = const_row(
            problem->incidence, x, problem->word_count);
        const uint64_t *incident_y = const_row(
            problem->incidence, y, problem->word_count);
        const uint64_t *same_sum_value = const_row(
            value_edges, edge->sum, problem->word_count);
        const uint64_t *same_difference_value = const_row(
            value_edges, edge->difference, problem->word_count);
        for (unsigned word = 0; word < problem->word_count; ++word) {
            destination[word] = incident_x[word] | incident_y[word] |
                                same_sum_value[word] |
                                same_difference_value[word];
        }
    }
    free(value_edges);
    return problem;
}

static void free_problem(Problem *problem)
{
    if (problem == NULL) {
        return;
    }
    free(problem->conflicts);
    free(problem->incidence);
    free(problem->edges);
    free(problem);
}

static inline void record_node(Worker *worker)
{
    ++worker->pending_nodes;
    if (worker->pending_nodes == NODE_FLUSH_INTERVAL) {
        atomic_fetch_add_explicit(&worker->queue->completed_nodes,
                                  worker->pending_nodes,
                                  memory_order_relaxed);
        worker->pending_nodes = 0;
    }
}

static void flush_nodes(Worker *worker)
{
    if (worker->pending_nodes != 0) {
        atomic_fetch_add_explicit(&worker->queue->completed_nodes,
                                  worker->pending_nodes,
                                  memory_order_relaxed);
        worker->pending_nodes = 0;
    }
}

static U128 search(Worker *worker, uint64_t remaining_vertices,
                   unsigned word_count,
                   const uint64_t active_edges[static word_count])
{
    const Problem *problem = worker->queue->problem;
    if (word_count != problem->word_count) {
        die("internal active-edge word-count mismatch");
    }
    record_node(worker);
    if (remaining_vertices == 0) {
        return 1;
    }

    unsigned best_vertex = UINT_MAX;
    unsigned best_degree = UINT_MAX;
    uint64_t vertices = remaining_vertices;
    while (vertices != 0) {
        const unsigned vertex = (unsigned)__builtin_ctzll(vertices);
        vertices &= vertices - 1;
        const uint64_t *incident = const_row(
            problem->incidence, vertex, word_count);
        unsigned degree = 0;
        for (unsigned word = 0; word < word_count; ++word) {
            degree += (unsigned)__builtin_popcountll(
                active_edges[word] & incident[word]);
        }
        if (degree == 0) {
            return 0;
        }
        if (degree < best_degree) {
            best_degree = degree;
            best_vertex = vertex;
            if (degree == 1) {
                break;
            }
        }
    }
    if (best_vertex == UINT_MAX) {
        die("internal failure while selecting a primary column");
    }

    const uint64_t *incident = const_row(
        problem->incidence, best_vertex, word_count);
    U128 total = 0;
    for (unsigned word = 0; word < word_count; ++word) {
        uint64_t candidates = active_edges[word] & incident[word];
        while (candidates != 0) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            candidates &= candidates - 1;
            const unsigned edge_index = 64 * word + bit;
            if (edge_index >= problem->edge_count) {
                die("internal out-of-range edge bit");
            }
            uint64_t next_active[word_count];
            const uint64_t *conflicts = const_row(
                problem->conflicts, edge_index, word_count);
            for (unsigned index = 0; index < word_count; ++index) {
                next_active[index] = active_edges[index] & ~conflicts[index];
            }
            const uint64_t next_remaining =
                remaining_vertices & ~problem->edges[edge_index].endpoints;
            add_u128(&total, search(worker, next_remaining, word_count,
                                    next_active));
        }
    }
    return total;
}

static void *worker_main(void *argument)
{
    Worker *worker = argument;
    TaskQueue *queue = worker->queue;
    const Problem *problem = queue->problem;
    uint64_t full_active[problem->word_count];
    for (unsigned word = 0; word < problem->word_count; ++word) {
        full_active[word] = UINT64_MAX;
    }
    if ((problem->edge_count & 63U) != 0) {
        full_active[problem->word_count - 1] =
            (UINT64_C(1) << (problem->edge_count & 63U)) - 1;
    }

    for (;;) {
        const unsigned task = atomic_fetch_add_explicit(
            &queue->next_root, 1, memory_order_relaxed);
        if (task >= queue->root_count) {
            break;
        }
        const unsigned edge_index = queue->root_edges[task];
        uint64_t next_active[problem->word_count];
        const uint64_t *conflicts = const_row(
            problem->conflicts, edge_index, problem->word_count);
        for (unsigned word = 0; word < problem->word_count; ++word) {
            next_active[word] = full_active[word] & ~conflicts[word];
        }
        const uint64_t remaining =
            problem->full_vertices & ~problem->edges[edge_index].endpoints;
        add_u128(&worker->result,
                 search(worker, remaining, problem->word_count, next_active));
        flush_nodes(worker);
        atomic_fetch_add_explicit(&queue->completed_roots, 1,
                                  memory_order_relaxed);
    }
    flush_nodes(worker);
    return NULL;
}

static void *progress_main(void *argument)
{
    TaskQueue *queue = argument;
    if (pthread_mutex_lock(&queue->progress_mutex) != 0) {
        die("could not lock the progress mutex");
    }
    while (!queue->done) {
        struct timespec deadline;
        if (clock_gettime(CLOCK_REALTIME, &deadline) != 0) {
            die_errno("clock_gettime failed for progress reporting");
        }
        deadline.tv_sec += (time_t)queue->progress_seconds;
        int wait_result = 0;
        while (!queue->done && wait_result != ETIMEDOUT) {
            wait_result = pthread_cond_timedwait(
                &queue->progress_condition, &queue->progress_mutex,
                &deadline);
            if (wait_result != 0 && wait_result != ETIMEDOUT) {
                die("progress condition wait failed");
            }
        }
        if (!queue->done && wait_result == ETIMEDOUT) {
            const double now = monotonic_seconds();
            const unsigned roots = atomic_load_explicit(
                &queue->completed_roots, memory_order_relaxed);
            const uint64_t nodes = atomic_load_explicit(
                &queue->completed_nodes, memory_order_relaxed);
            fprintf(stderr,
                    "progress: n=%u roots=%u/%u nodes=%" PRIu64
                    " elapsed=%.1f s\n",
                    queue->problem->n, roots, queue->root_count, nodes,
                    now - queue->start_time);
        }
    }
    if (pthread_mutex_unlock(&queue->progress_mutex) != 0) {
        die("could not unlock the progress mutex");
    }
    return NULL;
}

static U128 compute_term(unsigned n, unsigned requested_threads,
                         unsigned progress_seconds)
{
    if (n == 0) {
        return 1;
    }
    Problem *problem = make_problem(n);
    const uint64_t *root_incidence = const_row(
        problem->incidence, 0, problem->word_count);
    unsigned *root_edges = malloc(
        (size_t)(problem->vertex_count - 1) * sizeof(*root_edges));
    if (root_edges == NULL) {
        die("could not allocate the root task list");
    }
    unsigned root_count = 0;
    for (unsigned word = 0; word < problem->word_count; ++word) {
        uint64_t candidates = root_incidence[word];
        while (candidates != 0) {
            const unsigned bit = (unsigned)__builtin_ctzll(candidates);
            candidates &= candidates - 1;
            const unsigned edge = 64 * word + bit;
            if (edge >= problem->edge_count ||
                root_count >= problem->vertex_count - 1) {
                die("internal out-of-range root edge");
            }
            root_edges[root_count++] = edge;
        }
    }
    if (root_count != problem->vertex_count - 1) {
        die("internal root-task count mismatch");
    }

    const unsigned thread_count = requested_threads < root_count
        ? requested_threads : root_count;
    TaskQueue queue = {
        .problem = problem,
        .root_edges = root_edges,
        .root_count = root_count,
        .next_root = 0,
        .completed_roots = 0,
        .completed_nodes = 0,
        .done = false,
        .start_time = monotonic_seconds(),
        .progress_seconds = progress_seconds
    };
    Worker *workers = calloc(thread_count, sizeof(*workers));
    pthread_t *threads = calloc(thread_count, sizeof(*threads));
    if (workers == NULL || threads == NULL) {
        die("could not allocate worker descriptors");
    }

    pthread_t progress_thread;
    bool progress_started = false;
    if (progress_seconds != 0) {
        if (pthread_mutex_init(&queue.progress_mutex, NULL) != 0 ||
            pthread_cond_init(&queue.progress_condition, NULL) != 0) {
            die("could not initialize progress synchronization");
        }
        if (pthread_create(&progress_thread, NULL, progress_main, &queue) != 0) {
            die("could not create the progress thread");
        }
        progress_started = true;
    }

    for (unsigned index = 0; index < thread_count; ++index) {
        workers[index].queue = &queue;
        if (pthread_create(&threads[index], NULL, worker_main,
                           &workers[index]) != 0) {
            die("could not create a search worker");
        }
    }

    U128 answer = 0;
    for (unsigned index = 0; index < thread_count; ++index) {
        if (pthread_join(threads[index], NULL) != 0) {
            die("could not join a search worker");
        }
        add_u128(&answer, workers[index].result);
    }
    if (progress_started) {
        if (pthread_mutex_lock(&queue.progress_mutex) != 0) {
            die("could not lock the progress mutex at completion");
        }
        queue.done = true;
        if (pthread_cond_signal(&queue.progress_condition) != 0) {
            die("could not signal progress completion");
        }
        if (pthread_mutex_unlock(&queue.progress_mutex) != 0) {
            die("could not unlock the progress mutex at completion");
        }
        if (pthread_join(progress_thread, NULL) != 0) {
            die("could not join the progress thread");
        }
        if (pthread_cond_destroy(&queue.progress_condition) != 0 ||
            pthread_mutex_destroy(&queue.progress_mutex) != 0) {
            die("could not destroy progress synchronization");
        }
    }

    if (answer > pairing_upper_bound(n)) {
        die("answer exceeds the unrestricted pairing bound");
    }
    free(threads);
    free(workers);
    free(root_edges);
    free_problem(problem);
    return answer;
}

static void usage(FILE *stream, const char *program)
{
    fprintf(stream,
            "usage:\n"
            "  %s --term N [--threads T] [--progress SEC]\n"
            "  %s --upto N [--start S] [--threads T] [--progress SEC]\n"
            "  %s N [--start S] [--threads T] [--progress SEC]\n"
            "  %s --check N [--threads T] [--progress SEC]\n",
            program, program, program, program);
}

static Options parse_options(int argc, char **argv)
{
    Options options = {
        .mode = MODE_NONE,
        .n = -1,
        .start = 0,
        .threads = DEFAULT_THREADS,
        .progress_seconds = DEFAULT_PROGRESS_SECONDS,
        .write_bfile = true
    };
    bool start_seen = false;
    for (int index = 1; index < argc; ++index) {
        const char *argument = argv[index];
        if (strcmp(argument, "--help") == 0 ||
            strcmp(argument, "-h") == 0) {
            usage(stdout, argv[0]);
            exit(EXIT_SUCCESS);
        } else if (strcmp(argument, "--term") == 0 ||
                   strcmp(argument, "--upto") == 0 ||
                   strcmp(argument, "--check") == 0) {
            if (options.mode != MODE_NONE || index + 1 >= argc) {
                usage(stderr, argv[0]);
                exit(EXIT_FAILURE);
            }
            options.mode = strcmp(argument, "--term") == 0
                ? MODE_TERM
                : (strcmp(argument, "--upto") == 0
                    ? MODE_UPTO : MODE_CHECK);
            options.n = parse_integer(argv[++index], MIN_N,
                                      options.mode == MODE_CHECK
                                          ? KNOWN_MAX_N : MAX_N,
                                      "N");
        } else if (strcmp(argument, "--start") == 0) {
            if (start_seen || index + 1 >= argc) {
                die("--start requires exactly one argument");
            }
            options.start = parse_integer(argv[++index], MIN_N,
                                          MAX_N, "start");
            start_seen = true;
        } else if (strcmp(argument, "--threads") == 0) {
            if (index + 1 >= argc) {
                die("--threads requires an argument");
            }
            options.threads = parse_unsigned(
                argv[++index], 1, MAX_THREADS, "threads");
        } else if (strcmp(argument, "--progress") == 0) {
            if (index + 1 >= argc) {
                die("--progress requires an argument");
            }
            options.progress_seconds = parse_unsigned(
                argv[++index], 0, MAX_PROGRESS_SECONDS, "progress seconds");
        } else if (strcmp(argument, "--no-bfile") == 0) {
            options.write_bfile = false;
        } else if (argument[0] != '-') {
            if (options.mode != MODE_NONE) {
                die("more than one N or run mode was specified");
            }
            options.mode = MODE_UPTO;
            options.n = parse_integer(argument, MIN_N, MAX_N, "N");
        } else {
            fprintf(stderr, "error: unknown option: %s\n", argument);
            usage(stderr, argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    if (options.mode == MODE_NONE) {
        usage(stderr, argv[0]);
        exit(EXIT_FAILURE);
    }
    if (start_seen && options.mode != MODE_UPTO) {
        die("--start is only valid with --upto or positional N");
    }
    if (options.start > options.n) {
        die("start must not exceed N");
    }
    if (options.start > KNOWN_MAX_N + 1) {
        die("start would require unknown prefix terms");
    }
    if (options.mode != MODE_UPTO && !options.write_bfile) {
        die("--no-bfile is only meaningful with --upto or positional N");
    }
    return options;
}

static int acquire_bfile_lock(void)
{
    const int descriptor = open(LOCK_FILE_NAME, O_RDWR | O_CREAT, 0666);
    if (descriptor < 0) {
        die_errno("could not open the b-file lock");
    }
    struct flock lock = {
        .l_type = F_WRLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    int result;
    do {
        result = fcntl(descriptor, F_SETLKW, &lock);
    } while (result != 0 && errno == EINTR);
    if (result != 0) {
        close(descriptor);
        die_errno("could not lock the b-file");
    }
    return descriptor;
}

static void release_bfile_lock(int descriptor)
{
    struct flock lock = {
        .l_type = F_UNLCK,
        .l_whence = SEEK_SET,
        .l_start = 0,
        .l_len = 0
    };
    const int unlock_result = fcntl(descriptor, F_SETLK, &lock);
    const int close_result = close(descriptor);
    if (unlock_result != 0 || close_result != 0) {
        die_errno("could not release the b-file lock");
    }
}

static FILE *open_partial_bfile(void)
{
    FILE *stream = fopen(PART_FILE_NAME, "w");
    if (stream == NULL) {
        die_errno("could not open the partial b-file");
    }
    if (setvbuf(stream, NULL, _IOLBF, 0) != 0) {
        fclose(stream);
        die("could not make the partial b-file line buffered");
    }
    return stream;
}

static void finish_bfile(FILE *stream)
{
    if (fflush(stream) != 0) {
        die_errno("could not flush the partial b-file");
    }
    const int descriptor = fileno(stream);
    if (descriptor < 0 || fsync(descriptor) != 0) {
        die_errno("could not synchronize the partial b-file");
    }
    if (fclose(stream) != 0) {
        die_errno("could not close the partial b-file");
    }
    if (rename(PART_FILE_NAME, BFILE_NAME) != 0) {
        die_errno("could not publish the b-file");
    }
    const int directory = open(".", O_RDONLY);
    if (directory < 0) {
        die_errno("could not open the output directory");
    }
    const int sync_result = fsync(directory);
    const int close_result = close(directory);
    if (sync_result != 0 || close_result != 0) {
        die_errno("could not synchronize the output directory");
    }
}

int main(int argc, char **argv)
{
    const Options options = parse_options(argc, argv);
    if (options.mode == MODE_TERM) {
        const U128 answer = compute_term(
            (unsigned)options.n, options.threads, options.progress_seconds);
        write_term(stdout, options.n, answer);
        return EXIT_SUCCESS;
    }
    if (options.mode == MODE_CHECK) {
        for (int n = 0; n <= options.n; ++n) {
            const U128 answer = compute_term(
                (unsigned)n, options.threads, options.progress_seconds);
            if (answer != known_terms[n]) {
                fprintf(stderr, "error: check failed at n=%d\n", n);
                return EXIT_FAILURE;
            }
        }
        fprintf(stderr, "check passed through n=%d\n", options.n);
        return EXIT_SUCCESS;
    }

    const int lock_descriptor = options.write_bfile
        ? acquire_bfile_lock() : -1;
    FILE *bfile = options.write_bfile ? open_partial_bfile() : NULL;
    if (bfile != NULL) {
        for (int n = 0; n < options.start; ++n) {
            write_term(bfile, n, known_terms[n]);
        }
    }
    for (int n = options.start; n <= options.n; ++n) {
        const U128 answer = compute_term(
            (unsigned)n, options.threads, options.progress_seconds);
        write_term(stdout, n, answer);
        if (bfile != NULL) {
            write_term(bfile, n, answer);
        }
    }
    if (bfile != NULL) {
        finish_bfile(bfile);
        release_bfile_lock(lock_descriptor);
    }
    return EXIT_SUCCESS;
}
