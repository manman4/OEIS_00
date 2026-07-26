/*
 * 079156_02.c -- Independent exact enumeration of OEIS A079156,
 *                together with the corresponding A078717 values.
 *
 * Output columns:
 *
 *     n C_n M_n
 *
 * C_n is the number of n-step self-avoiding walks on the simple cubic
 * lattice with the first step specified (OEIS A078717). M_n is the sum of
 * their end-to-end Manhattan distances (OEIS A079156).
 *
 * METHOD
 * ------
 * This is an independent meet-in-the-middle implementation based on the
 * MIT-licensed 001337_08.c by manman4. That program was independently
 * derived from the published mathematical length-doubling idea and contains
 * no SAWdoubler source code. The all-endpoint weighted-bag organization was
 * first used in the likewise MIT-licensed 398322_01.c. This adaptation uses
 * no SAWdoubler source code.
 *
 * Split an n-step walk at a join point X:
 *
 *     O --N steps--> X --M steps--> Y,       N + M = n.
 *
 * Bag A contains every N-step self-avoiding half-walk O -> X. A read-only
 * template bag B contains every M-step self-avoiding continuation from a
 * relative origin to every endpoint. B is generated and compressed only
 * once, then translated to each X through a checked monotone site-id map.
 * The join X is omitted from both stored vertex sets. The origin O is omitted
 * from A, and B rows whose translation touches O are rejected. Therefore a
 * pair forms one full self-avoiding walk exactly when its two stored sets are
 * disjoint.
 *
 * Inclusion-exclusion counts disjoint pairs. Continuations having the same
 * visited vertex set share one compressed B row because their intersection
 * behavior is identical. That row retains an exact endpoint/multiplicity
 * table. Translation by X gives its Manhattan weight exactly as
 *
 *     sum_d multiplicity[d] *
 *           (|X_x+d_x| + |X_y+d_y| + |X_z+d_z|).
 *
 * The same recursion returns both the number of disjoint pairs and their
 * Manhattan-distance sum. Since the unrestricted B bag is the larger side,
 * the uniform split rule moves two steps from B to the fixed-join A side
 * whenever both halves remain nonempty. This changes only the decomposition
 * point, not the walks being counted.
 *
 * The 48 signed coordinate permutations fixing O preserve the simple cubic
 * lattice, self-avoidance, and Manhattan distance. Join points are reduced
 * to orbits under this group; one representative is evaluated and multiplied
 * by its exact orbit size. For each representative, A rows are additionally
 * canonicalized under the subgroup fixing that join. This is exact because
 * the complete B collection and its Manhattan weight are invariant under the
 * same subgroup. OpenMP, when enabled, distributes independent join-point
 * representatives.
 *
 * Only the Manhattan ball reachable in n steps is numbered. Up to n=36 it
 * has at most 64897 sites, so half-walk rows use checked 16-bit site IDs.
 * This reduces both the row traffic and the per-worker site tables without
 * changing the graph explored.
 *
 * The branch cut-off changes only whether an inclusion-exclusion subtree is
 * expanded or evaluated directly. It cannot change the mathematical result;
 * the self-test checks this invariance and also compares with a plain DFS.
 *
 * Based on:
 *   001337_08.c and 398322_01.c, independently written/adapted under MIT.
 *
 * Mathematical reference:
 *   R. D. Schram, G. T. Barkema and R. H. Bisseling,
 *   "Exact enumeration of self-avoiding walks",
 *   J. Stat. Mech. (2011) P06019.
 *
 * Copyright (c) 2026 manman4
 * A079156 adaptation by OpenAI Codex, 2026.
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Build:
 *   clang -O3 -std=c11 -Wall -Wextra -pedantic 079156_02.c -o 079156_02
 *   gcc-omp 079156_02.c -o 079156_02
 *
 * Usage:
 *   ./079156_02 N
 *   ./079156_02 --upto N
 *   ./079156_02 selftest
 *
 * The proved representation range is 2 <= N <= 36. This is an arithmetic
 * and array-safety limit, not the end of the currently known OEIS data.
 * Values well below 36 can already be impractical in time or memory.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>
#include <errno.h>
#include <inttypes.h>

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

#define MIN_INDEX 2
#define MAX_INDEX 36
#define SPLIT_BIAS 2
#define MAX_FIXED_HALF_STEPS (((MAX_INDEX + 1) / 2) + SPLIT_BIAS)
#define MAX_BAG_WIDTH MAX_FIXED_HALF_STEPS
#define MAX_LATTICE_RADIUS MAX_INDEX
#define MAX_REACHABLE_SITES 64897
#define DEFAULT_CUTOFF 32U
#define HALF_WALK_LIMIT UINT64_C(114440917968750) /* 6 * 5^19 */

_Static_assert(SIZE_MAX >= HALF_WALK_LIMIT,
               "079156_02 requires a sufficiently wide size_t");
_Static_assert(SPLIT_BIAS >= 0, "SPLIT_BIAS must not be negative");
_Static_assert(MAX_REACHABLE_SITES <= UINT16_MAX,
               "reachable sites must fit uint16_t");

/*
 * Integer bounds for 2 <= n <= 36:
 * - the uniformly biased split has fixed-join length at most 20 and
 *   unrestricted continuation length at most 16;
 * - their nonbacktracking bounds are 6*5^19 and 6*5^15;
 * - each multiplicity fits uint64_t and every continuation endpoint
 *   coordinate fits int64_t;
 * - translated continuation weights and disjoint-pair subtree values use
 *   signed 128-bit arithmetic, with worst-case subtree bound
 *   36*(6*5^19)*(6*5^15) < 2^127;
 * - the unrestricted count is at most 6*5^35;
 * - its Manhattan-distance sum is at most 36*6*5^35 < 2^127.
 * - the radius-36 Manhattan ball has 64897 sites, fitting uint16_t site
 *   identifiers; coordinates fit int16_t.
 * Signed 128-bit intermediate and output arithmetic therefore has ample
 * headroom. Allocation products are checked independently.
 */

static void die(const char *what)
{
    fprintf(stderr, "079156_02: %s\n", what);
    exit(EXIT_FAILURE);
}

static size_t checked_product(size_t a, size_t b)
{
    if (a != 0U && b > SIZE_MAX / a) die("allocation size overflow");
    return a * b;
}

static void *xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (!p) die("out of memory");
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
    p = calloc(count ? count : 1U, size ? size : 1U);
    if (!p) die("out of memory");
    return p;
}

static void *xrealloc_array(void *old, size_t count, size_t size)
{
    size_t bytes = checked_product(count, size);
    void *p = realloc(old, bytes ? bytes : 1U);
    if (!p) die("out of memory while collecting half-walks");
    return p;
}

