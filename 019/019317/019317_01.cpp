#include <inttypes.h>
#include <limits.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <omp.h>

/*
 * C++17 / OpenMP implementation.
 * Compile on Apple Silicon with:
 *   g++-omp 019317_01.cpp -o 019317_01_cpp
 *
 * Exact search for A001366 and A019317.
 * For n <= 30, the published A001366 value is only a pruning target: every
 * placement meeting or beating it is still searched, so the target is checked,
 * not blindly accepted.  For larger n, the optimum is discovered from scratch.
 * Board bitsets and square-index tables are allocated dynamically.
 *
 * Output status:
 *   OK              Both A001366 and A019317 match stored reference values
 *                   (currently n <= 16).
 *   A001366_OK      A001366 matches its stored reference value, but A019317
 *                   has no stored reference value and is therefore unchecked
 *                   against OEIS (currently 17 <= n <= 30).
 *   MISMATCH        At least one of A001366/A019317 does not match (n <= 16).
 *   A001366_MISMATCH  A001366 does not match where only it has a reference.
 * A019317, its orbit-size breakdown, and "all" are computed for every n even
 * when no stored A019317 reference value is available.
 */
#define SYMMETRIES 8
#define ORBIT_SIZE_KINDS 4
#define KNOWN_FREE_MAX_N 30
#define KNOWN_A019317_MAX_N 16

static const int known_free[KNOWN_FREE_MAX_N + 1] = {
    0,
    0, 0, 0, 1, 3, 5, 7, 11, 18, 22,
    30, 36, 47, 56, 72, 82, 97, 111, 132, 145,
    170, 186, 216, 240, 260, 290, 324, 360, 381, 420
};

static const uint64_t known_a019317[KNOWN_A019317_MAX_N + 1] = {
    0, 1, 2, 16, 25, 1, 3, 38, 7, 1, 1, 2, 7, 1, 4, 3, 1
};

typedef struct {
    int n;
    int size;
    int words;
    int attack_limit;
    int target_free;
    uint64_t *attack;
    int *transform;
    int *minimum_image;
} Problem;

typedef struct {
    const Problem *problem;
    int *chosen;
    int *image;
    int *next_square;
    int *attacked_count;
    uint64_t *attacked_stack;
    int best_attacked;
    bool found;
    bool overflow;
    uint64_t solutions;
    uint64_t all_solutions;
    uint64_t orbit_counts[ORBIT_SIZE_KINDS];
    uint64_t nodes;
} Search;

typedef struct {
    int best_attacked;
    bool found;
    bool overflow;
    uint64_t solutions;
    uint64_t all_solutions;
    uint64_t orbit_counts[ORBIT_SIZE_KINDS];
    uint64_t nodes;
} BranchResult;

typedef struct {
    int n;
    int free_squares;
    uint64_t solutions;
    uint64_t all_solutions;
    uint64_t orbit_counts[ORBIT_SIZE_KINDS];
    uint64_t nodes;
} Result;

static const int orbit_sizes[ORBIT_SIZE_KINDS] = {1, 2, 4, 8};

static bool checked_mul_size(size_t a, size_t b, size_t *result) {
    if (a != 0 && b > SIZE_MAX / a) return false;
    *result = a * b;
    return true;
}

static bool checked_add_u64(uint64_t *value, uint64_t amount) {
    if (amount > UINT64_MAX - *value) return false;
    *value += amount;
    return true;
}

static bool checked_mul_u64(uint64_t a, uint64_t b, uint64_t *result) {
    if (a != 0 && b > UINT64_MAX / a) return false;
    *result = a * b;
    return true;
}

static uint64_t *attack_mask(Problem *problem, int square) {
    return problem->attack + (size_t)square * (size_t)problem->words;
}

static const uint64_t *const_attack_mask(const Problem *problem, int square) {
    return problem->attack + (size_t)square * (size_t)problem->words;
}

static int *transform_map(Problem *problem, int symmetry) {
    return problem->transform + (size_t)symmetry * (size_t)problem->size;
}

static const int *const_transform_map(const Problem *problem, int symmetry) {
    return problem->transform + (size_t)symmetry * (size_t)problem->size;
}

static void bitset_set(uint64_t *set, int bit) {
    set[bit >> 6] |= UINT64_C(1) << (bit & 63);
}

