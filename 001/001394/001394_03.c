/*
 * 001394_03.c -- Exact memory-intensive enumeration of n-step
 *                 self-avoiding walks on the diamond lattice (OEIS A001394).
 *
 * This implementation applies the independently written meet-in-the-middle
 * bag method of MIT-licensed 001337_07.c to the diamond lattice.  It contains
 * no SAWdoubler source code.
 *
 * For an n-step walk, put na=ceil(n/2), nb=floor(n/2), and let X be the
 * vertex reached after na steps.  Every full walk has one and only one
 * decomposition
 *
 *      O --na steps--> X --nb steps--> free endpoint.
 *
 * A is the bag of O-to-X half-walk vertex sets with X removed.  B is the bag
 * of nb-step half-walk vertex sets rooted at X, again with X removed.  A pair
 * concatenates to a self-avoiding walk exactly when its two stored sets are
 * disjoint.  Equal sets are compressed to multiplicities.
 *
 * Two independently written exact disjointness engines are available:
 *
 *   word  (default)
 *     Index the smaller bag by vertex.  One machine word records 64 sets, so
 *     the union of all indexed sets meeting one query set is formed 64 pairs
 *     at a time.  Multiplicities are then summed over the complementary bits.
 *     Queries with the same exact projection onto the indexed sites reuse a
 *     bounded cache entry; hash matches are always verified by comparing the
 *     complete projected site sequence.
 *
 *   bag
 *     The inclusion-exclusion/direct-count hybrid inherited from the
 *     MIT-licensed 001337_07.c.  This is retained as a structurally different
 *     reference implementation and as a useful cross-check.
 *
 * Both engines implement only the published length-doubling identity:
 * enumerate two half-walk families and count their disjoint pairs.  The word
 * engine was derived from that identity for this file and contains no code,
 * data structure, control flow, or comments copied from SAWdoubler.
 *
 * Algorithmic reference (not a source-code origin):
 * R. D. Schram, G. T. Barkema and R. H. Bisseling,
 * "Exact enumeration of self-avoiding walks", J. Stat. Mech. (2011) P06019,
 * https://arxiv.org/abs/1104.2184 .
 *
 * The B bag is enumerated once at the origin.  A diamond-lattice automorphism
 *
 *      phi_X(v) = X+v  if X is on the origin sublattice,
 *                 X-v  otherwise
 *
 * transports it bijectively to a bag rooted at X.  The 24 signed coordinate
 * permutations with positive sign product fix the origin and preserve the
 * diamond lattice.  Thus midpoint contributions are constant on their
 * orbits; only one representative is evaluated and is multiplied by the
 * number of distinct points in its orbit.
 *
 * Copyright (c) 2026 manman4
 * SPDX-License-Identifier: MIT
 *
 * Build without OpenMP:
 *   clang -O3 -std=c11 001394_03.c -o 001394_03
 *
 * Build with OpenMP on macOS (using the user's gcc-omp alias):
 *   gcc-omp 001394_03.c -o 001394_03
 *
 * Usage:
 *   ./001394_03 n
 *   ./001394_03 --upto N
 *   ./001394_03 --endpoints N
 *   ./001394_03 --endpoints N 001396
 *   ./001394_03 --selftest
 *   ./001394_03 --engine word|bag n
 *   ./001394_03 --cutoff K n
 */

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _OPENMP
#if defined(__clang__)
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wpedantic"
#endif
#include <omp.h>
#if defined(__clang__)
#pragma clang diagnostic pop
#endif
#endif

typedef __int128 i128;
typedef unsigned __int128 u128;

#define MIN_STEPS 0
#define MAX_STEPS 40
#define MAX_HALF_STEPS ((MAX_STEPS + 1) / 2)
#define MAX_BAG_WIDTH MAX_HALF_STEPS
#define MAX_OMP_THREADS 8
#define DEFAULT_CUTOFF 32U
#define HALF_WALK_LIMIT UINT64_C(4649045868) /* 4 * 3^19 */
#define WORD_CACHE_MIN_QUERIES 64U
#define WORD_CACHE_MAX_CAPACITY (UINT32_C(1) << 18)
#define WORD_CACHE_MAX_KEY_IDS (UINT32_C(1) << 22)

typedef enum {
    ENGINE_WORD,
    ENGINE_BAG
} Engine;

_Static_assert(SIZE_MAX >= HALF_WALK_LIMIT,
               "001394_03 requires a 64-bit size_t");
_Static_assert(INT16_MAX >= MAX_STEPS,
               "lattice coordinates do not fit in int16_t");

/*
 * Integer and array safety for 0 <= n <= 40:
 * - each generated half has length at most 20;
 * - a half has at most H=4*3^19 < 2^33 nonbacktracking walks;
 * - a disjoint-pair product is at most H^2 < 2^65;
 * - the final count is at most 4*3^39 < 2^64;
 * - signed 128-bit arithmetic therefore has ample headroom;
 * - multiplicities use uint64_t, bag site identifiers use uint16_t, and every
 *   allocation-size product is checked before allocation.
 * - each word-engine worker's projection cache is capped at 2^18 entries and
 *   2^22 SiteIds (at most about 14 MiB), so cache growth is explicitly
 *   bounded and cache saturation affects speed only, never the result.
 *
 * At n=41 the same elementary upper bound exceeds UINT64_MAX, so the program
 * rejects n>40 before allocating the lattice or half-walk bags.  In practice,
 * memory or running time will usually become limiting well before n=40.
 */

_Noreturn static void die(const char *message)
{
    fprintf(stderr, "001394_03: %s\n", message);
    exit(EXIT_FAILURE);
}

static size_t checked_product(size_t a, size_t b)
{
    if (a != 0U && b > SIZE_MAX / a)
        die("allocation size overflow");
    return a * b;
}

static void *xmalloc(size_t bytes)
{
    void *p = malloc(bytes != 0U ? bytes : 1U);
    if (p == NULL)
        die("out of memory");
    return p;
}

static void *xmalloc_array(size_t count, size_t size)
{
    return xmalloc(checked_product(count, size));
}

static void *xcalloc_array(size_t count, size_t size)
{
    void *p;

    (void)checked_product(count, size);
    p = calloc(count != 0U ? count : 1U, size != 0U ? size : 1U);
    if (p == NULL)
        die("out of memory");
    return p;
}

static void *xrealloc_array(void *old, size_t count, size_t size)
{
    size_t bytes = checked_product(count, size);
    void *p = realloc(old, bytes != 0U ? bytes : 1U);

    if (p == NULL)
        die("out of memory while growing a half-walk bag");
    return p;
}

static void format_i128(char *out, i128 value)
{
    char reversed[48];
    int length = 0;
    int output = 0;
    int negative = value < 0;
    u128 magnitude;

    if (negative)
        magnitude = (u128)(-(value + 1)) + 1U;
    else
        magnitude = (u128)value;
    if (magnitude == 0U)
        reversed[length++] = '0';
    while (magnitude != 0U) {
        reversed[length++] = (char)('0' + (int)(magnitude % 10U));
        magnitude /= 10U;
    }
    if (negative)
        out[output++] = '-';
    while (length > 0)
        out[output++] = reversed[--length];
    out[output] = '\0';
}

static void print_i128(i128 value)
{
    char buffer[48];

    format_i128(buffer, value);
    fputs(buffer, stdout);
}

/* ------------------------------------------------------------------------- */
/* Diamond lattice                                                           */
/* ------------------------------------------------------------------------- */

/*
 * From an A-sublattice vertex the four increments are STEP[d]; from a
 * B-sublattice vertex they are -STEP[d].
 */
static const int STEP[4][3] = {
    { 1,  1,  1},
    { 1, -1, -1},
    {-1,  1, -1},
    {-1, -1,  1}
};

enum {
    NOT_A_DIAMOND_VERTEX = 0,
    A_SUBLATTICE = 1,
    B_SUBLATTICE = 2
};

typedef struct {
    int radius;
    int side;
    size_t cells;
    int nsite;
    int *cell_to_site;
    int16_t *x;
    int16_t *y;
    int16_t *z;
    int32_t *neighbor; /* nsite * 4 */
} Lattice;

static int positive_mod4(int value)
{
    int result = value % 4;
    return result < 0 ? result + 4 : result;
}

static int diamond_kind(int x, int y, int z)
{
    int x_even = x % 2 == 0;
    int y_even = y % 2 == 0;
    int z_even = z % 2 == 0;
    int residue = positive_mod4(x + y + z);

    if (x_even && y_even && z_even && residue == 0)
        return A_SUBLATTICE;
    if (!x_even && !y_even && !z_even && residue == 3)
        return B_SUBLATTICE;
    return NOT_A_DIAMOND_VERTEX;
}

static size_t cell_of(const Lattice *lattice, int x, int y, int z)
{
    size_t sx = (size_t)(x + lattice->radius);
    size_t sy = (size_t)(y + lattice->radius);
    size_t sz = (size_t)(z + lattice->radius);
    size_t side = (size_t)lattice->side;

    return (sx * side + sy) * side + sz;
}

static int site_of(const Lattice *lattice, int x, int y, int z)
{
    if (x < -lattice->radius || x > lattice->radius
        || y < -lattice->radius || y > lattice->radius
        || z < -lattice->radius || z > lattice->radius)
        return -1;
    return lattice->cell_to_site[cell_of(lattice, x, y, z)];
}

