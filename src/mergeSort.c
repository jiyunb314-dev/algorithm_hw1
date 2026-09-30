/* 병합 정렬 — 위에서 아래로(top-down), 보조 배열 n칸.
 *
 * 입력이 어떤 모양이든 반씩 나누므로 깊이가 log n으로 고정되고, 비교는
 * 최악에도 O(n log n)이다. 그 대가로 원소 n개만큼의 보조 배열을 쓴다.
 */
#include <stdlib.h>

#include "sortctx.h"

/* a[lo..mid)와 a[mid..hi)는 각각 정렬돼 있다. 둘을 buf에 옮긴 뒤 a로 병합한다. */
static void merge(SortCtx *c, char *buf, size_t lo, size_t mid, size_t hi) {
    for (size_t k = lo; k < hi; k++) {
        sortMove(c, buf + (k - lo) * c->size, sortElemAt(c, k));
    }
    size_t i = 0, j = mid - lo, end = hi - lo, k = lo;
    while (i < mid - lo && j < end) {
        /* 같으면 왼쪽을 먼저 내보낸다. 이 '<'(<=가 아니라) 하나가 안정성을 만든다. */
        if (sortCompare(c, buf + j * c->size, buf + i * c->size) < 0) {
            sortMove(c, sortElemAt(c, k++), buf + (j++) * c->size);
        } else {
            sortMove(c, sortElemAt(c, k++), buf + (i++) * c->size);
        }
    }
    while (i < mid - lo) {
        sortMove(c, sortElemAt(c, k++), buf + (i++) * c->size);
    }
    while (j < end) {
        sortMove(c, sortElemAt(c, k++), buf + (j++) * c->size);
    }
}

/* a[lo..hi) */
static void mergeSortRange(SortCtx *c, char *buf, size_t lo, size_t hi) {
    sortEnter(c);
    if (hi - lo >= 2) {
        size_t mid = lo + (hi - lo) / 2;
        mergeSortRange(c, buf, lo, mid);
        mergeSortRange(c, buf, mid, hi);
        merge(c, buf, lo, mid, hi);
    }
    sortLeave(c);
}

void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats) {
    SortCtx c;
    if (!sortBegin(&c, base, n, size, cmp, stats)) {
        return;
    }
    char *buf = (char *)malloc(n * size);
    if (buf != NULL) {
        sortAddExtra(&c, n * size);
        mergeSortRange(&c, buf, 0, n);
        free(buf);
    }
    sortEnd(&c);
}
