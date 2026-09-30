/* 정렬 비교 과제 — 병합 · 퀵 · 보고 정렬.
 *
 * 함수 구성은 참고한 Java 코드(Algorithms.java)를 그대로 따른다.
 *   merge / mergeSort(arr, left, right)          구간은 양끝 포함
 *   partitionRandom / quickSort(arr, low, high)  무작위 피벗, 작은 쪽만 재귀
 * Java의 java.util.Random 대신 Random 구조체를 넘긴다. 씨앗이 같으면 C와
 * Python(sort.py)이 같은 난수를 내므로 두 구현의 결과를 대조할 수 있다.
 */
#ifndef SORT_H
#define SORT_H

#include <stddef.h>
#include <stdint.h>

/* --- 난수 (xorshift32) ------------------------------------------------ */

typedef struct Random {
    uint32_t state;
} Random;

void randomInit(Random *rand, uint32_t seed);
uint32_t randomNext(Random *rand);
int randomNextInt(Random *rand, int bound); /* 0 이상 bound 미만 */

/* --- 측정값 ------------------------------------------------------------ */

/* 정렬 한 번 동안 모인 값. 정렬하기 전에 sortStatsReset()으로 비운다. */
typedef struct SortStats {
    long long compares; /* 원소끼리 비교한 횟수 */
    long long shuffles; /* 보고 정렬이 섞은 횟수 */
    size_t curBytes;    /* 지금 잡고 있는 임시 배열의 바이트 */
    size_t peakBytes;   /* 임시 배열이 가장 컸을 때의 바이트 (Java의 "MB used"에 해당) */
    int depth;          /* 지금 재귀 깊이 */
    int maxDepth;       /* 재귀 깊이의 최댓값 (스택 사용량의 대리 지표) */
} SortStats;

extern SortStats sortStats;
void sortStatsReset(void);

/* --- 병합 정렬 --------------------------------------------------------- */

void merge(int arr[], int left, int mid, int right);
void mergeSort(int arr[], int left, int right);

/* --- 퀵 정렬 ----------------------------------------------------------- */

int partitionRandom(int arr[], int low, int high, Random *rand);
void quickSort(int arr[], int low, int high, Random *rand);

/* --- 보고 정렬 (수업에서 다루지 않은 정렬) ----------------------------- */

int isSorted(const int arr[], int n);
void shuffle(int arr[], int n, Random *rand);
void bogoSort(int arr[], int n, Random *rand);

#endif /* SORT_H */
