/*
 * 001337_06.c -- Memory-intensive meet-in-the-middle enumeration of
 *                f.c.c. rooted closed self-avoiding walks (OEIS A001337).
 *
 * This implementation stores and compresses complete half-walk vertex-set
 * bags for each join point. It deliberately trades more memory for less
 * repeated traversal.
 *
 * METHOD
 * ------
 * Meet-in-the-middle (the "length-doubling" idea of R. D. Schram,
 * G. T. Barkema and R. H. Bisseling, "Exact enumeration of self-avoiding
 * walks", J. Stat. Mech. (2011) P06019), re-derived here as follows.
 *
 * Fix a join point X and split an L-step walk O -> C as
 *     O --N steps--> X --M steps--> C,      N + M = L.
 * Let A be the set of N-step SAWs from O to X and B the set of M-step SAWs
 * from C to X.  Represent each walk by its vertex set with X removed.  The
 * concatenation is self-avoiding exactly when a and b are disjoint, so
 *
 *     Answer(X) = #{ (a,b) in A x B : a n b = 0 }.
 *
 * By inclusion-exclusion over the shared set,
 *
 *     Answer(X) = sum_S (-1)^|S| f(S) g(S),
 *     f(S) = #{a in A : S subset of a},   g(S) = #{b in B : S subset of b}.
 *
 * Enumerate S by adding elements in increasing order.  Writing "last" for
 * the largest element chosen so far, the whole subtree below a prefix S
 * collapses to a closed form:
 *
 *     subtree(S,last) = (-1)^|S| * #{ (a,b) : a,b contain S,
 *                                     (a n b) has no element > last }.
 *
 * Two consequences drive the implementation:
 *   (1) the recursion is  SA*SB - sum over candidate elements s > last  of
 *       subtree(S+{s}, s), where SA, SB are the current list sizes;
 *   (2) at ANY node the subtree may instead be evaluated directly in
 *       O(|A_cur| * |B_cur|).  Recursing only while the lists are large
 *       gives a branch-and-bound whose cut-off threshold does not affect
 *       the result -- a strong built-in consistency check (--selftest).
 *
 * Join points are reduced modulo the stabiliser of the ordered pair (O,C)
 * inside the 48-element f.c.c. point group (all signed coordinate
 * permutations); the stabiliser is computed at run time, so an arbitrary
 * target point may be used for cross-validation.
 *
 * This is the A001337-specific, safety-hardened form of the independently
 * written 001337_07.c. It contains no SAWdoubler source code.
 *
 * Memory-intensive A001337 adaptation and safety hardening:
 * Modified by OpenAI Codex on 2026-07-23.
 *
 * SPDX-License-Identifier: MIT
 *
 * Build:   clang -O3 -std=c11 001337_06.c -o 001337_06
 *          gcc-omp 001337_06.c -o 001337_06
 *
 * Usage:   ./001337_06 N
 *          ./001337_06 --upto N
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

#define MIN_INDEX 1
#define MAX_INDEX 19
#define MAX_LATTICE_RADIUS 10
#define DEFAULT_CUTOFF 32U
#define HALF_WALK_LIMIT UINT64_C(2572306572) /* 12 * 11^8 */

/*
 * Integer bounds for 1 <= n <= 19:
 * - the open fixed-edge walk has at most 18 steps and is split 9+9;
 * - each half-walk bag has at most H = 12*11^8 entries;
 * - every multiplicity and half-walk sum is at most H < 2^32;
 * - every disjoint-pair subtree is between 0 and H^2 < 2^63;
 * - the rooted closed-walk result is at most 12*11^17 < 2^63,
 *   because the final closing step is forced.
 * Signed 128-bit intermediate arithmetic and uint64_t output therefore have
 * ample range. Allocation products are checked independently.
 */

