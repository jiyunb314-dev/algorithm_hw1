#include "sort.h"

#include <stdlib.h>
#include <string.h>

#include "sortctx.h"

const SortAlgorithm SORT_ALGORITHMS[] = {
    {"quickSort", "O(n log n)", "O(1)", 0, 0, quickSort},
    {"mergeSort", "O(n log n)", "O(n)", 1, 0, mergeSort},
    /* n = 10이면 평균 10! = 3,628,800번 섞는다. 그 위로는 기다릴 수 없다. */
    {"bogoSort", "O(n * n!)", "O(1)", 0, 10, bogoSort},
};

const size_t SORT_ALGORITHM_COUNT = sizeof(SORT_ALGORITHMS) / sizeof(SORT_ALGORITHMS[0]);

void sortStatsReset(SortStats *stats) {
    memset(stats, 0, sizeof(*stats));
}

int sortCompareInt(const void *a, const void *b) {
    int x = *(const int *)a;
    int y = *(const int *)b;
    return (x > y) - (x < y);
}

uint32_t sortRandomNext(uint32_t *state) {
    uint32_t x = *state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    *state = x;
    return x;
}

int sortBegin(SortCtx *c, void *base, size_t n, size_t size,
              SortCompare cmp, SortStats *stats) {
    if (n < 2 || size == 0) {
        return 0;
    }
    c->base = (char *)base;
    c->size = size;
    c->cmp = cmp;
    c->stats = stats;
    c->depth = 0;
    c->tmp = (char *)malloc(size);
    if (c->tmp == NULL) {
        return 0;
    }
    sortAddExtra(c, size);
    return 1;
}

void sortEnd(SortCtx *c) {
    free(c->tmp);
    c->tmp = NULL;
}

char *sortElemAt(const SortCtx *c, size_t i) {
    return c->base + i * c->size;
}

int sortCompare(SortCtx *c, const void *a, const void *b) {
    if (c->stats != NULL) {
        c->stats->compares++;
    }
    return c->cmp(a, b);
}

int sortCompareAt(SortCtx *c, size_t i, size_t j) {
    return sortCompare(c, sortElemAt(c, i), sortElemAt(c, j));
}

void sortMove(SortCtx *c, void *dst, const void *src) {
    memcpy(dst, src, c->size);
    if (c->stats != NULL) {
        c->stats->moves++;
    }
}

void sortSwap(SortCtx *c, size_t i, size_t j) {
    if (i == j) {
        return;
    }
    sortMove(c, c->tmp, sortElemAt(c, i));
    sortMove(c, sortElemAt(c, i), sortElemAt(c, j));
    sortMove(c, sortElemAt(c, j), c->tmp);
}

void sortAddExtra(SortCtx *c, size_t bytes) {
    if (c->stats != NULL) {
        c->stats->extraBytes += bytes;
    }
}

void sortEnter(SortCtx *c) {
    c->depth++;
    if (c->stats != NULL && c->depth > c->stats->maxDepth) {
        c->stats->maxDepth = c->depth;
    }
}

void sortLeave(SortCtx *c) {
    c->depth--;
}