static void lattice_build(Lattice *lattice, int radius)
{
    size_t cell;
    size_t head;
    size_t tail;
    size_t plane;
    int16_t *distance;
    size_t *queue;
    int x;
    int y;
    int z;
    int site;
    int direction;

    if (radius < 0 || radius > MAX_STEPS)
        die("internal lattice radius out of range");
    lattice->radius = radius;
    lattice->side = 2 * radius + 1;
    if (lattice->side <= 0)
        die("internal nonpositive lattice side");
    lattice->cells = checked_product(
        checked_product((size_t)lattice->side, (size_t)lattice->side),
        (size_t)lattice->side);
    if (lattice->cells == 0U)
        die("internal empty coordinate box");
    lattice->cell_to_site =
        xmalloc_array(lattice->cells, sizeof(*lattice->cell_to_site));
    distance = xmalloc_array(lattice->cells, sizeof(*distance));
    queue = xmalloc_array(lattice->cells, sizeof(*queue));
    for (cell = 0; cell < lattice->cells; ++cell) {
        lattice->cell_to_site[cell] = -1;
        distance[cell] = -1;
    }

    /*
     * Keep only the graph-distance ball of radius n around the origin.
     * Every vertex of an n-step walk is in this ball, whereas the surrounding
     * coordinate cube contains unreachable corner regions.  The reduction is
     * exact and keeps all supported balls small enough for 16-bit bag IDs.
     */
    head = 0U;
    tail = 0U;
    cell = cell_of(lattice, 0, 0, 0);
    distance[cell] = 0;
    queue[tail++] = cell;
    plane = checked_product((size_t)lattice->side,
                            (size_t)lattice->side);
    if (plane == 0U)
        die("internal empty coordinate plane");
    while (head < tail) {
        size_t current_cell = queue[head++];
        size_t sx = current_cell / plane;
        size_t remainder = current_cell % plane;
        size_t sy = remainder / (size_t)lattice->side;
        size_t sz = remainder % (size_t)lattice->side;
        int current_distance = distance[current_cell];
        int kind;
        int sign;

        x = (int)sx - radius;
        y = (int)sy - radius;
        z = (int)sz - radius;
        kind = diamond_kind(x, y, z);
        if (kind == NOT_A_DIAMOND_VERTEX)
            die("internal BFS reached a non-diamond vertex");
        if (current_distance == radius)
            continue;
        sign = kind == A_SUBLATTICE ? 1 : -1;
        for (direction = 0; direction < 4; ++direction) {
            int next_x = x + sign * STEP[direction][0];
            int next_y = y + sign * STEP[direction][1];
            int next_z = z + sign * STEP[direction][2];
            size_t next_cell;

            if (next_x < -radius || next_x > radius
                || next_y < -radius || next_y > radius
                || next_z < -radius || next_z > radius)
                die("internal graph ball escaped its coordinate box");
            next_cell = cell_of(lattice, next_x, next_y, next_z);
            if (distance[next_cell] < 0) {
                if (tail >= lattice->cells)
                    die("internal graph-ball queue overflow");
                distance[next_cell] = (int16_t)(current_distance + 1);
                queue[tail++] = next_cell;
            }
        }
    }
    if (tail == 0U)
        die("internal empty graph ball");
    if (tail > (size_t)UINT16_MAX)
        die("graph ball is too large for uint16_t bag identifiers");
    lattice->nsite = (int)tail;
    /*
     * Zero-initialize these arrays as an additional safety property.  The
     * exact fill count is checked below, so initialization is not relied on
     * for correctness, but it also makes every failure path well-defined.
     */
    lattice->x =
        xcalloc_array((size_t)lattice->nsite, sizeof(*lattice->x));
    lattice->y =
        xcalloc_array((size_t)lattice->nsite, sizeof(*lattice->y));
    lattice->z =
        xcalloc_array((size_t)lattice->nsite, sizeof(*lattice->z));

    site = 0;
    for (x = -radius; x <= radius; ++x) {
        for (y = -radius; y <= radius; ++y) {
            for (z = -radius; z <= radius; ++z) {
                if (distance[cell_of(lattice, x, y, z)] >= 0) {
                    lattice->cell_to_site[cell_of(lattice, x, y, z)] = site;
                    lattice->x[site] = (int16_t)x;
                    lattice->y[site] = (int16_t)y;
                    lattice->z[site] = (int16_t)z;
                    ++site;
                }
            }
        }
    }
    if (site != lattice->nsite)
        die("internal diamond-lattice site count mismatch");
    free(queue);
    free(distance);

    lattice->neighbor = xmalloc_array(
        checked_product((size_t)lattice->nsite, 4U),
        sizeof(*lattice->neighbor));
    for (site = 0; site < lattice->nsite; ++site) {
        int kind = diamond_kind(
            lattice->x[site], lattice->y[site], lattice->z[site]);
        int sign = kind == A_SUBLATTICE ? 1 : -1;

        if (kind == NOT_A_DIAMOND_VERTEX)
            die("internal invalid diamond-lattice site");
        for (direction = 0; direction < 4; ++direction) {
            lattice->neighbor[(size_t)site * 4U + (size_t)direction] =
                site_of(lattice,
                        lattice->x[site] + sign * STEP[direction][0],
                        lattice->y[site] + sign * STEP[direction][1],
                        lattice->z[site] + sign * STEP[direction][2]);
        }
    }
}

static void lattice_free(Lattice *lattice)
{
    free(lattice->cell_to_site);
    free(lattice->x);
    free(lattice->y);
    free(lattice->z);
    free(lattice->neighbor);
}

/* ------------------------------------------------------------------------- */
/* Rooted tetrahedral symmetry                                               */
/* ------------------------------------------------------------------------- */

static const int PERMUTATION[6][3] = {
    {0, 1, 2}, {0, 2, 1}, {1, 0, 2},
    {1, 2, 0}, {2, 0, 1}, {2, 1, 0}
};

static const int EVEN_SIGNS[4][3] = {
    { 1,  1,  1},
    { 1, -1, -1},
    {-1,  1, -1},
    {-1, -1,  1}
};

static void symmetry_apply(int symmetry, const int input[3], int output[3])
{
    int permutation = symmetry / 4;
    int signs = symmetry % 4;
    int coordinate;

    if (symmetry < 0 || symmetry >= 24)
        die("internal invalid diamond symmetry");
    for (coordinate = 0; coordinate < 3; ++coordinate) {
        output[coordinate] =
            EVEN_SIGNS[signs][coordinate]
            * input[PERMUTATION[permutation][coordinate]];
    }
}

static void build_orbits(const Lattice *lattice, int *representative,
                         int *orbit_size)
{
    int site;

    for (site = 0; site < lattice->nsite; ++site) {
        int point[3] = {
            lattice->x[site], lattice->y[site], lattice->z[site]
        };
        int images[24];
        int best = site;
        int distinct = 0;
        int symmetry;

        for (symmetry = 0; symmetry < 24; ++symmetry) {
            int image[3];
            int other;

            symmetry_apply(symmetry, point, image);
            other = site_of(lattice, image[0], image[1], image[2]);
            if (other < 0)
                die("diamond symmetry left the finite coordinate box");
            images[symmetry] = other;
            if (other < best)
                best = other;
        }
        for (symmetry = 0; symmetry < 24; ++symmetry) {
            int earlier;
            int fresh = 1;

            for (earlier = 0; earlier < symmetry; ++earlier) {
                if (images[earlier] == images[symmetry]) {
                    fresh = 0;
                    break;
                }
            }
            if (fresh)
                ++distinct;
        }
        representative[site] = best;
        orbit_size[site] = site == best ? distinct : 0;
    }
}

/*
 * Endpoint-x counts are not invariant under all 24 rooted symmetries: a
 * coordinate permutation can turn an x condition into a y or z condition,
 * and a sign change can turn x=t into x=-t.  The four symmetries satisfying
 *
 *     output_x = input_x
 *
 * for every point do preserve each requested x value separately.  Orbit
 * reduction under exactly this subgroup is therefore valid simultaneously
 * for x=0 and x=2, or for x=1 and x=3.
 */
static int symmetry_preserves_x(int symmetry)
{
    int permutation;
    int signs;

    if (symmetry < 0 || symmetry >= 24)
        die("internal invalid diamond symmetry");
    permutation = symmetry / 4;
    signs = symmetry % 4;
    return PERMUTATION[permutation][0] == 0
        && EVEN_SIGNS[signs][0] == 1;
}

static void build_x_preserving_orbits(const Lattice *lattice,
                                      int *representative,
                                      int *orbit_size)
{
    int site;

    for (site = 0; site < lattice->nsite; ++site) {
        int point[3] = {
            lattice->x[site], lattice->y[site], lattice->z[site]
        };
        int images[24];
        int image_count = 0;
        int best = site;
        int distinct = 0;
        int symmetry;

        for (symmetry = 0; symmetry < 24; ++symmetry) {
            int image[3];
            int other;

            if (!symmetry_preserves_x(symmetry))
                continue;
            symmetry_apply(symmetry, point, image);
            if (image[0] != point[0])
                die("x-preserving symmetry changed x");
            other = site_of(lattice, image[0], image[1], image[2]);
            if (other < 0)
                die("diamond symmetry left the finite coordinate box");
            images[image_count++] = other;
            if (other < best)
                best = other;
        }
        if (image_count != 4)
            die("internal x-preserving symmetry count is not four");
        for (symmetry = 0; symmetry < image_count; ++symmetry) {
            int earlier;
            int fresh = 1;

            for (earlier = 0; earlier < symmetry; ++earlier) {
                if (images[earlier] == images[symmetry]) {
                    fresh = 0;
                    break;
                }
            }
            if (fresh)
                ++distinct;
        }
        representative[site] = best;
        orbit_size[site] = site == best ? distinct : 0;
    }
}

/* ------------------------------------------------------------------------- */
/* Bags of sorted vertex sets                                                */
/* ------------------------------------------------------------------------- */

typedef uint16_t SiteId;

typedef struct {
    SiteId *row;
    uint64_t *multiplicity;
    int16_t *endpoint_x;
    size_t count;
    size_t capacity;
    int width;
    int track_endpoint;
} Bag;

static void bag_init(Bag *bag, int width)
{
    if (width < 0 || width > MAX_BAG_WIDTH)
        die("internal half-walk width out of range");
    bag->row = NULL;
    bag->multiplicity = NULL;
    bag->endpoint_x = NULL;
    bag->count = 0U;
    bag->capacity = 0U;
    bag->width = width;
    bag->track_endpoint = 0;
}

static void bag_init_endpoints(Bag *bag, int width)
{
    bag_init(bag, width);
    bag->track_endpoint = 1;
}

static void bag_free(Bag *bag)
{
    free(bag->row);
    free(bag->multiplicity);
    free(bag->endpoint_x);
}

static void sort_row(SiteId *row, int width)
{
    int i;

    for (i = 1; i < width; ++i) {
        SiteId value = row[i];
        int j = i - 1;

        while (j >= 0 && row[j] > value) {
            row[j + 1] = row[j];
            --j;
        }
        row[j + 1] = value;
    }
}

static void bag_reserve_one(Bag *bag)
{
    size_t new_capacity;

    if (bag->count < bag->capacity)
        return;
    if (bag->capacity == 0U) {
        new_capacity = 64U;
    } else {
        if (bag->capacity > SIZE_MAX / 2U)
            die("half-walk capacity overflow");
        new_capacity = bag->capacity * 2U;
    }
    if ((uint64_t)new_capacity > HALF_WALK_LIMIT)
        new_capacity = (size_t)HALF_WALK_LIMIT;
    if (new_capacity <= bag->capacity)
        die("half-walk capacity cannot grow");
    if (bag->width > 0) {
        bag->row = xrealloc_array(
            bag->row,
            checked_product(new_capacity, (size_t)bag->width),
            sizeof(*bag->row));
    }
    if (bag->track_endpoint) {
        bag->endpoint_x = xrealloc_array(
            bag->endpoint_x, new_capacity, sizeof(*bag->endpoint_x));
    }
    bag->capacity = new_capacity;
}

