"""report/results.csv로 비교 그래프(SVG)를 그린다. 표준 모듈만 쓴다.

실행: make charts   (또는 python3 tools/plot.py report/results.csv report)
"""

import csv
import math
import sys
from pathlib import Path

WIDTH, HEIGHT = 640, 380
LEFT, RIGHT, TOP, BOTTOM = 80, 150, 44, 56
COLORS = {"quickSort": "#2a6fdb", "mergeSort": "#1f9d55", "bogoSort": "#d64545"}
REF_COLOR = "#888888"
FONT = "font-family='Noto Sans KR, Apple SD Gothic Neo, Malgun Gothic, sans-serif'"


def load(path):
    with open(path, newline="", encoding="utf-8") as f:
        return list(csv.DictReader(f))


def select(rows, scope, input_kind, algo):
    out = [r for r in rows if r["scope"] == scope and r["input"] == input_kind and r["algo"] == algo]
    return sorted(out, key=lambda r: int(r["n"]))


def fmt(v):
    if v >= 1e6:
        return f"{v / 1e6:.4g}M"
    if v >= 1e3:
        return f"{v / 1e3:.4g}k"
    return f"{v:g}"


class Axis:
    def __init__(self, lo, hi, log, length, flip):
        if log:
            lo, hi = math.floor(math.log10(lo)), math.ceil(math.log10(hi))
        else:
            lo = 0
        self.lo, self.hi, self.log, self.length, self.flip = lo, hi, log, length, flip

    def pos(self, v):
        x = math.log10(v) if self.log else v
        t = (x - self.lo) / (self.hi - self.lo)
        return self.length * (1 - t) if self.flip else self.length * t

    def ticks(self):
        if self.log:
            return [10.0 ** e for e in range(self.lo, self.hi + 1)]
        step = 10 ** math.floor(math.log10(self.hi / 4))
        for mult in (1, 2, 5, 10):
            if self.hi / (step * mult) <= 6:
                step *= mult
                break
        return [i * step for i in range(int(self.hi / step) + 1)]


def line_chart(path, title, xlabel, ylabel, series, xlog=True, ylog=True, xticks=None):
    """series: [(이름, 색, [(x, y)], 점선?)]"""
    pts = [p for _, _, data, _ in series for p in data]
    xs = [x for x, _ in pts]
    ys = [y for _, y in pts if y > 0]
    pw, ph = WIDTH - LEFT - RIGHT, HEIGHT - TOP - BOTTOM
    ax = Axis(min(xs), max(xs), xlog, pw, False)
    if not xlog:
        ax.lo = min(xs)
    ay = Axis(min(ys), max(ys) * 1.05, ylog, ph, True)
    if not ylog:
        ay.hi = max(ys) * 1.05
    out = [
        f"<svg xmlns='http://www.w3.org/2000/svg' width='{WIDTH}' height='{HEIGHT}' "
        f"viewBox='0 0 {WIDTH} {HEIGHT}' {FONT} font-size='12'>",
        f"<rect width='{WIDTH}' height='{HEIGHT}' fill='white'/>",
        f"<text x='{WIDTH / 2}' y='24' text-anchor='middle' font-size='15' "
        f"font-weight='bold'>{title}</text>",
    ]
    for t in ay.ticks():
        y = TOP + ay.pos(t)
        out.append(f"<line x1='{LEFT}' y1='{y:.1f}' x2='{LEFT + pw}' y2='{y:.1f}' stroke='#e5e5e5'/>")
        out.append(f"<text x='{LEFT - 6}' y='{y + 4:.1f}' text-anchor='end'>{fmt(t)}</text>")
    for t in xticks or sorted(set(xs)):
        x = LEFT + ax.pos(t)
        out.append(f"<line x1='{x:.1f}' y1='{TOP + ph}' x2='{x:.1f}' y2='{TOP + ph + 4}' stroke='#333'/>")
        out.append(f"<text x='{x:.1f}' y='{TOP + ph + 18}' text-anchor='middle'>{fmt(t)}</text>")
    out.append(f"<rect x='{LEFT}' y='{TOP}' width='{pw}' height='{ph}' fill='none' stroke='#333'/>")
    out.append(f"<text x='{LEFT + pw / 2}' y='{HEIGHT - 12}' text-anchor='middle'>{xlabel}</text>")
    out.append(f"<text transform='translate(18 {TOP + ph / 2}) rotate(-90)' "
               f"text-anchor='middle'>{ylabel}</text>")
    for i, (name, color, data, dashed) in enumerate(series):
        data = [(x, y) for x, y in data if y > 0 or not ylog]
        d = " ".join(f"{LEFT + ax.pos(x):.1f},{TOP + ay.pos(y):.1f}" for x, y in data)
        dash = " stroke-dasharray='6 4'" if dashed else ""
        out.append(f"<polyline points='{d}' fill='none' stroke='{color}' stroke-width='2'{dash}/>")
        if not dashed:
            for x, y in data:
                out.append(f"<circle cx='{LEFT + ax.pos(x):.1f}' cy='{TOP + ay.pos(y):.1f}' "
                           f"r='3.5' fill='{color}'/>")
        ly = TOP + 10 + i * 20
        out.append(f"<line x1='{LEFT + pw + 12}' y1='{ly}' x2='{LEFT + pw + 36}' y2='{ly}' "
                   f"stroke='{color}' stroke-width='2'{dash}/>")
        out.append(f"<text x='{LEFT + pw + 42}' y='{ly + 4}'>{name}</text>")
    out.append("</svg>")
    Path(path).write_text("\n".join(out) + "\n", encoding="utf-8")


