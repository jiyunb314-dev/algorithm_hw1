# 빌드와 테스트를 한 단어로 돌리기 위한 Makefile.
# 컨테이너 안에서 실행한다 (docker compose exec lab bash).
#
#   make run      정렬 비교 실험 전체 (Java 코드의 main과 같은 흐름, 10초 남짓)
#   make results  실험 출력을 report/results.txt에 남긴다
#   make demo     작은 배열 하나로 세 정렬 실행 (C, Python — 같은 출력)
#   make test     유닛 테스트 (C, Python) + 두 구현의 출력이 같은지 확인
#   make debug    디버그 심볼을 넣어 빌드 (VS Code의 F5가 쓴다)
#   make clean    빌드 산출물 정리
#
# 실행 파일은 `*.out`으로 만든다. .gitignore가 그것만 걸러낸다.

CC ?= gcc
CFLAGS ?= -std=c17 -Wall -Wextra -O2
# 디버그 빌드: 최적화를 끄고 심볼을 남긴다 (VS Code의 F5가 이 결과물을 쓴다).
# `-I`는 아래 패턴 규칙이 대상 파일의 폴더로 붙인다. 여기서 고정하지 않는다.
DEBUGFLAGS ?= -std=c17 -Wall -Wextra -g -O0

# main.c와 테스트가 함께 쓰는 소스.
SORT_SRC = src/sort.c src/data.c
SORT_HDR = src/sort.h src/data.h

.PHONY: all run results demo test test-c test-py test-same debug clean

all: test

run: src/main.out
	@./src/main.out

results: src/main.out
	./src/main.out > report/results.txt

demo: src/main.out
	@./src/main.out --demo
	@python3 src/main.py

test: test-c test-py test-same

test-c: tests/test_sort.out
	@./tests/test_sort.out

test-py:
	@python3 -m unittest discover -s tests -v

# C와 Python이 같은 알고리즘인지 본다. 비교·섞기 횟수까지 같아야 통과한다.
test-same: src/main.out
	@./src/main.out --demo > src/demo-c.out
	@python3 src/main.py > src/demo-py.out
	@diff src/demo-c.out src/demo-py.out && echo "ok    C와 Python의 출력이 같다"

debug: src/main.debug.out

src/main.out: src/main.c $(SORT_SRC) $(SORT_HDR)
	$(CC) $(CFLAGS) -Isrc -o $@ src/main.c $(SORT_SRC)

# 파일 하나를 그 자리에서 빌드한다. 같은 폴더의 .c를 함께 링크하므로 헤더에
# 선언만 있고 구현이 옆 파일에 있어도 된다. 대신 **한 폴더에 main은 하나만** 둔다.
#   %.out        실행용 (Code Runner의 ▶ 버튼이 이 규칙을 부른다)
#   %.debug.out  디버그용 (VS Code의 "C 디버그 (현재 파일)"이 부른다)
# 명시 규칙(tests/test_sort.out 등)이 있으면 그쪽이 우선한다.
%.out: %.c
	$(CC) $(CFLAGS) -I$(@D) -o $@ $(wildcard $(@D)/*.c)

%.debug.out: %.c
	$(CC) $(DEBUGFLAGS) -I$(@D) -o $@ $(wildcard $(@D)/*.c)

tests/test_sort.out: tests/test_sort.c $(SORT_SRC) $(SORT_HDR)
	$(CC) $(CFLAGS) -Isrc -o $@ tests/test_sort.c $(SORT_SRC)

clean:
	rm -f src/*.out tests/*.out
	rm -rf src/__pycache__ tests/__pycache__
