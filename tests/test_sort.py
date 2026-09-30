"""유닛 테스트 — 표준 라이브러리의 unittest만 쓴다.

실행: make test-py
"""

import random
import sys
import unittest
from pathlib import Path

# src/를 import 경로에 넣는다. 패키지로 만들지 않아도 되도록.
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

from sort import SORT_ALGORITHMS, SortStats, bogo_sort, merge_sort, quick_sort  # noqa: E402

# Python에서 보고 정렬을 돌려 볼 최대 n. C(maxN = 10)보다 느려서 작게 잡는다.
BOGO_MAX_N = 8


class Key:
    """key로만 비교하고 tag로 입력 순서를 기억한다. 안정성을 재는 데 쓴다."""

    def __init__(self, key, tag):
        self.key = key
        self.tag = tag

    def __lt__(self, other):
        return self.key < other.key

    def __le__(self, other):
        return self.key <= other.key


def is_stable(a):
    return all(
        not (a[i - 1].key == a[i].key and a[i - 1].tag > a[i].tag) for i in range(1, len(a))
    )


class TestAllSorts(unittest.TestCase):
    CASES = [
        [6, 8, 5, 9, 10, 1, 7, 2, 4, 3],
        [1, 2, 3, 4, 5],
        [5, 4, 3, 2, 1],
        [3, 1, 3, 1, 2],
        [7, 7, 7, 7],
        [-3, 0, -7, 2],
        [42],
        [],
    ]

    def test_basic_cases(self):
        for name, sort in SORT_ALGORITHMS:
            for case in self.CASES:
                if sort is bogo_sort and len(case) > BOGO_MAX_N:
                    continue  # Python에서 10! 번 섞기는 너무 오래 걸린다
                with self.subTest(algo=name, case=case):
                    self.assertEqual(sort(list(case)), sorted(case))

    def test_sorts_in_place(self):
        for name, sort in SORT_ALGORITHMS:
            with self.subTest(algo=name):
                a = [3, 1, 2]
                sort(a)
                self.assertEqual(a, [1, 2, 3])

    def test_against_sorted(self):
        rng = random.Random(7)
        for name, sort in SORT_ALGORITHMS:
            top = BOGO_MAX_N if sort is bogo_sort else 200
            for n in range(top + 1):
                a = [rng.randrange(-50, 50) for _ in range(n)]
                with self.subTest(algo=name, n=n):
                    self.assertEqual(sort(list(a)), sorted(a))


class TestProperties(unittest.TestCase):
    def test_stability(self):
        expected = {"quickSort": False, "mergeSort": True, "bogoSort": False}
        for name, sort in SORT_ALGORITHMS:
            n = 10 if sort is bogo_sort else 200
            a = [Key((i * 7) % 3, i) for i in range(n)]
            sort(a)
            with self.subTest(algo=name):
                self.assertEqual(is_stable(a), expected[name])

    def test_quick_sort_worst_case(self):
        stats = SortStats()
        quick_sort(list(range(100)), stats)
        self.assertEqual(stats.compares, 100 * 99 // 2)
        self.assertEqual(stats.max_depth, 100)

    def test_merge_sort_depth(self):
        stats = SortStats()
        merge_sort(list(range(100)), stats)
        self.assertEqual(stats.max_depth, 8)

    def test_bogo_sort_best_case(self):
        stats = SortStats()
        bogo_sort(list(range(100)), stats)
        self.assertEqual((stats.compares, stats.shuffles), (99, 0))

    def test_bogo_sort_reproducible(self):
        first, second, other = SortStats(), SortStats(), SortStats()
        bogo_sort([6, 2, 5, 1, 7, 3, 4], first)
        bogo_sort([6, 2, 5, 1, 7, 3, 4], second)
        bogo_sort([6, 2, 5, 1, 7, 3, 4], other, seed=20260931)
        self.assertEqual(first.shuffles, second.shuffles)
        self.assertNotEqual(first.shuffles, other.shuffles)


if __name__ == "__main__":
    unittest.main()
