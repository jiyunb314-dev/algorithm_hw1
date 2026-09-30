"""report/results.txt로 효율성 비교 그래프(SVG)를 그린다. 표준 모듈만 쓴다.

실행: make charts

인쇄(PDF)용이라 흑백으로 그린다. 계열은 색이 아니라 표식 모양 · 선 모양 ·
막대 명암으로 구별하고, 범례와 끝점 이름표를 함께 단다.
"""

import math
import re
import sys
from pathlib import Path

INK = "#000"
MUTED = "#555"
GRID = "#e0e0e0"
AXIS = "#888"
FONT = "font-family=\"'Noto Sans KR', 'Apple SD Gothic Neo', 'Malgun Gothic', sans-serif\""

TYPES = ["Random", "Sorted", "ReverseSorted", "NearlySorted", "FewUnique", "AllEqual"]


# --- results.txt 읽기 ------------------------------------------------------------


def load(path):
    rows, bogo = {}, []
    n = t = None
    text = Path(path).read_text(encoding="utf-8")
    main_part, _, bogo_part = text.partition("=== Bogo Sort")
    for line in main_part.splitlines():
        m = re.match(r"=== Array size: (\d+)", line)
        if m:
            n = int(m.group(1))
            continue
        m = re.match(r"Input: (\w+)", line)
        if m:
            t = m.group(1)
            continue
        m = re.match(r"(Merge|Quick|Bogo) Sort:\s+([\d.]+) ms, [\d.]+ MB used,\s+(\d+) compares", line)
        if m:
            rows[(n, t, m.group(1))] = {"ms": float(m.group(2)), "cmp": int(m.group(3))}
    for line in bogo_part.splitlines():
        p = line.split()
        if len(p) == 7 and p[0].isdigit():
            bogo.append({"n": int(p[0]), "fact": float(p[1]), "avg": float(p[2])})
    return rows, bogo


# --- 공통 그리기 도구 ------------------------------------------------------------


def fmt(v):
    if v >= 1e9:
        return f"{v / 1e9:g}B"
    if v >= 1e6:
        return f"{v / 1e6:g}M"
    if v >= 1e3:
        return f"{v / 1e3:g}k"
    return f"{v:g}"


def fmt_full(v):
    return f"{v:,.3f}".rstrip("0").rstrip(".") if v < 1000 else f"{v:,.0f}"


class LogScale:
    def __init__(self, lo, hi, start, end):
        self.a, self.b = math.floor(math.log10(lo)), math.ceil(math.log10(hi))
        self.start, self.end = start, end

    def __call__(self, v):
        t = (math.log10(v) - self.a) / (self.b - self.a)
        return self.start + t * (self.end - self.start)

    def ticks(self):
        return [10.0 ** e for e in range(self.a, self.b + 1)]


def svg_open(w, h, title):
    return [
        f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="{h}" viewBox="0 0 {w} {h}" '
        f'{FONT} font-size="12" fill="{INK}">',
        f'<rect width="{w}" height="{h}" fill="#fff"/>',
        f'<text x="12" y="22" font-size="14" font-weight="700">{title}</text>',
    ]


def marker(shape, x, y, filled=True):
    fill = INK if filled else "#fff"
    s = 4.5
    if shape == "circle":
        return f'<circle cx="{x:.1f}" cy="{y:.1f}" r="{s}" fill="{fill}" stroke="{INK}" stroke-width="1.5"/>'
    if shape == "square":
        return (f'<rect x="{x - s:.1f}" y="{y - s:.1f}" width="{2 * s}" height="{2 * s}" '
                f'fill="{fill}" stroke="{INK}" stroke-width="1.5"/>')
    if shape == "triangle":
        return (f'<path d="M{x:.1f},{y - s - 1:.1f} L{x + s + 0.5:.1f},{y + s - 0.5:.1f} '
                f'L{x - s - 0.5:.1f},{y + s - 0.5:.1f} Z" fill="{fill}" stroke="{INK}" stroke-width="1.5"/>')
    return (f'<path d="M{x:.1f},{y - s - 1:.1f} L{x + s + 1:.1f},{y:.1f} L{x:.1f},{y + s + 1:.1f} '
            f'L{x - s - 1:.1f},{y:.1f} Z" fill="{fill}" stroke="{INK}" stroke-width="1.5"/>')


# --- 그림 1 · 3: 가로 막대 (로그 축) ---------------------------------------------