def bar_chart(path, title, ylabel, groups, names, values, ylog=True):
    """values[(group, name)] = 값. 로그 축 막대."""
    pw, ph = WIDTH - LEFT - RIGHT, HEIGHT - TOP - BOTTOM
    vals = [v for v in values.values() if v > 0]
    ay = Axis(min(vals), max(vals), ylog, ph, True)
    out = [
        f"<svg xmlns='http://www.w3.org/2000/svg' width='{WIDTH}' height='{HEIGHT}' "
        f"viewBox='0 0 {WIDTH} {HEIGHT}' {FONT} font-size='12'>",
        f"<rect width='{WIDTH}' height='{HEIGHT}' fill='white'/>",
        f"<text x='{WIDTH / 2}' y='24' text-anchor='middle' font-size='15' "
        f"font-weight='bold'>{title}</text>",
    ]
    for t in ay.ticks():
        y = TOP + ay.pos(t)
        out.append(f"<line x1='{LEFT}' y1='{y:.1f}' x2='{LEFT + pw}' y2='{y:.1f}' stroke='#e5e5e5'/>")
        out.append(f"<text x='{LEFT - 6}' y='{y + 4:.1f}' text-anchor='end'>{fmt(t)}</text>")
    gw = pw / len(groups)
    bw = gw * 0.7 / len(names)
    for g, (key, label) in enumerate(groups):
        gx = LEFT + g * gw + gw * 0.15
        out.append(f"<text x='{LEFT + g * gw + gw / 2:.1f}' y='{TOP + ph + 18}' "
                   f"text-anchor='middle'>{label}</text>")
        for k, name in enumerate(names):
            v = values.get((key, name), 0)
            if v <= 0:
                continue
            y = TOP + ay.pos(v)
            out.append(f"<rect x='{gx + k * bw:.1f}' y='{y:.1f}' width='{bw - 2:.1f}' "
                       f"height='{TOP + ph - y:.1f}' fill='{COLORS[name]}'/>")
            out.append(f"<text x='{gx + k * bw + bw / 2 - 1:.1f}' y='{y - 4:.1f}' "
                       f"text-anchor='middle' font-size='10'>{fmt(round(v))}</text>")
    out.append(f"<rect x='{LEFT}' y='{TOP}' width='{pw}' height='{ph}' fill='none' stroke='#333'/>")
    out.append(f"<text transform='translate(18 {TOP + ph / 2}) rotate(-90)' "
               f"text-anchor='middle'>{ylabel}</text>")
    for i, name in enumerate(names):
        ly = TOP + 10 + i * 20
        out.append(f"<rect x='{LEFT + pw + 12}' y='{ly - 6}' width='24' height='12' "
                   f"fill='{COLORS[name]}'/>")
        out.append(f"<text x='{LEFT + pw + 42}' y='{ly + 4}'>{name}</text>")
    out.append("</svg>")
    Path(path).write_text("\n".join(out) + "\n", encoding="utf-8")


