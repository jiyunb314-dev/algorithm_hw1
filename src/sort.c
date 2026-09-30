#include "sort.h"

#include <stdlib.h>
#include <string.h>

SortStats sortStats;

void sortStatsReset(void) {
    memset(&sortStats, 0, sizeof(sortStats));
}

static void enterCall(void) {
    sortStats.depth++;
    if (sortStats.depth > sortStats.maxDepth) {
        sortStats.maxDepth = sortStats.depth;
    }
}

static void leaveCall(void) {
    sortStats.depth--;
}

static void swap(int arr[], int a, int b) {
    int temp = arr[a];
    arr[a] = arr[b];
    arr[b] = temp;
}

/* --- 난수 ---------------------------------------------------------------- */

void randomInit(Random *rand, uint32_t seed) {
    rand->state = seed != 0 ? seed : 1u; /* xorshift는 0에서 멈춘다 */
}

uint32_t randomNext(Random *rand) {
    uint32_t x = rand->state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    rand->state = x;
    return x;
}

int randomNextInt(Random *rand, int bound) {
    return (int)(randomNext(rand) % (uint32_t)bound);
}

/* --- 병합 정렬 ----------------------------------------------------------- */

/* arr[left..mid]와 arr[mid+1..right]는 각각 정렬돼 있다. 둘을 L, R에 복사해 두고
 * 앞머리끼리 비교하며 작은 쪽을 arr에 다시 채운다. */
void merge(int arr[], int left, int mid, int right) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    int *L = (int *)malloc((size_t)n1 * sizeof(int));
    int *R = (int *)malloc((size_t)n2 * sizeof(int));
    if (L == NULL || R == NULL) {
        free(L);
        free(R);
        return;
    }
    sortStats.curBytes += (size_t)(n1 + n2) * sizeof(int);
    if (sortStats.curBytes > sortStats.peakBytes) {
        sortStats.peakBytes = sortStats.curBytes;
    }
    for (int i = 0; i < n1; i++) L[i] = arr[left + i];
    for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

    int i = 0, j = 0, k = left;
    while (i < n1 && j < n2) {
        sortStats.compares++;
        /* 같으면 왼쪽을 먼저 내보낸다(<=). 이것이 병합 정렬을 안정 정렬로 만든다. */
        if (L[i] <= R[j]) arr[k++] = L[i++];
        else arr[k++] = R[j++];
    }
    while (i < n1) arr[k++] = L[i++];
    while (j < n2) arr[k++] = R[j++];

    free(L);
    free(R);
    sortStats.curBytes -= (size_t)(n1 + n2) * sizeof(int);
}

void mergeSort(int arr[], int left, int right) {
    if (left >= right) return;
    enterCall();
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid);
    mergeSort(arr, mid + 1, right);
    merge(arr, left, mid, right);
    leaveCall();
}

/* --- 퀵 정렬 ------------------------------------------------------------- */

/* 무작위로 고른 피벗을 맨 뒤로 보낸 뒤 Lomuto 방식으로 나눈다.
 * 피벗보다 "작은" 값만 왼쪽으로 모으므로, 피벗과 같은 값은 모두 오른쪽에 남는다. */
int partitionRandom(int arr[], int low, int high, Random *rand) {
    int pivotIndex = low + randomNextInt(rand, high - low + 1);
    swap(arr, pivotIndex, high);

    int pivot = arr[high];
    int i = low - 1;

    for (int j = low; j < high; j++) {
        sortStats.compares++;
        if (arr[j] < pivot) {
            i++;
            swap(arr, i, j);
        }
    }
    swap(arr, i + 1, high);
    return i + 1;
}

/* 교과서형 퀵 정렬과 다르다: 양쪽을 모두 재귀하지 않고 **작은 쪽만** 재귀하고
 * 큰 쪽은 반복문으로 처리한다. 분할이 아무리 치우쳐도 재귀 깊이가 log n을 넘지
 * 않아 스택 오버플로를 막는다 (Java 코드의 주석과 같은 이유). */
void quickSort(int arr[], int low, int high, Random *rand) {
    if (low >= high) return;
    enterCall();
    while (low < high) {
        int pi = partitionRandom(arr, low, high, rand);
        if (pi - low < high - pi) {
            quickSort(arr, low, pi - 1, rand);
            low = pi + 1;
        } else {
            quickSort(arr, pi + 1, high, rand);
            high = pi - 1;
        }
    }
    leaveCall();
}

/* --- 보고 정렬 ----------------------------------------------------------- */

/* 이웃끼리 비교하다 처음 어긋난 곳에서 멈춘다. */
int isSorted(const int arr[], int n) {
    for (int i = 1; i < n; i++) {
        sortStats.compares++;
        if (arr[i - 1] > arr[i]) return 0;
    }
    return 1;
}

/* Fisher-Yates 셔플. n!가지 순서가 모두 같은 확률로 나온다. */
void shuffle(int arr[], int n, Random *rand) {
    for (int i = n - 1; i > 0; i--) {
        int j = randomNextInt(rand, i + 1);
        swap(arr, i, j);
    }
}

/* 정렬될 때까지 섞는다. 평균 n!번 섞으므로 n이 10을 넘으면 쓸 수 없다. */
void bogoSort(int arr[], int n, Random *rand) {
    while (!isSorted(arr, n)) {
        shuffle(arr, n, rand);
        sortStats.shuffles++;
    }
}