def hbar_chart(path, title, xlabel, groups, series, values, label_series=None, label_fmt=fmt_full):
    """groups: 세로로 늘어설 묶음 이름, series: [(이름, 명암)], values[(묶음, 이름)] = 값."""
    bar, gap, group_gap = 11, 2, 14
    left, right, top = 118, 96, 58
    group_h = len(series) * (bar + gap) - gap
    plot_h = len(groups) * (group_h + group_gap) - group_gap
    w, h = 640, top + plot_h + 52
    vals = [v for v in values.values() if v > 0]
    x = LogScale(min(vals), max(vals), left, w - right)
    out = svg_open(w, h, title)
    # 범례
    lx = left
    for name, shade in series:
        out.append(f'<rect x="{lx}" y="33" width="12" height="12" fill="{shade}" stroke="{INK}" stroke-width="0.8"/>')
        out.append(f'<text x="{lx + 17}" y="43.5">{name}</text>')
        lx += 17 + 12 * len(name) + 22  # 한글 한 글자 ≈ 12px
    # 격자와 x축
    for t in x.ticks():
        px = x(t)
        out.append(f'<line x1="{px:.1f}" y1="{top - 4}" x2="{px:.1f}" y2="{top + plot_h}" stroke="{GRID}"/>')
        out.append(f'<text x="{px:.1f}" y="{top + plot_h + 16}" text-anchor="middle" fill="{MUTED}" '
                   f'font-size="11">{fmt(t)}</text>')
    out.append(f'<line x1="{left}" y1="{top + plot_h}" x2="{w - right}" y2="{top + plot_h}" stroke="{AXIS}"/>')
    out.append(f'<text x="{(left + w - right) / 2}" y="{h - 10}" text-anchor="middle" fill="{MUTED}">{xlabel}</text>')
    # 막대
    for g, group in enumerate(groups):
        gy = top + g * (group_h + group_gap)
        out.append(f'<text x="{left - 8}" y="{gy + group_h / 2 + 4}" text-anchor="end">{group}</text>')
        for s, (name, shade) in enumerate(series):
            v = values.get((group, name))
            if v is None or v <= 0:
                continue
            by = gy + s * (bar + gap)
            bw = max(x(v) - left, 1.5)
            out.append(f'<rect x="{left}" y="{by}" width="{bw:.1f}" height="{bar}" fill="{shade}" '
                       f'stroke="{INK}" stroke-width="0.8"/>')
            if label_series is None or name in label_series:
                out.append(f'<text x="{left + bw + 5:.1f}" y="{by + bar - 1.5}" font-size="10.5" '
                           f'fill="{MUTED}">{label_fmt(v)}</text>')
    out.append("</svg>")
    Path(path).write_text("\n".join(out) + "\n", encoding="utf-8")


# --- 그림 2 · 4: 꺾은선 ----------------------------------------------------------


def line_chart(path, title, xlabel, ylabel, series, xlog=True, xticks=None, height=300):
    """series: [(이름, 표식, 채움?, 점선?, [(x, y)])]"""
    left, right, top, bottom = 70, 160, 40, 48
    w, h = 640, height
    xs = [p[0] for s in series for p in s[4]]
    ys = [p[1] for s in series for p in s[4]]
    if xlog:
        x = LogScale(min(xs), max(xs), left, w - right)
    else:
        lo, hi = min(xs), max(xs)
        x = lambda v: left + (v - lo) / (hi - lo) * (w - right - left)  # noqa: E731
    y = LogScale(min(ys), max(ys), h - bottom, top)
    out = svg_open(w, h, title)
    for t in y.ticks():
        py = y(t)
        out.append(f'<line x1="{left}" y1="{py:.1f}" x2="{w - right}" y2="{py:.1f}" stroke="{GRID}"/>')
        out.append(f'<text x="{left - 6}" y="{py + 4:.1f}" text-anchor="end" fill="{MUTED}" font-size="11">{fmt(t)}</text>')
    for t in xticks or sorted(set(xs)):
        px = x(t)
        out.append(f'<line x1="{px:.1f}" y1="{h - bottom}" x2="{px:.1f}" y2="{h - bottom + 4}" stroke="{AXIS}"/>')
        out.append(f'<text x="{px:.1f}" y="{h - bottom + 17}" text-anchor="middle" fill="{MUTED}" font-size="11">{fmt(t)}</text>')
    out.append(f'<line x1="{left}" y1="{h - bottom}" x2="{w - right}" y2="{h - bottom}" stroke="{AXIS}"/>')
    out.append(f'<line x1="{left}" y1="{top}" x2="{left}" y2="{h - bottom}" stroke="{AXIS}"/>')
    out.append(f'<text x="{(left + w - right) / 2}" y="{h - 10}" text-anchor="middle" fill="{MUTED}">{xlabel}</text>')
    out.append(f'<text transform="translate(16 {(top + h - bottom) / 2}) rotate(-90)" text-anchor="middle" '
               f'fill="{MUTED}">{ylabel}</text>')
    ends = []
    for name, shape, filled, dashed, pts in series:
        d = " ".join(f"{x(a):.1f},{y(b):.1f}" for a, b in pts)
        dash = ' stroke-dasharray="6 4"' if dashed else ""
        out.append(f'<polyline points="{d}" fill="none" stroke="{INK}" stroke-width="1.6"{dash}/>')
        for a, b in pts:
            out.append(marker(shape, x(a), y(b), filled))
        ends.append([y(pts[-1][1]), name, shape, filled, x(pts[-1][0])])
    # 끝점 이름표: 겹치지 않게 최소 간격 14px로 벌린다
    ends.sort()
    for i in range(1, len(ends)):
        ends[i][0] = max(ends[i][0], ends[i - 1][0] + 14)
    for ly, name, shape, filled, ex in ends:
        out.append(marker(shape, w - right + 24, ly, filled))
        out.append(f'<text x="{w - right + 34}" y="{ly + 4:.1f}">{name}</text>')
    out.append("</svg>")
    Path(path).write_text("\n".join(out) + "\n", encoding="utf-8")