/* ------------------------------------------------------------------ */
/* 128-bit decimal output                                             */
/* ------------------------------------------------------------------ */

static void fmt_i128(char *out, i128 v)
{
    char tmp[48];
    int p = 0, neg = 0, i;
    u128 magnitude;

    if (v < 0) {
        neg = 1;
        magnitude = (u128)(-(v + 1)) + 1U;
    } else {
        magnitude = (u128)v;
    }
    if (magnitude == 0U) tmp[p++] = '0';
    while (magnitude > 0U) {
        tmp[p++] = (char)('0' + (int)(magnitude % 10U));
        magnitude /= 10U;
    }
    i = 0;
    if (neg) out[i++] = '-';
    while (p > 0) out[i++] = tmp[--p];
    out[i] = '\0';
}

static void print_i128(i128 v)
{
    char buf[48];
    fmt_i128(buf, v);
    fputs(buf, stdout);
}

/* ------------------------------------------------------------------ */
/* Simple cubic lattice inside its reachable Manhattan ball           */
/* ------------------------------------------------------------------ */

#define DEGREE 6

/* The six axis-aligned nearest neighbours. */
static const int STEP[DEGREE][3] = {
    { 1, 0, 0 }, { -1, 0, 0 },
    { 0, 1, 0 }, { 0, -1, 0 },
    { 0, 0, 1 }, { 0, 0, -1 }
};

typedef struct {
    int      radius;    /* coordinates run over [-radius, radius] */
    int      side;      /* 2*radius + 1                           */
    long     cells;     /* side^3                                 */
    int      nsite;     /* sites in the reachable Manhattan ball  */
    int     *cell2id;   /* box cell -> dense site id, or -1        */
    int16_t *px, *py, *pz;
    int32_t *adj;       /* nsite * DEGREE -> site id, or -1       */
} Lattice;

static long cell_of(const Lattice *g, int x, int y, int z)
{
    return ((long)(x + g->radius) * g->side + (y + g->radius)) * g->side
           + (z + g->radius);
}

static int site_of(const Lattice *g, int x, int y, int z)
{
    if (x < -g->radius || x > g->radius) return -1;
    if (y < -g->radius || y > g->radius) return -1;
    if (z < -g->radius || z > g->radius) return -1;
    return g->cell2id[cell_of(g, x, y, z)];
}

static int is_reachable_site(int x, int y, int z, int radius)
{
    return abs(x) + abs(y) + abs(z) <= radius;
}

static void lattice_build(Lattice *g, int radius)
{
    long cell;
    int x, y, z, n, d;

    if (radius < 0 || radius > MAX_LATTICE_RADIUS)
        die("internal lattice radius out of range");

    g->radius = radius;
    g->side   = 2 * radius + 1;
    g->cells  = (long)g->side * g->side * g->side;

    if (g->cells <= 0) die("internal lattice contains no sites");
    g->cell2id = xmalloc_array((size_t)g->cells, sizeof(*g->cell2id));
    for (cell = 0; cell < g->cells; cell++) g->cell2id[cell] = -1;

    n = 0;
    for (x = -radius; x <= radius; x++)
        for (y = -radius; y <= radius; y++)
            for (z = -radius; z <= radius; z++)
                if (is_reachable_site(x, y, z, radius)) n++;
    if (n <= 0) die("internal lattice contains no sites");
    if (n > (int)UINT16_MAX)
        die("reachable site count does not fit uint16_t");
    g->nsite = n;
    g->px = xmalloc_array((size_t)g->nsite, sizeof(*g->px));
    g->py = xmalloc_array((size_t)g->nsite, sizeof(*g->py));
    g->pz = xmalloc_array((size_t)g->nsite, sizeof(*g->pz));

    n = 0;
    for (x = -radius; x <= radius; x++)
        for (y = -radius; y <= radius; y++)
            for (z = -radius; z <= radius; z++)
                if (is_reachable_site(x, y, z, radius)) {
                    g->cell2id[cell_of(g, x, y, z)] = n;
                    g->px[n] = (int16_t)x;
                    g->py[n] = (int16_t)y;
                    g->pz[n] = (int16_t)z;
                    n++;
                }
    if (n != g->nsite)
        die("internal lattice enumeration count mismatch");

    g->adj = xmalloc_array(
        checked_product((size_t)g->nsite, DEGREE), sizeof(*g->adj));
    for (n = 0; n < g->nsite; n++)
        for (d = 0; d < DEGREE; d++)
            g->adj[(long)n * DEGREE + d] =
                site_of(g, g->px[n] + STEP[d][0],
                           g->py[n] + STEP[d][1],
                           g->pz[n] + STEP[d][2]);
}

static void lattice_free(Lattice *g)
{
    free(g->cell2id); free(g->px); free(g->py); free(g->pz); free(g->adj);
}

/* ------------------------------------------------------------------ */
/* Bag of half-walks, each stored as its sorted vertex set             */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t *row;          /* count * width entries, ascending rows */
    uint64_t *mult;         /* row multiplicity after compression    */
    uint16_t *endpoint;     /* endpoint variants of tracked rows     */
    uint64_t *endpoint_mult;
    size_t   *endpoint_offset; /* count+1 offsets into endpoint[]    */
    size_t    endpoint_count;
    size_t    count;
    size_t    cap;
    int       width;
    int       track_endpoint;
} Bag;

static void bag_init(Bag *b, int width, int track_endpoint)
{
    if (width < 0 || width > MAX_BAG_WIDTH)
        die("internal half-walk width out of range");
    b->row = NULL; b->mult = NULL; b->endpoint = NULL;
    b->endpoint_mult = NULL; b->endpoint_offset = NULL;
    b->endpoint_count = 0U;
    b->count = 0; b->cap = 0; b->width = width;
    b->track_endpoint = track_endpoint;
}

static void bag_free(Bag *b)
{
    free(b->row); free(b->mult); free(b->endpoint);
    free(b->endpoint_mult); free(b->endpoint_offset);
}

