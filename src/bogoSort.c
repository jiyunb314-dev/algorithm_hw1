/* 보고 정렬(bogosort) — 수업에서 다루지 않은 정렬.
 *
 *   while (정렬되지 않았다) 배열을 무작위로 섞는다;
 *
 * 서로 다른 원소 n개라면 한 번 섞어 정렬될 확률이 1/n!이므로, 기대 섞기 횟수는
 * 약 n!이고 한 번마다 O(n)을 쓴다. 평균 O(n * n!), 최악은 끝이 보장되지 않는다.
 * "나쁜 정렬이 얼마나 나쁠 수 있는가"를 보여 주는 기준선으로 쓴다.
 */
#include "sortctx.h"

uint32_t bogoSortSeed = 20260930u;

/* 이웃끼리만 본다. 처음으로 어긋난 곳에서 멈추므로 무작위 배열은 보통 1~2번이면 끝난다. */
static int isSorted(SortCtx *c, size_t n) {
    for (size_t i = 1; i < n; i++) {
        if (sortCompareAt(c, i - 1, i) > 0) {
            return 0;
        }
    }
    return 1;
}

/* Fisher-Yates. n!가지 순서가 모두 같은 확률로 나온다. */
static void shuffle(SortCtx *c, size_t n, uint32_t *state) {
    for (size_t i = n - 1; i > 0; i--) {
        size_t j = (size_t)(sortRandomNext(state) % (uint32_t)(i + 1));
        sortSwap(c, i, j);
    }
}

void bogoSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    uint32_t state = bogoSortSeed != 0 ? bogoSortSeed : 1u; /* xorshift는 0에서 멈춘다 */
    sortEnter(&c);
    while (!isSorted(&c, n)) {
        shuffle(&c, n, &state);
        if (stats != NULL) {
            stats->shuffles++;
        }
    }
    sortLeave(&c);
    sortEnd(&c);
}