static void bag_add_internal(Bag *bag, const int *sites,
                             int have_endpoint, int endpoint_x)
{
    SiteId *destination;
    int index;

    if ((uint64_t)bag->count >= HALF_WALK_LIMIT)
        die("half-walk count exceeded its proved bound");
    if (bag->track_endpoint != have_endpoint)
        die("half-walk endpoint tracking mode mismatch");
    if (endpoint_x < INT16_MIN || endpoint_x > INT16_MAX)
        die("half-walk endpoint x does not fit in int16_t");
    if (bag->width == 0 && !bag->track_endpoint) {
        ++bag->count;
        return;
    }
    bag_reserve_one(bag);
    if (bag->track_endpoint)
        bag->endpoint_x[bag->count] = (int16_t)endpoint_x;
    if (bag->width == 0) {
        ++bag->count;
        return;
    }
    destination =
        bag->row + bag->count * (size_t)bag->width;
    for (index = 0; index < bag->width; ++index) {
        if (sites[index] < 0 || sites[index] > (int)UINT16_MAX)
            die("internal lattice site does not fit in uint16_t");
        destination[index] = (SiteId)sites[index];
    }
    sort_row(destination, bag->width);
    ++bag->count;
}

static void bag_add(Bag *bag, const int *sites)
{
    bag_add_internal(bag, sites, 0, 0);
}

static void bag_add_endpoint(Bag *bag, const int *sites, int endpoint_x)
{
    bag_add_internal(bag, sites, 1, endpoint_x);
}

static void bag_compress(Bag *bag, int nsite)
{
    size_t original_count = bag->count;
    size_t *order;
    size_t *scratch;
    size_t *bucket;
    SiteId *output_rows;
    uint64_t *output_multiplicity;
    int16_t *output_endpoint_x = NULL;
    size_t row;
    size_t unique;
    int column;
    int site;

    if (original_count == 0U) {
        bag->multiplicity = NULL;
        return;
    }
    if (bag->width == 0 && !bag->track_endpoint) {
        bag->multiplicity =
            xmalloc_array(1U, sizeof(*bag->multiplicity));
        bag->multiplicity[0] = (uint64_t)original_count;
        bag->count = 1U;
        bag->capacity = 1U;
        return;
    }

    order = xmalloc_array(original_count, sizeof(*order));
    scratch = xmalloc_array(original_count, sizeof(*scratch));
    bucket = xcalloc_array((size_t)nsite + 1U, sizeof(*bucket));
    for (row = 0; row < original_count; ++row)
        order[row] = row;

    if (bag->track_endpoint) {
        size_t endpoint_bucket[2U * MAX_HALF_STEPS + 2U] = {0U};
        size_t endpoint_bins = 2U * MAX_HALF_STEPS + 1U;

        for (row = 0; row < original_count; ++row) {
            int endpoint = bag->endpoint_x[order[row]];
            size_t value;

            if (endpoint < -MAX_HALF_STEPS
                || endpoint > MAX_HALF_STEPS)
                die("template endpoint x is outside its proved range");
            value = (size_t)(endpoint + MAX_HALF_STEPS);
            ++endpoint_bucket[value + 1U];
        }
        for (row = 0; row < endpoint_bins; ++row)
            endpoint_bucket[row + 1U] += endpoint_bucket[row];
        for (row = 0; row < original_count; ++row) {
            int endpoint = bag->endpoint_x[order[row]];
            size_t value =
                (size_t)(endpoint + MAX_HALF_STEPS);

            scratch[endpoint_bucket[value]++] = order[row];
        }
        memcpy(order, scratch,
               checked_product(original_count, sizeof(*order)));
    }

    for (column = bag->width - 1; column >= 0; --column) {
        if (column != bag->width - 1) {
            memset(bucket, 0,
                   checked_product((size_t)nsite + 1U, sizeof(*bucket)));
        }
        for (row = 0; row < original_count; ++row) {
            SiteId value =
                bag->row[order[row] * (size_t)bag->width + (size_t)column];
            ++bucket[(size_t)value + 1U];
        }
        for (site = 0; site < nsite; ++site)
            bucket[(size_t)site + 1U] += bucket[(size_t)site];
        for (row = 0; row < original_count; ++row) {
            SiteId value =
                bag->row[order[row] * (size_t)bag->width + (size_t)column];
            scratch[bucket[value]++] = order[row];
        }
        memcpy(order, scratch,
               checked_product(original_count, sizeof(*order)));
    }

    output_rows = xcalloc_array(
        checked_product(original_count, (size_t)bag->width),
        sizeof(*output_rows));
    output_multiplicity =
        xmalloc_array(original_count, sizeof(*output_multiplicity));
    if (bag->track_endpoint) {
        output_endpoint_x =
            xmalloc_array(original_count, sizeof(*output_endpoint_x));
    }
    unique = 0U;
    for (row = 0; row < original_count; ++row) {
        const SiteId *source =
            bag->width > 0
            ? bag->row + order[row] * (size_t)bag->width : NULL;
        int same_row = 0;
        int same_endpoint =
            !bag->track_endpoint
            || (unique > 0U
                && output_endpoint_x[unique - 1U]
                   == bag->endpoint_x[order[row]]);

        if (unique > 0U) {
            if (bag->width == 0) {
                same_row = 1;
            } else {
                if (source == NULL)
                    die("internal null compressed half-walk row");
                same_row =
                    memcmp(
                        output_rows
                            + (unique - 1U) * (size_t)bag->width,
                        source,
                        checked_product((size_t)bag->width,
                                        sizeof(*output_rows))) == 0;
            }
        }
        if (same_row && same_endpoint) {
            if (output_multiplicity[unique - 1U] == UINT64_MAX)
                die("half-walk multiplicity overflow");
            ++output_multiplicity[unique - 1U];
        } else {
            if (bag->width > 0) {
                memcpy(output_rows + unique * (size_t)bag->width,
                       source,
                       checked_product((size_t)bag->width,
                                       sizeof(*output_rows)));
            }
            output_multiplicity[unique] = 1U;
            if (bag->track_endpoint) {
                output_endpoint_x[unique] =
                    bag->endpoint_x[order[row]];
            }
            ++unique;
        }
    }
    free(order);
    free(scratch);
    free(bucket);
    free(bag->row);
    free(bag->endpoint_x);
    bag->row = xrealloc_array(
        output_rows,
        checked_product(unique, (size_t)bag->width),
        sizeof(*output_rows));
    bag->multiplicity = xrealloc_array(
        output_multiplicity, unique, sizeof(*output_multiplicity));
    if (bag->track_endpoint) {
        bag->endpoint_x = xrealloc_array(
            output_endpoint_x, unique, sizeof(*output_endpoint_x));
    } else {
        bag->endpoint_x = NULL;
    }
    bag->count = unique;
    bag->capacity = unique;
}

/* ------------------------------------------------------------------------- */
/* Half-walk generation                                                      */
/* ------------------------------------------------------------------------- */

typedef enum {
    GENERATE_MIDPOINT_BAGS,
    GENERATE_TEMPLATE_BAG
} GenerationMode;

typedef struct {
    const Lattice *lattice;
    unsigned char *visited;
    int *path;
    int steps;
    GenerationMode mode;
    const int *representative;
    Bag *midpoint_bags;
    Bag *template_bag;
} Generator;

static void generate_recursively(Generator *generator, int depth)
{
    int current = generator->path[depth];
    int direction;

    if (depth == generator->steps) {
        if (generator->mode == GENERATE_MIDPOINT_BAGS) {
            if (generator->representative[current] == current)
                bag_add(&generator->midpoint_bags[current], generator->path);
        } else {
            if (generator->template_bag->track_endpoint) {
                bag_add_endpoint(
                    generator->template_bag, generator->path + 1,
                    generator->lattice->x[current]);
            } else {
                bag_add(generator->template_bag, generator->path + 1);
            }
        }
        return;
    }

    generator->visited[current] = 1U;
    for (direction = 0; direction < 4; ++direction) {
        int next = generator->lattice->neighbor[
            (size_t)current * 4U + (size_t)direction];

        if (next >= 0 && generator->visited[next] == 0U) {
            generator->path[depth + 1] = next;
            generate_recursively(generator, depth + 1);
        }
    }
    generator->visited[current] = 0U;
}

static void generate_midpoint_bags(const Lattice *lattice, int steps,
                                   int origin, const int *representative,
                                   Bag *bags)
{
    Generator generator;
    unsigned char *visited =
        xcalloc_array((size_t)lattice->nsite, sizeof(*visited));
    int *path = xcalloc_array((size_t)steps + 1U, sizeof(*path));

    generator.lattice = lattice;
    generator.visited = visited;
    generator.path = path;
    generator.steps = steps;
    generator.mode = GENERATE_MIDPOINT_BAGS;
    generator.representative = representative;
    generator.midpoint_bags = bags;
    generator.template_bag = NULL;
    path[0] = origin;
    generate_recursively(&generator, 0);
    free(path);
    free(visited);
}

static void generate_template_bag(const Lattice *lattice, int steps,
                                  int origin, Bag *bag)
{
    Generator generator;
    unsigned char *visited =
        xcalloc_array((size_t)lattice->nsite, sizeof(*visited));
    int *path = xcalloc_array((size_t)steps + 1U, sizeof(*path));

    generator.lattice = lattice;
    generator.visited = visited;
    generator.path = path;
    generator.steps = steps;
    generator.mode = GENERATE_TEMPLATE_BAG;
    generator.representative = NULL;
    generator.midpoint_bags = NULL;
    generator.template_bag = bag;
    path[0] = origin;
    generate_recursively(&generator, 0);
    free(path);
    free(visited);
}

static void transform_template_bag(const Lattice *lattice,
                                   const Bag *template_bag,
                                   int midpoint, Bag *transformed)
{
    int midpoint_x = lattice->x[midpoint];
    int midpoint_y = lattice->y[midpoint];
    int midpoint_z = lattice->z[midpoint];
    int kind = diamond_kind(midpoint_x, midpoint_y, midpoint_z);
    int sign = kind == A_SUBLATTICE ? 1 : -1;
    size_t row;

    if (template_bag->track_endpoint)
        die("endpoint-tracked template cannot use materialized transport");
    if (kind == NOT_A_DIAMOND_VERTEX)
        die("internal midpoint is not on the diamond lattice");
    bag_init(transformed, template_bag->width);
    transformed->count = template_bag->count;
    transformed->capacity = template_bag->count;
    transformed->multiplicity =
        xmalloc_array(template_bag->count,
                      sizeof(*transformed->multiplicity));
    memcpy(transformed->multiplicity, template_bag->multiplicity,
           checked_product(template_bag->count,
                           sizeof(*transformed->multiplicity)));
    if (template_bag->width == 0)
        return;

    transformed->row = xcalloc_array(
        checked_product(template_bag->count,
                        (size_t)template_bag->width),
        sizeof(*transformed->row));
    for (row = 0; row < template_bag->count; ++row) {
        SiteId *destination =
            transformed->row + row * (size_t)template_bag->width;
        const SiteId *source =
            template_bag->row + row * (size_t)template_bag->width;
        int column;

        /*
         * Site identifiers follow lexicographic (x,y,z) order.  Translation
         * by the midpoint preserves that order, while v -> midpoint-v
         * reverses it.  The source row is already sorted, so a forward copy
         * for sign=+1 or a reverse copy for sign=-1 produces a sorted row
         * directly; no per-row insertion sort is needed.
         */
        for (column = 0; column < template_bag->width; ++column) {
            int source_column =
                sign == 1 ? column : template_bag->width - 1 - column;
            SiteId template_site = source[source_column];
            int mapped = site_of(
                lattice,
                midpoint_x + sign * lattice->x[template_site],
                midpoint_y + sign * lattice->y[template_site],
                midpoint_z + sign * lattice->z[template_site]);

            if (mapped < 0 || mapped > (int)UINT16_MAX)
                die("transported half-walk left the finite lattice box");
            destination[column] = (SiteId)mapped;
        }
    }
}