static void die(const char *what)
{
    fprintf(stderr, "001337_06: %s\n", what);
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
/* f.c.c. lattice inside a coordinate box                             */
/* ------------------------------------------------------------------ */

/* The 12 nearest neighbours: every permutation of (+-1,+-1,0). */
static const int STEP[12][3] = {
    {  1,  1,  0 }, {  1, -1,  0 }, { -1,  1,  0 }, { -1, -1,  0 },
    {  1,  0,  1 }, {  1,  0, -1 }, { -1,  0,  1 }, { -1,  0, -1 },
    {  0,  1,  1 }, {  0,  1, -1 }, {  0, -1,  1 }, {  0, -1, -1 }
};

typedef struct {
    int      radius;    /* coordinates run over [-radius, radius] */
    int      side;      /* 2*radius + 1                           */
    long     cells;     /* side^3                                 */
    int      nsite;     /* number of lattice sites in the box     */
    int     *cell2id;   /* box cell -> site id, or -1             */
    int16_t *px, *py, *pz;
    int32_t *adj;       /* nsite * 12 -> site id, or -1           */
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

/* A point belongs to the f.c.c. lattice iff its coordinate sum is even. */
static int on_lattice(int x, int y, int z)
{
    int s = x + y + z;
    return ((s % 2) + 2) % 2 == 0;
}

static void lattice_build(Lattice *g, int radius)
{
    long c;
    int x, y, z, n, d;

    if (radius < 0 || radius > MAX_LATTICE_RADIUS)
        die("internal lattice radius out of range");

    g->radius = radius;
    g->side   = 2 * radius + 1;
    g->cells  = (long)g->side * g->side * g->side;

    g->cell2id = xmalloc_array((size_t)g->cells, sizeof(*g->cell2id));
    for (c = 0; c < g->cells; c++) g->cell2id[c] = -1;

    n = 0;
    for (x = -radius; x <= radius; x++)
        for (y = -radius; y <= radius; y++)
            for (z = -radius; z <= radius; z++)
                if (on_lattice(x, y, z)) n++;

    if (n > 65535) die("box too large: site ids must fit in uint16_t");

    g->nsite = n;
    g->px = xmalloc_array((size_t)n, sizeof(*g->px));
    g->py = xmalloc_array((size_t)n, sizeof(*g->py));
    g->pz = xmalloc_array((size_t)n, sizeof(*g->pz));

    n = 0;
    for (x = -radius; x <= radius; x++)
        for (y = -radius; y <= radius; y++)
            for (z = -radius; z <= radius; z++)
                if (on_lattice(x, y, z)) {
                    g->cell2id[cell_of(g, x, y, z)] = n;
                    g->px[n] = (int16_t)x;
                    g->py[n] = (int16_t)y;
                    g->pz[n] = (int16_t)z;
                    n++;
                }

    g->adj = xmalloc_array(
        checked_product((size_t)g->nsite, 12U), sizeof(*g->adj));
    for (n = 0; n < g->nsite; n++)
        for (d = 0; d < 12; d++)
            g->adj[(long)n * 12 + d] =
                site_of(g, g->px[n] + STEP[d][0],
                           g->py[n] + STEP[d][1],
                           g->pz[n] + STEP[d][2]);
}

static void lattice_free(Lattice *g)
{
    free(g->cell2id); free(g->px); free(g->py); free(g->pz); free(g->adj);
}

/* Breadth-first graph distance; unreachable sites get INT_MAX/2. */
static void bfs(const Lattice *g, int src, int *dist, int *queue)
{
    int head = 0, tail = 0, i, d;

    for (i = 0; i < g->nsite; i++) dist[i] = INT_MAX / 2;
    dist[src] = 0;
    queue[tail++] = src;
    while (head < tail) {
        int u = queue[head++];
        const int32_t *nb = g->adj + (long)u * 12;
        for (d = 0; d < 12; d++) {
            int v = nb[d];
            if (v >= 0 && dist[v] > dist[u] + 1) {
                dist[v] = dist[u] + 1;
                queue[tail++] = v;
            }
        }
    }
}

/* ------------------------------------------------------------------ */
/* Bag of half-walks, each stored as its sorted vertex set             */
/* ------------------------------------------------------------------ */

typedef struct {
    uint16_t *row;   /* count * width entries, each row ascending */
    uint64_t *mult;  /* multiplicity, valid after bag_compress()  */
    size_t    count;
    size_t    cap;
    int       width;
} Bag;

static void bag_init(Bag *b, int width)
{
    b->row = NULL; b->mult = NULL;
    b->count = 0; b->cap = 0; b->width = width;
}

static void bag_free(Bag *b) { free(b->row); free(b->mult); }

static void bag_add(Bag *b, const int *path)
{
    uint16_t *r;
    int i, j;

    if (b->count == b->cap) {
        size_t cap;
        uint16_t *grown;
        if ((uint64_t)b->count >= HALF_WALK_LIMIT)
            die("half-walk count exceeded its proved bound");
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
        b->row = grown;
        b->cap = cap;
    }
    r = b->row + b->count * (size_t)b->width;
    for (i = 0; i < b->width; i++) r[i] = (uint16_t)path[i];
    for (i = 1; i < b->width; i++) {          /* insertion sort, width <= 12 */
        uint16_t v = r[i];
        j = i - 1;
        while (j >= 0 && r[j] > v) { r[j + 1] = r[j]; j--; }
        r[j + 1] = v;
    }
    b->count++;
}

/* Sort rows lexicographically (LSD radix over the columns) and merge
   duplicates into multiplicities.  Distinct walks with the same vertex set
   are interchangeable for the disjointness count. */
static void bag_compress(Bag *b, int nsite)
{
    size_t n = b->count, i, m;
    int w = b->width, col, s;
    size_t *ord, *scratch;
    size_t *bucket;
    uint16_t *out;
    uint64_t *mul;

    if (n == 0) { b->mult = NULL; return; }

    ord     = xmalloc_array(n, sizeof(*ord));
    scratch = xmalloc_array(n, sizeof(*scratch));
    bucket  = xcalloc_array((size_t)nsite + 1U, sizeof(*bucket));

    for (i = 0; i < n; i++) ord[i] = i;
    for (col = w - 1; col >= 0; col--) {
        if (col != w - 1)
            memset(bucket, 0,
                   checked_product((size_t)nsite + 1U, sizeof(*bucket)));
        for (i = 0; i < n; i++)
            bucket[b->row[ord[i] * (size_t)w + (size_t)col] + 1U]++;
        for (s = 0; s < nsite; s++) bucket[s + 1] += bucket[s];
        for (i = 0; i < n; i++)
            scratch[bucket[b->row[
                ord[i] * (size_t)w + (size_t)col]]++] = ord[i];
        memcpy(ord, scratch, checked_product(n, sizeof(*ord)));
    }

    out = xmalloc_array(
        checked_product(n, (size_t)w), sizeof(*out));
    mul = xmalloc_array(n, sizeof(*mul));
    m = 0;
    for (i = 0; i < n; i++) {
        const uint16_t *r = b->row + ord[i] * (size_t)w;
        if (m > 0U &&
            memcmp(out + (m - 1U) * (size_t)w, r,
                   checked_product((size_t)w, sizeof(*out))) == 0) {
            if (mul[m - 1U] == UINT64_MAX)
                die("half-walk multiplicity overflow");
            mul[m - 1]++;
        } else {
            memcpy(out + m * (size_t)w, r,
                   checked_product((size_t)w, sizeof(*out)));
            mul[m] = 1U;
            m++;
        }
    }

    free(ord); free(scratch); free(bucket); free(b->row);
    b->row = out; b->mult = mul; b->count = m;
}

/* ------------------------------------------------------------------ */
/* Half-walk generation                                                */
/* ------------------------------------------------------------------ */

typedef struct {
    const Lattice *g;
    const int     *dist;   /* graph distance to the join point */
    unsigned char *seen;
    int            steps;  /* walk length                      */
    int           *path;   /* path[0..steps]                   */
    Bag           *bag;
} Walker;

static void walk_rec(Walker *w, int depth)
{
    int here = w->path[depth];
    int left = w->steps - depth;
    const int32_t *nb;
    int d;

    /* Cannot possibly reach the join point in the steps that remain. */
    if (w->dist[here] > left) return;

    if (depth == w->steps) {
        /* dist == 0 here, so this vertex is the join point; it is excluded
           from the stored set, which holds path[0 .. steps-1]. */
        bag_add(w->bag, w->path);
        return;
    }

    w->seen[here] = 1;
    nb = w->g->adj + (long)here * 12;
    for (d = 0; d < 12; d++) {
        int next = nb[d];
        if (next >= 0 && !w->seen[next]) {
            w->path[depth + 1] = next;
            walk_rec(w, depth + 1);
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
    uint64_t  *seenA, *seenB;  /* epoch stamps over the sites    */
    uint16_t  *cand;           /* per-depth candidate buffers    */
    uint64_t   epoch;
    int        nsite;
    size_t     cutoff;         /* switch to direct counting here */
} Counter;

/* Do a and b share an element strictly greater than "last"? */
static int shares_above(const uint16_t *a, int ka,
                        const uint16_t *b, int kb, int last)
{
    int i = 0, j = 0;

    while (i < ka && a[i] <= last) i++;
    while (j < kb && b[j] <= last) j++;
    while (i < ka && j < kb) {
        if (a[i] == b[j]) return 1;
        if (a[i] < b[j]) i++; else j++;
    }
    return 0;
}

/* Move every row containing "s" to the front of [lo,hi); return the split.
   Only the order changes, so the range may be re-partitioned afterwards. */
static size_t partition_on(size_t *idx, size_t lo, size_t hi,
                           const uint16_t *rows, int width, int s)
{
    size_t i, w = lo;

    for (i = lo; i < hi; i++) {
        const uint16_t *r = rows + idx[i] * (size_t)width;
        int j, hit = 0;
        for (j = 0; j < width; j++)
            if (r[j] == (uint16_t)s) { hit = 1; break; }
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
static i128 subtree(Counter *c, size_t alo, size_t ahi,
                    size_t blo, size_t bhi,
                    int last, int depth)
{
    size_t na = ahi - alo, nb = bhi - blo, i, j;
    int ka = c->A->width, kb = c->B->width;
    i128 sa = 0, sb = 0, total;
    uint64_t stamp;
    uint16_t *cand;
    int ncand = 0, k;

    if (na <= 0 || nb <= 0) return 0;

    if (na <= c->cutoff / nb) {                    /* direct evaluation */
        i128 acc = 0;
        for (i = alo; i < ahi; i++) {
            const uint16_t *a =
                c->A->row + c->aidx[i] * (size_t)ka;
            uint64_t ma = c->A->mult[c->aidx[i]];
            for (j = blo; j < bhi; j++) {
                const uint16_t *b =
                    c->B->row + c->bidx[j] * (size_t)kb;
                if (!shares_above(a, ka, b, kb, last))
                    acc += (i128)ma * c->B->mult[c->bidx[j]];
            }
        }
        return acc;
    }

    for (i = alo; i < ahi; i++) sa += c->A->mult[c->aidx[i]];
    for (j = blo; j < bhi; j++) sb += c->B->mult[c->bidx[j]];
    total = sa * sb;                                /* the T = {} term */

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
        const uint16_t *a =
            c->A->row + c->aidx[i] * (size_t)ka;
        for (k = 0; k < ka; k++)
            if (a[k] > last) c->seenA[a[k]] = stamp;
    }
    for (j = blo; j < bhi; j++) {
        const uint16_t *b =
            c->B->row + c->bidx[j] * (size_t)kb;
        for (k = 0; k < kb; k++) {
            int e = b[k];
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
        size_t amid = partition_on(c->aidx, alo, ahi, c->A->row, ka, s);
        size_t bmid = partition_on(c->bidx, blo, bhi, c->B->row, kb, s);
        total -= subtree(c, alo, amid, blo, bmid, s, depth + 1);
    }
    return total;
}

/* ------------------------------------------------------------------ */
/* Point-group stabiliser of the ordered pair (origin, target)         */
/* ------------------------------------------------------------------ */

static const int PERM[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
};

/* g = 8*p + s enumerates all 48 signed coordinate permutations, each of
   which maps the f.c.c. lattice onto itself and fixes the origin. */
static void sym_apply(int gsym, const int *v, int *out)
{
    const int *p = PERM[gsym >> 3];
    int s = gsym & 7, i;

    for (i = 0; i < 3; i++) {
        int val = v[p[i]];
        out[i] = (s >> i) & 1 ? -val : val;
    }
}

/* ------------------------------------------------------------------ */
/* Driver                                                              */
/* ------------------------------------------------------------------ */

typedef struct {
    int stab[48];
    int nstab;
} Stabiliser;

static void stabiliser_of(const int *target, Stabiliser *st)
{
    int g, img[3];

    st->nstab = 0;
    for (g = 0; g < 48; g++) {
        sym_apply(g, target, img);
        if (img[0] == target[0] && img[1] == target[1] && img[2] == target[2])
            st->stab[st->nstab++] = g;
    }
}

/* Is site "id" the canonical representative of its orbit?  If so, report
   how many distinct points the orbit contains. */
static int orbit_rep(const Lattice *g, const Stabiliser *st, int id,
                     int *orbit_size)
{
    int v[3], img[3], i, j, best = id, distinct = 0;
    int seen[48];

    v[0] = g->px[id]; v[1] = g->py[id]; v[2] = g->pz[id];
    for (i = 0; i < st->nstab; i++) {
        int other;
        sym_apply(st->stab[i], v, img);
        other = site_of(g, img[0], img[1], img[2]);
        if (other < 0) return 0;              /* orbit leaves the box */
        seen[i] = other;
        if (other < best) best = other;
    }
    for (i = 0; i < st->nstab; i++) {
        int fresh = 1;
        for (j = 0; j < i; j++) if (seen[j] == seen[i]) { fresh = 0; break; }
        if (fresh) distinct++;
    }
    *orbit_size = distinct;
    return best == id;
}

typedef struct {
    Lattice   *g;
    int        origin, target;
    int        nsteps_a, nsteps_b;
    size_t     cutoff;
    uint64_t   max_walks;
} Problem;

/* Count the walks that pass through join point "join". */
static i128 solve_join(const Problem *pr, int join)
{
    const Lattice *g = pr->g;
    Bag A, B;
    Walker w;
    Counter c;
    int *dist, *queue, *path;
    unsigned char *seen;
    size_t *aidx, *bidx;
    uint64_t *seenA, *seenB;
    uint16_t *cand;
    int maxdepth;
    size_t i;
    i128 answer;

    dist  = xmalloc_array((size_t)g->nsite, sizeof(*dist));
    queue = xmalloc_array((size_t)g->nsite, sizeof(*queue));
    seen  = xmalloc_array((size_t)g->nsite, sizeof(*seen));
    path  = xmalloc_array(
        (size_t)pr->nsteps_a + (size_t)pr->nsteps_b + 2U, sizeof(*path));
    memset(seen, 0, (size_t)g->nsite);

    bfs(g, join, dist, queue);

    bag_init(&A, pr->nsteps_a);
    bag_init(&B, pr->nsteps_b);

    w.g = g; w.dist = dist; w.seen = seen; w.path = path;

    w.steps = pr->nsteps_a; w.bag = &A;
    path[0] = pr->origin;
    walk_rec(&w, 0);

    if (A.count > 0) {
        w.steps = pr->nsteps_b; w.bag = &B;
        path[0] = pr->target;
        walk_rec(&w, 0);
    }

    if (A.count == 0 || B.count == 0) {
        bag_free(&A); bag_free(&B);
        free(dist); free(queue); free(seen); free(path);
        return 0;
    }
    if (pr->max_walks > 0U &&
        ((uint64_t)A.count > pr->max_walks ||
         (uint64_t)B.count > pr->max_walks))
        die("half-walk bag exceeded the configured limit");

    bag_compress(&A, g->nsite);
    bag_compress(&B, g->nsite);

    maxdepth = (A.width < B.width ? A.width : B.width) + 2;
    aidx  = xmalloc_array(A.count, sizeof(*aidx));
    bidx  = xmalloc_array(B.count, sizeof(*bidx));
    seenA = xmalloc_array((size_t)g->nsite, sizeof(*seenA));
    seenB = xmalloc_array((size_t)g->nsite, sizeof(*seenB));
    cand  = xmalloc_array(
        checked_product((size_t)g->nsite, (size_t)maxdepth),
        sizeof(*cand));
    memset(seenA, 0,
           checked_product((size_t)g->nsite, sizeof(*seenA)));
    memset(seenB, 0,
           checked_product((size_t)g->nsite, sizeof(*seenB)));

    for (i = 0; i < A.count; i++) aidx[i] = i;
    for (i = 0; i < B.count; i++) bidx[i] = i;

    c.A = &A; c.B = &B;
    c.aidx = aidx; c.bidx = bidx;
    c.seenA = seenA; c.seenB = seenB; c.cand = cand;
    c.epoch = 0U; c.nsite = g->nsite; c.cutoff = pr->cutoff;

    answer = subtree(&c, 0, A.count, 0, B.count, -1, 0);

    free(aidx); free(bidx); free(seenA); free(seenB); free(cand);
    bag_free(&A); bag_free(&B);
    free(dist); free(queue); free(seen); free(path);
    return answer;
}

/* Number of L-step self-avoiding walks from the origin to "target". */
static i128 count_walks(int nsteps, const int target[3], size_t cutoff,
                        int verbose)
{
    Lattice g;
    Stabiliser st;
    Problem pr;
    int na, nb, radius, i, origin, tgt;
    int *dist_o, *dist_t, *queue;
    int *reps, nreps = 0;
    int *repsize;
    i128 total = 0;

    if (nsteps < 1) return 0;
    if (nsteps == 1) {
        for (i = 0; i < 12; i++)
            if (STEP[i][0] == target[0] && STEP[i][1] == target[1]
                && STEP[i][2] == target[2]) return 1;
        return 0;
    }

    na = (nsteps + 1) / 2;          /* O  -> X */
    nb = nsteps - na;               /* T  -> X */

    /* A half-walk from the origin stays within |v_i| <= na.  A half-walk from
       the target reaches the join point X, so a vertex k steps along it obeys
       |v_i| <= min(|T_i| + k, na + nb - k); the worst case is k such that the
       two bounds meet, giving (nsteps + |T_i|)/2, rounded up.  Taking less
       than this silently truncates walks at the box boundary. */
    radius = na;
    for (i = 0; i < 3; i++) {
        int mag = target[i] < 0 ? -target[i] : target[i];
        int need = (nsteps + mag + 1) / 2;
        if (mag > radius) radius = mag;
        if (need > radius) radius = need;
    }
    lattice_build(&g, radius);

    origin = site_of(&g, 0, 0, 0);
    tgt    = site_of(&g, target[0], target[1], target[2]);
    if (tgt < 0) die("target is not an f.c.c. lattice point");

    stabiliser_of(target, &st);

    dist_o = xmalloc_array((size_t)g.nsite, sizeof(*dist_o));
    dist_t = xmalloc_array((size_t)g.nsite, sizeof(*dist_t));
    queue  = xmalloc_array((size_t)g.nsite, sizeof(*queue));
    bfs(&g, origin, dist_o, queue);
    bfs(&g, tgt,    dist_t, queue);

    reps    = xmalloc_array((size_t)g.nsite, sizeof(*reps));
    repsize = xmalloc_array((size_t)g.nsite, sizeof(*repsize));
    for (i = 0; i < g.nsite; i++) {
        int osz;
        if (dist_o[i] > na || dist_t[i] > nb) continue;
        if (!orbit_rep(&g, &st, i, &osz)) continue;
        reps[nreps] = i;
        repsize[nreps] = osz;
        nreps++;
    }

    if (verbose)
        fprintf(stderr,
                "  steps=%d split=%d+%d radius=%d sites=%d joins=%d "
                "(|stab|=%d)\n",
                nsteps, na, nb, radius, g.nsite, nreps, st.nstab);

    pr.g = &g; pr.origin = origin; pr.target = tgt;
    pr.nsteps_a = na; pr.nsteps_b = nb;
    pr.cutoff = cutoff; pr.max_walks = HALF_WALK_LIMIT;

#ifdef _OPENMP
#pragma omp parallel for schedule(dynamic, 1) reduction(+:total)
#endif
    for (i = 0; i < nreps; i++)
        total += (i128)repsize[i] * solve_join(&pr, reps[i]);

    free(dist_o); free(dist_t); free(queue); free(reps); free(repsize);
    lattice_free(&g);
    return total;
}

/* ------------------------------------------------------------------ */
/* Reference values and self-test                                      */
/* ------------------------------------------------------------------ */

/* A003287(1..16), f.c.c. SAWs from (0,0,0) to (0,1,1), taken from OEIS. */
static const uint64_t A003287[] = {
    1ULL, 4ULL, 22ULL, 140ULL, 970ULL, 7196ULL, 56092ULL, 452064ULL,
    3735700ULL, 31484244ULL, 269613896ULL, 2339571468ULL, 20529434520ULL,
    181871459580ULL, 1624587752400ULL, 14617165101216ULL
};

/* A001337(1..17), f.c.c. polygons, taken from OEIS. */
static const uint64_t A001337[] = {
    0ULL, 0ULL, 48ULL, 264ULL, 1680ULL, 11640ULL, 86352ULL, 673104ULL,
    5424768ULL, 44828400ULL, 377810928ULL, 3235366752ULL, 28074857616ULL,
    246353214240ULL, 2182457514960ULL, 19495053028800ULL, 175405981214592ULL
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
    int  steps, goal;
    i128 found;
} Brute;

static void brute_rec(Brute *b, int here, int depth)
{
    const int32_t *nb;
    int d;

    if (depth == b->steps) {
        if (here == b->goal) b->found++;
        return;
    }
    b->seen[here] = 1;
    nb = b->g->adj + (long)here * 12;
    for (d = 0; d < 12; d++) {
        int next = nb[d];
        if (next >= 0 && !b->seen[next]) brute_rec(b, next, depth + 1);
    }
    b->seen[here] = 0;
}

static i128 brute_count(int nsteps, const int target[3])
{
    Lattice g;
    Brute b;
    int origin, i, radius = nsteps;

    for (i = 0; i < 3; i++) {
        int need = target[i] < 0 ? -target[i] : target[i];
        if (need > radius) radius = need;
    }
    lattice_build(&g, radius);
    origin = site_of(&g, 0, 0, 0);
    b.g = &g;
    b.seen = xmalloc_array((size_t)g.nsite, sizeof(*b.seen));
    memset(b.seen, 0, (size_t)g.nsite);
    b.steps = nsteps;
    b.goal  = site_of(&g, target[0], target[1], target[2]);
    b.found = 0;
    if (b.goal >= 0) brute_rec(&b, origin, 0);
    free(b.seen);
    lattice_free(&g);
    return b.found;
}

static i128 polygon_count(int n, size_t cutoff, int verbose)
{
    static const int C[3] = { 0, 1, 1 };
    if (n <= 2) return 0;
    return (i128)12 * count_walks(n - 1, C, cutoff, verbose);
}

static int selftest(size_t cutoff)
{
    static const int endpoints[8][3] = {
        { 0, 1, 1 }, { 0, 0, 2 }, { 0, 2, 2 }, { 1, 1, 2 }, { 2, 2, 2 },
        { 0, 0, 4 }, { 0, 2, 4 }, { 2, 2, 4 }   /* large |t_i|: box sizing */
    };
    static const int C[3] = { 0, 1, 1 };
    int n, e, bad = 0, limit;

    printf("[1] against OEIS A003287, walks (0,0,0) -> (0,1,1)\n");
    limit = (int)(sizeof A003287 / sizeof A003287[0]);
    if (limit > 12) limit = 12;                 /* keep the test quick */
    for (n = 1; n <= limit; n++) {
        i128 got = count_walks(n, C, cutoff, 0);
        int ok = (got == (i128)A003287[n - 1]);
        printf("     n=%2d  ", n); print_i128(got);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok) bad = 1;
    }

    printf("[2] against OEIS A001337, polygon counts\n");
    limit = (int)(sizeof A001337 / sizeof A001337[0]);
    if (limit > 13) limit = 13;
    for (n = 1; n <= limit; n++) {
        i128 got = polygon_count(n, cutoff, 0);
        int ok = (got == (i128)A001337[n - 1]);
        printf("     n=%2d  ", n); print_i128(got);
        printf("  %s\n", ok ? "ok" : "MISMATCH");
        if (!ok) bad = 1;
    }

    printf("[3] against brute-force enumeration, several endpoints\n");
    for (e = 0; e < 8; e++) {
        for (n = 2; n <= 9; n++) {
            i128 fast = count_walks(n, endpoints[e], cutoff, 0);
            i128 slow = brute_count(n, endpoints[e]);
            int ok = (fast == slow);
            printf("     (%d,%d,%d) n=%d  ", endpoints[e][0],
                   endpoints[e][1], endpoints[e][2], n);
            print_i128(fast);
            printf("  vs  "); print_i128(slow);
            printf("  %s\n", ok ? "ok" : "MISMATCH");
            if (!ok) bad = 1;
        }
    }

    printf("[4] divisibility, a(n) must be a multiple of 2n and of 12\n");
    for (n = 3; n <= 12; n++) {
        i128 a = polygon_count(n, cutoff, 0);
        int ok = (a % (2 * n) == 0) && (a % 12 == 0);
        printf("     n=%2d  %s\n", n, ok ? "ok" : "MISMATCH");
        if (!ok) bad = 1;
    }

    printf("[5] cut-off invariance, the result must not depend on --cutoff\n");
    {
        size_t trial[4] = { 1U, 64U, 4096U, 1000000U };
        i128 ref = 0;
        int k;
        for (k = 0; k < 4; k++) {
            i128 got = count_walks(10, C, trial[k], 0);
            if (k == 0) ref = got;
            printf("     cutoff=%-8zu ", trial[k]); print_i128(got);
            printf("  %s\n", got == ref ? "ok" : "MISMATCH");
            if (got != ref) bad = 1;
        }
    }

    printf("%s\n", bad ? "SELFTEST FAILED" : "selftest passed");
    return bad;
}

/* ------------------------------------------------------------------ */

static void usage(const char *prog)
{
    fprintf(stderr,
        "usage: %s N\n"
        "       %s --upto N\n"
        "       %s --selftest\n"
        "where %d <= N <= %d\n",
        prog, prog, prog, MIN_INDEX, MAX_INDEX);
}

static int parse_index(const char *text)
{
    char *end = NULL;
    long value;

    if (text == NULL || *text == '\0') die("missing sequence index");
    errno = 0;
    value = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0')
        die("sequence index must be an integer");
    if (value < MIN_INDEX || value > MAX_INDEX)
        die("sequence index out of range");
    return (int)value;
}

static void print_term(int n)
{
    i128 a = polygon_count(n, DEFAULT_CUTOFF, 0);

    if (a < 0 || (u128)a > (u128)UINT64_MAX)
        die("result is outside uint64_t range");
    if (n >= 3 && (a % (2 * n) != 0 || a % 12 != 0))
        die("internal divisibility check failed");
    printf("%d ", n);
    print_i128(a);
    putchar('\n');
}

int main(int argc, char **argv)
{
    int n;

    if (argc == 2 && strcmp(argv[1], "--selftest") == 0)
        return selftest(DEFAULT_CUTOFF) ? EXIT_FAILURE : EXIT_SUCCESS;

    if (argc == 2) {
        print_term(parse_index(argv[1]));
        return EXIT_SUCCESS;
    }

    if (argc == 3 && strcmp(argv[1], "--upto") == 0) {
        int limit = parse_index(argv[2]);
        for (n = MIN_INDEX; n <= limit; n++) print_term(n);
        return EXIT_SUCCESS;
    }

    usage(argv[0]);
    return EXIT_FAILURE;
}
