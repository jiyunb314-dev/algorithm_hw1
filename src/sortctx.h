/* 정렬 구현들이 함께 쓰는 작업 문맥 — 구현 전용 헤더.
 * 정렬을 부르는 코드(main.c, bench.c, 테스트)는 이 파일을 include하지 않는다.
 */
#ifndef SORTCTX_H
#define SORTCTX_H

#include <stddef.h>

#include "sort.h"

typedef struct SortCtx {
    char *base;       /* 배열의 첫 바이트 */
    size_t size;      /* 원소 한 개의 바이트 수 */
    SortCompare cmp;
    SortStats *stats; /* NULL이면 측정하지 않는다 */
    char *tmp;        /* 교환할 때 거쳐 가는 원소 한 칸 */
    size_t depth;     /* 지금 재귀 깊이 */
} SortCtx;

/* 정렬할 것이 없거나(n < 2) 메모리를 못 잡으면 0을 돌려준다.
 * 그때는 sortEnd를 부르지 않는다. */
int sortBegin(SortCtx *c, void *base, size_t n, size_t size,
              SortCompare cmp, SortStats *stats);
void sortEnd(SortCtx *c);

char *sortElemAt(const SortCtx *c, size_t i);
int sortCompare(SortCtx *c, const void *a, const void *b); /* 세면서 비교한다 */
int sortCompareAt(SortCtx *c, size_t i, size_t j);         /* a[i]와 a[j] */
void sortMove(SortCtx *c, void *dst, const void *src);
void sortSwap(SortCtx *c, size_t i, size_t j);            /* i == j면 아무것도 안 한다 */
void sortAddExtra(SortCtx *c, size_t bytes);

/* 재귀 함수의 입구와 출구에서 부른다. maxDepth를 갱신한다. */
void sortEnter(SortCtx *c);
void sortLeave(SortCtx *c);

#endif /* SORTCTX_H */