static int transformed_square(int symmetry, int n, int square) {
    const int r = square / n;
    const int c = square % n;
    int rr;
    int cc;

    switch (symmetry) {
    case 0: rr = r;         cc = c;         break;
    case 1: rr = c;         cc = n - 1 - r; break;
    case 2: rr = n - 1 - r; cc = n - 1 - c; break;
    case 3: rr = n - 1 - c; cc = r;         break;
    case 4: rr = r;         cc = n - 1 - c; break;
    case 5: rr = n - 1 - c; cc = n - 1 - r; break;
    case 6: rr = n - 1 - r; cc = c;         break;
    default: rr = c;        cc = r;         break;
    }
    return rr * n + cc;
}

static bool problem_init(Problem *problem, int n) {
    memset(problem, 0, sizeof(*problem));
    problem->n = n;
    problem->size = n * n;
    problem->words = (problem->size - 1) / 64 + 1;
    problem->target_free = n <= KNOWN_FREE_MAX_N ? known_free[n] : -1;
    problem->attack_limit = problem->target_free >= 0
                                ? problem->size - problem->target_free
                                : problem->size;

    size_t attack_elements;
    size_t attack_bytes;
    size_t transform_elements;
    size_t transform_bytes;
    size_t minimum_image_bytes;
    if (!checked_mul_size((size_t)problem->size, (size_t)problem->words,
                          &attack_elements) ||
        !checked_mul_size(attack_elements, sizeof(*problem->attack),
                          &attack_bytes) ||
        !checked_mul_size(SYMMETRIES, (size_t)problem->size,
                          &transform_elements) ||
        !checked_mul_size(transform_elements, sizeof(*problem->transform),
                          &transform_bytes) ||
        !checked_mul_size((size_t)problem->size,
                          sizeof(*problem->minimum_image),
                          &minimum_image_bytes)) {
        return false;
    }
    if (attack_bytes == 0 || transform_bytes == 0 || minimum_image_bytes == 0) {
        return false;
    }
    problem->attack = static_cast<uint64_t *>(calloc(1, attack_bytes));
    problem->transform = static_cast<int *>(malloc(transform_bytes));
    problem->minimum_image = static_cast<int *>(malloc(minimum_image_bytes));
    if (problem->attack == NULL || problem->transform == NULL ||
        problem->minimum_image == NULL) {
        free(problem->attack);
        free(problem->transform);
        free(problem->minimum_image);
        memset(problem, 0, sizeof(*problem));
        return false;
    }

    for (int queen = 0; queen < problem->size; ++queen) {
        uint64_t *mask = attack_mask(problem, queen);
        const int qr = queen / n;
        const int qc = queen % n;
        for (int square = 0; square < problem->size; ++square) {
            const int r = square / n;
            const int c = square % n;
            int dr = r - qr;
            int dc = c - qc;
            if (dr < 0) dr = -dr;
            if (dc < 0) dc = -dc;
            if (r == qr || c == qc || dr == dc) {
                bitset_set(mask, square);
            }
        }
    }

    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        int *map = transform_map(problem, symmetry);
        for (int square = 0; square < problem->size; ++square) {
            map[square] = transformed_square(symmetry, n, square);
        }
    }

    for (int square = 0; square < problem->size; ++square) {
        int minimum = square;
        for (int symmetry = 1; symmetry < SYMMETRIES; ++symmetry) {
            const int image = const_transform_map(problem, symmetry)[square];
            if (image < minimum) minimum = image;
        }
        problem->minimum_image[square] = minimum;
    }
    return true;
}

static void problem_destroy(Problem *problem) {
    free(problem->attack);
    free(problem->transform);
    free(problem->minimum_image);
}

static bool canonical_pair(const Problem *problem, int first, int second) {
    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        const int *map = const_transform_map(problem, symmetry);
        int a = map[first];
        int b = map[second];
        if (a > b) { const int temporary = a; a = b; b = temporary; }
        if (a < first || (a == first && b < second)) return false;
    }
    return true;
}

static bool canonical_triple(const Problem *problem, int first, int second, int third) {
    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        const int *map = const_transform_map(problem, symmetry);
        int a = map[first];
        int b = map[second];
        int c = map[third];
        int temporary;
        if (a > b) { temporary = a; a = b; b = temporary; }
        if (b > c) { temporary = b; b = c; c = temporary; }
        if (a > b) { temporary = a; a = b; b = temporary; }
        if (a < first ||
            (a == first &&
             (b < second || (b == second && c < third)))) {
            return false;
        }
    }
    return true;
}

