/* 실험 입력 만들기 — Java 코드의 generateXxx 함수들과 같다.
 * 돌려받은 배열은 부른 쪽이 free한다. */
#ifndef DATA_H
#define DATA_H

#include "sort.h"

int *generateRandom(int n, Random *rand);       /* 0 ~ 999,999 무작위 */
int *generateSorted(int n);                     /* 0, 1, ..., n-1 */
int *generateReverseSorted(int n);              /* n-1, ..., 1, 0 */
int *generateNearlySorted(int n, Random *rand); /* 정렬된 배열에서 n/20번만 무작위 교환 */
int *generateFewUnique(int n, Random *rand);    /* 0 ~ 4, 다섯 가지 값만 */
int *generateAllEqual(int n);                   /* 모두 42 */

#endif /* DATA_H */