static void bag_add(Bag *b, const int *path, int endpoint)
{
    uint16_t *r;
    int i, j;

    if ((!b->track_endpoint && endpoint != -1) ||
        (b->track_endpoint &&
         endpoint < 0))
        die("internal endpoint tracking mismatch");

    if ((uint64_t)b->count >= HALF_WALK_LIMIT)
        die("half-walk count exceeded its proved bound");

    if (b->width == 0) {
        if (b->track_endpoint)
            die("internal tracked endpoint has a width-zero row");
        b->count++;
        return;
    }

    if (b->count == b->cap) {
        size_t cap;
        uint16_t *grown;
        uint16_t *grown_endpoint;
        if (b->cap == 0U) {
            cap = 4096U;
        } else {
            if (b->cap > SIZE_MAX / 2U) die("half-walk capacity overflow");
            cap = b->cap * 2U;
        }
        if ((uint64_t)cap > HALF_WALK_LIMIT)
            cap = (size_t)HALF_WALK_LIMIT;
        grown = xrealloc_array(
            b->row,
            checked_product(cap, (size_t)b->width),
            sizeof(*b->row));
        grown_endpoint = b->endpoint;
        if (b->track_endpoint) {
            grown_endpoint = xrealloc_array(
                b->endpoint, cap, sizeof(*b->endpoint));
        }
        b->row = grown;
        b->endpoint = grown_endpoint;
        b->cap = cap;
    }
    r = b->row + b->count * (size_t)b->width;
    for (i = 0; i < b->width; i++) {
        if (path[i] < 0 || path[i] > (int)UINT16_MAX)
            die("half-walk site id does not fit uint16_t");
        r[i] = (uint16_t)path[i];
    }
    for (i = 1; i < b->width; i++) { /* insertion sort, short fixed-width row */
        uint16_t v = r[i];
        j = i - 1;
        while (j >= 0 && r[j] > v) { r[j + 1] = r[j]; j--; }
        r[j + 1] = v;
    }
    if (b->track_endpoint) {
        if (endpoint > (int)UINT16_MAX)
            die("continuation endpoint does not fit uint16_t");
        b->endpoint[b->count] = (uint16_t)endpoint;
    }
    b->count++;
}

/*
 * Sort rows lexicographically (LSD radix over the columns) and merge equal
 * vertex sets.  A tracked row retains a compact list of endpoint variants and
 * their multiplicities.  Endpoint is deliberately not part of the row key:
 * intersection tests depend only on the visited set, while the endpoint
 * variants are used later to form that row's exact Manhattan-weight sum.
 */
static void bag_compress(Bag *b, int nsite)
{
    size_t n = b->count, i, m, vcount = 0U;
    int w = b->width, col, s;
    size_t *ord, *scratch;
    size_t *bucket;
    uint16_t *out, *endpoint = NULL;
    uint64_t *mul, *endpoint_mult = NULL;
    size_t *endpoint_offset = NULL;

    if (n == 0) {
        b->mult = NULL;
        b->endpoint = NULL;
        b->endpoint_mult = NULL;
        b->endpoint_offset = NULL;
        b->endpoint_count = 0U;
        return;
    }
    if (w == 0) {                                /* one empty set, n copies */
        b->mult = xmalloc_array(1U, sizeof(*b->mult));
        b->mult[0] = (uint64_t)n;
        if (b->track_endpoint)
            die("internal tracked endpoint has a width-zero row");
        b->count = 1U;
        return;
    }

    ord     = xmalloc_array(n, sizeof(*ord));
    scratch = xmalloc_array(n, sizeof(*scratch));
    bucket  = xcalloc_array((size_t)nsite + 1U, sizeof(*bucket));

    for (i = 0; i < n; i++) ord[i] = i;

    /* Stable secondary-key sort by endpoint before the row-key radix passes. */
    if (b->track_endpoint) {
        for (i = 0; i < n; i++) {
            if ((int)b->endpoint[ord[i]] >= nsite)
                die("continuation endpoint is outside its lattice");
            bucket[(size_t)b->endpoint[ord[i]] + 1U]++;
        }
        for (s = 0; s < nsite; s++) bucket[s + 1] += bucket[s];
        for (i = 0; i < n; i++)
            scratch[bucket[b->endpoint[ord[i]]]++] = ord[i];
        memcpy(ord, scratch, checked_product(n, sizeof(*ord)));
    }

    for (col = w - 1; col >= 0; col--) {
        memset(bucket, 0,
               checked_product((size_t)nsite + 1U, sizeof(*bucket)));
        for (i = 0; i < n; i++) {
            uint16_t site =
                b->row[ord[i] * (size_t)w + (size_t)col];
            if ((int)site >= nsite)
                die("half-walk site is outside its lattice");
            bucket[(size_t)site + 1U]++;
        }
        for (s = 0; s < nsite; s++) bucket[s + 1] += bucket[s];
        for (i = 0; i < n; i++)
            scratch[bucket[b->row[
                ord[i] * (size_t)w + (size_t)col]]++] = ord[i];
        memcpy(ord, scratch, checked_product(n, sizeof(*ord)));
    }

    out = xmalloc_array(
        checked_product(n, (size_t)w), sizeof(*out));
    mul = xmalloc_array(n, sizeof(*mul));
    if (b->track_endpoint) {
        endpoint = xmalloc_array(n, sizeof(*endpoint));
        endpoint_mult = xmalloc_array(n, sizeof(*endpoint_mult));
        endpoint_offset = xmalloc_array(n + 1U, sizeof(*endpoint_offset));
    }
    m = 0;
    for (i = 0; i < n; i++) {
        const uint16_t *r = b->row + ord[i] * (size_t)w;
        if (m > 0U &&
            memcmp(out + (m - 1U) * (size_t)w, r,
                   checked_product((size_t)w, sizeof(*out))) == 0) {
            if (mul[m - 1U] == UINT64_MAX)
                die("half-walk multiplicity overflow");
            mul[m - 1U]++;
            if (b->track_endpoint) {
                uint16_t ep = b->endpoint[ord[i]];
                if (vcount > endpoint_offset[m - 1U] &&
                    endpoint[vcount - 1U] == ep) {
                    if (endpoint_mult[vcount - 1U] == UINT64_MAX)
                        die("endpoint multiplicity overflow");
                    endpoint_mult[vcount - 1U]++;
                } else {
                    endpoint[vcount] = ep;
                    endpoint_mult[vcount] = 1U;
                    vcount++;
                }
            }
        } else {
            memcpy(out + m * (size_t)w, r,
                   checked_product((size_t)w, sizeof(*out)));
            mul[m] = 1U;
            if (b->track_endpoint) {
                endpoint_offset[m] = vcount;
                endpoint[vcount] = b->endpoint[ord[i]];
                endpoint_mult[vcount] = 1U;
                vcount++;
            }
            m++;
        }
    }
    if (b->track_endpoint) endpoint_offset[m] = vcount;

    free(ord); free(scratch); free(bucket);
    free(b->row); free(b->endpoint);
    b->row = out;
    b->mult = mul;
    b->endpoint = endpoint;
    b->endpoint_mult = endpoint_mult;
    b->endpoint_offset = endpoint_offset;
    b->endpoint_count = vcount;
    b->count = m;
    b->cap = m;
}

