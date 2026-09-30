"""정렬 비교 과제 — 병합 · 퀵 · 보고 정렬 (C 구현 src/sort.c와 같은 알고리즘).

함수 구성과 난수(xorshift32), 비교를 세는 곳이 C와 같아서, 같은 입력 · 같은 씨앗이면 두 구현의 횟수가 똑같다.
"""

_MASK32 = 0xFFFFFFFF


class Random:
    """xorshift32. C의 Random 구조체와 비트 단위로 같은 수열을 낸다."""

    def __init__(self, seed):
        self.state = (seed & _MASK32) or 1  # xorshift는 0에서 멈춘다

    def next(self):
        x = self.state
        x ^= (x << 13) & _MASK32
        x ^= x >> 17
        x ^= (x << 5) & _MASK32
        self.state = x
        return x

    def next_int(self, bound):
        return self.next() % bound


class SortStats:
    """정렬 한 번 동안 모인 값 (C의 SortStats)."""

    def __init__(self):
        self.compares = 0
        self.shuffles = 0
        self.depth = 0
        self.max_depth = 0

    def enter(self):
        self.depth += 1
        self.max_depth = max(self.max_depth, self.depth)

    def leave(self):
        self.depth -= 1


stats = SortStats()


def sort_stats_reset():
    global stats
    stats = SortStats()
    return stats


# --- 병합 정렬 -----------------------------------------------------------------


def merge(arr, left, mid, right):
    L = arr[left:mid + 1]
    R = arr[mid + 1:right + 1]
    i = j = 0
    k = left
    while i < len(L) and j < len(R):
        stats.compares += 1
        if L[i] <= R[j]:  # 같으면 왼쪽 먼저 → 안정 정렬
            arr[k] = L[i]
            i += 1
        else:
            arr[k] = R[j]
            j += 1
        k += 1
    rest = L[i:] + R[j:]
    arr[k:k + len(rest)] = rest


def merge_sort(arr, left, right):
    if left >= right:
        return
    stats.enter()
    mid = left + (right - left) // 2
    merge_sort(arr, left, mid)
    merge_sort(arr, mid + 1, right)
    merge(arr, left, mid, right)
    stats.leave()


# --- 퀵 정렬 -------------------------------------------------------------------


def partition_random(arr, low, high, rand):
    pivot_index = low + rand.next_int(high - low + 1)
    arr[pivot_index], arr[high] = arr[high], arr[pivot_index]
    pivot = arr[high]
    i = low - 1
    for j in range(low, high):
        stats.compares += 1
        if arr[j] < pivot:
            i += 1
            arr[i], arr[j] = arr[j], arr[i]
    arr[i + 1], arr[high] = arr[high], arr[i + 1]
    return i + 1


def quick_sort(arr, low, high, rand):
    """작은 쪽만 재귀하고 큰 쪽은 반복한다 (재귀 깊이 O(log n))."""
    if low >= high:
        return
    stats.enter()
    while low < high:
        pi = partition_random(arr, low, high, rand)
        if pi - low < high - pi:
            quick_sort(arr, low, pi - 1, rand)
            low = pi + 1
        else:
            quick_sort(arr, pi + 1, high, rand)
            high = pi - 1
    stats.leave()


# --- 보고 정렬 -----------------------------------------------------------------


def is_sorted(arr):
    for i in range(1, len(arr)):
        stats.compares += 1
        if arr[i - 1] > arr[i]:
            return False
    return True


def shuffle(arr, rand):
    """Fisher-Yates 셔플."""
    for i in range(len(arr) - 1, 0, -1):
        j = rand.next_int(i + 1)
        arr[i], arr[j] = arr[j], arr[i]


def bogo_sort(arr, rand):
    """정렬될 때까지 섞는다. 평균 n!번 섞으므로 작은 배열에만 쓴다."""
    while not is_sorted(arr):
        shuffle(arr, rand)
        stats.shuffles += 1
