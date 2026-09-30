"""정렬 비교 과제 — 퀵 · 병합 · 보고 정렬 (C 구현 src/*.c와 같은 알고리즘).

C와 같은 곳에서 비교·이동을 세므로, 같은 입력이면 두 구현의 횟수가 똑같이
나온다. 보고 정렬의 난수(xorshift32)도 C와 비트 단위로 같다.
"""

BOGO_SORT_SEED = 20260930
_MASK32 = 0xFFFFFFFF


class SortStats:
    """한 번 정렬하는 동안 모인 측정값 (C의 SortStats와 같은 뜻)."""

    def __init__(self):
        self.compares = 0
        self.moves = 0
        self.max_depth = 0
        self.shuffles = 0


def random_next(state):
    """xorshift32 한 걸음. (새 상태, 난수)를 돌려준다. C의 sortRandomNext와 같다."""
    x = state
    x ^= (x << 13) & _MASK32
    x ^= x >> 17
    x ^= (x << 5) & _MASK32
    return x, x


class _Ctx:
    """정렬 구현들이 함께 쓰는 작업 문맥 (C의 SortCtx)."""

    def __init__(self, a, stats):
        self.a = a
        self.stats = stats if stats is not None else SortStats()
        self.depth = 0

    def less_eq(self, x, y):
        self.stats.compares += 1
        return x <= y

    def less(self, x, y):
        self.stats.compares += 1
        return x < y

    def swap(self, i, j):
        if i == j:
            return
        a = self.a
        a[i], a[j] = a[j], a[i]
        self.stats.moves += 3  # tmp를 거치는 교환 한 번은 이동 3회

    def enter(self):
        self.depth += 1
        self.stats.max_depth = max(self.stats.max_depth, self.depth)

    def leave(self):
        self.depth -= 1


# --- 퀵 정렬: Lomuto 분할, 마지막 원소를 피벗으로 ----------------------------


def _partition(c, lo, hi):
    a = c.a
    store = lo
    for i in range(lo, hi):
        if c.less_eq(a[i], a[hi]):
            c.swap(store, i)
            store += 1
    c.swap(store, hi)
    return store


def _quick_sort_range(c, lo, hi):
    c.enter()
    if lo < hi:
        p = _partition(c, lo, hi)
        if p > lo:
            _quick_sort_range(c, lo, p - 1)
        _quick_sort_range(c, p + 1, hi)
    c.leave()


def quick_sort(a, stats=None):
    """a를 제자리에서 오름차순으로 정렬한다. 불안정. 정렬된 입력에서 O(n^2)."""
    if len(a) >= 2:
        _quick_sort_range(_Ctx(a, stats), 0, len(a) - 1)
    return a


# --- 병합 정렬: top-down, 보조 배열 -----------------------------------------


def _merge(c, lo, mid, hi):
    a = c.a
    buf = a[lo:hi]
    c.stats.moves += hi - lo
    i, j, end, k = 0, mid - lo, hi - lo, lo
    while i < mid - lo and j < end:
        # 같으면 왼쪽을 먼저. 이것이 안정성을 만든다.
        if c.less(buf[j], buf[i]):
            a[k] = buf[j]
            j += 1
        else:
            a[k] = buf[i]
            i += 1
        k += 1
    rest = buf[i:mid - lo] + buf[j:end]
    a[k:hi] = rest
    c.stats.moves += (k - lo) + len(rest)


def _merge_sort_range(c, lo, hi):
    c.enter()
    if hi - lo >= 2:
        mid = lo + (hi - lo) // 2
        _merge_sort_range(c, lo, mid)
        _merge_sort_range(c, mid, hi)
        _merge(c, lo, mid, hi)
    c.leave()


def merge_sort(a, stats=None):
    """a를 오름차순으로 정렬한다. 안정. 항상 O(n log n), 보조 메모리 O(n)."""
    if len(a) >= 2:
        _merge_sort_range(_Ctx(a, stats), 0, len(a))
    return a


# --- 보고 정렬: 정렬될 때까지 섞는다 -----------------------------------------


def _is_sorted(c):
    a = c.a
    for i in range(1, len(a)):
        if not c.less_eq(a[i - 1], a[i]):
            return False
    return True


def _shuffle(c, state):
    """Fisher-Yates. 바뀐 난수 상태를 돌려준다."""
    for i in range(len(c.a) - 1, 0, -1):
        state, r = random_next(state)
        c.swap(i, r % (i + 1))
    return state


def bogo_sort(a, stats=None, seed=None):
    """a가 정렬될 때까지 무작위로 섞는다. 평균 O(n * n!). 작은 a에만 쓴다."""
    if len(a) < 2:
        return a
    c = _Ctx(a, stats)
    state = (BOGO_SORT_SEED if seed is None else seed) & _MASK32
    if state == 0:
        state = 1  # xorshift는 0에서 멈춘다
    c.enter()
    while not _is_sorted(c):
        state = _shuffle(c, state)
        c.stats.shuffles += 1
    c.leave()
    return a


# C의 SORT_ALGORITHMS와 같은 순서 · 같은 이름.
SORT_ALGORITHMS = [
    ("quickSort", quick_sort),
    ("mergeSort", merge_sort),
    ("bogoSort", bogo_sort),
]
