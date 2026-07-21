/*
 * OEIS A001411: number of n-step self-avoiding walks on the square lattice.
 *
 * Copyright (c) 2026 manman4
 * SPDX-License-Identifier: MIT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * This is an independent depth-first-search implementation written from the
 * definition of a self-avoiding walk. It contains no SAWdoubler source code.
 */

#include <errno.h>
#include <inttypes.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum {
    /* Every SAW is nonbacktracking, so for n <= 40 its count is at most
       4*3^(n-1) <= 4*3^39 = 16210220612075905068 < UINT64_MAX. */
    MAXIMUM_SUPPORTED_N = 40,
    ROTATION_FACTOR = 4
};

static const int dx[4] = {1, -1, 0, 0};
static const int dy[4] = {0, 0, 1, -1};

static uint8_t *visited;
static uint64_t *counts;
static ptrdiff_t offsets[4];
static int maximum;

static void enumerate(size_t position, int length)
{
    int direction;

    ++counts[length];
    if (length == maximum)
        return;

    for (direction = 0; direction < 4; ++direction) {
        const size_t next =
            (size_t)((ptrdiff_t)position + offsets[direction]);

        if (visited[next] == 0) {
            visited[next] = 1;
            enumerate(next, length + 1);
            visited[next] = 0;
        }
    }
}

static int parse_maximum(const char *text, int *value)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || parsed < 0 ||
        parsed > MAXIMUM_SUPPORTED_N)
        return 0;

    *value = (int)parsed;
    return 1;
}

static int walk_count(int length, uint64_t *result)
{
    if (length == 0) {
        *result = 1;
        return 1;
    }

    if (counts[length] > UINT64_MAX / ROTATION_FACTOR)
        return 0;

    *result = counts[length] * ROTATION_FACTOR;
    return 1;
}

int main(int argc, char **argv)
{
    size_t side;
    size_t site_count;
    size_t origin;
    size_t first;
    uint64_t result;
    int print_all = 0;
    int n;

    if (argc == 2) {
        if (!parse_maximum(argv[1], &maximum)) {
            fprintf(stderr, "n must be an integer from 0 to %d\n",
                    MAXIMUM_SUPPORTED_N);
            return EXIT_FAILURE;
        }
    } else if (argc == 3 && strcmp(argv[1], "--upto") == 0) {
        if (!parse_maximum(argv[2], &maximum)) {
            fprintf(stderr, "N must be an integer from 0 to %d\n",
                    MAXIMUM_SUPPORTED_N);
            return EXIT_FAILURE;
        }
        print_all = 1;
    } else {
        fprintf(stderr, "usage: %s n\n", argv[0]);
        fprintf(stderr, "       %s --upto N\n", argv[0]);
        return EXIT_FAILURE;
    }

    side = (size_t)(2 * maximum + 1);
    site_count = side * side;
    visited = calloc(site_count, sizeof(*visited));
    counts = calloc((size_t)maximum + 1, sizeof(*counts));
    if (visited == NULL || counts == NULL) {
        fprintf(stderr, "not enough memory for n=%d\n", maximum);
        free(counts);
        free(visited);
        return EXIT_FAILURE;
    }

    counts[0] = 1;
    if (maximum >= 1) {
        int direction;

        for (direction = 0; direction < 4; ++direction)
            offsets[direction] =
                (ptrdiff_t)dx[direction] * (ptrdiff_t)side + dy[direction];

        origin = (size_t)maximum * side + (size_t)maximum;
        first = origin + side; /* Fix the first step as (1,0). */
        visited[origin] = 1;
        visited[first] = 1;
        enumerate(first, 1);
    }

    if (print_all) {
        for (n = 0; n <= maximum; ++n) {
            if (!walk_count(n, &result)) {
                fprintf(stderr, "walk count overflow at n=%d\n", n);
                free(counts);
                free(visited);
                return EXIT_FAILURE;
            }
            printf("%d %" PRIu64 "\n", n, result);
        }
    } else {
        if (!walk_count(maximum, &result)) {
            fprintf(stderr, "walk count overflow at n=%d\n", maximum);
            free(counts);
            free(visited);
            return EXIT_FAILURE;
        }
        printf("%d %" PRIu64 "\n", maximum, result);
    }

    free(counts);
    free(visited);
    return EXIT_SUCCESS;
}
