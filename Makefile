CC ?= cc
BISON ?= bison
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
CPPFLAGS ?= -Iinclude
PARSER_SRC := Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c Analisis_Sintactico/ast.c
SRC := src/main.c src/cli.c src/lexer.c src/token.c $(PARSER_SRC)

.PHONY: all test test-parser test-ast clean
all: balc

# Objetivos agrupados: una sola invocacion incluso con make -j (GNU Make >= 4.3).
Analisis_Sintactico/parser.c Analisis_Sintactico/parser.h &: Analisis_Sintactico/gramatica.y
	$(BISON) -Wall -Werror -d -o Analisis_Sintactico/parser.c $<

balc: $(SRC) include/cli.h include/lexer.h include/token.h include/sintactico.h include/ast.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@

tests/test_lexer: tests/test_lexer.c src/lexer.c src/token.c include/lexer.h include/token.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c src/token.c -o $@

test: tests/test_lexer balc test-parser test-ast
	./tests/test_lexer
	./balc -v examples/factorial.bal

tests/test_parser: tests/test_parser.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

test-parser: balc tests/test_parser
	./tests/test_parser

tests/test_ast: tests/test_ast.c $(PARSER_SRC) src/lexer.c src/token.c include/ast.h include/sintactico.h include/lexer.h include/token.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_ast.c $(PARSER_SRC) src/lexer.c src/token.c -o $@

test-ast: tests/test_ast
	./tests/test_ast

clean:
	rm -f balc tests/test_lexer tests/test_parser tests/test_ast
