release:
	gcc -O3 -Wall -Wextra  json_parser.c lexer.c parser.c -o json_parser

debug:
	gcc -g -O0 -Wall -Wextra json_parser.c lexer.c parser.c -o json_parser

test:
	gcc -O3 -Wall -Wextra  run_tests.c lexer.c parser.c -o json_parser_tests

all: test debug

clean:
	rm -rf json_parser json_parser_tests ./*.dSYM