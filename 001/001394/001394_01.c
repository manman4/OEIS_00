#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef _OPENMP
#include <omp.h>
#endif

static const int directions[4][3] = {
    { 1,  1,  1},
    { 1, -1, -1},
    {-1,  1, -1},
    {-1, -1,  1}
};

enum {
    MAXIMUM_SUPPORTED_N = 40,
    PARALLEL_SPLIT_DEPTH = 7,
    TWO_STEP_SYMMETRY_FACTOR = 12
};

static const unsigned char next_directions[4][3] = {
    {1, 2, 3},
    {0, 2, 3},
    {0, 1, 3},
    {0, 1, 2}
};

struct prefix {
    size_t path[PARALLEL_SPLIT_DEPTH + 1];
    size_t vertex;
    int previous_direction;
};

static int maximum;
static int side;
static int split_depth;
static size_t visited_size;
static ptrdiff_t direction_offsets[4];
static unsigned char *visited;
static uint64_t *counts;
static struct prefix *prefixes;
static size_t prefix_count;
static size_t prefix_capacity;

static size_t vertex_index(int x, int y, int z)
{
    return ((size_t)(x + maximum) * (size_t)side + (size_t)(y + maximum))
           * (size_t)side + (size_t)(z + maximum);
}

static void save_prefix(const size_t path[], size_t vertex,
                        int previous_direction)
{
    if (prefix_count == prefix_capacity) {
        const size_t new_capacity = prefix_capacity == 0 ? 128 : 2 * prefix_capacity;
        struct prefix *new_prefixes =
            realloc(prefixes, new_capacity * sizeof(*prefixes));
        if (new_prefixes == NULL) {
            fprintf(stderr, "failed to allocate the prefix array\n");
            free(prefixes);
            free(visited);
            free(counts);
            exit(EXIT_FAILURE);
        }
        prefixes = new_prefixes;
        prefix_capacity = new_capacity;
    }

    struct prefix *prefix = &prefixes[prefix_count++];
    for (int i = 0; i <= split_depth; ++i)
        prefix->path[i] = path[i];
    prefix->vertex = vertex;
    prefix->previous_direction = previous_direction;
}

static void collect_prefixes(size_t vertex, int length,
                             int previous_direction, size_t path[])
{
    /* This branch represents all 4 * 3 symmetry-equivalent first two steps. */
    counts[length] += TWO_STEP_SYMMETRY_FACTOR;
    if (length == maximum)
        return;
    if (length == split_depth) {
        save_prefix(path, vertex, previous_direction);
        return;
    }

    const int sign = (length & 1) ? -1 : 1;

    for (int choice = 0; choice < 3; ++choice) {
        const int i = next_directions[previous_direction][choice];
        const ptrdiff_t offset = (ptrdiff_t)sign * direction_offsets[i];
        const size_t next = (size_t)((ptrdiff_t)vertex + offset);

        if (!visited[next]) {
            visited[next] = 1;
            path[length + 1] = next;
            collect_prefixes(next, length + 1, i, path);
            visited[next] = 0;
        }
    }
}

static void search_tail(size_t vertex, int length, int previous_direction,
                        unsigned char local_visited[], uint64_t local_counts[])
{
    const int sign = (length & 1) ? -1 : 1;

    for (int choice = 0; choice < 3; ++choice) {
        const int i = next_directions[previous_direction][choice];
        const ptrdiff_t offset = (ptrdiff_t)sign * direction_offsets[i];
        const size_t next = (size_t)((ptrdiff_t)vertex + offset);

        if (!local_visited[next]) {
            local_counts[length + 1] += TWO_STEP_SYMMETRY_FACTOR;
            if (length + 1 < maximum) {
                local_visited[next] = 1;
                search_tail(next, length + 1, i, local_visited, local_counts);
                local_visited[next] = 0;
            }
        }
    }
}

