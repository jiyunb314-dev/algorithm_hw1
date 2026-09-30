/* 유닛 테스트 — 외부 프레임워크 없이 표준 C만 쓴다.
 * 실행: make test-c
 *
 * 구현 표(SORT_ALGORITHMS)를 훑으므로 정렬마다 같은 검사를 받는다.
 * 보고 정렬은 n이 maxN을 넘는 검사를 건너뛴다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

static int checks = 0;
static int failures = 0;

static void check(int ok, const char *algo, const char *name) {
    checks++;
    if (!ok) {
        failures++;
        printf("FAIL  %-10s %s\n", algo, name);
        return;
    }
    printf("ok    %-10s %s\n", algo, name);
}

static int fits(const SortAlgorithm *algo, size_t n) {
    return algo->maxN == 0 || n <= algo->maxN;
}

/* input을 정렬한 결과가 want와 같은지 본다. */
static void expectSorted(const SortAlgorithm *algo, const char *name,
                         const int *input, const int *want, size_t n) {
    int a[16];
    memcpy(a, input, n * sizeof(int));
    algo->sort(a, n, sizeof(int), sortCompareInt, NULL);
    check(n == 0 || memcmp(a, want, n * sizeof(int)) == 0, algo->name, name);
}

static void basicCases(const SortAlgorithm *algo) {
    {
        const int a[] = {6, 8, 5, 9, 10, 1, 7, 2, 4, 3};
        const int want[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
        expectSorted(algo, "섞인 배열", a, want, 10);
    }
    {
        const int a[] = {1, 2, 3, 4, 5};
        const int want[] = {1, 2, 3, 4, 5};
        expectSorted(algo, "이미 정렬된 배열", a, want, 5);
    }
    {
        const int a[] = {5, 4, 3, 2, 1};
        const int want[] = {1, 2, 3, 4, 5};
        expectSorted(algo, "역순 배열", a, want, 5);
    }
    {
        const int a[] = {3, 1, 3, 1, 2};
        const int want[] = {1, 1, 2, 3, 3};
        expectSorted(algo, "중복이 있는 배열", a, want, 5);
    }
    {
        const int a[] = {7, 7, 7, 7};
        const int want[] = {7, 7, 7, 7};
        expectSorted(algo, "모두 같은 값", a, want, 4);
    }
    {
        const int a[] = {-3, 0, -7, 2};
        const int want[] = {-7, -3, 0, 2};
        expectSorted(algo, "음수가 섞인 배열", a, want, 4);
    }
    {
        const int a[] = {42};
        const int want[] = {42};
        expectSorted(algo, "원소 하나", a, want, 1);
    }
    {
        const int a[1] = {0};
        const int want[1] = {0};
        expectSorted(algo, "빈 배열", a, want, 0);
    }
}

/* 표준 라이브러리 qsort와 결과를 맞춰 본다. */
static void againstQsort(const SortAlgorithm *algo) {
    size_t n = fits(algo, 500) ? 500 : algo->maxN;
    int *a = (int *)malloc(n * sizeof(int));
    int *want = (int *)malloc(n * sizeof(int));
    srand(7);
    for (size_t i = 0; i < n; i++) {
        a[i] = rand() % 1000 - 500;
        want[i] = a[i];
    }
    qsort(want, n, sizeof(int), sortCompareInt);
    algo->sort(a, n, sizeof(int), sortCompareInt, NULL);
    check(memcmp(a, want, n * sizeof(int)) == 0, algo->name, "qsort와 같은 결과 (난수)");
    free(a);
    free(want);
}

/* n = 0부터 하나씩 키우며 전부 정렬해 본다. 경계(홀수 · 2의 거듭제곱 아닌 크기)에서 깨지기 쉽다. */
static void sizeSweep(const SortAlgorithm *algo) {
    size_t top = fits(algo, 200) ? 200 : 8;
    Record *a = (Record *)malloc((top + 1) * sizeof(Record));
    int ok = 1;
    for (size_t n = 0; n <= top && ok; n++) {
        makeInput(a, n, INPUT_RANDOM, (unsigned)n);
        algo->sort(a, n, sizeof(Record), recordCompare, NULL);
        ok = recordsSorted(a, n);
    }
    free(a);
    check(ok, algo->name, top == 200 ? "크기 훑기 n = 0..200" : "크기 훑기 n = 0..8");
}

/* 표의 stable 값이 실측과 맞는지. 안정이라 주장하면 지켜야 하고, 불안정이라
 * 주장하면 실제로 순서가 뒤바뀌는 입력이 있어야 한다. */
static void stability(const SortAlgorithm *algo) {
    size_t n = fits(algo, 200) ? 200 : algo->maxN;
    Record *a = (Record *)malloc(n * sizeof(Record));
    for (size_t i = 0; i < n; i++) {
        a[i].key = (int)((i * 7) % 3);
        a[i].tag = (int)i;
    }
    algo->sort(a, n, sizeof(Record), recordCompare, NULL);
    int stable = recordsStable(a, n);
    free(a);
    check(stable == algo->stable, algo->name,
          algo->stable ? "안정 정렬 (주장과 실측 일치)" : "불안정 정렬 (주장과 실측 일치)");
}

static const SortAlgorithm *find(const char *name) {
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        if (strcmp(SORT_ALGORITHMS[k].name, name) == 0) {
            return &SORT_ALGORITHMS[k];
        }
    }
    return NULL;
}