def xy(rows, column):
    return [(int(r["n"]), float(r[column])) for r in rows]


def main():
    src = sys.argv[1] if len(sys.argv) > 1 else "report/results.csv"
    dst = Path(sys.argv[2] if len(sys.argv) > 2 else "report")
    rows = load(src)
    fast = ["quickSort", "mergeSort"]

    # 1. 입력 모양별 비교 횟수 (n = 4000)
    kinds = [("random", "무작위"), ("sorted", "정렬됨"), ("reversed", "역순"),
             ("few-unique", "중복많음")]
    values = {}
    for key, _ in kinds:
        for name in fast:
            for r in select(rows, "kinds", key, name):
                values[(key, name)] = float(r["compares"])
    bar_chart(dst / "kinds-compares.svg", "입력 모양별 비교 횟수 (n = 4,000, 로그 축)",
              "비교 횟수", kinds, fast, values)

    # 2. n을 키우며 — 무작위 입력의 비교 횟수 (로그-로그)
    series = [(name, COLORS[name], xy(select(rows, "growth", "random", name), "compares"), False)
              for name in fast]
    ns = [x for x, _ in series[0][2]]
    series.append(("n log₂ n", REF_COLOR, [(n, n * math.log2(n)) for n in ns], True))
    line_chart(dst / "growth-random.svg", "무작위 입력: n에 따른 비교 횟수 (로그-로그)",
               "n", "비교 횟수", series)

    # 3. n을 키우며 — 정렬된 입력 (퀵 정렬의 최악)
    series = [(name, COLORS[name], xy(select(rows, "growth", "sorted", name), "compares"), False)
              for name in fast]
    ns = [x for x, _ in series[0][2]]
    series.append(("n²/2", REF_COLOR, [(n, n * n / 2) for n in ns], True))
    line_chart(dst / "growth-sorted.svg", "정렬된 입력: n에 따른 비교 횟수 (로그-로그)",
               "n", "비교 횟수", series)

    # 4. 작은 n에서 보고 정렬 — 섞기 횟수와 n!
    bogo = select(rows, "bogo", "random", "bogoSort")
    ns = [int(r["n"]) for r in bogo]
    series = [("bogo 섞기 횟수", COLORS["bogoSort"], xy(bogo, "shuffles"), False),
              ("n!", REF_COLOR, [(n, math.factorial(n)) for n in ns], True)]
    line_chart(dst / "bogo-shuffles.svg", "보고 정렬: 섞기 횟수(10벌 평균)와 n! (로그 축)",
               "n", "섞기 횟수", series, xlog=False, xticks=ns)

    # 5. 작은 n에서 셋의 비교 횟수
    series = [(name, COLORS[name], xy(select(rows, "bogo", "random", name), "compares"), False)
              for name in ["bogoSort", "quickSort", "mergeSort"]]
    line_chart(dst / "bogo-compares.svg", "n ≤ 10에서 세 정렬의 비교 횟수 (로그 축)",
               "n", "비교 횟수", series, xlog=False, xticks=ns)


if __name__ == "__main__":
    main()