static SiteId *collect_bag_sites(const Bag *bag, int nsite,
                                 size_t *active_count)
{
    unsigned char *seen;
    SiteId *active;
    size_t row;

    if (nsite <= 0 || nsite > (int)UINT16_MAX)
        die("internal lattice size cannot be indexed by SiteId");
    seen = xcalloc_array((size_t)nsite, sizeof(*seen));
    active = xmalloc_array((size_t)nsite, sizeof(*active));
    *active_count = 0U;
    if (bag->width == 0) {
        free(seen);
        return xrealloc_array(active, 0U, sizeof(*active));
    }
    for (row = 0; row < bag->count; ++row) {
        const SiteId *set =
            bag->row + row * (size_t)bag->width;
        int column;

        for (column = 0; column < bag->width; ++column) {
            SiteId site = set[column];

            if ((int)site >= nsite)
                die("half-walk bag contains an invalid site");
            if (seen[site] == 0U) {
                seen[site] = 1U;
                active[(*active_count)++] = site;
            }
        }
    }
    free(seen);
    return xrealloc_array(active, *active_count, sizeof(*active));
}

/*
 * Build phi_X only on sites that actually occur in the reusable template
 * bag.  Applying the same bijection to every stored set preserves both set
 * multiplicities and disjointness, so the full transported bag need not be
 * allocated for the word engine.
 */
static SiteId *build_transport_map(const Lattice *lattice, int midpoint,
                                   const SiteId *active,
                                   size_t active_count)
{
    SiteId *map =
        xmalloc_array((size_t)lattice->nsite, sizeof(*map));
    int midpoint_x = lattice->x[midpoint];
    int midpoint_y = lattice->y[midpoint];
    int midpoint_z = lattice->z[midpoint];
    int kind = diamond_kind(midpoint_x, midpoint_y, midpoint_z);
    int sign;
    size_t index;

    if (kind == NOT_A_DIAMOND_VERTEX)
        die("internal midpoint is not on the diamond lattice");
    sign = kind == A_SUBLATTICE ? 1 : -1;
    for (index = 0; index < (size_t)lattice->nsite; ++index)
        map[index] = UINT16_MAX;
    for (index = 0; index < active_count; ++index) {
        SiteId source = active[index];
        int mapped;

        if ((int)source >= lattice->nsite)
            die("transport map contains an invalid source site");
        mapped = site_of(
            lattice,
            midpoint_x + sign * lattice->x[source],
            midpoint_y + sign * lattice->y[source],
            midpoint_z + sign * lattice->z[source]);
        if (mapped < 0 || mapped >= lattice->nsite
            || mapped > (int)UINT16_MAX)
            die("transport map left the finite lattice ball");
        map[source] = (SiteId)mapped;
    }
    return map;
}

/* ------------------------------------------------------------------------- */
/* Exact word-parallel disjoint-pair counter                                 */
/* ------------------------------------------------------------------------- */

static const SiteId *bag_row(const Bag *bag, size_t index);

/*
 * This is a direct implementation of
 *
 *     sum_{q in Q} mult(q)
 *         * sum_{s in S, q intersection s = empty} mult(s).
 *
 * The smaller family S is indexed.  For every lattice site v, bit k of
 * site_bits[v] says whether indexed set k contains v.  For a query q, OR-ing
 * the bit vectors for v in q marks every intersecting indexed set.  Taking
 * the complement therefore gives exactly the disjoint sets.  No hashing,
 * probabilistic filter, or unchecked arithmetic is involved.
 */
static unsigned least_set_bit(uint64_t word)
{
    if (word == 0U)
        die("internal least-set-bit call with a zero word");
#if defined(__clang__) || defined(__GNUC__)
    return (unsigned)__builtin_ctzll((unsigned long long)word);
#else
    unsigned bit = 0U;

    while ((word & UINT64_C(1)) == 0U) {
        word >>= 1;
        ++bit;
    }
    return bit;
#endif
}

static unsigned set_bit_count(uint64_t word)
{
#if defined(__clang__) || defined(__GNUC__)
    return (unsigned)__builtin_popcountll((unsigned long long)word);
#else
    unsigned count = 0U;

    while (word != 0U) {
        word &= word - UINT64_C(1);
        ++count;
    }
    return count;
#endif
}

typedef struct {
    uint64_t hash;
    uint64_t disjoint_weight;
    uint32_t key_offset;
    uint8_t length;
    uint8_t used;
    uint16_t reserved;
} ProjectionEntry;

typedef struct {
    ProjectionEntry *entry;
    SiteId *key;
    uint32_t key_count;
    uint32_t key_capacity;
    uint32_t capacity;
    uint32_t count;
} ProjectionCache;

_Static_assert(sizeof(ProjectionEntry) == 24U,
               "unexpected projection-cache padding");

static int build_projection_key(const Bag *queries, size_t row,
                                const int *query_site_slot, int nsite,
                                SiteId key[MAX_BAG_WIDTH])
{
    const SiteId *query = bag_row(queries, row);
    int length = 0;
    int column;

    for (column = 0; column < queries->width; ++column) {
        SiteId source_site = query[column];
        int slot;

        if ((int)source_site >= nsite)
            die("query half-walk contains an invalid source site");
        slot = query_site_slot[source_site];
        if (slot == INT_MIN)
            die("query half-walk contains an unmapped site");
        if (slot >= 0) {
            if (slot > (int)UINT16_MAX)
                die("projection slot does not fit in SiteId");
            key[length++] = (SiteId)slot;
        }
    }
    sort_row(key, length);
    return length;
}

static uint64_t projection_hash(const SiteId *key, int length)
{
    uint64_t hash =
        UINT64_C(0x9e3779b97f4a7c15) ^ (uint64_t)(unsigned)length;
    int index;

    for (index = 0; index < length; ++index) {
        hash ^= (uint64_t)key[index] + UINT64_C(0x517cc1b727220a95);
        hash = (uint64_t)(
            (u128)hash * (u128)UINT64_C(0xbf58476d1ce4e5b9));
        hash ^= hash >> 29;
    }
    return hash;
}

static void projection_cache_init(ProjectionCache *cache,
                                  size_t query_count)
{
    uint32_t capacity = WORD_CACHE_MIN_QUERIES;

    cache->entry = NULL;
    cache->key = NULL;
    cache->key_count = 0U;
    cache->key_capacity = 0U;
    cache->capacity = 0U;
    cache->count = 0U;
    if (query_count < WORD_CACHE_MIN_QUERIES)
        return;
    while ((size_t)capacity < query_count * 2U
           && capacity < WORD_CACHE_MAX_CAPACITY)
        capacity *= 2U;
    cache->entry =
        xcalloc_array((size_t)capacity, sizeof(*cache->entry));
    cache->capacity = capacity;
}

static void projection_cache_free(ProjectionCache *cache)
{
    free(cache->key);
    free(cache->entry);
}

static int projection_cache_find(
    const ProjectionCache *cache, uint64_t hash,
    const SiteId *key, int length, uint64_t *disjoint_weight)
{
    uint32_t mask;
    uint32_t slot;

    if (cache->capacity == 0U)
        return 0;
    mask = cache->capacity - 1U;
    slot = (uint32_t)(hash ^ (hash >> 32)) & mask;
    for (;;) {
        const ProjectionEntry *entry = &cache->entry[slot];

        if (entry->used == 0U)
            return 0;
        if (entry->hash == hash && (int)entry->length == length) {
            if ((uint64_t)entry->key_offset + (uint64_t)(unsigned)length
                > (uint64_t)cache->key_count)
                die("projection-cache key range is invalid");
            if (length == 0
                || memcmp(key, cache->key + entry->key_offset,
                          checked_product((size_t)length,
                                          sizeof(*key))) == 0) {
                *disjoint_weight = entry->disjoint_weight;
                return 1;
            }
        }
        slot = (slot + 1U) & mask;
    }
}

static void projection_cache_insert(ProjectionCache *cache,
                                    uint64_t hash, int length,
                                    const SiteId *key,
                                    uint64_t disjoint_weight)
{
    uint32_t required_keys;
    uint32_t mask;
    uint32_t slot;

    if (cache->capacity == 0U
        || (uint64_t)(cache->count + 1U) * 10U
           >= (uint64_t)cache->capacity * 7U)
        return;
    if (length < 0 || length > MAX_BAG_WIDTH)
        die("projection-cache key length is invalid");
    if ((uint64_t)cache->key_count + (uint64_t)(unsigned)length
        > (uint64_t)WORD_CACHE_MAX_KEY_IDS)
        return;
    required_keys = cache->key_count + (uint32_t)(unsigned)length;
    if (required_keys > cache->key_capacity) {
        uint32_t new_capacity =
            cache->key_capacity == 0U ? 1024U : cache->key_capacity;

        while (new_capacity < required_keys) {
            if (new_capacity > WORD_CACHE_MAX_KEY_IDS / 2U) {
                new_capacity = WORD_CACHE_MAX_KEY_IDS;
                break;
            }
            new_capacity *= 2U;
        }
        cache->key = xrealloc_array(
            cache->key, (size_t)new_capacity, sizeof(*cache->key));
        cache->key_capacity = new_capacity;
    }
    mask = cache->capacity - 1U;
    slot = (uint32_t)(hash ^ (hash >> 32)) & mask;
    while (cache->entry[slot].used != 0U)
        slot = (slot + 1U) & mask;
    cache->entry[slot].hash = hash;
    cache->entry[slot].disjoint_weight = disjoint_weight;
    cache->entry[slot].key_offset = cache->key_count;
    cache->entry[slot].length = (uint8_t)length;
    cache->entry[slot].used = 1U;
    if (length > 0) {
        memcpy(cache->key + cache->key_count, key,
               checked_product((size_t)length, sizeof(*key)));
    }
    cache->key_count = required_keys;
    ++cache->count;
}