static int compare_int(const void *left, const void *right) {
    const int a = *(const int *)left;
    const int b = *(const int *)right;
    return (a > b) - (a < b);
}

/* Returns zero for a noncanonical placement, otherwise its D4 orbit size. */
static int canonical_orbit_size(const Search *search) {
    const Problem *problem = search->problem;
    int stabilizer_size = 0;

    for (int symmetry = 0; symmetry < SYMMETRIES; ++symmetry) {
        const int *map = const_transform_map(problem, symmetry);
        for (int i = 0; i < problem->n; ++i) {
            search->image[i] = map[search->chosen[i]];
        }
        qsort(search->image, (size_t)problem->n, sizeof(*search->image), compare_int);

        int comparison = 0;
        for (int i = 0; i < problem->n; ++i) {
            if (search->chosen[i] < search->image[i]) { comparison = -1; break; }
            if (search->chosen[i] > search->image[i]) { comparison = 1; break; }
        }
        if (comparison > 0) return 0;
        if (comparison == 0) ++stabilizer_size;
    }
    return SYMMETRIES / stabilizer_size;
}

static int orbit_size_index(int orbit_size) {
    for (int i = 0; i < ORBIT_SIZE_KINDS; ++i) {
        if (orbit_sizes[i] == orbit_size) return i;
    }
    return -1;
}

/* Counts new attack bits, stopping as soon as the budget is exceeded. */
static int added_attacks(const Problem *problem, const uint64_t *attacked,
                         int square, int budget, uint64_t *next) {
    const uint64_t *mask = const_attack_mask(problem, square);
    int added = 0;
    for (int word = 0; word < problem->words; ++word) {
        const uint64_t new_bits = mask[word] & ~attacked[word];
        next[word] = attacked[word] | new_bits;
        added += __builtin_popcountll(new_bits);
        if (added > budget) return added;
    }
    return added;
}

static void record_leaf(Search *search, int attacked_count) {
    if (attacked_count < search->best_attacked) {
        search->best_attacked = attacked_count;
        search->found = false;
        search->solutions = 0;
        search->all_solutions = 0;
        memset(search->orbit_counts, 0, sizeof(search->orbit_counts));
    }
    if (attacked_count != search->best_attacked) return;

    const int orbit_size = canonical_orbit_size(search);
    if (orbit_size != 0) {
        const int orbit_index = orbit_size_index(orbit_size);
        if (orbit_index < 0) {
            search->overflow = true;
            return;
        }
        search->found = true;
        if (!checked_add_u64(&search->solutions, 1) ||
            !checked_add_u64(&search->all_solutions, (uint64_t)orbit_size) ||
            !checked_add_u64(&search->orbit_counts[orbit_index], 1)) {
            search->overflow = true;
        }
    }
}

/* Iterative DFS: search depth no longer consumes the C call stack. */
static void search_branch(Search *search, int first, int first_attacked_count) {
    const Problem *problem = search->problem;
    int depth = 1;
    search->attacked_count[depth] = first_attacked_count;
    search->next_square[depth] = first + 1;
    if (!checked_add_u64(&search->nodes, 1)) {
        search->overflow = true;
        return;
    }

    while (depth >= 1 && !search->overflow) {
        if (depth == problem->n) {
            record_leaf(search, search->attacked_count[depth]);
            --depth;
            continue;
        }

        const int remaining = problem->n - depth;
        const int last = problem->size - remaining;
        const uint64_t *attacked = search->attacked_stack +
                                   (size_t)depth * (size_t)problem->words;
        bool descended = false;

        while (search->next_square[depth] <= last) {
            const int square = search->next_square[depth]++;
            if (problem->minimum_image[square] < first) continue;
            if (depth == 1 && !canonical_pair(problem, first, square)) continue;
            if (depth == 2 &&
                !canonical_triple(problem, first, search->chosen[1], square)) continue;

            const int budget = search->best_attacked - search->attacked_count[depth];
            uint64_t *next = search->attacked_stack +
                             (size_t)(depth + 1) * (size_t)problem->words;
            const int added = added_attacks(problem, attacked, square, budget, next);
            if (added > budget) continue;

            search->chosen[depth] = square;
            search->attacked_count[depth + 1] =
                search->attacked_count[depth] + added;
            search->next_square[depth + 1] = square + 1;
            ++depth;
            if (!checked_add_u64(&search->nodes, 1)) {
                search->overflow = true;
            }
            descended = true;
            break;
        }

        if (!descended) --depth;
    }
}