/* ------------------------------------------------------------------ */
/* Half-walk generation                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    const Lattice *g;
    unsigned char *seen;
    int            steps;  /* walk length                      */
    int           *path;   /* path[0..steps]                   */
    int            banned; /* the opposite endpoint; see below */
    int            join_x, join_y, join_z;
    Bag           *bag;
} Walker;

static void walk_rec(Walker *w, int depth)
{
    int here = w->path[depth];
    int left = w->steps - depth;
    const int32_t *nb;
    int d;

    /*
     * Simple-cubic graph distance is exactly Manhattan distance.  This is
     * the same pruning test as a BFS distance table, evaluated without
     * constructing a full-box table for every join point.
     */
    if (abs((int)w->g->px[here] - w->join_x)
        + abs((int)w->g->py[here] - w->join_y)
        + abs((int)w->g->pz[here] - w->join_z) > left)
        return;

    if (depth == w->steps) {
        /* Distance is 0 here, so this vertex is the join point. Neither it nor
           path[0] is stored: path[0] is the walk's own start point, which by
           construction lies on every row of this bag and on no row of the
           other one (walks through the opposite endpoint were rejected
           below), so it can never be a shared site.  Dropping both keeps the
           row width at steps-1. */
        bag_add(w->bag, w->path + 1, -1);
        return;
    }

    w->seen[here] = 1;
    nb = w->g->adj + (long)here * DEGREE;
    for (d = 0; d < DEGREE; d++) {
        int next = nb[d];
        /* A walk that touches the opposite endpoint would meet every row of
           the other bag there, so no pair containing it can be disjoint. */
        if (next >= 0 && next != w->banned && !w->seen[next]) {
            w->path[depth + 1] = next;
            walk_rec(w, depth + 1);
        }
    }
    w->seen[here] = 0;
}

typedef struct {
    const Lattice *g;
    unsigned char *seen;
    int            steps;
    int           *path;
    int            banned;
    Bag           *bag;
} ContinuationWalker;

/* Generate every self-avoiding continuation from the join point. The stored
   row contains path[1..steps], including the final endpoint. */
static void continuation_rec(ContinuationWalker *w, int depth)
{
    int here = w->path[depth];
    const int32_t *nb;
    int d;

    if (depth == w->steps) {
        bag_add(w->bag, w->path + 1, here);
        return;
    }

    w->seen[here] = 1;
    nb = w->g->adj + (long)here * DEGREE;
    for (d = 0; d < DEGREE; d++) {
        int next = nb[d];
        if (next >= 0 && next != w->banned && !w->seen[next]) {
            w->path[depth + 1] = next;
            continuation_rec(w, depth + 1);
        }
    }
    w->seen[here] = 0;
}

/* ------------------------------------------------------------------ */
/* Disjoint-pair counting                                              */
/* ------------------------------------------------------------------ */

typedef struct {
    const Bag *A, *B;
    size_t    *aidx, *bidx;    /* index lists, permuted in place */
    const int *b_site_map;      /* relative B site -> translated global site */
    const i128 *b_weight;       /* translated endpoint weight per B row */
    uint64_t  *seenA, *seenB;  /* epoch stamps over the sites    */
    uint16_t  *cand;           /* per-depth candidate buffers    */
    uint64_t   epoch;
    int        nsite;
    size_t     cutoff;         /* switch to direct counting here */
} Counter;

typedef struct {
    i128 z;
    i128 m;
} PairSum;

static const uint16_t EMPTY_ROW[1] = { 0U };

static const uint16_t *bag_row_at(const Bag *b, size_t index)
{
    if (b->width == 0) return EMPTY_ROW;
    return b->row + index * (size_t)b->width;
}

/* Do a and b share an element strictly greater than "last"? */
static int shares_above(const uint16_t *a, int ka,
                        const uint16_t *b, int kb, int last,
                        const int *b_site_map)
{
    int i = 0, j = 0;

    while (i < ka && a[i] <= last) i++;
    while (j < kb && b_site_map[b[j]] <= last) j++;
    while (i < ka && j < kb) {
        int mapped_b = b_site_map[b[j]];
        if (a[i] == mapped_b) return 1;
        if (a[i] < mapped_b) i++; else j++;
    }
    return 0;
}

/* Move every row containing "s" to the front of [lo,hi); return the split.
   Only the order changes, so the range may be re-partitioned afterwards. */
static size_t partition_on(size_t *idx, size_t lo, size_t hi,
                           const uint16_t *rows, int width, int s,
                           const int *site_map)
{
    size_t i, w = lo;

    if (width == 0) return lo;

    for (i = lo; i < hi; i++) {
        const uint16_t *r = rows + idx[i] * (size_t)width;
        int j, hit = 0;
        for (j = 0; j < width; j++)
            if ((site_map == NULL ? (int)r[j] : site_map[r[j]]) == s) {
                hit = 1;
                break;
            }
        if (hit) {
            size_t t = idx[w]; idx[w] = idx[i]; idx[i] = t;
            w++;
        }
    }
    return w;
}

/*
 * Value of the subtree hanging below a prefix set whose largest element is
 * "last", with the sign (-1)^|prefix| factored out:
 *
 *     sum over T subset of {elements > last} of
 *         (-1)^|T| * f(prefix+T) * g(prefix+T)
 *
 *   = #{ (a,b) in current lists : a n b has no element > last }.
 */