static void count_disjoint_pairs_word_oriented_multi(
    const Bag *indexed, const Bag *queries, int nsite,
    const SiteId *query_site_map, const int *target_x,
    size_t target_count, int midpoint_x, int transport_sign,
    i128 *answer)
{
    int *site_slot;
    int *query_site_slot;
    uint64_t *site_bits;
    uint64_t *word_total;
    ProjectionCache projection_cache;
    size_t word_count;
    size_t active_sites = 0U;
    size_t row;
    size_t word;
    size_t target;

    if (target_count == 0U || answer == NULL)
        die("word engine requires at least one output target");
    for (target = 0U; target < target_count; ++target)
        answer[target] = 0;
    if (indexed->count == 0U || queries->count == 0U)
        return;
    if (nsite <= 0)
        die("internal nonpositive lattice size");
    if (target_x != NULL && !queries->track_endpoint)
        die("endpoint targets require an endpoint-tracked query bag");
    if (target_x == NULL && target_count != 1U)
        die("unfiltered word count must have exactly one output");
    if (transport_sign != 1 && transport_sign != -1)
        die("word engine received an invalid transport sign");
    if (indexed->count > SIZE_MAX - 63U)
        die("word-index size overflow");
    word_count = (indexed->count + 63U) / 64U;
    if (word_count == 0U)
        die("internal empty word index");

    site_slot = xmalloc_array((size_t)nsite, sizeof(*site_slot));
    for (row = 0; row < (size_t)nsite; ++row)
        site_slot[row] = -1;

    for (row = 0; row < indexed->count; ++row) {
        const SiteId *set = bag_row(indexed, row);
        int column;

        for (column = 0; column < indexed->width; ++column) {
            SiteId site = set[column];

            if ((int)site >= nsite)
                die("indexed half-walk contains an invalid site");
            if (site_slot[site] < 0) {
                if (active_sites >= (size_t)INT_MAX)
                    die("too many active sites for the word index");
                site_slot[site] = (int)active_sites++;
            }
        }
    }

    site_bits = xcalloc_array(
        checked_product(active_sites, word_count), sizeof(*site_bits));
    word_total = xcalloc_array(word_count, sizeof(*word_total));
    for (row = 0; row < indexed->count; ++row) {
        const SiteId *set = bag_row(indexed, row);
        size_t word_index = row / 64U;
        unsigned bit = (unsigned)(row % 64U);
        uint64_t mask = UINT64_C(1) << bit;
        uint64_t multiplicity = indexed->multiplicity[row];
        int column;

        if (word_total[word_index] > UINT64_MAX - multiplicity)
            die("word-index multiplicity overflow");
        word_total[word_index] += multiplicity;
        for (column = 0; column < indexed->width; ++column) {
            int slot = site_slot[set[column]];

            if (slot < 0 || (size_t)slot >= active_sites)
                die("internal invalid active-site slot");
            site_bits[(size_t)slot * word_count + word_index] |= mask;
        }
    }

    query_site_slot =
        xmalloc_array((size_t)nsite, sizeof(*query_site_slot));
    for (row = 0; row < (size_t)nsite; ++row) {
        if (query_site_map != NULL) {
            SiteId mapped = query_site_map[row];

            if ((int)mapped < nsite)
                query_site_slot[row] = site_slot[mapped];
            else
                query_site_slot[row] = INT_MIN;
        } else {
            query_site_slot[row] = site_slot[row];
        }
    }
    projection_cache_init(&projection_cache, queries->count);

    for (row = 0; row < queries->count; ++row) {
        SiteId projection[MAX_BAG_WIDTH];
        size_t category = 0U;
        int projection_length;
        uint64_t hash;
        uint64_t disjoint_weight = 0U;
        int projection_index;

        if (target_x != NULL) {
            int absolute_endpoint =
                midpoint_x
                + transport_sign * (int)queries->endpoint_x[row];

            for (category = 0U; category < target_count; ++category) {
                if (absolute_endpoint == target_x[category])
                    break;
            }
            if (category == target_count)
                continue;
        }
        projection_length = build_projection_key(
            queries, row, query_site_slot, nsite, projection);
        hash = projection_hash(projection, projection_length);

        /*
         * Sites outside the indexed family's support cannot create an
         * intersection and are dropped.  Equal projected sets have exactly
         * the same weighted disjoint count; cache hits are confirmed by a
         * complete key comparison, not by the hash alone.
         */
        if (projection_cache_find(
                &projection_cache, hash, projection, projection_length,
                &disjoint_weight)) {
            answer[category] +=
                (i128)queries->multiplicity[row]
                * (i128)disjoint_weight;
            continue;
        }

        for (word = 0; word < word_count; ++word) {
            uint64_t intersects = 0U;
            uint64_t valid_bits;
            uint64_t intersecting_bits;
            uint64_t disjoint_bits;
            uint64_t word_weight;

            for (projection_index = 0;
                 projection_index < projection_length;
                 ++projection_index) {
                size_t slot = projection[projection_index];

                if (slot >= active_sites)
                    die("internal invalid projected site slot");
                intersects |=
                    site_bits[slot * word_count + word];
            }
            valid_bits = UINT64_MAX;
            if (word + 1U == word_count
                && indexed->count % 64U != 0U) {
                valid_bits =
                    (UINT64_C(1) << (indexed->count % 64U)) - UINT64_C(1);
            }
            intersecting_bits = intersects & valid_bits;
            disjoint_bits = (~intersecting_bits) & valid_bits;
            if (intersecting_bits == 0U) {
                word_weight = word_total[word];
            } else if (disjoint_bits == 0U) {
                word_weight = 0U;
            } else if (set_bit_count(intersecting_bits)
                       <= set_bit_count(disjoint_bits)) {
                /*
                 * Usually only a few indexed sets meet one query.  Starting
                 * from the precomputed total and subtracting those sets is
                 * exactly the same weighted complement sum, but visits the
                 * smaller side of the 64-bit partition.
                 */
                word_weight = word_total[word];
                while (intersecting_bits != 0U) {
                    unsigned bit = least_set_bit(intersecting_bits);
                    size_t indexed_row = word * 64U + (size_t)bit;
                    uint64_t multiplicity;

                    if (indexed_row >= indexed->count)
                        die("word index exposed an invalid intersecting bit");
                    multiplicity = indexed->multiplicity[indexed_row];
                    if (word_weight < multiplicity)
                        die("word-weight subtraction underflow");
                    word_weight -= multiplicity;
                    intersecting_bits &=
                        intersecting_bits - UINT64_C(1);
                }
            } else {
                word_weight = 0U;
                while (disjoint_bits != 0U) {
                    unsigned bit = least_set_bit(disjoint_bits);
                    size_t indexed_row = word * 64U + (size_t)bit;
                    uint64_t multiplicity;

                    if (indexed_row >= indexed->count)
                        die("word index exposed an invalid tail bit");
                    multiplicity = indexed->multiplicity[indexed_row];
                    if (word_weight > UINT64_MAX - multiplicity)
                        die("word-weight addition overflow");
                    word_weight += multiplicity;
                    disjoint_bits &= disjoint_bits - UINT64_C(1);
                }
            }
            if (disjoint_weight > UINT64_MAX - word_weight)
                die("disjoint multiplicity overflow");
            disjoint_weight += word_weight;
        }
        projection_cache_insert(
            &projection_cache, hash, projection_length,
            projection, disjoint_weight);
        answer[category] +=
            (i128)queries->multiplicity[row]
            * (i128)disjoint_weight;
    }

    projection_cache_free(&projection_cache);
    free(word_total);
    free(site_bits);
    free(query_site_slot);
    free(site_slot);
    for (target = 0U; target < target_count; ++target) {
        if (answer[target] < 0)
            die("internal negative word-engine result");
    }
}

static i128 count_disjoint_pairs_word_oriented(
    const Bag *indexed, const Bag *queries, int nsite,
    const SiteId *query_site_map)
{
    i128 answer;

    count_disjoint_pairs_word_oriented_multi(
        indexed, queries, nsite, query_site_map,
        NULL, 1U, 0, 1, &answer);
    return answer;
}

static void count_disjoint_pairs_word_endpoint_targets(
    const Bag *indexed, const Bag *queries, int nsite,
    const SiteId *query_site_map, int midpoint_x, int transport_sign,
    const int *target_x, size_t target_count, i128 *answer)
{
    count_disjoint_pairs_word_oriented_multi(
        indexed, queries, nsite, query_site_map,
        target_x, target_count, midpoint_x, transport_sign, answer);
}

static i128 count_disjoint_pairs_word(const Bag *a, const Bag *b,
                                      int nsite)
{
    /*
     * Either orientation performs the same Cartesian-product test.  Indexing
     * the smaller bag reduces both the bit-vector footprint and its working
     * set, while preserving multiplicities exactly.
     */
    if (a->count <= b->count)
        return count_disjoint_pairs_word_oriented(a, b, nsite, NULL);
    return count_disjoint_pairs_word_oriented(b, a, nsite, NULL);
}

/* ------------------------------------------------------------------------- */
/* Exact disjoint-pair counter                                               */
/* ------------------------------------------------------------------------- */

typedef struct {
    const Bag *a;
    const Bag *b;
    size_t *a_index;
    size_t *b_index;
    uint64_t *seen_a;
    uint64_t *seen_b;
    SiteId *candidate;
    uint64_t epoch;
    int nsite;
    size_t cutoff;
} Counter;

static const SiteId EMPTY_ROW[1] = {0U};

static const SiteId *bag_row(const Bag *bag, size_t index)
{
    if (bag->width == 0)
        return EMPTY_ROW;
    return bag->row + index * (size_t)bag->width;
}

static int rows_share_above(const SiteId *a, int a_width,
                            const SiteId *b, int b_width, int64_t last)
{
    int ai = 0;
    int bi = 0;

    while (ai < a_width && (int64_t)a[ai] <= last)
        ++ai;
    while (bi < b_width && (int64_t)b[bi] <= last)
        ++bi;
    while (ai < a_width && bi < b_width) {
        if (a[ai] == b[bi])
            return 1;
        if (a[ai] < b[bi])
            ++ai;
        else
            ++bi;
    }
    return 0;
}

static size_t partition_on(size_t *index, size_t low, size_t high,
                           const SiteId *rows, int width, SiteId site)
{
    size_t input;
    size_t write = low;

    if (width == 0)
        return low;
    for (input = low; input < high; ++input) {
        const SiteId *row =
            rows + index[input] * (size_t)width;
        int column;
        int found = 0;

        for (column = 0; column < width; ++column) {
            if (row[column] == site) {
                found = 1;
                break;
            }
        }
        if (found) {
            size_t temporary = index[write];
            index[write] = index[input];
            index[input] = temporary;
            ++write;
        }
    }
    return write;
}