static Result solve(int n) {
    Problem problem;
    if (!problem_init(&problem, n)) {
        fprintf(stderr, "memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    const int last_first = problem.size - n;
    const size_t branch_count = (size_t)last_first + 1;
    size_t branch_bytes;
    size_t n_int_bytes;
    size_t level_int_bytes;
    size_t attacked_stack_elements;
    size_t attacked_stack_bytes;
    if (!checked_mul_size(branch_count, sizeof(BranchResult), &branch_bytes) ||
        !checked_mul_size((size_t)n, sizeof(int), &n_int_bytes) ||
        !checked_mul_size((size_t)n + 1, sizeof(int), &level_int_bytes) ||
        !checked_mul_size((size_t)n + 1, (size_t)problem.words,
                          &attacked_stack_elements) ||
        !checked_mul_size(attacked_stack_elements, sizeof(uint64_t),
                          &attacked_stack_bytes)) {
        problem_destroy(&problem);
        fprintf(stderr, "memory size overflow\n");
        exit(EXIT_FAILURE);
    }
    BranchResult *branches = static_cast<BranchResult *>(calloc(1, branch_bytes));
    if (branches == NULL) {
        problem_destroy(&problem);
        fprintf(stderr, "memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

#pragma omp parallel for schedule(dynamic, 1)
    for (int first = 0; first <= last_first; ++first) {
        BranchResult *branch = &branches[first];
        branch->best_attacked = problem.attack_limit;
        if (problem.minimum_image[first] < first) continue;

        Search search;
        memset(&search, 0, sizeof(search));
        search.problem = &problem;
        search.best_attacked = problem.attack_limit;
        search.chosen = static_cast<int *>(malloc(n_int_bytes));
        search.image = static_cast<int *>(malloc(n_int_bytes));
        search.next_square = static_cast<int *>(malloc(level_int_bytes));
        search.attacked_count = static_cast<int *>(malloc(level_int_bytes));
        search.attacked_stack = static_cast<uint64_t *>(malloc(attacked_stack_bytes));
        if (search.chosen == NULL || search.image == NULL ||
            search.next_square == NULL || search.attacked_count == NULL ||
            search.attacked_stack == NULL) {
            free(search.chosen);
            free(search.image);
            free(search.next_square);
            free(search.attacked_count);
            free(search.attacked_stack);
            branch->best_attacked = -1;
            continue;
        }
        search.chosen[0] = first;

        uint64_t *attacked = search.attacked_stack + problem.words;
        memcpy(attacked, const_attack_mask(&problem, first),
               (size_t)problem.words * sizeof(*attacked));
        int attacked_count = 0;
        for (int word = 0; word < problem.words; ++word) {
            attacked_count += __builtin_popcountll(attacked[word]);
        }
        if (attacked_count <= search.best_attacked) {
            search_branch(&search, first, attacked_count);
        }

        branch->best_attacked = search.best_attacked;
        branch->found = search.found;
        branch->overflow = search.overflow;
        branch->solutions = search.solutions;
        branch->all_solutions = search.all_solutions;
        memcpy(branch->orbit_counts, search.orbit_counts,
               sizeof(branch->orbit_counts));
        branch->nodes = search.nodes;
        free(search.chosen);
        free(search.image);
        free(search.next_square);
        free(search.attacked_count);
        free(search.attacked_stack);
    }

    int best_attacked = INT_MAX;
    for (int first = 0; first <= last_first; ++first) {
        if (branches[first].best_attacked < 0) {
            free(branches);
            problem_destroy(&problem);
            fprintf(stderr, "memory allocation failed\n");
            exit(EXIT_FAILURE);
        }
        if (branches[first].overflow) {
            free(branches);
            problem_destroy(&problem);
            fprintf(stderr, "64-bit search counter overflow\n");
            exit(3);
        }
        if (branches[first].found &&
            branches[first].best_attacked < best_attacked) {
            best_attacked = branches[first].best_attacked;
        }
    }

    Result result = {};
    result.n = n;
    result.free_squares = best_attacked == INT_MAX
                              ? -1
                              : problem.size - best_attacked;
    result.nodes = 1;
    for (int first = 0; first <= last_first; ++first) {
        if (!checked_add_u64(&result.nodes, branches[first].nodes)) {
            free(branches);
            problem_destroy(&problem);
            fprintf(stderr, "64-bit node counter overflow\n");
            exit(3);
        }
        if (branches[first].found &&
            branches[first].best_attacked == best_attacked) {
            if (!checked_add_u64(&result.solutions, branches[first].solutions) ||
                !checked_add_u64(&result.all_solutions,
                                 branches[first].all_solutions)) {
                free(branches);
                problem_destroy(&problem);
                fprintf(stderr, "64-bit solution counter overflow\n");
                exit(3);
            }
            for (int i = 0; i < ORBIT_SIZE_KINDS; ++i) {
                if (!checked_add_u64(&result.orbit_counts[i],
                                     branches[first].orbit_counts[i])) {
                    free(branches);
                    problem_destroy(&problem);
                    fprintf(stderr, "64-bit orbit counter overflow\n");
                    exit(3);
                }
            }
        }
    }

    uint64_t orbit_total = 0;
    uint64_t orbit_weighted_total = 0;
    for (int i = 0; i < ORBIT_SIZE_KINDS; ++i) {
        uint64_t weighted;
        if (!checked_add_u64(&orbit_total, result.orbit_counts[i]) ||
            !checked_mul_u64(result.orbit_counts[i], (uint64_t)orbit_sizes[i],
                             &weighted) ||
            !checked_add_u64(&orbit_weighted_total, weighted)) {
            free(branches);
            problem_destroy(&problem);
            fprintf(stderr, "64-bit orbit verification overflow\n");
            exit(3);
        }
    }
    if (orbit_total != result.solutions ||
        orbit_weighted_total != result.all_solutions) {
        free(branches);
        problem_destroy(&problem);
        fprintf(stderr, "internal orbit-count verification failed\n");
        exit(4);
    }

    free(branches);
    problem_destroy(&problem);
    return result;
}

static void usage(const char *program) {
    fprintf(stderr, "Usage: %s N\n", program);
    fprintf(stderr, "       %s --upto N\n", program);
}

static bool parse_n(const char *text, int *value) {
    char *end = NULL;
    errno = 0;
    const long parsed = strtol(text, &end, 10);
    if (text[0] == '\0' || *end != '\0' || errno == ERANGE || parsed < 1 ||
        parsed > INT_MAX || parsed > INT_MAX / parsed) {
        return false;
    }
    *value = (int)parsed;
    return true;
}

int main(int argc, char **argv) {
    int first_n;
    int last_n;
    if (argc == 2 && parse_n(argv[1], &last_n)) {
        first_n = last_n;
    } else if (argc == 3 && strcmp(argv[1], "--upto") == 0 &&
               parse_n(argv[2], &last_n)) {
        first_n = 1;
    } else {
        usage(argv[0]);
        return EXIT_FAILURE;
    }

    bool all_verified = true;
    for (int n = first_n; n <= last_n; ++n) {
        const Result result = solve(n);
        const bool checks_free = n <= KNOWN_FREE_MAX_N;
        const bool checks_a019317 = n <= KNOWN_A019317_MAX_N;
        const bool free_matches = !checks_free || result.free_squares == known_free[n];
        const bool a019317_matches = !checks_a019317 ||
                                      result.solutions == known_a019317[n];
        const bool matches = free_matches && a019317_matches;
        if (!matches) all_verified = false;

        const char *status = "";
        if (checks_a019317) {
            status = matches ? " OK" : " MISMATCH";
        } else if (checks_free) {
            status = free_matches ? " A001366_OK" : " A001366_MISMATCH";
        }
        printf("n=%d A001366(n)=%d A019317(n)=%" PRIu64
               " orbits={1:%" PRIu64 ",2:%" PRIu64
               ",4:%" PRIu64 ",8:%" PRIu64 "}"
               " all=%" PRIu64 " nodes=%" PRIu64 "%s\n",
               n, result.free_squares, result.solutions,
               result.orbit_counts[0], result.orbit_counts[1],
               result.orbit_counts[2], result.orbit_counts[3],
               result.all_solutions, result.nodes, status);
    }
    return all_verified ? EXIT_SUCCESS : 2;
}
