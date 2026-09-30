#include "data.h"

#include <stdlib.h>

static int *newArray(int n) {
    return (int *)malloc((size_t)(n > 0 ? n : 1) * sizeof(int));
}

int *generateRandom(int n, Random *rand) {
    int *arr = newArray(n);
    for (int i = 0; i < n; i++) arr[i] = randomNextInt(rand, 1000000);
    return arr;
}

int *generateSorted(int n) {
    int *arr = newArray(n);
    for (int i = 0; i < n; i++) arr[i] = i;
    return arr;
}

int *generateReverseSorted(int n) {
    int *arr = newArray(n);
    for (int i = 0; i < n; i++) arr[i] = n - 1 - i;
    return arr;
}

int *generateNearlySorted(int n, Random *rand) {
    int *arr = generateSorted(n);
    int swaps = n / 20;
    for (int i = 0; i < swaps; i++) {
        int a = randomNextInt(rand, n);
        int b = randomNextInt(rand, n);
        int temp = arr[a];
        arr[a] = arr[b];
        arr[b] = temp;
    }
    return arr;
}

int *generateFewUnique(int n, Random *rand) {
    int *arr = newArray(n);
    for (int i = 0; i < n; i++) arr[i] = randomNextInt(rand, 5);
    return arr;
}

int *generateAllEqual(int n) {
    int *arr = newArray(n);
    for (int i = 0; i < n; i++) arr[i] = 42;
    return arr;
}
