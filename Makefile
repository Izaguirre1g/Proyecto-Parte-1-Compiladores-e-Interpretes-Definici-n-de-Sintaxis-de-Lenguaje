CC ?= cc
BISON ?= bison
FLEX ?= flex
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2
CPPFLAGS ?= -Iinclude -Isrc
PARSER_SRC := Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c Analisis_Sintactico/ast.c
LEXER_GEN := src/lexer_flex.c
SRC := src/main.c src/cli.c src/lexer.c $(LEXER_GEN) src/token.c src/generacion_basica.c $(PARSER_SRC)

.PHONY: all test test-parser test-ast test-cli test-generacion clean
all: balc

# El scanner se genera desde reglas declarativas; no se modifica la interfaz del lexer.
$(LEXER_GEN): src/lexer.l src/lexer_flex.h include/token.h
	$(FLEX) -o $@ $<

# Objetivos agrupados: una sola invocacion incluso con make -j (GNU Make >= 4.3).
Analisis_Sintactico/parser.c Analisis_Sintactico/parser.h &: Analisis_Sintactico/gramatica.y
	$(BISON) -Wall -Werror -d -o Analisis_Sintactico/parser.c $<

balc: $(SRC) include/cli.h include/lexer.h include/token.h include/sintactico.h include/ast.h include/generacion_basica.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@

tests/test_lexer: tests/test_lexer.c src/lexer.c $(LEXER_GEN) src/token.c include/lexer.h include/token.h src/lexer_flex.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c $(LEXER_GEN) src/token.c -o $@

test: tests/test_lexer balc test-parser test-ast test-cli test-generacion
	./tests/test_lexer
	./balc -v examples/factorial.bal

tests/test_parser: tests/test_parser.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

test-parser: balc tests/test_parser
	./tests/test_parser

tests/test_ast: tests/test_ast.c $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c include/ast.h include/sintactico.h include/lexer.h include/token.h src/lexer_flex.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_ast.c $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c -o $@

test-ast: tests/test_ast
	./tests/test_ast

tests/test_cli: tests/test_cli.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

test-cli: balc tests/test_cli
	./tests/test_cli

tests/test_generacion: tests/test_generacion.c src/generacion_basica.c include/generacion_basica.h $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c include/ast.h include/sintactico.h include/lexer.h include/token.h src/lexer_flex.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_generacion.c src/generacion_basica.c $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c -o $@

test-generacion: tests/test_generacion
	./tests/test_generacion

clean:
	rm -f balc tests/test_lexer tests/test_parser tests/test_ast tests/test_cli tests/test_generacion $(LEXER_GEN)