static i128 disjoint_subtree(Counter *counter,
                             size_t a_low, size_t a_high,
                             size_t b_low, size_t b_high,
                             int64_t last, int depth)
{
    size_t a_count = a_high - a_low;
    size_t b_count = b_high - b_low;
    size_t ai;
    size_t bi;
    int a_width = counter->a->width;
    int b_width = counter->b->width;
    i128 a_sum = 0;
    i128 b_sum = 0;
    i128 total;
    uint64_t stamp;
    SiteId *candidates;
    int candidate_count = 0;
    int column;

    if (a_count == 0U || b_count == 0U)
        return 0;
    if (a_count <= counter->cutoff / b_count) {
        i128 direct = 0;

        for (ai = a_low; ai < a_high; ++ai) {
            size_t a_row_index = counter->a_index[ai];
            const SiteId *a_row = bag_row(counter->a, a_row_index);
            uint64_t a_multiplicity =
                counter->a->multiplicity[a_row_index];

            for (bi = b_low; bi < b_high; ++bi) {
                size_t b_row_index = counter->b_index[bi];
                const SiteId *b_row = bag_row(counter->b, b_row_index);

                if (!rows_share_above(a_row, a_width,
                                      b_row, b_width, last)) {
                    direct += (i128)a_multiplicity
                              * counter->b->multiplicity[b_row_index];
                }
            }
        }
        return direct;
    }

    for (ai = a_low; ai < a_high; ++ai)
        a_sum += counter->a->multiplicity[counter->a_index[ai]];
    for (bi = b_low; bi < b_high; ++bi)
        b_sum += counter->b->multiplicity[counter->b_index[bi]];
    total = a_sum * b_sum;

    if (counter->epoch == UINT64_MAX) {
        memset(counter->seen_a, 0,
               checked_product((size_t)counter->nsite,
                               sizeof(*counter->seen_a)));
        memset(counter->seen_b, 0,
               checked_product((size_t)counter->nsite,
                               sizeof(*counter->seen_b)));
        counter->epoch = 0U;
    }
    stamp = ++counter->epoch;
    candidates = counter->candidate
                 + (size_t)depth * (size_t)counter->nsite;

    for (ai = a_low; ai < a_high; ++ai) {
        const SiteId *row =
            bag_row(counter->a, counter->a_index[ai]);

        for (column = 0; column < a_width; ++column) {
            if ((int64_t)row[column] > last)
                counter->seen_a[row[column]] = stamp;
        }
    }
    for (bi = b_low; bi < b_high; ++bi) {
        const SiteId *row =
            bag_row(counter->b, counter->b_index[bi]);

        for (column = 0; column < b_width; ++column) {
            SiteId site = row[column];

            if ((int64_t)site > last
                && counter->seen_a[site] == stamp
                && counter->seen_b[site] != stamp) {
                counter->seen_b[site] = stamp;
                candidates[candidate_count++] = site;
            }
        }
    }
    sort_row(candidates, candidate_count);

    for (column = 0; column < candidate_count; ++column) {
        SiteId site = candidates[column];
        size_t a_middle = partition_on(
            counter->a_index, a_low, a_high,
            counter->a->row, a_width, site);
        size_t b_middle = partition_on(
            counter->b_index, b_low, b_high,
            counter->b->row, b_width, site);

        total -= disjoint_subtree(counter,
                                  a_low, a_middle,
                                  b_low, b_middle,
                                  (int64_t)site, depth + 1);
    }
    if (total < 0)
        die("internal inclusion-exclusion invariant failed");
    return total;
}

static i128 count_disjoint_pairs_bag(const Bag *a, const Bag *b,
                                     int nsite, size_t cutoff)
{
    Counter counter;
    size_t row;
    int max_depth = (a->width < b->width ? a->width : b->width) + 2;
    i128 answer;

    if (a->count == 0U || b->count == 0U)
        return 0;
    counter.a = a;
    counter.b = b;
    counter.a_index = xmalloc_array(a->count, sizeof(*counter.a_index));
    counter.b_index = xmalloc_array(b->count, sizeof(*counter.b_index));
    counter.seen_a =
        xcalloc_array((size_t)nsite, sizeof(*counter.seen_a));
    counter.seen_b =
        xcalloc_array((size_t)nsite, sizeof(*counter.seen_b));
    counter.candidate = xmalloc_array(
        checked_product((size_t)nsite, (size_t)max_depth),
        sizeof(*counter.candidate));
    counter.epoch = 0U;
    counter.nsite = nsite;
    counter.cutoff = cutoff;
    for (row = 0; row < a->count; ++row)
        counter.a_index[row] = row;
    for (row = 0; row < b->count; ++row)
        counter.b_index[row] = row;

    answer = disjoint_subtree(
        &counter, 0U, a->count, 0U, b->count, -1, 0);
    free(counter.candidate);
    free(counter.seen_b);
    free(counter.seen_a);
    free(counter.b_index);
    free(counter.a_index);
    return answer;
}

/* ------------------------------------------------------------------------- */
/* A001394 driver                                                            */
/* ------------------------------------------------------------------------- */

static i128 count_diamond_saws(int steps, size_t cutoff, Engine engine,
                               int verbose)
{
    Lattice lattice;
    int origin;
    int first_half;
    int second_half;
    int *representative;
    int *orbit_size;
    Bag *midpoint_bags;
    Bag template_bag;
    SiteId *template_sites;
    size_t template_site_count;
    int *midpoints;
    int midpoint_count = 0;
    int site;
    i128 total = 0;
    int thread_count = 1;

    if (steps == 0)
        return 1;
    if (steps < MIN_STEPS || steps > MAX_STEPS)
        die("internal step count out of range");
    if (cutoff == 0U)
        die("internal zero cutoff");

    first_half = (steps + 1) / 2;
    second_half = steps - first_half;
    lattice_build(&lattice, steps);
    origin = site_of(&lattice, 0, 0, 0);
    if (origin < 0)
        die("diamond-lattice origin is missing");

    representative =
        xcalloc_array((size_t)lattice.nsite, sizeof(*representative));
    orbit_size =
        xcalloc_array((size_t)lattice.nsite, sizeof(*orbit_size));
    build_orbits(&lattice, representative, orbit_size);

    midpoint_bags =
        xcalloc_array((size_t)lattice.nsite, sizeof(*midpoint_bags));
    for (site = 0; site < lattice.nsite; ++site)
        bag_init(&midpoint_bags[site], first_half);
    generate_midpoint_bags(
        &lattice, first_half, origin, representative, midpoint_bags);

    bag_init(&template_bag, second_half);
    generate_template_bag(&lattice, second_half, origin, &template_bag);
    bag_compress(&template_bag, lattice.nsite);
    template_sites = collect_bag_sites(
        &template_bag, lattice.nsite, &template_site_count);

    midpoints =
        xmalloc_array((size_t)lattice.nsite, sizeof(*midpoints));
    for (site = 0; site < lattice.nsite; ++site) {
        if (midpoint_bags[site].count != 0U) {
            if (representative[site] != site || orbit_size[site] <= 0)
                die("nonrepresentative midpoint bag is nonempty");
            bag_compress(&midpoint_bags[site], lattice.nsite);
            midpoints[midpoint_count++] = site;
        }
    }

#ifdef _OPENMP
    thread_count = omp_get_max_threads();
    if (thread_count < 1)
        die("invalid OpenMP thread count");
    if (thread_count > MAX_OMP_THREADS)
        thread_count = MAX_OMP_THREADS;
#endif
    if (verbose) {
        fprintf(stderr,
                "n=%d split=%d+%d sites=%d midpoint_orbits=%d "
                "template_sets=%zu template_sites=%zu threads=%d engine=%s\n",
                steps, first_half, second_half, lattice.nsite,
                midpoint_count, template_bag.count, template_site_count,
                thread_count,
                engine == ENGINE_WORD ? "word" : "bag");
    }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1) reduction(+:total) \
    num_threads(thread_count)
#endif
    for (site = 0; site < midpoint_count; ++site) {
        int midpoint = midpoints[site];
        i128 contribution;

        if (engine == ENGINE_WORD
            && midpoint_bags[midpoint].count <= template_bag.count) {
            SiteId *transport_map = build_transport_map(
                &lattice, midpoint, template_sites, template_site_count);

            contribution = count_disjoint_pairs_word_oriented(
                &midpoint_bags[midpoint], &template_bag,
                lattice.nsite, transport_map);
            free(transport_map);
        } else if (engine == ENGINE_WORD || engine == ENGINE_BAG) {
            Bag transported;

            transform_template_bag(
                &lattice, &template_bag, midpoint, &transported);
            if (engine == ENGINE_WORD) {
                contribution = count_disjoint_pairs_word(
                    &midpoint_bags[midpoint], &transported, lattice.nsite);
            } else {
                contribution = count_disjoint_pairs_bag(
                    &midpoint_bags[midpoint], &transported,
                    lattice.nsite, cutoff);
            }
            bag_free(&transported);
        } else {
            die("internal invalid disjointness engine");
        }
        total += (i128)orbit_size[midpoint] * contribution;
    }

    for (site = 0; site < lattice.nsite; ++site)
        bag_free(&midpoint_bags[site]);
    free(midpoints);
    free(template_sites);
    bag_free(&template_bag);
    free(midpoint_bags);
    free(orbit_size);
    free(representative);
    lattice_free(&lattice);

    if (total < 0 || (u128)total > (u128)UINT64_MAX)
        die("final A001394 count is outside uint64_t range");
    return total;
}

/*
 * Count one or two endpoint-x classes for one walk length in a single pass.
 *
 * The second-half template retains its relative endpoint x coordinate.
 * Under phi_X(v)=X+v or X-v, that endpoint becomes
 *
 *     X.x + transport_sign * relative_endpoint_x.
 *
 * Consequently the same disjointness index and projection cache serve both
 * requested endpoint classes.  Midpoint reduction uses only the four rooted
 * symmetries that preserve x exactly; using the full 24-element group here
 * would mix different endpoint conditions and would be incorrect.
 */
