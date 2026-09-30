/* 정렬 비교 — 퀵 / 병합 / 보고.
 *
 *   make run                 작은 배열 하나로 세 정렬을 돌린다 (Python과 같은 출력)
 *   make bench               사람이 읽는 비교 표
 *   ./src/main.out --csv     같은 측정을 CSV로 (tools/plot.py가 쓴다)
 *
 * 부르는 쪽은 정렬 이름을 하나도 적지 않는다. 구현 표(SORT_ALGORITHMS)를
 * 훑을 뿐이다. 무엇을 잴지도 아래 SPECS 한 곳에만 적는다.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bench.h"
#include "sort.h"

#define SEED 20260930u

/* --- 무엇을 잴 것인가 -------------------------------------------------- */

typedef struct Spec {
    const char *scope; /* kinds · growth · bogo · dup */
    InputKind kind;
    size_t n;
    int reps;   /* 같은 입력을 몇 번 돌려 시간을 평균 낼지 */
    int trials; /* 입력(과 보고 정렬의 씨앗)을 바꿔 가며 몇 번 잴지 */
} Spec;

static const Spec SPECS[] = {
    {"kinds", INPUT_RANDOM, 4000, 3, 1},
    {"kinds", INPUT_SORTED, 4000, 3, 1},
    {"kinds", INPUT_REVERSED, 4000, 3, 1},
    {"kinds", INPUT_FEW_UNIQUE, 4000, 3, 1},
    {"growth", INPUT_RANDOM, 1000, 3, 1},
    {"growth", INPUT_RANDOM, 2000, 3, 1},
    {"growth", INPUT_RANDOM, 4000, 3, 1},
    {"growth", INPUT_RANDOM, 8000, 3, 1},
    {"growth", INPUT_RANDOM, 16000, 3, 1},
    {"growth", INPUT_RANDOM, 32000, 3, 1},
    {"growth", INPUT_RANDOM, 64000, 3, 1},
    {"growth", INPUT_SORTED, 1000, 1, 1},
    {"growth", INPUT_SORTED, 2000, 1, 1},
    {"growth", INPUT_SORTED, 4000, 1, 1},
    {"growth", INPUT_SORTED, 8000, 1, 1},
    {"bogo", INPUT_RANDOM, 2, 1, 10},
    {"bogo", INPUT_RANDOM, 3, 1, 10},
    {"bogo", INPUT_RANDOM, 4, 1, 10},
    {"bogo", INPUT_RANDOM, 5, 1, 10},
    {"bogo", INPUT_RANDOM, 6, 1, 10},
    {"bogo", INPUT_RANDOM, 7, 1, 10},
    {"bogo", INPUT_RANDOM, 8, 1, 10},
    {"bogo", INPUT_RANDOM, 9, 1, 10},
    {"bogo", INPUT_RANDOM, 10, 1, 10},
    {"dup", INPUT_FEW_UNIQUE, 10, 1, 10},
};

static const size_t SPEC_COUNT = sizeof(SPECS) / sizeof(SPECS[0]);

/* 한 정렬 × 한 Spec의 결과. trials번 잰 것을 평균 낸다. */
typedef struct Row {
    const SortAlgorithm *algo;
    int skipped;       /* n이 algo->maxN을 넘어 돌리지 않았다 */
    double millis;
    double compares;
    double moves;
    double shuffles;
    size_t shufflesMin;
    size_t shufflesMax;
    size_t extraBytes;
    size_t maxDepth;
    int sortedTrials;
    int stableTrials;
} Row;

static Row measure(const Spec *spec, const SortAlgorithm *algo) {
    Row row;
    memset(&row, 0, sizeof(row));
    row.algo = algo;
    row.shufflesMin = (size_t)-1;
    if (algo->maxN != 0 && spec->n > algo->maxN) {
        row.skipped = 1;
        return row;
    }
    Record *input = (Record *)malloc(spec->n * sizeof(Record));
    if (input == NULL) {
        row.skipped = 1;
        return row;
    }
    for (int t = 0; t < spec->trials; t++) {
        /* 입력과 섞기 씨앗을 함께 바꾼다. 세 정렬은 회차마다 같은 입력을 받는다. */
        makeInput(input, spec->n, spec->kind, SEED + (unsigned)t);
        bogoSortSeed = SEED + (uint32_t)t;
        BenchResult r = benchRun(algo, input, spec->n, spec->reps);
        row.millis += r.millis;
        row.compares += (double)r.stats.compares;
        row.moves += (double)r.stats.moves;
        row.shuffles += (double)r.stats.shuffles;
        if (r.stats.shuffles < row.shufflesMin) {
            row.shufflesMin = r.stats.shuffles;
        }
        if (r.stats.shuffles > row.shufflesMax) {
            row.shufflesMax = r.stats.shuffles;
        }
        row.extraBytes = r.stats.extraBytes;
        if (r.stats.maxDepth > row.maxDepth) {
            row.maxDepth = r.stats.maxDepth;
        }
        row.sortedTrials += r.sorted;
        row.stableTrials += r.stable;
    }
    row.millis /= spec->trials;
    row.compares /= spec->trials;
    row.moves /= spec->trials;
    row.shuffles /= spec->trials;
    bogoSortSeed = SEED;
    free(input);
    return row;
}

/* 결과 한 줄을 받아 가는 곳. 표로 찍을지 CSV로 찍을지만 다르다. */
typedef void (*RowSink)(const Spec *spec, const Row *row);

