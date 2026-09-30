/* 퀵 정렬 — Lomuto 분할, 마지막 원소를 피벗으로.
 *
 * 수업 교재의 기본형 그대로다. 피벗을 고르는 요령(중앙값 등)을 일부러 넣지
 * 않았다. 그래야 이미 정렬된 입력에서 O(n^2)로 무너지는 모습을 잴 수 있다.
 */
#include "sortctx.h"

/* a[lo..hi]를 a[hi] 기준으로 나누고 피벗이 놓인 자리를 돌려준다. */
static size_t partition(SortCtx *c, size_t lo, size_t hi) {
    size_t store = lo;
    for (size_t i = lo; i < hi; i++) {
        /* 피벗 이하를 왼쪽으로 모은다. 멀리 건너뛰는 교환이라 안정성이 깨진다. */
        if (sortCompareAt(c, i, hi) <= 0) {
            sortSwap(c, store, i);
            store++;
        }
    }
    sortSwap(c, store, hi);
    return store;
}

/* a[lo..hi] (양끝 포함) */
static void quickSortRange(SortCtx *c, size_t lo, size_t hi) {
    sortEnter(c);
    if (lo < hi) {
        size_t p = partition(c, lo, hi);
        if (p > lo) {
            quickSortRange(c, lo, p - 1);
        }
        quickSortRange(c, p + 1, hi);
    }
    sortLeave(c);
}

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    quickSortRange(&c, 0, n - 1);
    sortEnd(&c);
}