static void count_diamond_endpoint_x_targets(int steps,
                                             const int *target_x,
                                             size_t target_count,
                                             int verbose, i128 *answer)
{
    Lattice lattice;
    int origin;
    int first_half;
    int second_half;
    int *representative;
    int *orbit_size;
    Bag *midpoint_bags;
    Bag template_bag;
    SiteId *template_sites;
    size_t template_site_count;
    int *midpoints;
    int midpoint_count = 0;
    int site;
    i128 total0 = 0;
    i128 total1 = 0;
    int thread_count = 1;

    if (target_x == NULL || answer == NULL)
        die("internal null endpoint-count argument");
    if (target_count == 0U || target_count > 2U)
        die("internal endpoint target count must be one or two");
    if (target_count == 2U && target_x[0] == target_x[1])
        die("endpoint targets must be distinct");
    if (steps < 0 || steps > MAX_STEPS)
        die("internal endpoint-count step count out of range");

    first_half = (steps + 1) / 2;
    second_half = steps - first_half;
    lattice_build(&lattice, steps);
    origin = site_of(&lattice, 0, 0, 0);
    if (origin < 0)
        die("diamond-lattice origin is missing");

    representative =
        xcalloc_array((size_t)lattice.nsite, sizeof(*representative));
    orbit_size =
        xcalloc_array((size_t)lattice.nsite, sizeof(*orbit_size));
    build_x_preserving_orbits(&lattice, representative, orbit_size);

    midpoint_bags =
        xcalloc_array((size_t)lattice.nsite, sizeof(*midpoint_bags));
    for (site = 0; site < lattice.nsite; ++site)
        bag_init(&midpoint_bags[site], first_half);
    generate_midpoint_bags(
        &lattice, first_half, origin, representative, midpoint_bags);

    bag_init_endpoints(&template_bag, second_half);
    generate_template_bag(&lattice, second_half, origin, &template_bag);
    bag_compress(&template_bag, lattice.nsite);
    template_sites = collect_bag_sites(
        &template_bag, lattice.nsite, &template_site_count);

    midpoints =
        xmalloc_array((size_t)lattice.nsite, sizeof(*midpoints));
    for (site = 0; site < lattice.nsite; ++site) {
        if (midpoint_bags[site].count != 0U) {
            if (representative[site] != site || orbit_size[site] <= 0)
                die("nonrepresentative endpoint midpoint bag is nonempty");
            bag_compress(&midpoint_bags[site], lattice.nsite);
            midpoints[midpoint_count++] = site;
        }
    }

#ifdef _OPENMP
    thread_count = omp_get_max_threads();
    if (thread_count < 1)
        die("invalid OpenMP thread count");
    if (thread_count > MAX_OMP_THREADS)
        thread_count = MAX_OMP_THREADS;
#endif
    if (verbose) {
        if (target_count == 1U) {
            fprintf(stderr,
                    "endpoint-x steps=%d split=%d+%d target=%d sites=%d "
                    "midpoint_orbits=%d template_sets=%zu "
                    "template_sites=%zu threads=%d engine=word\n",
                    steps, first_half, second_half, target_x[0],
                    lattice.nsite, midpoint_count, template_bag.count,
                    template_site_count, thread_count);
        } else {
            fprintf(stderr,
                    "endpoint-x steps=%d split=%d+%d targets=%d,%d "
                    "sites=%d midpoint_orbits=%d template_sets=%zu "
                    "template_sites=%zu threads=%d engine=word\n",
                    steps, first_half, second_half, target_x[0],
                    target_x[1], lattice.nsite, midpoint_count,
                    template_bag.count, template_site_count, thread_count);
        }
    }

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1) reduction(+:total0,total1) \
    num_threads(thread_count)
#endif
    for (site = 0; site < midpoint_count; ++site) {
        int midpoint = midpoints[site];
        int kind = diamond_kind(
            lattice.x[midpoint], lattice.y[midpoint], lattice.z[midpoint]);
        int transport_sign;
        SiteId *transport_map;
        i128 contribution[2] = {0, 0};

        if (kind == NOT_A_DIAMOND_VERTEX)
            die("internal endpoint midpoint is not on the diamond lattice");
        transport_sign = kind == A_SUBLATTICE ? 1 : -1;
        transport_map = build_transport_map(
            &lattice, midpoint, template_sites, template_site_count);
        count_disjoint_pairs_word_endpoint_targets(
            &midpoint_bags[midpoint], &template_bag, lattice.nsite,
            transport_map, lattice.x[midpoint], transport_sign,
            target_x, target_count, contribution);
        free(transport_map);
        total0 += (i128)orbit_size[midpoint] * contribution[0];
        if (target_count == 2U)
            total1 += (i128)orbit_size[midpoint] * contribution[1];
    }

    for (site = 0; site < lattice.nsite; ++site)
        bag_free(&midpoint_bags[site]);
    free(midpoints);
    free(template_sites);
    bag_free(&template_bag);
    free(midpoint_bags);
    free(orbit_size);
    free(representative);
    lattice_free(&lattice);

    if (total0 < 0 || (u128)total0 > (u128)UINT64_MAX
        || (target_count == 2U
            && (total1 < 0 || (u128)total1 > (u128)UINT64_MAX)))
        die("endpoint-x count is outside uint64_t range");
    answer[0] = total0;
    if (target_count == 2U)
        answer[1] = total1;
}

static void count_diamond_endpoint_x_pair(int steps,
                                          const int target_x[2],
                                          int verbose, i128 answer[2])
{
    count_diamond_endpoint_x_targets(
        steps, target_x, 2U, verbose, answer);
}

static i128 count_diamond_endpoint_x_single(int steps, int target_x,
                                            int verbose)
{
    i128 answer;

    count_diamond_endpoint_x_targets(
        steps, &target_x, 1U, verbose, &answer);
    return answer;
}

/* ------------------------------------------------------------------------- */
/* Independent DFS and known values                                         */
/* ------------------------------------------------------------------------- */

static const uint64_t KNOWN_A001394[] = {
    UINT64_C(1), UINT64_C(4), UINT64_C(12), UINT64_C(36),
    UINT64_C(108), UINT64_C(324), UINT64_C(948), UINT64_C(2796),
    UINT64_C(8196), UINT64_C(24060), UINT64_C(70188), UINT64_C(205284),
    UINT64_C(597996), UINT64_C(1744548), UINT64_C(5073900),
    UINT64_C(14774652), UINT64_C(42922452), UINT64_C(124814484),
    UINT64_C(362267652), UINT64_C(1052271732), UINT64_C(3051900516),
    UINT64_C(8857050204), UINT64_C(25671988020), UINT64_C(74449697484),
    UINT64_C(215677847460), UINT64_C(625096195404),
    UINT64_C(1810062340812), UINT64_C(5243388472212)
};

/*
 * Related endpoint-x sequences from OEIS.  Only terms whose walks have at
 * most SELFTEST_DFS_STEPS steps are kept here, because these references are
 * checked by the deliberately independent brute-force implementation.
 */
static const uint64_t KNOWN_A001395[] = {
    UINT64_C(2), UINT64_C(10), UINT64_C(74),
    UINT64_C(518), UINT64_C(3934), UINT64_C(29914)
};

static const uint64_t KNOWN_A001396[] = {
    UINT64_C(1), UINT64_C(4), UINT64_C(28), UINT64_C(188),
    UINT64_C(1428), UINT64_C(10708), UINT64_C(82948)
};

static const uint64_t KNOWN_A001397[] = {
    UINT64_C(4), UINT64_C(24), UINT64_C(188),
    UINT64_C(1368), UINT64_C(10572), UINT64_C(81376)
};

static const uint64_t KNOWN_A001398[] = {
    UINT64_C(8), UINT64_C(56), UINT64_C(464),
    UINT64_C(3520), UINT64_C(27768)
};

#define SELFTEST_DFS_STEPS 12
#define SELFTEST_X_BINS (2 * SELFTEST_DFS_STEPS + 1)

typedef struct {
    const char *name;
    int step_multiplier;
    int step_addend;
    int target_x;
    int offset;
    const uint64_t *values;
    size_t value_count;
} EndpointXReference;

static const EndpointXReference ENDPOINT_X_REFERENCES[] = {
    {
        "A001395", 2, 1, 1, 0, KNOWN_A001395,
        sizeof KNOWN_A001395 / sizeof KNOWN_A001395[0]
    },
    {
        "A001396", 2, 0, 0, 0, KNOWN_A001396,
        sizeof KNOWN_A001396 / sizeof KNOWN_A001396[0]
    },
    {
        "A001397", 2, 0, 2, 1, KNOWN_A001397,
        sizeof KNOWN_A001397 / sizeof KNOWN_A001397[0]
    },
    {
        "A001398", 2, 1, 3, 1, KNOWN_A001398,
        sizeof KNOWN_A001398 / sizeof KNOWN_A001398[0]
    }
};

typedef struct {
    const Lattice *lattice;
    unsigned char *visited;
    i128 *x_counts;
    int steps;
    i128 count;
} BruteForce;

static void brute_force_recursively(BruteForce *brute, int current, int depth)
{
    int direction;

    if (depth == brute->steps) {
        int x_index =
            brute->lattice->x[current] + SELFTEST_DFS_STEPS;

        if (x_index < 0 || x_index >= SELFTEST_X_BINS)
            die("internal brute-force endpoint x is out of range");
        ++brute->count;
        ++brute->x_counts[x_index];
        return;
    }
    brute->visited[current] = 1U;
    for (direction = 0; direction < 4; ++direction) {
        int next = brute->lattice->neighbor[
            (size_t)current * 4U + (size_t)direction];

        if (next >= 0 && brute->visited[next] == 0U)
            brute_force_recursively(brute, next, depth + 1);
    }
    brute->visited[current] = 0U;
}

static i128 brute_force_count(int steps, i128 *x_counts)
{
    Lattice lattice;
    BruteForce brute;
    int origin;

    if (steps < 0 || steps > SELFTEST_DFS_STEPS)
        die("internal brute-force step count out of range");
    memset(x_counts, 0,
           checked_product((size_t)SELFTEST_X_BINS, sizeof(*x_counts)));
    if (steps == 0) {
        x_counts[SELFTEST_DFS_STEPS] = 1;
        return 1;
    }
    lattice_build(&lattice, steps);
    origin = site_of(&lattice, 0, 0, 0);
    brute.lattice = &lattice;
    brute.visited =
        xcalloc_array((size_t)lattice.nsite, sizeof(*brute.visited));
    brute.x_counts = x_counts;
    brute.steps = steps;
    brute.count = 0;
    brute_force_recursively(&brute, origin, 0);
    free(brute.visited);
    lattice_free(&lattice);
    return brute.count;
}