/* 이론과 맞아떨어지는 횟수를 직접 확인한다. */
static void countsMatchTheory(void) {
    enum { N = 100 };
    int a[N];
    SortStats s;

    for (int i = 0; i < N; i++) {
        a[i] = i;
    }
    sortStatsReset(&s);
    quickSort(a, N, sizeof(int), sortCompareInt, &s);
    check(s.compares == N * (N - 1) / 2 && s.maxDepth == N, "quickSort",
          "정렬된 입력: 비교 n(n-1)/2, 깊이 n (최악)");

    sortStatsReset(&s);
    mergeSort(a, N, sizeof(int), sortCompareInt, &s);
    check(s.maxDepth == 8 && s.extraBytes == (N + 1) * sizeof(int), "mergeSort",
          "깊이 ceil(log2 n)+1, 보조 배열 n칸 + tmp");

    sortStatsReset(&s);
    bogoSort(a, N, sizeof(int), sortCompareInt, &s);
    check(s.compares == N - 1 && s.shuffles == 0, "bogoSort",
          "정렬된 입력: 섞지 않고 비교 n-1번 (최선)");
}

/* 같은 씨앗이면 섞는 횟수까지 같다. 씨앗을 바꾸면 달라진다. */
static void bogoIsReproducible(void) {
    const int input[] = {6, 2, 5, 1, 7, 3, 4};
    int a[7];
    SortStats first, second, other;
    const uint32_t saved = bogoSortSeed;

    memcpy(a, input, sizeof(a));
    sortStatsReset(&first);
    bogoSort(a, 7, sizeof(int), sortCompareInt, &first);
    memcpy(a, input, sizeof(a));
    sortStatsReset(&second);
    bogoSort(a, 7, sizeof(int), sortCompareInt, &second);
    bogoSortSeed = saved + 1;
    memcpy(a, input, sizeof(a));
    sortStatsReset(&other);
    bogoSort(a, 7, sizeof(int), sortCompareInt, &other);
    bogoSortSeed = saved;

    check(first.shuffles == second.shuffles && first.shuffles != other.shuffles,
          "bogoSort", "같은 씨앗이면 같은 횟수, 다른 씨앗이면 다른 횟수");
}

int main(void) {
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];
        basicCases(algo);
        againstQsort(algo);
        sizeSweep(algo);
        stability(algo);
    }
    if (find("quickSort") && find("mergeSort") && find("bogoSort")) {
        countsMatchTheory();
        bogoIsReproducible();
    }

    printf("\n%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