static PairSum subtree(Counter *c, size_t alo, size_t ahi,
                       size_t blo, size_t bhi,
                       int last, int depth)
{
    size_t na = ahi - alo, nb = bhi - blo, i, j;
    int ka = c->A->width, kb = c->B->width;
    i128 sa = 0, sb = 0, sm = 0;
    PairSum total = { 0, 0 };
    uint64_t stamp;
    uint16_t *cand;
    int ncand = 0, k;

    if (na == 0U || nb == 0U) return total;

    if (na <= c->cutoff / nb) {                    /* direct evaluation */
        for (i = alo; i < ahi; i++) {
            const uint16_t *a = bag_row_at(c->A, c->aidx[i]);
            uint64_t ma = c->A->mult[c->aidx[i]];
            for (j = blo; j < bhi; j++) {
                const uint16_t *b = bag_row_at(c->B, c->bidx[j]);
                if (!shares_above(a, ka, b, kb, last, c->b_site_map)) {
                    total.z += (i128)ma * c->B->mult[c->bidx[j]];
                    total.m += (i128)ma * c->b_weight[c->bidx[j]];
                }
            }
        }
        return total;
    }

    for (i = alo; i < ahi; i++) sa += c->A->mult[c->aidx[i]];
    for (j = blo; j < bhi; j++) {
        sb += c->B->mult[c->bidx[j]];
        sm += c->b_weight[c->bidx[j]];
    }
    total.z = sa * sb;                              /* the T = {} term */
    total.m = sa * sm;

    /* Candidates: sites above "last" present on both sides. */
    if (c->epoch == UINT64_MAX) {
        memset(c->seenA, 0,
               checked_product((size_t)c->nsite, sizeof(*c->seenA)));
        memset(c->seenB, 0,
               checked_product((size_t)c->nsite, sizeof(*c->seenB)));
        c->epoch = 0U;
    }
    stamp = ++c->epoch;
    cand  = c->cand + (size_t)depth * (size_t)c->nsite;
    for (i = alo; i < ahi; i++) {
        const uint16_t *a = bag_row_at(c->A, c->aidx[i]);
        for (k = 0; k < ka; k++)
            if (a[k] > last) c->seenA[a[k]] = stamp;
    }
    for (j = blo; j < bhi; j++) {
        const uint16_t *b = bag_row_at(c->B, c->bidx[j]);
        for (k = 0; k < kb; k++) {
            int e = c->b_site_map[b[k]];
            if (e > last && c->seenA[e] == stamp && c->seenB[e] != stamp) {
                c->seenB[e] = stamp;
                cand[ncand++] = (uint16_t)e;
            }
        }
    }
    for (k = 1; k < ncand; k++) {                   /* small, ascending */
        uint16_t v = cand[k];
        int m = k - 1;
        while (m >= 0 && cand[m] > v) { cand[m + 1] = cand[m]; m--; }
        cand[m + 1] = v;
    }

    for (k = 0; k < ncand; k++) {
        int s = cand[k];
        size_t amid =
            partition_on(c->aidx, alo, ahi, c->A->row, ka, s, NULL);
        size_t bmid = partition_on(c->bidx, blo, bhi, c->B->row, kb, s,
                                   c->b_site_map);
        PairSum child =
            subtree(c, alo, amid, blo, bmid, s, depth + 1);
        total.z -= child.z;
        total.m -= child.m;
    }
    if (total.z < 0 || total.m < 0)
        die("internal inclusion-exclusion invariant failed");
    return total;
}

/* ------------------------------------------------------------------ */
/* Full point group fixing the origin                                 */
/* ------------------------------------------------------------------ */

static const int PERM[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
};

/* g = 8*p + s enumerates all 48 signed coordinate permutations, each of
   which maps the simple cubic lattice onto itself and fixes the origin. */
static void sym_apply(int gsym, const int *v, int *out)
{
    const int *p = PERM[gsym >> 3];
    int s = gsym & 7, i;

    for (i = 0; i < 3; i++) {
        int val = v[p[i]];
        out[i] = (s >> i) & 1 ? -val : val;
    }
}

/* Is site "id" the canonical representative of its orbit?  If so, report
   how many distinct points the orbit contains. */
static int orbit_rep(const Lattice *g, int id, int *orbit_size)
{
    int v[3], img[3], i, j, best = id, distinct = 0;
    int seen[48];

    v[0] = g->px[id]; v[1] = g->py[id]; v[2] = g->pz[id];
    for (i = 0; i < 48; i++) {
        int other;
        sym_apply(i, v, img);
        other = site_of(g, img[0], img[1], img[2]);
        if (other < 0) return 0;              /* orbit leaves the box */
        seen[i] = other;
        if (other < best) best = other;
    }
    for (i = 0; i < 48; i++) {
        int fresh = 1;
        for (j = 0; j < i; j++) if (seen[j] == seen[i]) { fresh = 0; break; }
        if (fresh) distinct++;
    }
    *orbit_size = distinct;
    return best == id;
}

/*
 * Canonicalize every fixed-join A row under the stabilizer of the join.
 *
 * Every operation used here fixes both the origin and the join.  It maps the
 * unrestricted B collection bijectively to itself and preserves Manhattan
 * endpoint distance.  Consequently the total B contribution paired with an
 * A row is constant on the A row's stabilizer orbit.  Replacing all rows in
 * that orbit by one canonical row and then summing their multiplicities is
 * therefore exact; no walk or weight is discarded.
 */
static void canonicalize_fixed_join_rows(Bag *b, const Lattice *g, int join)
{
    int join_vector[3], image[3], stabilizer[48];
    int nstabilizer = 0, sym, k;
    size_t i;
    uint16_t *site_map;

    if (b->track_endpoint || b->mult != NULL)
        die("fixed-join rows must be canonicalized before compression");
    if (b->width == 0 || b->count == 0U) return;

    join_vector[0] = g->px[join];
    join_vector[1] = g->py[join];
    join_vector[2] = g->pz[join];
    for (sym = 0; sym < 48; sym++) {
        sym_apply(sym, join_vector, image);
        if (image[0] == join_vector[0] &&
            image[1] == join_vector[1] &&
            image[2] == join_vector[2])
            stabilizer[nstabilizer++] = sym;
    }
    if (nstabilizer <= 0)
        die("join stabilizer unexpectedly has no identity");
    if (nstabilizer == 1) return;

    site_map = xmalloc_array(
        checked_product((size_t)nstabilizer, (size_t)g->nsite),
        sizeof(*site_map));
    for (sym = 0; sym < nstabilizer; sym++) {
        for (i = 0; i < (size_t)g->nsite; i++) {
            int point[3], mapped;
            point[0] = g->px[i];
            point[1] = g->py[i];
            point[2] = g->pz[i];
            sym_apply(stabilizer[sym], point, image);
            mapped = site_of(g, image[0], image[1], image[2]);
            if (mapped < 0)
                die("join stabilizer mapped a site outside the lattice");
            site_map[(size_t)sym * (size_t)g->nsite + i] =
                (uint16_t)mapped;
        }
    }

    for (i = 0; i < b->count; i++) {
        uint16_t *row = b->row + i * (size_t)b->width;
        uint16_t best[MAX_BAG_WIDTH];
        uint16_t candidate[MAX_BAG_WIDTH];
        int have_best = 0;
        int h;

        for (h = 0; h < nstabilizer; h++) {
            for (k = 0; k < b->width; k++) {
                int id = row[k];
                if (id < 0 || id >= g->nsite)
                    die("fixed-join row contains an invalid site");
                candidate[k] =
                    site_map[(size_t)h * (size_t)g->nsite + (size_t)id];
            }
            for (k = 1; k < b->width; k++) {
                uint16_t value = candidate[k];
                int j = k - 1;
                while (j >= 0 && candidate[j] > value) {
                    candidate[j + 1] = candidate[j];
                    j--;
                }
                candidate[j + 1] = value;
            }
            {
                int better = !have_best;
                if (have_best) {
                    for (k = 0; k < b->width; k++) {
                        if (candidate[k] < best[k]) {
                            better = 1;
                            break;
                        }
                        if (candidate[k] > best[k]) break;
                    }
                }
                if (!better) continue;
                memcpy(best, candidate,
                       checked_product((size_t)b->width, sizeof(*best)));
                have_best = 1;
            }
        }
        if (!have_best)
            die("failed to canonicalize a fixed-join row");
        memcpy(row, best,
               checked_product((size_t)b->width, sizeof(*row)));
    }
    free(site_map);
}

