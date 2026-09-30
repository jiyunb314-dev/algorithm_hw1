/* 정렬 비교 과제 — 퀵 · 병합 · 보고 정렬을 하나의 공통 인터페이스로 묶는다.
 *
 * C에는 interface가 없으므로 함수 포인터를 담은 구조체(SortAlgorithm)를 쓴다.
 * 비교 규약은 표준 라이브러리의 qsort와 같다. 그래서 정렬은 원소의 타입을
 * 모르고, 측정 코드는 (key, tag) 원소로 안정성까지 잴 수 있다.
 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>
#include <stdint.h>

/* a<b면 음수, a==b면 0, a>b면 양수. */
typedef int (*SortCompare)(const void *a, const void *b);

/* 한 번 정렬하는 동안 모인 측정값. 시계와 무관하게 재현되는 값만 담는다. */
typedef struct SortStats {
    size_t compares;   /* 비교 함수를 부른 횟수 */
    size_t moves;      /* 원소를 복사한 횟수 (교환 한 번은 3) */
    size_t extraBytes; /* 입력 배열 밖에 잡은 작업 공간 (바이트) */
    size_t maxDepth;   /* 재귀 깊이의 최댓값. 반복문만 쓰면 1 */
    size_t shuffles;   /* 보고 정렬이 배열을 섞은 횟수. 다른 정렬은 0 */
} SortStats;

typedef struct SortAlgorithm {
    const char *name;
    const char *timeComplexity;  /* 평균 시간복잡도 */
    const char *spaceComplexity; /* 추가 메모리 */
    int stable;                  /* 안정 정렬이라고 주장하는 값. 테스트가 실측과 맞춰 본다 */
    size_t maxN;                 /* 현실적으로 돌려 볼 수 있는 최대 n. 0이면 제한 없음 */
    void (*sort)(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
} SortAlgorithm;

void quickSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void mergeSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);
void bogoSort(void *base, size_t n, size_t size, SortCompare cmp, SortStats *stats);

/* 구현 표. 부르는 쪽(main.c · bench.c · 테스트)은 이 표만 훑는다. */
extern const SortAlgorithm SORT_ALGORITHMS[];
extern const size_t SORT_ALGORITHM_COUNT;

/* 보고 정렬이 섞을 때 쓰는 난수 씨앗. 호출마다 이 값에서 다시 시작하므로
 * 같은 입력 · 같은 씨앗이면 섞는 횟수까지 똑같이 재현된다 (Python 구현과도 같다). */
extern uint32_t bogoSortSeed;

/* xorshift32. Python 쪽(sort.py)과 비트 단위로 같은 수열을 낸다. */
uint32_t sortRandomNext(uint32_t *state);

void sortStatsReset(SortStats *stats);
int sortCompareInt(const void *a, const void *b); /* int 배열용 기본 비교 함수 */

#endif /* SORT_H */
