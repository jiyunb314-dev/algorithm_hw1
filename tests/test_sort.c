/* 유닛 테스트 — 외부 프레임워크 없이 표준 C만 쓴다.
 * 실행: make test-c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "data.h"
#include "sort.h"

static int checks = 0;
static int failures = 0;

static void check(int ok, const char *name) {
    checks++;
    if (!ok) {
        failures++;
        printf("FAIL  %s\n", name);
        return;
    }
    printf("ok    %s\n", name);
}

static int cmpInt(const void *a, const void *b) {
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

enum { MERGE, QUICK, BOGO, ALGO_COUNT };
static const char *ALGO_NAMES[] = {"mergeSort", "quickSort", "bogoSort"};

static void runSort(int algo, int arr[], int n, Random *rand) {
    sortStatsReset();
    if (algo == MERGE) mergeSort(arr, 0, n - 1);
    else if (algo == QUICK) quickSort(arr, 0, n - 1, rand);
    else bogoSort(arr, n, rand);
}

/* arr을 정렬한 결과가 qsort의 결과와 같은지 본다. */
static int sortsLikeQsort(int algo, const int input[], int n, Random *rand) {
    int *got = (int *)malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    int *want = (int *)malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
    memcpy(got, input, (size_t)n * sizeof(int));
    memcpy(want, input, (size_t)n * sizeof(int));
    runSort(algo, got, n, rand);
    qsort(want, (size_t)n, sizeof(int), cmpInt);
    int ok = memcmp(got, want, (size_t)n * sizeof(int)) == 0;
    free(got);
    free(want);
    return ok;
}

static void basicCases(int algo) {
    static const int shuffled[] = {6, 8, 5, 9, 10, 1, 7, 2, 4, 3};
    static const int sorted[] = {1, 2, 3, 4, 5};
    static const int reversed[] = {5, 4, 3, 2, 1};
    static const int dups[] = {3, 1, 3, 1, 2};
    static const int equal[] = {7, 7, 7, 7};
    static const int negative[] = {-3, 0, -7, 2};
    static const int one[] = {42};
    char name[80];
    Random rand;
    randomInit(&rand, 1);

    struct { const char *label; const int *a; int n; } cases[] = {
        {"섞인 배열", shuffled, 10}, {"이미 정렬된 배열", sorted, 5},
        {"역순 배열", reversed, 5},  {"중복이 있는 배열", dups, 5},
        {"모두 같은 값", equal, 4},  {"음수가 섞인 배열", negative, 4},
        {"원소 하나", one, 1},       {"빈 배열", one, 0},
    };
    for (size_t i = 0; i < sizeof(cases) / sizeof(cases[0]); i++) {
        snprintf(name, sizeof(name), "%-9s %s", ALGO_NAMES[algo], cases[i].label);
        check(sortsLikeQsort(algo, cases[i].a, cases[i].n, &rand), name);
    }
}

/* n을 0부터 키우며 무작위 배열을 전부 정렬해 본다. */
static void sizeSweep(int algo, int top) {
    char name[80];
    Random rand;
    randomInit(&rand, 7);
    int ok = 1;
    for (int n = 0; n <= top && ok; n++) {
        int *a = generateRandom(n, &rand);
        ok = sortsLikeQsort(algo, a, n, &rand);
        free(a);
    }
    snprintf(name, sizeof(name), "%-9s 무작위 배열 n = 0..%d (qsort와 대조)", ALGO_NAMES[algo], top);
    check(ok, name);
}

/* 실험에 쓰는 여섯 가지 입력을 병합 · 퀵 정렬이 모두 정렬하는지. */
static void allInputTypes(void) {
    const int n = 2000;
    Random rand;
    randomInit(&rand, 3);
    int *inputs[] = {generateRandom(n, &rand),       generateSorted(n),
                     generateReverseSorted(n),       generateNearlySorted(n, &rand),
                     generateFewUnique(n, &rand),    generateAllEqual(n)};
    int ok = 1;
    for (int i = 0; i < 6; i++) {
        ok = ok && sortsLikeQsort(MERGE, inputs[i], n, &rand)
                && sortsLikeQsort(QUICK, inputs[i], n, &rand);
        free(inputs[i]);
    }
    check(ok, "병합 · 퀵 정렬: 여섯 가지 입력 모두 정렬 (n = 2000)");
}

static void generators(void) {
    const int n = 1000;
    Random rand;
    randomInit(&rand, 5);
    int *s = generateSorted(n), *r = generateReverseSorted(n);
    int *f = generateFewUnique(n, &rand), *e = generateAllEqual(n);
    int *x = generateRandom(n, &rand);
    int ok = s[0] == 0 && s[n - 1] == n - 1 && r[0] == n - 1 && r[n - 1] == 0;
    for (int i = 0; i < n; i++) {
        ok = ok && f[i] >= 0 && f[i] < 5 && e[i] == 42 && x[i] >= 0 && x[i] < 1000000;
    }
    check(ok, "입력 생성 함수: 정렬 · 역순 · 값 범위");
    free(s); free(r); free(f); free(e); free(x);
}

/* 이론과 맞아떨어지는 값들을 직접 확인한다. */
static void theory(void) {
    const int n = 1024;
    Random rand;
    randomInit(&rand, 9);

    int *a = generateSorted(n);
    sortStatsReset();
    mergeSort(a, 0, n - 1);
    check(sortStats.maxDepth == 10 && sortStats.peakBytes == (size_t)n * sizeof(int),
          "mergeSort 깊이 log2 n, 임시 배열 최대 n칸");
    free(a);

    /* 모두 같은 값: 피벗보다 작은 값이 없어 매번 한 칸만 줄어든다 → 비교 n(n-1)/2. */
    a = generateAllEqual(n);
    sortStatsReset();
    quickSort(a, 0, n - 1, &rand);
    check(sortStats.compares == (long long)n * (n - 1) / 2,
          "quickSort 모두 같은 값: 비교 n(n-1)/2 (최악)");
    check(sortStats.maxDepth <= 11,
          "quickSort 작은 쪽만 재귀: 최악에도 깊이 log2 n 이하");
    free(a);

    a = generateSorted(n);
    sortStatsReset();
    bogoSort(a, n, &rand);
    check(sortStats.compares == n - 1 && sortStats.shuffles == 0,
          "bogoSort 정렬된 입력: 섞지 않고 비교 n-1번 (최선)");
    free(a);
}

/* 같은 씨앗이면 섞는 횟수까지 같다. 씨앗을 바꾸면 달라진다. */
static void reproducible(void) {
    const int input[] = {6, 2, 5, 1, 7, 3, 4};
    long long counts[3];
    uint32_t seeds[3] = {11, 11, 12};
    for (int k = 0; k < 3; k++) {
        int a[7];
        Random rand;
        memcpy(a, input, sizeof(a));
        randomInit(&rand, seeds[k]);
        runSort(BOGO, a, 7, &rand);
        counts[k] = sortStats.shuffles;
    }
    check(counts[0] == counts[1] && counts[0] != counts[2],
          "bogoSort 같은 씨앗이면 같은 섞기 횟수, 다른 씨앗이면 다름");
}

int main(void) {
    for (int algo = 0; algo < ALGO_COUNT; algo++) {
        basicCases(algo);
        sizeSweep(algo, algo == BOGO ? 8 : 300);
    }
    allInputTypes();
    generators();
    theory();
    reproducible();

    printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