static int selftest(size_t cutoff, Engine engine)
{
    i128 fast_values[21] = {0};
    i128 brute_x[SELFTEST_DFS_STEPS + 1][SELFTEST_X_BINS] = {{0}};
    int failed = 0;
    int steps;

    printf("[1] A001394 known values\n");
    for (steps = 0; steps <= 20; ++steps) {
        i128 fast = count_diamond_saws(steps, cutoff, engine, 0);
        int ok = fast == (i128)KNOWN_A001394[steps];

        fast_values[steps] = fast;
        printf("    n=%2d  ", steps);
        print_i128(fast);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok)
            failed = 1;
    }

    printf("[2] independent DFS through n=%d\n", SELFTEST_DFS_STEPS);
    for (steps = 0; steps <= SELFTEST_DFS_STEPS; ++steps) {
        i128 slow = brute_force_count(steps, brute_x[steps]);
        int ok = fast_values[steps] == slow;

        printf("    n=%2d  ", steps);
        print_i128(slow);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok)
            failed = 1;
    }

    printf("[3] related endpoint-x sequences from independent DFS\n");
    {
        size_t reference_index;

        for (reference_index = 0;
             reference_index
                 < sizeof ENDPOINT_X_REFERENCES
                   / sizeof ENDPOINT_X_REFERENCES[0];
             ++reference_index) {
            const EndpointXReference *reference =
                &ENDPOINT_X_REFERENCES[reference_index];
            size_t term;

            printf("    %s (endpoint x=%d)\n",
                   reference->name, reference->target_x);
            for (term = 0; term < reference->value_count; ++term) {
                int sequence_index = reference->offset + (int)term;
                int walk_steps =
                    reference->step_multiplier * sequence_index
                    + reference->step_addend;
                int x_index =
                    reference->target_x + SELFTEST_DFS_STEPS;
                i128 value = brute_x[walk_steps][x_index];
                int ok = value == (i128)reference->values[term];

                printf("        n=%2d steps=%2d  ",
                       sequence_index, walk_steps);
                print_i128(value);
                printf("  %s\n", ok ? "ok" : "MISMATCH");
                if (!ok)
                    failed = 1;
            }
        }
    }

    printf("[4] bag-engine cutoff invariance at n=12\n");
    {
        static const size_t cutoffs[] = {1U, 32U, 4096U, 1000000U};
        i128 reference = 0;
        size_t index;

        for (index = 0; index < sizeof cutoffs / sizeof cutoffs[0]; ++index) {
            i128 value =
                count_diamond_saws(12, cutoffs[index], ENGINE_BAG, 0);
            int ok;

            if (index == 0U)
                reference = value;
            ok = value == reference;
            printf("    cutoff=%-8zu ", cutoffs[index]);
            print_i128(value);
            printf("  %s\n", ok ? "ok" : "MISMATCH");
            if (!ok)
                failed = 1;
        }
    }
    printf("[5] independent word/bag engine agreement through n=14\n");
    for (steps = 0; steps <= 14; ++steps) {
        i128 word = count_diamond_saws(steps, cutoff, ENGINE_WORD, 0);
        i128 bag = count_diamond_saws(steps, cutoff, ENGINE_BAG, 0);
        int ok = word == bag;

        printf("    n=%2d  ", steps);
        print_i128(word);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok)
            failed = 1;
    }
    printf("[6] batched endpoint-x engine against known values\n");
    {
        int sequence_index;

        for (sequence_index = 1; sequence_index <= 6; ++sequence_index) {
            static const int even_targets[2] = {0, 2};
            i128 even[2];
            int ok396;
            int ok397;

            count_diamond_endpoint_x_pair(
                2 * sequence_index, even_targets, 0, even);
            ok396 =
                even[0] == (i128)KNOWN_A001396[sequence_index];
            ok397 =
                even[1] == (i128)KNOWN_A001397[sequence_index - 1];
            printf("    A001396(%d) ", sequence_index);
            print_i128(even[0]);
            printf("  %s; A001397(%d) ", ok396 ? "ok" : "MISMATCH",
                   sequence_index);
            print_i128(even[1]);
            printf("  %s\n", ok397 ? "ok" : "MISMATCH");
            if (!ok396 || !ok397)
                failed = 1;

            if (sequence_index <= 5) {
                static const int odd_targets[2] = {1, 3};
                i128 odd[2];
                int ok395;
                int ok398;

                count_diamond_endpoint_x_pair(
                    2 * sequence_index + 1, odd_targets, 0, odd);
                ok395 =
                    odd[0] == (i128)KNOWN_A001395[sequence_index];
                ok398 =
                    odd[1] == (i128)KNOWN_A001398[sequence_index - 1];
                printf("    A001395(%d) ", sequence_index);
                print_i128(odd[0]);
                printf("  %s; A001398(%d) ",
                       ok395 ? "ok" : "MISMATCH", sequence_index);
                print_i128(odd[1]);
                printf("  %s\n", ok398 ? "ok" : "MISMATCH");
                if (!ok395 || !ok398)
                    failed = 1;
            }
        }
        {
            i128 single =
                count_diamond_endpoint_x_single(10, 0, 0);
            int ok = single == (i128)KNOWN_A001396[5];

            printf("    single-target A001396(5) ");
            print_i128(single);
            printf("  %s\n", ok ? "ok" : "MISMATCH");
            if (!ok)
                failed = 1;
        }
    }
    printf("%s\n", failed ? "SELFTEST FAILED" : "selftest passed");
    return failed;
}

/* ------------------------------------------------------------------------- */
/* Command line                                                              */
/* ------------------------------------------------------------------------- */

typedef struct {
    const char *name;
    int step_addend;
    int target_x;
    int minimum_index;
    int maximum_index;
} EndpointRequest;

static const EndpointRequest ENDPOINT_REQUESTS[] = {
    {"A001395", 1, 1, 0, (MAX_STEPS - 1) / 2},
    {"A001396", 0, 0, 0, MAX_STEPS / 2},
    {"A001397", 0, 2, 1, MAX_STEPS / 2},
    {"A001398", 1, 3, 1, (MAX_STEPS - 1) / 2}
};

static int parse_steps(const char *text)
{
    char *end = NULL;
    long value;

    if (text == NULL || *text == '\0')
        die("missing step count");
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0')
        die("step count must be an integer");
    if (value < MIN_STEPS || value > MAX_STEPS)
        die("step count must be an integer from 0 to 40");
    return (int)value;
}

static int parse_endpoint_index(const char *text)
{
    char *end = NULL;
    long value;

    if (text == NULL || *text == '\0')
        die("missing endpoint sequence index");
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0')
        die("endpoint sequence index must be an integer");
    if (value < 0 || value > MAX_STEPS / 2)
        die("endpoint sequence index must be an integer from 0 to 20");
    return (int)value;
}

static const EndpointRequest *parse_endpoint_request(const char *text)
{
    size_t index;

    if (text == NULL || *text == '\0')
        die("missing endpoint sequence number");
    for (index = 0U;
         index < sizeof ENDPOINT_REQUESTS / sizeof ENDPOINT_REQUESTS[0];
         ++index) {
        const EndpointRequest *request = &ENDPOINT_REQUESTS[index];

        if (strcmp(text, request->name) == 0
            || strcmp(text, request->name + 1) == 0)
            return request;
    }
    die("endpoint sequence must be 001395, 001396, 001397, or 001398");
    return NULL;
}

static size_t parse_cutoff(const char *text)
{
    char *end = NULL;
    uintmax_t value;

    if (text == NULL || *text == '\0' || *text == '-')
        die("--cutoff must be a positive integer");
    errno = 0;
    value = strtoumax(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0'
        || value == 0U || value > (uintmax_t)SIZE_MAX)
        die("--cutoff must be a positive integer");
    return (size_t)value;
}

static Engine parse_engine(const char *text)
{
    if (text != NULL && strcmp(text, "word") == 0)
        return ENGINE_WORD;
    if (text != NULL && strcmp(text, "bag") == 0)
        return ENGINE_BAG;
    die("--engine must be word or bag");
    return ENGINE_WORD;
}

static void print_term(int steps, size_t cutoff, Engine engine)
{
    int verbose = getenv("A001394_VERBOSE") != NULL;
    i128 value = count_diamond_saws(steps, cutoff, engine, verbose);

    printf("%d ", steps);
    print_i128(value);
    putchar('\n');
    fflush(stdout);
}

static void print_endpoint_terms(int sequence_index,
                                 const EndpointRequest *request)
{
    static const int odd_targets[2] = {1, 3};
    static const int even_targets[2] = {0, 2};
    int verbose = getenv("A001394_VERBOSE") != NULL;
    i128 odd[2];
    i128 even[2];

    if (request != NULL) {
        int steps;
        i128 value;

        if (sequence_index < request->minimum_index
            || sequence_index > request->maximum_index)
            die("index is outside the supported range for that sequence");
        steps = 2 * sequence_index + request->step_addend;
        value = count_diamond_endpoint_x_single(
            steps, request->target_x, verbose);
        printf("%s(%d) ", request->name, sequence_index);
        print_i128(value);
        putchar('\n');
        fflush(stdout);
        return;
    }
    /*
     * A001397 and A001398 have offset 1, and the odd-length requests must
     * remain within MAX_STEPS.  Hence 1..19 is the common four-series range.
     */
    if (sequence_index < 1
        || sequence_index > (MAX_STEPS - 1) / 2)
        die("--endpoints without a sequence requires an index from 1 to 19");
    count_diamond_endpoint_x_pair(
        2 * sequence_index + 1, odd_targets, verbose, odd);
    count_diamond_endpoint_x_pair(
        2 * sequence_index, even_targets, verbose, even);

    printf("A001395(%d) ", sequence_index);
    print_i128(odd[0]);
    printf("\nA001396(%d) ", sequence_index);
    print_i128(even[0]);
    printf("\nA001397(%d) ", sequence_index);
    print_i128(even[1]);
    printf("\nA001398(%d) ", sequence_index);
    print_i128(odd[1]);
    putchar('\n');
    fflush(stdout);
}

static void usage(const char *program)
{
    fprintf(stderr,
            "usage: %s [--engine word|bag] [--cutoff K] n\n"
            "       %s [--engine word|bag] [--cutoff K] --upto N\n"
            "       %s [--engine word] --endpoints N "
            "[001395|001396|001397|001398]\n"
            "       %s [--engine word|bag] [--cutoff K] --selftest\n"
            "four-series range: 1 <= endpoint N <= 19\n"
            "single-series ranges: A001395 0..19, A001396 0..20,\n"
            "                      A001397 1..20, A001398 1..19\n",
            program, program, program, program);
}

int main(int argc, char **argv)
{
    size_t cutoff = DEFAULT_CUTOFF;
    Engine engine = ENGINE_WORD;
    int argument = 1;

    while (argument < argc) {
        if (strcmp(argv[argument], "--cutoff") == 0) {
            if (argument + 1 >= argc)
                die("--cutoff requires an argument");
            cutoff = parse_cutoff(argv[argument + 1]);
            argument += 2;
        } else if (strcmp(argv[argument], "--engine") == 0) {
            if (argument + 1 >= argc)
                die("--engine requires an argument");
            engine = parse_engine(argv[argument + 1]);
            argument += 2;
        } else {
            break;
        }
    }
    if (argument < argc && strcmp(argv[argument], "--selftest") == 0) {
        if (argument + 1 != argc) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        return selftest(cutoff, engine) ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    if (argument < argc && strcmp(argv[argument], "--upto") == 0) {
        int limit;
        int steps;

        if (argument + 2 != argc) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        limit = parse_steps(argv[argument + 1]);
        for (steps = MIN_STEPS; steps <= limit; ++steps)
            print_term(steps, cutoff, engine);
        return EXIT_SUCCESS;
    }
    if (argument < argc && strcmp(argv[argument], "--endpoints") == 0) {
        int sequence_index;
        const EndpointRequest *request = NULL;

        if (argument + 2 != argc && argument + 3 != argc) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        if (engine != ENGINE_WORD)
            die("--endpoints requires --engine word");
        sequence_index = parse_endpoint_index(argv[argument + 1]);
        if (argument + 3 == argc)
            request = parse_endpoint_request(argv[argument + 2]);
        print_endpoint_terms(sequence_index, request);
        return EXIT_SUCCESS;
    }
    if (argument + 1 == argc) {
        print_term(parse_steps(argv[argument]), cutoff, engine);
        return EXIT_SUCCESS;
    }
    usage(argv[0]);
    return EXIT_FAILURE;
}
