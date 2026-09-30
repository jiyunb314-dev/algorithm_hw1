/* 정렬 비교 실험 — 병합 · 퀵 · 보고 정렬.
 *
 *   make run                  아래 main의 실험 전체 (Java 코드의 main과 같은 흐름)
 *   ./src/main.out --demo     작은 배열 하나 (Python의 main.py와 같은 출력)
 *
 * 배열 크기 × 입력 종류마다 같은 입력을 복사해 세 정렬에 넘기고, 걸린 시간과
 * 임시 배열 메모리, 비교 횟수, 재귀 깊이를 찍는다. 보고 정렬은 n <= 10에서만 돌린다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "data.h"
#include "sort.h"

#define SEED 2026193117u
#define BOGO_MAX_N 10

static double elapsedMs(clock_t start, clock_t end) {
    return (double)(end - start) * 1000.0 / CLOCKS_PER_SEC;
}

static double toMB(size_t bytes) {
    return (double)bytes / (1024.0 * 1024.0);
}

static int *copyOf(const int base[], int n) {
    int *arr = (int *)malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    memcpy(arr, base, (size_t)n * sizeof(int));
    return arr;
}

static int *generate(const char *type, int n, Random *rand) {
    if (strcmp(type, "Random") == 0) return generateRandom(n, rand);
    if (strcmp(type, "Sorted") == 0) return generateSorted(n);
    if (strcmp(type, "ReverseSorted") == 0) return generateReverseSorted(n);
    if (strcmp(type, "NearlySorted") == 0) return generateNearlySorted(n, rand);
    if (strcmp(type, "FewUnique") == 0) return generateFewUnique(n, rand);
    return generateAllEqual(n);
}

/* 결과가 정렬됐는지 본다. sort.c의 isSorted와 달리 비교 횟수를 세지 않는다. */
static int verifySorted(const int arr[], int n) {
    for (int i = 1; i < n; i++) {
        if (arr[i - 1] > arr[i]) return 0;
    }
    return 1;
}

/* 한 줄 출력. 정렬이 틀렸으면 바로 보이게 표시한다. */
static void printResult(const char *name, double ms, const int arr[], int n) {
    printf("%-12s %10.3f ms, %.2f MB used, %12lld compares, depth %3d%s",
           name, ms, toMB(sortStats.peakBytes), sortStats.compares, sortStats.maxDepth,
           verifySorted(arr, n) ? "" : "  [NOT SORTED]");
}

static void experiment(void) {
    static const int sizes[] = {10, 100, 1000, 10000, 50000, 100000};
    static const char *dataTypes[] = {"Random", "Sorted", "ReverseSorted",
                                      "NearlySorted", "FewUnique", "AllEqual"};
    const int sizeCount = (int)(sizeof(sizes) / sizeof(sizes[0]));
    const int typeCount = (int)(sizeof(dataTypes) / sizeof(dataTypes[0]));
    Random rand;
    randomInit(&rand, SEED);

    for (int s = 0; s < sizeCount; s++) {
        int n = sizes[s];
        printf("=== Array size: %d ===\n", n);
        for (int t = 0; t < typeCount; t++) {
            int *base = generate(dataTypes[t], n, &rand);
            int *a = copyOf(base, n);
            int *b = copyOf(base, n);
            int *c = copyOf(base, n);
            clock_t start, end;

            printf("Input: %s\n", dataTypes[t]);

            sortStatsReset();
            start = clock();
            mergeSort(a, 0, n - 1);
            end = clock();
            printResult("Merge Sort:", elapsedMs(start, end), a, n);
            printf("\n");

            sortStatsReset();
            start = clock();
            quickSort(b, 0, n - 1, &rand);
            end = clock();
            printResult("Quick Sort:", elapsedMs(start, end), b, n);
            printf("\n");

            if (n <= BOGO_MAX_N) {
                sortStatsReset();
                start = clock();
                bogoSort(c, n, &rand);
                end = clock();
                printResult("Bogo Sort:", elapsedMs(start, end), c, n);
                printf(", %lld shuffles\n", sortStats.shuffles);
            } else {
                printf("Bogo Sort:   (skipped, too slow)\n");
            }
            printf("\n");
            free(base);
            free(a);
            free(b);
            free(c);
        }
        printf("--------------------------------------------\n");
    }
}

/* 보고 정렬만 n = 1..10에서 10번씩 돌려 섞기 횟수를 n!과 견준다. */
static void bogoExperiment(void) {
    const int trials = 10;
    Random rand;
    randomInit(&rand, SEED);
    double factorial = 1.0;

    printf("=== Bogo Sort: Random input, %d trials each ===\n", trials);
    printf(" n          n!   avg shuffles    min shuffles    max shuffles   avg compares   avg ms\n");
    for (int n = 1; n <= BOGO_MAX_N; n++) {
        factorial *= n;
        double sumShuffles = 0, sumCompares = 0, sumMs = 0;
        long long minShuffles = -1, maxShuffles = 0;
        for (int t = 0; t < trials; t++) {
            int *arr = generateRandom(n, &rand);
            sortStatsReset();
            clock_t start = clock();
            bogoSort(arr, n, &rand);
            clock_t end = clock();
            sumMs += elapsedMs(start, end);
            sumShuffles += (double)sortStats.shuffles;
            sumCompares += (double)sortStats.compares;
            if (minShuffles < 0 || sortStats.shuffles < minShuffles) minShuffles = sortStats.shuffles;
            if (sortStats.shuffles > maxShuffles) maxShuffles = sortStats.shuffles;
            free(arr);
        }
        printf("%2d %11.0f %14.1f %15lld %15lld %14.1f %8.3f\n", n, factorial,
               sumShuffles / trials, minShuffles, maxShuffles, sumCompares / trials,
               sumMs / trials);
    }
}

/* 작은 배열 하나. Python(main.py)과 글자 하나까지 같은 출력을 낸다. */
static void demo(void) {
    static const int INPUT[] = {6, 2, 5, 1, 7, 3, 4};
    const int n = (int)(sizeof(INPUT) / sizeof(INPUT[0]));
    const char *names[] = {"mergeSort", "quickSort", "bogoSort"};

    printf("input:");
    for (int i = 0; i < n; i++) printf(" %d", INPUT[i]);
    printf("\n");
    for (int k = 0; k < 3; k++) {
        int a[sizeof(INPUT) / sizeof(INPUT[0])];
        Random rand;
        memcpy(a, INPUT, sizeof(INPUT));
        randomInit(&rand, SEED);
        sortStatsReset();
        if (k == 0) mergeSort(a, 0, n - 1);
        else if (k == 1) quickSort(a, 0, n - 1, &rand);
        else bogoSort(a, n, &rand);

        printf("%-9s", names[k]);
        for (int i = 0; i < n; i++) printf(" %d", a[i]);
        printf("  compares=%lld depth=%d shuffles=%lld\n", sortStats.compares,
               sortStats.maxDepth, sortStats.shuffles);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--demo") == 0) {
        demo();
        return 0;
    }
    experiment();
    bogoExperiment();
    return 0;
}