/* ------------------------------------------------------------------ */
/* Driver                                                              */
/* ------------------------------------------------------------------ */

/* Build all M-step continuations once in coordinates relative to their
   start. Translation to a particular join point preserves row equality and
   lexicographic row order, so the compressed bag is immutable and shareable
   by every OpenMP worker. */
static size_t build_continuation_template(int steps, Lattice *g, Bag *bag)
{
    ContinuationWalker w;
    unsigned char *seen;
    int *path;
    int origin;
    size_t raw_count;

    lattice_build(g, steps);
    origin = site_of(g, 0, 0, 0);
    if (origin < 0) die("internal continuation origin is missing");

    seen = xcalloc_array((size_t)g->nsite, sizeof(*seen));
    path = xmalloc_array((size_t)steps + 1U, sizeof(*path));
    bag_init(bag, steps, 1);

    w.g = g; w.seen = seen; w.steps = steps;
    w.path = path; w.banned = -1; w.bag = bag;
    path[0] = origin;
    continuation_rec(&w, 0);

    raw_count = bag->count;
    if ((uint64_t)raw_count > HALF_WALK_LIMIT)
        die("continuation template exceeded its proved bound");
    bag_compress(bag, g->nsite);

    free(path);
    free(seen);
    return raw_count;
}

typedef struct {
    Lattice   *g;
    const Lattice *continuation_lattice;
    const Bag *continuations;
    int        origin;
    int        nsteps_a, nsteps_b;
    size_t     cutoff;
    uint64_t   max_walks;
} Problem;

/* Count all full walks that pass through join point "join", and sum the
   Manhattan distances of their unrestricted final endpoints. */
static PairSum solve_join(const Problem *pr, int join)
{
    const Lattice *g = pr->g;
    const Lattice *continuation_lattice = pr->continuation_lattice;
    const Bag *B = pr->continuations;
    Bag A;
    Walker w;
    Counter c;
    int *path;
    int *b_site_map;
    unsigned char *seen;
    size_t *aidx, *bidx;
    i128 *b_weight;
    uint64_t *seenA, *seenB;
    uint16_t *cand;
    int banned_relative, maxdepth;
    int join_x = g->px[join];
    int join_y = g->py[join];
    int join_z = g->pz[join];
    size_t bcount = 0, i;
    PairSum answer = { 0, 0 };

    seen  = xmalloc_array((size_t)g->nsite, sizeof(*seen));
    path  = xmalloc_array(
        (size_t)pr->nsteps_a + 1U, sizeof(*path));
    memset(seen, 0, (size_t)g->nsite);

    bag_init(&A, pr->nsteps_a - 1, 0);

    w.g = g; w.seen = seen; w.path = path;
    w.join_x = join_x; w.join_y = join_y; w.join_z = join_z;

    w.steps = pr->nsteps_a; w.bag = &A; w.banned = -1;
    path[0] = pr->origin;
    walk_rec(&w, 0);

    if (A.count == 0 || B->count == 0) {
        bag_free(&A);
        free(seen); free(path);
        return answer;
    }
    if (pr->max_walks > 0U && (uint64_t)A.count > pr->max_walks)
        die("half-walk bag exceeded the configured limit");

    canonicalize_fixed_join_rows(&A, g, join);
    bag_compress(&A, g->nsite);

    maxdepth = (A.width < B->width ? A.width : B->width) + 2;
    aidx  = xmalloc_array(A.count, sizeof(*aidx));
    bidx  = xmalloc_array(B->count, sizeof(*bidx));
    b_weight = xmalloc_array(B->count, sizeof(*b_weight));
    b_site_map = xmalloc_array(
        (size_t)continuation_lattice->nsite, sizeof(*b_site_map));
    seenA = xmalloc_array((size_t)g->nsite, sizeof(*seenA));
    seenB = xmalloc_array((size_t)g->nsite, sizeof(*seenB));
    cand  = xmalloc_array(
        checked_product((size_t)g->nsite, (size_t)maxdepth),
        sizeof(*cand));
    memset(seenA, 0,
           checked_product((size_t)g->nsite, sizeof(*seenA)));
    memset(seenB, 0,
           checked_product((size_t)g->nsite, sizeof(*seenB)));

    /*
     * Translation by the join point preserves lexicographic coordinate
     * order. Verify that the resulting site-id map is strictly increasing;
     * shares_above() relies on every translated B row remaining sorted.
     */
    for (i = 0; i < (size_t)continuation_lattice->nsite; i++) {
        int mapped = site_of(
            g, join_x + continuation_lattice->px[i],
               join_y + continuation_lattice->py[i],
               join_z + continuation_lattice->pz[i]);
        if (mapped < 0)
            die("translated continuation left the proved lattice box");
        if (i > 0U && mapped <= b_site_map[i - 1U])
            die("continuation translation did not preserve site order");
        b_site_map[i] = mapped;
    }
    banned_relative = site_of(
        continuation_lattice, -join_x, -join_y, -join_z);
    if (!B->track_endpoint || B->endpoint == NULL ||
        B->endpoint_mult == NULL || B->endpoint_offset == NULL ||
        B->endpoint_offset[B->count] != B->endpoint_count)
        die("invalid compressed continuation endpoint table");
    for (i = 0; i < B->count; i++) {
        const uint16_t *row = bag_row_at(B, i);
        int banned = 0;
        int k;
        size_t e;
        i128 translated_weight = 0;

        if (banned_relative >= 0) {
            for (k = 0; k < B->width; k++) {
                if (row[k] == banned_relative) {
                    banned = 1;
                    break;
                }
            }
        }
        if (banned) continue;

        if (B->endpoint_offset[i] > B->endpoint_offset[i + 1U] ||
            B->endpoint_offset[i + 1U] > B->endpoint_count)
            die("invalid compressed continuation endpoint offsets");
        for (e = B->endpoint_offset[i];
             e < B->endpoint_offset[i + 1U]; e++) {
            int endpoint = B->endpoint[e];
            int end_x, end_y, end_z, manhattan;

            if (endpoint >= continuation_lattice->nsite)
                die("continuation endpoint is outside its lattice");
            end_x = join_x + continuation_lattice->px[endpoint];
            end_y = join_y + continuation_lattice->py[endpoint];
            end_z = join_z + continuation_lattice->pz[endpoint];
            manhattan = abs(end_x) + abs(end_y) + abs(end_z);
            translated_weight +=
                (i128)B->endpoint_mult[e] * manhattan;
        }
        if (translated_weight < 0)
            die("translated continuation weight is negative");
        b_weight[i] = translated_weight;
        bidx[bcount++] = i;
    }

    if (bcount == 0U) {
        free(b_site_map); free(b_weight); free(aidx); free(bidx);
        free(seenA); free(seenB); free(cand);
        bag_free(&A);
        free(seen); free(path);
        return answer;
    }

    for (i = 0; i < A.count; i++) aidx[i] = i;

    c.A = &A; c.B = B;
    c.aidx = aidx; c.bidx = bidx;
    c.b_site_map = b_site_map; c.b_weight = b_weight;
    c.seenA = seenA; c.seenB = seenB; c.cand = cand;
    c.epoch = 0U; c.nsite = g->nsite; c.cutoff = pr->cutoff;

    answer = subtree(&c, 0, A.count, 0, bcount, -1, 0);

    free(b_site_map); free(b_weight);
    free(aidx); free(bidx); free(seenA); free(seenB); free(cand);
    bag_free(&A);
    free(seen); free(path);
    return answer;
}

