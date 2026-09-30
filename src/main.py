"""실행: make run-py — C(make run-c)와 글자 하나까지 같은 출력을 낸다."""

from sort import SORT_ALGORITHMS, SortStats

INPUT = [6, 2, 5, 1, 7, 3, 4]

if __name__ == "__main__":
    print("input: ", " ".join(str(x) for x in INPUT))
    for name, sort in SORT_ALGORITHMS:
        stats = SortStats()
        a = sort(list(INPUT), stats)
        print(
            f"{name:<9} {' '.join(str(x) for x in a)}"
            f"  compares={stats.compares} moves={stats.moves}"
            f" depth={stats.max_depth} shuffles={stats.shuffles}"
        )
