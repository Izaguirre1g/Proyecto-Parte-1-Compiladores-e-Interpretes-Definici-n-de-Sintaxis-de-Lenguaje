CC ?= cc
BISON ?= bison
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
CPPFLAGS ?= -Iinclude
PARSER_SRC := Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c
SRC := src/main.c src/cli.c src/lexer.c src/token.c $(PARSER_SRC)

.PHONY: all test test-parser clean
all: micomp

# Objetivos agrupados: una sola invocacion incluso con make -j (GNU Make >= 4.3).
Analisis_Sintactico/parser.c Analisis_Sintactico/parser.h &: Analisis_Sintactico/gramatica.y
	$(BISON) -Wall -Werror -d -o Analisis_Sintactico/parser.c $<

micomp: $(SRC) include/cli.h include/lexer.h include/token.h include/sintactico.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@

tests/test_lexer: tests/test_lexer.c src/lexer.c src/token.c include/lexer.h include/token.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c src/token.c -o $@

test: tests/test_lexer micomp test-parser
	./tests/test_lexer
	./micomp -v examples/factorial.bal

tests/test_parser: tests/test_parser.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

test-parser: micomp tests/test_parser
	./tests/test_parser

clean:
	rm -f micomp tests/test_lexer tests/test_parser
