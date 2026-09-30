/* 정렬 셋을 같은 잣대로 재는 도구.
 * 정렬은 자기가 측정당하는 줄 모르고, 측정은 어떤 정렬인지 모른다.
 */
#ifndef BENCH_H
#define BENCH_H

#include <stddef.h>

#include "sort.h"

/* key로 정렬하고 tag에는 입력 순서를 새겨 둔다.
 * 정렬 뒤 같은 key끼리 tag가 오름차순이면 안정 정렬이다. */
typedef struct Record {
    int key;
    int tag;
} Record;

/* key만 본다. tag까지 넣으면 모든 정렬이 안정해 보인다. */
int recordCompare(const void *a, const void *b);

typedef enum InputKind {
    INPUT_RANDOM,     /* 무작위 */
    INPUT_SORTED,     /* 이미 정렬됨 */
    INPUT_REVERSED,   /* 역순 */
    INPUT_FEW_UNIQUE, /* 중복 많음 (값 8종류) */
    INPUT_KIND_COUNT
} InputKind;

const char *inputKindName(InputKind kind); /* 표에 찍는 한글 이름 */
const char *inputKindKey(InputKind kind);  /* CSV에 찍는 ASCII 이름 */

/* seed를 고정하면 매번 같은 입력이 나온다. */
void makeInput(Record *a, size_t n, InputKind kind, unsigned seed);

int recordsSorted(const Record *a, size_t n);
int recordsStable(const Record *a, size_t n);

typedef struct BenchResult {
    const SortAlgorithm *algo;
    size_t n;
    double millis;   /* 한 번 도는 데 걸린 평균 시간 */
    SortStats stats; /* 한 회차의 측정값 (입력이 같으니 회차마다 같다) */
    int sorted;
    int stable;      /* 구현 표의 주장이 아니라 실측 */
} BenchResult;

/* input을 복사해 reps번 정렬하고 평균 시간을 남긴다. 복사 시간은 빼고 잰다. */
BenchResult benchRun(const SortAlgorithm *algo, const Record *input, size_t n, int reps);

#endif /* BENCH_H */
