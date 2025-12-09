VALGRIND_LOG := /tmp/json_parser_valgrind.log
OUTPUT := /tmp/json_parser
TEST_OUTPUT := /tmp/json_parser_tests

release:
	gcc -O3 -Wall -Wextra -Winline main.c lexer.c parser.c -o $(OUTPUT)

debug:
	gcc -g -O0 -Wall -Wextra -Winline main.c lexer.c parser.c -o $(OUTPUT)

test:
	gcc -g -Wall -Wextra -Winline runner.c lexer.c parser.c -o $(TEST_OUTPUT)

memleak-check: test
	@valgrind -s --leak-check=full --track-origins=yes --show-leak-kinds=all $(TEST_OUTPUT) 2> $(VALGRIND_LOG)
	@grep -Fq "All heap blocks were freed -- no leaks are possible" $(VALGRIND_LOG) && \
	grep -Fq "ERROR SUMMARY: 0 errors from 0 contexts" $(VALGRIND_LOG) && \
	echo "✅ No leaks or errors detected." || \
	(echo "❌ Memory/resource leaks or errors found!"; cat $(VALGRIND_LOG); exit 1)

clean:
	rm -rf $(VALGRIND_LOG) $(OUTPUT) $(TEST_OUTPUT)