/* Count all n-step simple-cubic SAWs and their Manhattan-distance sum. */
static PairSum count_all_walks(int nsteps, size_t cutoff, int verbose)
{
    Lattice g, continuation_lattice;
    Bag continuations;
    Problem pr;
    int na, nb, i, origin;
    int *reps, nreps = 0;
    int *repsize;
    PairSum total = { 0, 0 };
    i128 total_z = 0, total_m = 0;
    size_t raw_continuations;

    if (nsteps < 0 || nsteps > MAX_INDEX)
        die("internal index out of range");
    if (nsteps == 0) {
        total.z = 1;
        return total;
    }
    if (nsteps == 1) {
        total.z = 6;
        total.m = 6;
        return total;
    }

    /*
     * The unrestricted B bag is much larger than a fixed-join A bag. Bias
     * the split by two steps toward A whenever B can remain nonempty.
     * This one uniform rule reduces B without changing the set of full walks
     * or any inclusion-exclusion identity.
     */
    nb = nsteps / 2;
    if (nb > SPLIT_BIAS) nb -= SPLIT_BIAS;
    else nb = 1;
    na = nsteps - nb;               /* O -> X; X -> unrestricted Y */

    /* Every reachable point has Manhattan distance at most n. */
    lattice_build(&g, nsteps);
    raw_continuations =
        build_continuation_template(nb, &continuation_lattice,
                                    &continuations);

    origin = site_of(&g, 0, 0, 0);

    reps    = xmalloc_array((size_t)g.nsite, sizeof(*reps));
    repsize = xmalloc_array((size_t)g.nsite, sizeof(*repsize));
    for (i = 0; i < g.nsite; i++) {
        int osz;
        int distance = abs((int)g.px[i])
                       + abs((int)g.py[i])
                       + abs((int)g.pz[i]);
        if (distance > na) continue;
        if (!orbit_rep(&g, i, &osz)) continue;
        reps[nreps] = i;
        repsize[nreps] = osz;
        nreps++;
    }

    if (verbose)
        fprintf(stderr,
                "  n=%d split=%d+%d radius=%d sites=%d join_orbits=%d "
                "continuations=%zu compressed=%zu\n",
                nsteps, na, nb, nsteps, g.nsite, nreps,
                raw_continuations, continuations.count);

    pr.g = &g;
    pr.continuation_lattice = &continuation_lattice;
    pr.continuations = &continuations;
    pr.origin = origin;
    pr.nsteps_a = na; pr.nsteps_b = nb;
    pr.cutoff = cutoff; pr.max_walks = HALF_WALK_LIMIT;

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1) reduction(+:total_z,total_m)
#endif
    for (i = 0; i < nreps; i++) {
        PairSum one = solve_join(&pr, reps[i]);
        total_z += (i128)repsize[i] * one.z;
        total_m += (i128)repsize[i] * one.m;
    }

    total.z = total_z;
    total.m = total_m;

    free(reps); free(repsize);
    bag_free(&continuations);
    lattice_free(&continuation_lattice);
    lattice_free(&g);
    return total;
}

/* ------------------------------------------------------------------ */
/* Reference values and self-test                                      */
/* ------------------------------------------------------------------ */

/* OEIS A078717(2..18): simple-cubic SAWs with the first step specified. */
static const uint64_t KNOWN_C[] = {
    5ULL, 25ULL, 121ULL, 589ULL, 2821ULL, 13565ULL, 64661ULL,
    308981ULL, 1468313ULL, 6989025ULL, 33140457ULL, 157329085ULL,
    744818613ULL, 3529191009ULL, 16686979329ULL, 78955042017ULL,
    372953947349ULL
};

/* OEIS A079156(2..18): their total endpoint Manhattan distances. */
static const uint64_t KNOWN_M[] = {
    10ULL, 67ULL, 396ULL, 2201ULL, 11870ULL, 62571ULL, 324896ULL,
    1665349ULL, 8457890ULL, 42605267ULL, 213305636ULL, 1061939193ULL,
    5263752278ULL, 25984214383ULL, 127848694424ULL, 627084275649ULL,
    3067923454498ULL
};

/*
 * Independent reference: plain depth-first enumeration with no splitting,
 * no inclusion-exclusion and no symmetry reduction.  Exponential, but it
 * shares no logic with the fast path, so agreement between the two checks
 * the whole method rather than one component of it.
 */
typedef struct {
    const Lattice *g;
    unsigned char *seen;
    int  steps;
    PairSum total;
} Brute;

static void brute_rec(Brute *b, int here, int depth)
{
    const int32_t *nb;
    int d;

    if (depth == b->steps) {
        b->total.z++;
        b->total.m += abs(b->g->px[here])
                      + abs(b->g->py[here])
                      + abs(b->g->pz[here]);
        return;
    }
    b->seen[here] = 1;
    nb = b->g->adj + (long)here * DEGREE;
    for (d = 0; d < DEGREE; d++) {
        int next = nb[d];
        if (next >= 0 && !b->seen[next]) brute_rec(b, next, depth + 1);
    }
    b->seen[here] = 0;
}

static PairSum brute_count_all(int nsteps)
{
    Lattice g;
    Brute b;
    int origin;

    lattice_build(&g, nsteps);
    origin = site_of(&g, 0, 0, 0);
    b.g = &g;
    b.seen = xmalloc_array((size_t)g.nsite, sizeof(*b.seen));
    memset(b.seen, 0, (size_t)g.nsite);
    b.steps = nsteps;
    b.total.z = 0;
    b.total.m = 0;
    brute_rec(&b, origin, 0);
    free(b.seen);
    lattice_free(&g);
    return b.total;
}

