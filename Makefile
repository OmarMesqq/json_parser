release:
	gcc -O3 -Wall -Wextra  main.c lexer.c parser.c -o zon

debug:
	gcc -g -O0 -Wall -Wextra main.c lexer.c parser.c -o zon

clean:
	rm -f zon