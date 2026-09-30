"""유닛 테스트 — 표준 라이브러리의 unittest만 쓴다.

실행: make test-py
"""

import random as pyrandom
import sys
import unittest
from pathlib import Path

# src/를 import 경로에 넣는다. 패키지로 만들지 않아도 되도록.
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "src"))

import sort  # noqa: E402

# Python에서 보고 정렬을 돌려 볼 최대 n. C보다 느려서 작게 잡는다.
BOGO_MAX_N = 7


def run(name, arr, seed=1):
    rand = sort.Random(seed)
    stats = sort.sort_stats_reset()
    if name == "mergeSort":
        sort.merge_sort(arr, 0, len(arr) - 1)
    elif name == "quickSort":
        sort.quick_sort(arr, 0, len(arr) - 1, rand)
    else:
        sort.bogo_sort(arr, rand)
    return stats


NAMES = ["mergeSort", "quickSort", "bogoSort"]


class TestAllSorts(unittest.TestCase):
    CASES = [
        [6, 8, 5, 9, 10, 1, 7, 2],
        [1, 2, 3, 4, 5],
        [5, 4, 3, 2, 1],
        [3, 1, 3, 1, 2],
        [7, 7, 7, 7],
        [-3, 0, -7, 2],
        [42],
        [],
    ]

    def test_basic_cases(self):
        for name in NAMES:
            for case in self.CASES:
                if name == "bogoSort" and len(case) > BOGO_MAX_N:
                    continue
                with self.subTest(algo=name, case=case):
                    a = list(case)
                    run(name, a)
                    self.assertEqual(a, sorted(case))

    def test_against_sorted(self):
        rng = pyrandom.Random(7)
        for name in NAMES:
            top = BOGO_MAX_N if name == "bogoSort" else 200
            for n in range(top + 1):
                a = [rng.randrange(-50, 50) for _ in range(n)]
                with self.subTest(algo=name, n=n):
                    b = list(a)
                    run(name, b)
                    self.assertEqual(b, sorted(a))


class TestTheory(unittest.TestCase):
    def test_merge_sort_depth(self):
        stats = run("mergeSort", list(range(1024)))
        self.assertEqual(stats.max_depth, 10)

    def test_quick_sort_all_equal(self):
        n = 300
        stats = run("quickSort", [42] * n)
        self.assertEqual(stats.compares, n * (n - 1) // 2)
        self.assertLessEqual(stats.max_depth, 9)

    def test_bogo_sort_best_case(self):
        stats = run("bogoSort", list(range(100)))
        self.assertEqual((stats.compares, stats.shuffles), (99, 0))

    def test_bogo_sort_reproducible(self):
        counts = [run("bogoSort", [6, 2, 5, 1, 7, 3, 4], seed).shuffles for seed in (11, 11, 12)]
        self.assertEqual(counts[0], counts[1])
        self.assertNotEqual(counts[0], counts[2])


if __name__ == "__main__":
    unittest.main()
