# Herramientas que usamos para compilar el proyecto.
CC ?= cc
BISON ?= bison
FLEX ?= flex

# Opciones de compilación: C11, advertencias y optimización.
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

# Carpetas donde se buscan los archivos .h.
CPPFLAGS ?= -Iinclude -Isrc

# Archivos del parser y del AST.
PARSER_SRC := Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c Analisis_Sintactico/ast.c

# Archivo C que genera Flex a partir de lexer.l.
LEXER_GEN := src/lexer_flex.c

# Archivos que forman el compilador.
SRC := src/main.c src/cli.c src/lexer.c $(LEXER_GEN) src/token.c src/generacion_basica.c $(PARSER_SRC)

# Estos nombres son comandos de Make, no archivos.
.PHONY: all test test-parser test-ast test-cli test-generacion clean

# Al ejecutar make, se construye balc.
all: balc

# Flex convierte las reglas de lexer.l en código C.
# $@ es el archivo generado y $< es lexer.l.
$(LEXER_GEN): src/lexer.l src/lexer_flex.h include/token.h
	$(FLEX) -o $@ $<

# Bison genera parser.c y parser.h usando la gramática.
# &: evita generar ambos archivos dos veces con make -j.
Analisis_Sintactico/parser.c Analisis_Sintactico/parser.h &: Analisis_Sintactico/gramatica.y
	$(BISON) -Wall -Werror -d -o Analisis_Sintactico/parser.c $<

# Compila todos los módulos y crea el ejecutable balc.
balc: $(SRC) include/cli.h include/lexer.h include/token.h include/sintactico.h include/ast.h include/generacion_basica.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@

# Compila las pruebas del lexer, sin necesitar el parser.
tests/test_lexer: tests/test_lexer.c src/lexer.c $(LEXER_GEN) src/token.c include/lexer.h include/token.h src/lexer_flex.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_lexer.c src/lexer.c $(LEXER_GEN) src/token.c -o $@

# Ejecuta las pruebas del proyecto y analiza el factorial.
test: tests/test_lexer balc test-parser test-ast test-cli test-generacion
	./tests/test_lexer
	./balc -v examples/factorial.bal

# Compila las pruebas del parser.
tests/test_parser: tests/test_parser.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

# Ejecuta las pruebas sintácticas.
test-parser: balc tests/test_parser
	./tests/test_parser

# Compila las pruebas del AST.
tests/test_ast: tests/test_ast.c $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c include/ast.h include/sintactico.h include/lexer.h include/token.h src/lexer_flex.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_ast.c $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c -o $@

# Comprueba que el árbol se construya correctamente.
test-ast: tests/test_ast
	./tests/test_ast

# Compila las pruebas de los comandos de la terminal.
tests/test_cli: tests/test_cli.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

# Prueba las opciones y respuestas del ejecutable.
test-cli: balc tests/test_cli
	./tests/test_cli

# Compila las pruebas de generación básica de ensamblador.
tests/test_generacion: tests/test_generacion.c src/generacion_basica.c include/generacion_basica.h $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c include/ast.h include/sintactico.h include/lexer.h include/token.h src/lexer_flex.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_generacion.c src/generacion_basica.c $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c -o $@

# Comprueba la traducción de operaciones básicas.
test-generacion: tests/test_generacion
	./tests/test_generacion

# Elimina los ejecutables y el archivo generado por Flex.
# Sirve para volver a compilar todo desde cero.
clean:
	rm -f balc tests/test_lexer tests/test_parser tests/test_ast tests/test_cli tests/test_generacion $(LEXER_GEN)
