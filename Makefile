CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
CPPFLAGS ?= -Iinclude
SRC := src/main.c src/cli.c src/lexer.c src/token.c

.PHONY: all test clean
all: micomp

micomp: $(SRC) include/cli.h include/lexer.h include/token.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@

tests/test_lexer: tests/test_lexer.c src/lexer.c src/token.c include/lexer.h include/token.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c src/token.c -o $@

test: tests/test_lexer micomp
	./tests/test_lexer
	./micomp -v examples/factorial.bal

clean:
	rm -f micomp tests/test_lexer