static void search_prefixes(void)
{
    int allocation_failed = 0;
    int counter_overflow = 0;

#ifdef _OPENMP
#pragma omp parallel shared(allocation_failed, counter_overflow)
#endif
    {
        unsigned char *local_visited = calloc(visited_size, 1);
        uint64_t *local_counts = calloc((size_t)maximum + 1,
                                        sizeof(*local_counts));

        if (local_visited == NULL || local_counts == NULL) {
#ifdef _OPENMP
#pragma omp atomic write
#endif
            allocation_failed = 1;
        }

#ifdef _OPENMP
#pragma omp barrier
#endif
        if (!allocation_failed) {
#ifdef _OPENMP
#pragma omp for schedule(dynamic)
#endif
            for (size_t p = 0; p < prefix_count; ++p) {
                for (int i = 0; i <= split_depth; ++i)
                    local_visited[prefixes[p].path[i]] = 1;

                search_tail(prefixes[p].vertex, split_depth,
                            prefixes[p].previous_direction,
                            local_visited, local_counts);

                for (int i = 0; i <= split_depth; ++i)
                    local_visited[prefixes[p].path[i]] = 0;
            }

#ifdef _OPENMP
#pragma omp critical
#endif
            {
                for (int n = split_depth + 1; n <= maximum; ++n) {
                    if (UINT64_MAX - counts[n] < local_counts[n])
                        counter_overflow = 1;
                    else
                        counts[n] += local_counts[n];
                }
            }
        }

        free(local_visited);
        free(local_counts);
    }

    if (allocation_failed || counter_overflow) {
        fprintf(stderr, "%s\n", allocation_failed
                ? "failed to allocate per-thread working arrays"
                : "walk count overflowed uint64_t");
        free(prefixes);
        free(visited);
        free(counts);
        exit(EXIT_FAILURE);
    }
}

static void count_walks(void)
{
    side = 2 * maximum + 1;
    split_depth = maximum < PARALLEL_SPLIT_DEPTH
                  ? maximum : PARALLEL_SPLIT_DEPTH;

    visited_size = (size_t)side * (size_t)side * (size_t)side;
    visited = calloc(visited_size, sizeof(*visited));
    counts = calloc((size_t)maximum + 1, sizeof(*counts));
    if (visited == NULL || counts == NULL) {
        fprintf(stderr, "failed to allocate the working arrays\n");
        free(visited);
        free(counts);
        exit(EXIT_FAILURE);
    }

    counts[0] = 1;
    if (maximum > 0) {
        const ptrdiff_t plane = (ptrdiff_t)side * (ptrdiff_t)side;
        for (int i = 0; i < 4; ++i) {
            direction_offsets[i] = (ptrdiff_t)directions[i][0] * plane
                                   + (ptrdiff_t)directions[i][1] * side
                                   + directions[i][2];
        }

        const size_t origin = vertex_index(0, 0, 0);
        const size_t first = (size_t)((ptrdiff_t)origin + direction_offsets[0]);
        size_t path[PARALLEL_SPLIT_DEPTH + 1];

        counts[1] = 4;
        visited[origin] = 1;
        visited[first] = 1;
        path[0] = origin;
        path[1] = first;

        if (maximum > 1) {
            /*
             * The three legal second steps are equivalent under symmetries
             * that fix the first edge.  Explore one and count the 4 * 3
             * choices represented by this fixed two-step prefix.
             */
            const size_t second =
                (size_t)((ptrdiff_t)first - direction_offsets[1]);
            visited[second] = 1;
            path[2] = second;
            collect_prefixes(second, 2, 1, path);
        }

        if (prefix_count > 0)
            search_prefixes();
    }

    free(prefixes);
    prefixes = NULL;
}

int main(int argc, char **argv)
{
    /*
     * Limit guide for this implementation:
     * a(n) <= 4 * 3^(n - 1), so uint64_t is guaranteed to hold the
     * result through n = 40.  Values above 40 are rejected because the
     * counter could overflow.  In practice, the exponential running time
     * makes the useful limit much smaller than 40.
     */
    maximum = 12;

    if (argc > 2) {
        fprintf(stderr, "usage: %s [maximum_n]\n", argv[0]);
        return EXIT_FAILURE;
    }

    if (argc == 2) {
        char *end;
        const long value = strtol(argv[1], &end, 10);

        if (*argv[1] == '\0' || *end != '\0' ||
            value < 0 || value > MAXIMUM_SUPPORTED_N) {
            fprintf(stderr, "maximum_n must be an integer from 0 to %d\n",
                    MAXIMUM_SUPPORTED_N);
            return EXIT_FAILURE;
        }
        maximum = (int)value;
    }

    count_walks();
    for (int n = 0; n <= maximum; ++n)
        printf("%d %" PRIu64 "\n", n, counts[n]);

    free(visited);
    free(counts);

    return EXIT_SUCCESS;
}