static int selftest(size_t cutoff)
{
    int n, bad = 0;

    printf("[1] against OEIS A078717 and A079156\n");
    for (n = 2; n <= 12; n++) {                 /* keep the test quick */
        PairSum got = count_all_walks(n, cutoff, 0);
        int ok = got.z % 6 == 0 && got.m % 6 == 0
                 && got.z / 6 == (i128)KNOWN_C[n - 2]
                 && got.m / 6 == (i128)KNOWN_M[n - 2];
        printf("     n=%2d  C=", n); print_i128(got.z / 6);
        printf("  M="); print_i128(got.m / 6);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok) bad = 1;
    }

    printf("[2] against plain depth-first enumeration\n");
    for (n = 2; n <= 8; n++) {
        PairSum fast = count_all_walks(n, cutoff, 0);
        PairSum slow = brute_count_all(n);
        int ok = fast.z == slow.z && fast.m == slow.m;
        printf("     n=%2d  Z=", n); print_i128(fast.z);
        printf("  M="); print_i128(fast.m);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok) bad = 1;
    }

    printf("[3] cut-off invariance, both results must be unchanged\n");
    {
        size_t trial[4] = { 1U, 64U, 4096U, 1000000U };
        PairSum ref = { 0, 0 };
        int k;
        for (k = 0; k < 4; k++) {
            PairSum got = count_all_walks(9, trial[k], 0);
            if (k == 0) ref = got;
            printf("     cutoff=%-8zu Z=", trial[k]); print_i128(got.z);
            printf("  M="); print_i128(got.m);
            printf("  %s\n",
                   got.z == ref.z && got.m == ref.m ? "ok" : "MISMATCH");
            if (got.z != ref.z || got.m != ref.m) bad = 1;
        }
    }

    printf("[4] n=%d site-id representation boundary\n", MAX_INDEX);
    {
        Lattice g;
        int i, d;
        int ok;

        lattice_build(&g, MAX_INDEX);
        ok = g.nsite == MAX_REACHABLE_SITES
             && site_of(&g, 0, 0, 0) >= 0
             && site_of(&g, MAX_INDEX, 0, 0) >= 0
             && site_of(&g, MAX_INDEX, 1, 0) < 0
             && site_of(&g, MAX_INDEX + 1, 0, 0) < 0;
        for (i = 0; ok && i < g.nsite; i++) {
            if (site_of(&g, g.px[i], g.py[i], g.pz[i]) != i)
                ok = 0;
            for (d = 0; ok && d < DEGREE; d++) {
                int adjacent = g.adj[(long)i * DEGREE + d];
                if (adjacent < -1 || adjacent >= g.nsite) ok = 0;
            }
        }
        printf("     sites=%d (uint16_t max=%u)  %s\n",
               g.nsite, (unsigned)UINT16_MAX,
               ok ? "ok" : "MISMATCH");
        if (!ok) bad = 1;
        lattice_free(&g);
    }

    printf("%s\n", bad ? "SELFTEST FAILED" : "selftest passed");
    return bad;
}

/* ------------------------------------------------------------------ */

static void usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s [--cutoff K] [--quiet] N\n"
        "       %s [--cutoff K] [--quiet] --upto N\n"
        "       %s [--cutoff K] selftest\n"
        "where %d <= N <= %d\n",
        prog, prog, prog, MIN_INDEX, MAX_INDEX);
}

static long parse_long_range(const char *text, long minimum, long maximum,
                             const char *description)
{
    char *end = NULL;
    long value;

    if (text == NULL || *text == '\0') die(description);
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0')
        die(description);
    if (value < minimum || value > maximum) die(description);
    return value;
}

static size_t parse_cutoff(const char *text)
{
    char *end = NULL;
    uintmax_t value;

    if (text == NULL || *text == '\0' || *text == '-')
        die("--cutoff must be a positive integer");
    errno = 0;
    value = strtoumax(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || value == 0U ||
        value > (uintmax_t)SIZE_MAX)
        die("--cutoff must be a positive integer");
    return (size_t)value;
}

static i128 nonbacktracking_bound(int n)
{
    i128 bound;
    int i;

    if (n == 0) return 1;
    bound = 6;
    for (i = 1; i < n; i++) bound *= 5;
    return bound;
}

static void print_term(int n, size_t cutoff, int verbose)
{
    PairSum value = count_all_walks(n, cutoff, verbose);
    i128 z_bound = nonbacktracking_bound(n);
    i128 m_bound = (i128)n * z_bound;

    if (value.z < 0 || value.m < 0 ||
        value.z > z_bound || value.m > m_bound)
        die("result exceeds its proved nonbacktracking bound");
    if (value.z % 6 != 0 || value.m % 6 != 0)
        die("first-step symmetry normalization is not exact");
    printf("%d ", n);
    print_i128(value.z / 6);
    putchar(' ');
    print_i128(value.m / 6);
    putchar('\n');
}

int main(int argc, char **argv)
{
    size_t cutoff = DEFAULT_CUTOFF;
    int verbose = 1;
    int i = 1;

    while (i < argc && strncmp(argv[i], "--", 2U) == 0) {
        if (strcmp(argv[i], "--cutoff") == 0) {
            if (i + 1 >= argc) die("--cutoff requires an argument");
            cutoff = parse_cutoff(argv[i + 1]);
            i += 2;
        } else if (strcmp(argv[i], "--quiet") == 0) {
            verbose = 0;
            i++;
        } else if (strcmp(argv[i], "--upto") == 0) {
            break;
        } else {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (i < argc && strcmp(argv[i], "--upto") == 0) {
        int n, limit;
        if (i + 2 != argc) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        limit = (int)parse_long_range(
            argv[i + 1], MIN_INDEX, MAX_INDEX,
            "index is outside the proved safe range 2..36");
        for (n = MIN_INDEX; n <= limit; n++)
            print_term(n, cutoff, verbose);
        return EXIT_SUCCESS;
    }

    if (i < argc && strcmp(argv[i], "selftest") == 0) {
        if (i + 1 != argc) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        return selftest(cutoff) ? EXIT_FAILURE : EXIT_SUCCESS;
    }

    if (i < argc) {
        int n;
        if (i + 1 != argc) {
            usage(argv[0]);
            return EXIT_FAILURE;
        }
        n = (int)parse_long_range(
            argv[i], MIN_INDEX, MAX_INDEX,
            "index is outside the proved safe range 2..36");
        print_term(n, cutoff, verbose);
        return EXIT_SUCCESS;
    }

    usage(argv[0]);
    return EXIT_FAILURE;
}
