"""실행: make demo — C(./src/main.out --demo)와 글자 하나까지 같은 출력을 낸다."""

import sort

SEED = 2026193117
INPUT = [6, 2, 5, 1, 7, 3, 4]

if __name__ == "__main__":
    print("input:", " ".join(str(x) for x in INPUT))
    runs = [
        ("mergeSort", lambda a, r: sort.merge_sort(a, 0, len(a) - 1)),
        ("quickSort", lambda a, r: sort.quick_sort(a, 0, len(a) - 1, r)),
        ("bogoSort", lambda a, r: sort.bogo_sort(a, r)),
    ]
    for name, run in runs:
        a = list(INPUT)
        stats = sort.sort_stats_reset()
        run(a, sort.Random(SEED))
        print(
            f"{name:<9} {' '.join(str(x) for x in a)}"
            f"  compares={stats.compares} depth={stats.max_depth} shuffles={stats.shuffles}"
        )