# --- 그림 네 장 ------------------------------------------------------------------


def main():
    src = sys.argv[1] if len(sys.argv) > 1 else "report/results.txt"
    dst = Path(sys.argv[2] if len(sys.argv) > 2 else "report")
    rows, bogo = load(src)
    N = 100000
    sizes = sorted({k[0] for k in rows})

    # 그림 1. n = 100,000에서 입력 종류별 시간
    values = {}
    for t in TYPES:
        values[(t, "병합 정렬")] = rows[(N, t, "Merge")]["ms"]
        values[(t, "퀵 정렬")] = rows[(N, t, "Quick")]["ms"]
    hbar_chart(dst / "fig1-time-by-input.svg", "그림 1. 입력 종류별 걸린 시간 (n = 100,000)",
               "걸린 시간 (ms, 로그 축)", TYPES, [("병합 정렬", "#333"), ("퀵 정렬", "#ccc")], values,
               label_fmt=lambda v: f"{v:,.1f} ms")

    # 그림 2. n에 따른 비교 횟수
    def pts(t, alg):
        return [(n, rows[(n, t, alg)]["cmp"]) for n in sizes]
    line_chart(dst / "fig2-compares-by-n.svg", "그림 2. 배열 크기에 따른 비교 횟수 (로그-로그)",
               "배열 크기 n (로그 축)", "비교 횟수 (로그 축)",
               [("병합 · Random", "circle", True, False, pts("Random", "Merge")),
                ("퀵 · Random", "square", False, False, pts("Random", "Quick")),
                ("퀵 · FewUnique", "triangle", False, True, pts("FewUnique", "Quick")),
                ("퀵 · AllEqual", "diamond", True, True, pts("AllEqual", "Quick"))])

    # 그림 3. n = 10에서 세 정렬의 비교 횟수
    values = {}
    for t in TYPES:
        values[(t, "병합")] = rows[(10, t, "Merge")]["cmp"]
        values[(t, "퀵")] = rows[(10, t, "Quick")]["cmp"]
        values[(t, "보고")] = rows[(10, t, "Bogo")]["cmp"]
    hbar_chart(dst / "fig3-n10-compares.svg", "그림 3. n = 10에서 세 정렬의 비교 횟수",
               "비교 횟수 (로그 축)", TYPES, [("병합", "#333"), ("퀵", "#999"), ("보고", "#e6e6e6")],
               values, label_series={"보고"}, label_fmt=lambda v: f"{v:,.0f}")

    # 그림 4. 보고 정렬의 평균 섞기 횟수와 n!
    b = [r for r in bogo if r["n"] >= 3]
    line_chart(dst / "fig4-bogo-shuffles.svg", "그림 4. 보고 정렬의 평균 섞기 횟수와 n! (10번 평균)",
               "배열 크기 n", "섞기 횟수 (로그 축)",
               [("측정 평균", "circle", True, False, [(r["n"], r["avg"]) for r in b]),
                ("n!", "square", False, True, [(r["n"], r["fact"]) for r in b])],
               xlog=False, xticks=[r["n"] for r in b], height=260)


if __name__ == "__main__":
    main()
