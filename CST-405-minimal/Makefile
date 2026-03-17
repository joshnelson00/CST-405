CC = gcc
LEX = flex
YACC = bison
CFLAGS = -g -Wall

TARGET = minicompiler
OBJS = lex.yy.o parser.tab.o main.o ast.o symtab.o codegen.o tac.o optimizer.o mips.o benchmark.o stringpool.o

all: $(TARGET)

# Build the Compiler
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

lex.yy.c: scanner.l parser.tab.h
	$(LEX) scanner.l

parser.tab.c parser.tab.h: parser.y
	$(YACC) -d parser.y

lex.yy.o: lex.yy.c
	$(CC) $(CFLAGS) -c lex.yy.c

parser.tab.o: parser.tab.c
	$(CC) $(CFLAGS) -c parser.tab.c

main.o: main.c ast.h codegen.h tac.h optimizer.h benchmark.h
	$(CC) $(CFLAGS) -c main.c

ast.o: ast.c ast.h
	$(CC) $(CFLAGS) -c ast.c

symtab.o: symtab.c symtab.h
	$(CC) $(CFLAGS) -c symtab.c

codegen.o: codegen.c codegen.h ast.h symtab.h
	$(CC) $(CFLAGS) -c codegen.c

tac.o: tac.c tac.h ast.h
	$(CC) $(CFLAGS) -c tac.c

optimizer.o: optimizer.c optimizer.h mips.h symtab.h tac.h
	$(CC) $(CFLAGS) -c optimizer.c

mips.o: mips.c mips.h
	$(CC) $(CFLAGS) -c mips.c

benchmark.o: benchmark.c benchmark.h
	$(CC) $(CFLAGS) -c benchmark.c

stringpool.o: stringpool.c stringpool.h
	$(CC) $(CFLAGS) -c stringpool.c

clean:
	rm -f $(TARGET) $(OBJS) lex.yy.c parser.tab.c parser.tab.h *.s *.tac.txt *.tac.optimized.txt tac.txt unoptimizedtac.txt tac-optimized.txt

# ── Helper macro: compile + run one test ──────────────────────────────────────
# Usage: $(call run_test, stem, expected)
define run_test
	@printf '\n\033[1;36m══════════════════════════════════════════\033[0m\n'
	@printf '\033[1;36m  TEST: $(1).cm\033[0m\n'
	@printf '\033[1;36m══════════════════════════════════════════\033[0m\n'
	@./$(TARGET) $(1).cm $(1).s 2>/dev/null
	@printf 'Expected: \033[33m$(2)\033[0m\n'
	@printf 'Got:      \033[32m'
	@spim -file $(1).s 2>/dev/null | tail -n +2 | tr '\n' ' '
	@printf '\033[0m\n'
endef

test: $(TARGET)
	./$(TARGET) test.cm test.s
	@echo "\n=== Generated MIPS Code ==="
	@cat test.s

# Activity 2: If-statement integration test
test-if: $(TARGET)
	@echo "=== Compiling test_if.cm ==="
	./$(TARGET) test_if.cm test_if.s 2>/dev/null
	@echo "\n=== Merge-point labels in assembly ==="
	@grep -n "merge point" test_if.s || true
	@echo "\n=== Running in SPIM ==="
	@echo "Expected: 1 2 3 4 5 6 6 6 6 6 6 7 8  (one per line)"
	@echo "---"
	@spim -file test_if.s 2>/dev/null | tail -n +2

# ── Individual test targets ────────────────────────────────────────────────────

test-arithmetic: $(TARGET)
	$(call run_test,test_arithmetic,13 7 30 3 19 39 91 78 10)

test-functions: $(TARGET)
	$(call run_test,test_functions,7 30 49 15 20 25 0 3 60)

test-loops: $(TARGET)
	$(call run_test,test_loops,15 10 0 25 25 120 7 21)

test-arrays: $(TARGET)
	$(call run_test,test_arrays,10 30 50 150 30 3 50 10)

test-comparisons: $(TARGET)
	$(call run_test,test_comparisons,1 0 1 0 0 1 0 0 1 1 1 0 1 1 0 0 0)

test-factorial: $(TARGET)
	$(call run_test,test_factorial,1 2 6 24 120 1 1 8 13 10)

# ── Run every test suite in one shot ──────────────────────────────────────────
test-all: $(TARGET)
	@printf '\n\033[1;35m████████████████████████████████████████████\033[0m\n'
	@printf '\033[1;35m  CST-405 Compiler — Full Test Suite\033[0m\n'
	@printf '\033[1;35m████████████████████████████████████████████\033[0m\n'
	$(MAKE) -s test-if
	$(MAKE) -s test-arithmetic
	$(MAKE) -s test-functions
	$(MAKE) -s test-loops
	$(MAKE) -s test-arrays
	$(MAKE) -s test-comparisons
	$(MAKE) -s test-factorial
	@printf '\n\033[1;32m✓ All tests complete.\033[0m\n'

.PHONY: all clean test test-if test-arithmetic test-functions test-loops test-arrays test-comparisons test-factorial test-all