static void measureAll(RowSink sink, void (*onSpec)(const Spec *spec)) {
    for (size_t s = 0; s < SPEC_COUNT; s++) {
        if (onSpec != NULL) {
            onSpec(&SPECS[s]);
        }
        for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
            Row row = measure(&SPECS[s], &SORT_ALGORITHMS[k]);
            sink(&SPECS[s], &row);
        }
    }
}

/* --- 사람이 읽는 표 ---------------------------------------------------- */

#define ROW_HEADER "알고리즘       시간(ms)          비교          이동      섞기   메모리 재귀깊이  정렬 안정\n"
#define ROW_RULE   "--------------------------------------------------------------------------------------------\n"

static void tableRow(const Spec *spec, const Row *row) {
    if (row->skipped) {
        printf("%-11s  (n > %zu이라 돌리지 않음)\n", row->algo->name, row->algo->maxN);
        return;
    }
    printf("%-11s %11.3f %13.0f %13.0f %9.0f %7zu B %8zu %3d/%d %2d/%d\n", row->algo->name,
           row->millis, row->compares, row->moves, row->shuffles, row->extraBytes,
           row->maxDepth, row->sortedTrials, spec->trials, row->stableTrials, spec->trials);
}

static void tableSpecHeader(const Spec *spec) {
    static const char *lastScope = NULL;

    if (lastScope == NULL || strcmp(lastScope, spec->scope) != 0) {
        if (strcmp(spec->scope, "kinds") == 0) {
            printf("\n== 입력 모양별 (n = %zu, %d회 평균) ==\n", spec->n, spec->reps);
        } else if (strcmp(spec->scope, "growth") == 0) {
            printf("\n== n을 키우며 ==\n");
        } else if (strcmp(spec->scope, "bogo") == 0) {
            printf("\n== 작은 n에서 셋 모두 (무작위, 입력 %d벌 평균) ==\n", spec->trials);
        } else {
            printf("\n== 중복이 있는 작은 입력 (안정성, 입력 %d벌) ==\n", spec->trials);
        }
        lastScope = spec->scope;
    }
    printf("\n[%s, n = %zu]\n%s%s", inputKindName(spec->kind), spec->n, ROW_HEADER, ROW_RULE);
}

static void reportTable(void) {
    printf("=== 정렬 비교: 퀵 · 병합 · 보고 ===\n");
    printf("원소는 (key, tag) %zu바이트. key로 정렬하고 tag로 안정성을 본다.\n\n",
           sizeof(Record));
    printf("알고리즘    시간복잡도  메모리  안정성    최대 n\n%s", ROW_RULE);
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *a = &SORT_ALGORITHMS[k];
        printf("%-11s %-11s %-7s %-9s ", a->name, a->timeComplexity, a->spaceComplexity,
               a->stable ? "stable" : "unstable");
        if (a->maxN == 0) {
            printf("-\n");
        } else {
            printf("%zu\n", a->maxN);
        }
    }
    measureAll(tableRow, tableSpecHeader);
    printf("\n정렬·안정 열은 '그렇게 나온 회차 / 전체 회차'다.\n");
}

/* --- 기계가 읽는 CSV --------------------------------------------------- */

static void csvRow(const Spec *spec, const Row *row) {
    if (row->skipped) {
        return;
    }
    printf("%s,%s,%zu,%s,%d,%.4f,%.1f,%.1f,%zu,%zu,%.1f,%zu,%zu,%d,%d\n", spec->scope,
           inputKindKey(spec->kind), spec->n, row->algo->name, spec->trials, row->millis,
           row->compares, row->moves, row->extraBytes, row->maxDepth, row->shuffles,
           row->shufflesMin, row->shufflesMax, row->sortedTrials, row->stableTrials);
}

static void reportCsv(void) {
    printf("scope,input,n,algo,trials,millis,compares,moves,extraBytes,maxDepth,"
           "shuffles,shufflesMin,shufflesMax,sortedTrials,stableTrials\n");
    measureAll(csvRow, NULL);
}

/* --- 작은 예제: Python(main.py)과 글자 하나까지 같은 출력을 낸다 ------------ */

static void demo(void) {
    static const int INPUT[] = {6, 2, 5, 1, 7, 3, 4};
    const size_t n = sizeof(INPUT) / sizeof(INPUT[0]);

    printf("input: ");
    for (size_t i = 0; i < n; i++) {
        printf(" %d", INPUT[i]);
    }
    printf("\n");
    for (size_t k = 0; k < SORT_ALGORITHM_COUNT; k++) {
        const SortAlgorithm *algo = &SORT_ALGORITHMS[k];
        int a[sizeof(INPUT) / sizeof(INPUT[0])];
        SortStats stats;
        memcpy(a, INPUT, sizeof(INPUT));
        sortStatsReset(&stats);
        algo->sort(a, n, sizeof(int), sortCompareInt, &stats);

        printf("%-9s", algo->name);
        for (size_t i = 0; i < n; i++) {
            printf(" %d", a[i]);
        }
        printf("  compares=%zu moves=%zu depth=%zu shuffles=%zu\n", stats.compares,
               stats.moves, stats.maxDepth, stats.shuffles);
    }
}

int main(int argc, char **argv) {
    if (argc > 1 && strcmp(argv[1], "--csv") == 0) {
        reportCsv();
    } else if (argc > 1 && strcmp(argv[1], "--bench") == 0) {
        reportTable();
    } else {
        demo();
    }
    return 0;
}
