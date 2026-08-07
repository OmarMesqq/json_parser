VALGRIND_LOG := /tmp/json_parser_valgrind.log
CACHEGRIND_LOG := /tmp/cachegrind.out
OUTPUT := /tmp/json_parser
TEST_OUTPUT := /tmp/json_parser_tests
CFLAGS_COMMON = -Wall -Wextra \
								-Winline -Wshadow \
								-Wconversion -Wsign-conversion -Wdouble-promotion \

SOURCES = main.c lexer.c parser.c
TEST_SOURCES = runner.c lexer.c parser.c

# JSON parser tasks
release: CFLAGS = $(CFLAGS_COMMON) -O3
release:
	gcc $(CFLAGS) $(SOURCES) -o $(OUTPUT)

debug: CFLAGS = $(CFLAGS_COMMON) -g -O0 \
								-fsanitize=address,undefined -fsanitize-trap=undefined \
								-fno-omit-frame-pointer -fstrict-overflow \
								-funwind-tables -fasynchronous-unwind-tables
debug:
	gcc $(CFLAGS) $(SOURCES) -o $(OUTPUT)

profile: CFLAGS = $(CFLAGS_COMMON) -g -O3
profile:
	gcc $(CFLAGS) $(SOURCES) -o $(OUTPUT)

# Test runner
test: CFLAGS = $(CFLAGS_COMMON) -g
test:
	gcc $(CFLAGS) $(TEST_SOURCES) -o $(TEST_OUTPUT)

# Resource leaks and profiling
memleak-check: test
	@valgrind -s --leak-check=full --track-origins=yes --show-leak-kinds=all $(TEST_OUTPUT) 2> $(VALGRIND_LOG)
	@grep -Fq "All heap blocks were freed -- no leaks are possible" $(VALGRIND_LOG) && \
	grep -Fq "ERROR SUMMARY: 0 errors from 0 contexts" $(VALGRIND_LOG) && \
	echo "✅ No leaks or errors detected." || \
	(echo "❌ Memory/resource leaks or errors found!"; cat $(VALGRIND_LOG); exit 1)

cachegrind: profile
	valgrind --tool=cachegrind --cachegrind-out-file=$(CACHEGRIND_LOG) $(OUTPUT) ./tests/custom/2_million_ints_4M.json
	cg_annotate $(CACHEGRIND_LOG)

clean:
	rm -rf $(VALGRIND_LOG) $(OUTPUT) $(TEST_OUTPUT) $(CACHEGRIND_LOG)