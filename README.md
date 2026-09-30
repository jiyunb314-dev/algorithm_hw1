# 고급알고리즘 과제 1 — 정렬 비교 (병합 · 퀵 · 보고)

2026-2 **고급알고리즘**(SIT2001-01) 과제 1 · 배지윤 (2026193117)

수업에서 배운 **병합 정렬 · 퀵 정렬**과 배우지 않은 **보고 정렬**(bogosort)을 C로 구현해
배열 크기 × 입력 종류별로 비교한다.

- **보고서: [report/REPORT.md](report/REPORT.md)**
- 실습 환경: [lec-algorithm/algorithm-env](https://github.com/lec-algorithm/algorithm-env) template에서 시작

## 시작하기

Codespaces(**Code** → **Codespaces** → **Create codespace on main**)로 열면 터미널이
곧 컨테이너 안이다. 로컬에서는 Git과 Docker만 있으면 된다.

로컬에서:

```sh
git clone https://github.com/jiyunb314-dev/algorithm_hw1.git
cd algorithm_hw1
docker compose up -d
docker compose exec lab bash
```

처음 한 번은 이미지를 받느라 몇 분 걸립니다. 이후에는 몇 초면 뜹니다.
**이후 모든 `docker compose` 명령은 이 폴더에서 칩니다.**

VS Code를 쓴다면 Dev Containers 확장의 **Reopen in Container**를 골라도
됩니다. Codespaces와 같은 설정을 씁니다.

## 돌려보기

컨테이너 안에서 `make` 한 단어면 된다.

| 명령 | 하는 일 |
| --- | --- |
| `make run` | 정렬 비교 실험 전체 (배열 크기 6가지 × 입력 6가지, 10초 남짓) |
| `make results` | 실험 출력을 `report/results.txt`에 남긴다 |
| `make charts` | `results.txt`로 보고서 그래프(`report/fig*.svg`)를 다시 그린다 |
| `make demo` | 작은 배열 하나로 세 정렬 실행 (C · Python, 같은 출력) |
| `make test` | 유닛 테스트 (C · Python) + 두 구현의 출력 대조 |
| `make debug` | 디버그 심볼을 넣어 빌드 |
| `make clean` | 빌드 산출물 정리 |

```console
$ make run
=== Array size: 100000 ===
Input: Random
Merge Sort:      13.218 ms, 0.38 MB used,      1536246 compares, depth  17
Quick Sort:       8.466 ms, 0.00 MB used,      1941216 compares, depth  10
Bogo Sort:   (skipped, too slow)
...
```

보고 정렬은 평균 `n!`번을 섞어야 하므로 n = 10에서만 돌리고, 더 큰 배열은 건너뛴다.

```console
$ make test
...
34 checks, 0 failures
...
OK
ok    C와 Python의 출력이 같다
```

## VS Code에서 실행·디버그

Codespaces나 Dev Containers로 열었다면 편집기에서 바로 됩니다.

| 하고 싶은 것 | 방법 |
| --- | --- |
| 파일 하나 실행 | 편집기 오른쪽 위 **▶ 버튼** (Code Runner) |
| 전체 실행 | `Cmd/Ctrl + Shift + B` (기본 빌드 작업이 `make run`) |
| 테스트 | 명령 팔레트 → **Tasks: Run Test Task** |
| C 디버그 | `F5` → **C 디버그 (현재 파일)** |
| Python 디버그 | `F5` → **Python 디버그 (현재 파일)** |

`F5`를 누르면 빌드가 먼저 돌아 심볼이 있는 바이너리를 만들고 디버거가
붙습니다. 중단점을 걸고 변수를 들여다볼 수 있습니다.

### 파일 하나만 실행·디버그하기

**C 디버그 (현재 파일)** 구성은 열려 있는 `.c` 파일을 그대로 디버깅합니다. 폴더가
늘어나도 구성을 새로 만들 필요가 없습니다.

같은 폴더의 `.c`를 함께 링크하므로, 구현이 옆 파일에 있어도 됩니다. 대신
**한 폴더에 `main`은 하나만** 두세요.

터미널에서 직접 부를 수도 있습니다.

```sh
make src/main.debug.out && ./src/main.debug.out
```

### ▶ 버튼에 대해

편집기 오른쪽 위의 ▶ 버튼은 **Code Runner** 확장이 제공합니다. C든 Python이든
열려 있는 파일을 그대로 실행합니다.

두 확장이 각각 ▶ 버튼을 내놓으면 헷갈리므로, C/C++ 확장 쪽은 꺼 두었습니다
(`C_Cpp.debugShortcut`). 그쪽 버튼은 **파일 하나만** 컴파일해서 이런 오류를
냅니다.

```console
undefined reference to `quickSort'
collect2: error: ld returned 1 exit status
```

Code Runner도 기본 설정 그대로면 같은 문제가 나고, Python은 이미지에 없는
`python`을 찾습니다. 그래서 `.vscode/settings.json`에서 두 가지를 고쳐
두었습니다.

- C는 `Makefile`의 `%.out` 규칙을 거쳐 **같은 폴더의 `.c`를 함께** 빌드합니다
- Python은 `python3`로 실행합니다
- 출력 패널이 아니라 **터미널**에서 돌립니다. 그래야 `scanf`나 `input()`이 멈추지 않습니다

## 저장소 구조

```plaintext
algorithm_hw1/
├── Makefile                          # run · results · demo · test · debug · clean
├── src/
│   ├── sort.h · sort.c               # merge · mergeSort · partitionRandom · quickSort · bogoSort
│   ├── data.h · data.c               # generateRandom 등 입력 생성 6가지
│   ├── main.c                        # 실험 (배열 크기 × 입력 종류) / --demo
│   └── sort.py · main.py             # 같은 세 정렬의 Python 구현
├── tests/
│   ├── test_sort.c                   # C 유닛 테스트 (표준 C만 사용)
│   └── test_sort.py                  # Python 유닛 테스트 (unittest)
├── tools/plot.py                     # results.txt → 그래프 SVG (표준 모듈만 사용)
├── report/                           # 보고서 · 그래프 · 실험 출력(results.txt)
├── .devcontainer/ · compose.yml · Dockerfile   # 실습 컨테이너
└── .vscode/                          # 빌드·디버그 설정 (F5, Cmd+Shift+B)
```

## 규약

- **실행 파일은 `*.out`으로 만듭니다.** `.gitignore`가 `*.out`만 걸러내므로,
  컨테이너에서 컴파일한 Linux 바이너리가 커밋에 섞이지 않습니다.
- **외부 라이브러리를 쓰지 않습니다.** C는 표준 라이브러리만, Python은 표준
  모듈만 씁니다. C 테스트도 프레임워크 없이 `assert` 수준으로 직접 씁니다.
- **C와 Python은 같은 알고리즘을 같은 이름의 함수로 구현합니다.** 언어 차이가
  알고리즘 차이로 보이지 않게 합니다.
- 파일명은 각 언어의 관례를 따릅니다. C는 camelCase(`quickSort`), Python은
  snake_case(`quick_sort`)입니다.

## 정리

```sh
docker compose down
```

컨테이너를 지워도 코드는 그대로 남습니다.
