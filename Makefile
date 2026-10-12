# Herramientas que usamos para compilar el proyecto.
CC ?= cc
BISON ?= bison
FLEX ?= flex

# Evita que Make aplique reglas implicitas que sobrescriban lexer.c.
.SUFFIXES:

# Opciones de compilación: C11, advertencias y optimización.
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -Werror -O2

# Carpetas donde se buscan los archivos .h.
CPPFLAGS ?= -Iinclude -Isrc 

# Archivos del parser y del AST.
PARSER_SRC := Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c Analisis_Sintactico/ast.c

# Archivo C que genera Flex a partir de lexer.l.
LEXER_GEN := src/lexer_flex.c

# Archivos que forman el compilador.
SRC := src/main.c src/cli.c src/lexer.c $(LEXER_GEN) src/token.c src/generacion_basica.c $(PARSER_SRC) Analisis_Semantico/symbol_table.c Analisis_Semantico/semantic.c

# Estos nombres son comandos de Make, no archivos.
.PHONY: all test test-parser test-ast test-cli test-generacion test-semantic-modules clean

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
test: tests/test_lexer balc test-parser test-ast test-cli test-generacion test-symbol-table test-semantic test-semantic-functions test-semantic-arrays test-semantic-matrix-cycle test-semantic-returns test-semantic-initializers test-semantic-dynamic-arrays test-semantic-lists test-semantic-list-indices test-semantic-composite-params test-semantic-pipeline test-semantic-modules
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
# Elimina los ejecutables y archivos generados.
clean:
	rm -f balc tests/test_lexer tests/test_parser tests/test_ast tests/test_cli tests/test_generacion tests/test_symbol_table tests/test_semantic tests/test_semantic_functions tests/test_semantic_arrays tests/test_semantic_matrix_cycle tests/test_semantic_returns test/test_semantic_dynamic_arrays test/test_semantic_lists test/test_semantic_list_indices test/test_semantic_composite_params test/test_semantic_pipeline tests/test_semantic_modules $(LEXER_GEN)

# Pruebas de la tabla de símbolos
.PHONY: test-symbol-table

tests/test_symbol_table: tests/test_symbol_table.c \
                        Analisis_Semantico/symbol_table.c \
                        include/symbol_table.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_symbol_table.c \
	    Analisis_Semantico/symbol_table.c \
	    -o $@

# Ejecutar pruebas de la tabla de símbolos
test-symbol-table: tests/test_symbol_table
	./tests/test_symbol_table

# Compilar pruebas del análisis semántico.
tests/test_semantic: tests/test_semantic.c \
                     Analisis_Semantico/semantic.c \
                     Analisis_Semantico/symbol_table.c \
                     Analisis_Sintactico/ast.c \
                     include/semantic.h \
                     include/symbol_table.h \
                     include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

# Ejecutar pruebas del análisis semántico.
.PHONY: test-semantic

test-semantic: tests/test_semantic
	./tests/test_semantic

# Compilar pruebas semánticas de funciones.
tests/test_semantic_functions: tests/test_semantic_functions.c \
                               Analisis_Semantico/semantic.c \
                               Analisis_Semantico/symbol_table.c \
                               Analisis_Sintactico/ast.c \
                               include/semantic.h \
                               include/symbol_table.h \
                               include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_functions.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

# Ejecutar pruebas de funciones.
.PHONY: test-semantic-functions

test-semantic-functions: tests/test_semantic_functions
	./tests/test_semantic_functions

# Compilar pruebas de arreglos.
tests/test_semantic_arrays: tests/test_semantic_arrays.c \
                            Analisis_Semantico/semantic.c \
                            Analisis_Semantico/symbol_table.c \
                            Analisis_Sintactico/ast.c \
                            include/semantic.h \
                            include/symbol_table.h \
                            include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_arrays.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-arrays

test-semantic-arrays: tests/test_semantic_arrays
	./tests/test_semantic_arrays

# =============================================
# Pruebas de matrices bidimensionales y cycle
# =============================================

tests/test_semantic_matrix_cycle: tests/test_semantic_matrix_cycle.c \
                                  Analisis_Semantico/semantic.c \
                                  Analisis_Semantico/symbol_table.c \
                                  Analisis_Sintactico/ast.c \
                                  include/semantic.h \
                                  include/symbol_table.h \
                                  include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_matrix_cycle.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-matrix-cycle

test-semantic-matrix-cycle: tests/test_semantic_matrix_cycle
	./tests/test_semantic_matrix_cycle

# ==========================================
# Pruebas de retornos obligatorios
# ==========================================

tests/test_semantic_returns: tests/test_semantic_returns.c \
                             Analisis_Semantico/semantic.c \
                             Analisis_Semantico/symbol_table.c \
                             Analisis_Sintactico/ast.c \
                             include/semantic.h \
                             include/symbol_table.h \
                             include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_returns.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-returns

test-semantic-returns: tests/test_semantic_returns
	./tests/test_semantic_returns

# ===========================================
# Pruebas de inicializadores de matrices
# ===========================================

tests/test_semantic_initializers: tests/test_semantic_initializers.c \
                                  Analisis_Semantico/semantic.c \
                                  Analisis_Semantico/symbol_table.c \
                                  Analisis_Sintactico/ast.c \
                                  include/semantic.h \
                                  include/symbol_table.h \
                                  include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_initializers.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-initializers

test-semantic-initializers: tests/test_semantic_initializers
	./tests/test_semantic_initializers

# ===========================================
# Pruebas de dimensiones dinamicas
# ===========================================

tests/test_semantic_dynamic_arrays: tests/test_semantic_dynamic_arrays.c \
                                    Analisis_Semantico/semantic.c \
                                    Analisis_Semantico/symbol_table.c \
                                    Analisis_Sintactico/ast.c \
                                    include/semantic.h \
                                    include/symbol_table.h \
                                    include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_dynamic_arrays.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-dynamic-arrays

test-semantic-dynamic-arrays: tests/test_semantic_dynamic_arrays
	./tests/test_semantic_dynamic_arrays

# ===========================================
# Pruebas semanticas de listas
# ===========================================

tests/test_semantic_lists: tests/test_semantic_lists.c \
                           Analisis_Semantico/semantic.c \
                           Analisis_Semantico/symbol_table.c \
                           Analisis_Sintactico/ast.c \
                           include/semantic.h \
                           include/symbol_table.h \
                           include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_lists.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-lists

test-semantic-lists: tests/test_semantic_lists
	./tests/test_semantic_lists

# ===========================================
# Pruebas de indices de listas
# ===========================================

tests/test_semantic_list_indices: tests/test_semantic_list_indices.c \
                                  Analisis_Semantico/semantic.c \
                                  Analisis_Semantico/symbol_table.c \
                                  Analisis_Sintactico/ast.c \
                                  include/semantic.h \
                                  include/symbol_table.h \
                                  include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_list_indices.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-list-indices

test-semantic-list-indices: tests/test_semantic_list_indices
	./tests/test_semantic_list_indices

# ===========================================
# Pruebas de parametros compuestos
# ===========================================

tests/test_semantic_composite_params: tests/test_semantic_composite_params.c \
                                      Analisis_Semantico/semantic.c \
                                      Analisis_Semantico/symbol_table.c \
                                      Analisis_Sintactico/ast.c \
                                      include/semantic.h \
                                      include/symbol_table.h \
                                      include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_composite_params.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    Analisis_Sintactico/ast.c \
	    -o $@

.PHONY: test-semantic-composite-params

test-semantic-composite-params: tests/test_semantic_composite_params
	./tests/test_semantic_composite_params

# ===========================================
# ETAPA 2K
# Integracion parser + analisis semantico
# ===========================================

tests/test_semantic_pipeline: \
    tests/test_semantic_pipeline.c \
    Analisis_Sintactico/parser.c \
    Analisis_Sintactico/puente_lexer.c \
    Analisis_Sintactico/ast.c \
    Analisis_Semantico/semantic.c \
    Analisis_Semantico/symbol_table.c \
    src/lexer.c \
    src/lexer_flex.c \
    src/token.c \
    include/semantic.h \
    include/symbol_table.h \
    include/ast.h
	$(CC) $(CPPFLAGS) $(CFLAGS) \
	    tests/test_semantic_pipeline.c \
	    Analisis_Sintactico/parser.c \
	    Analisis_Sintactico/puente_lexer.c \
	    Analisis_Sintactico/ast.c \
	    Analisis_Semantico/semantic.c \
	    Analisis_Semantico/symbol_table.c \
	    src/lexer.c \
	    src/lexer_flex.c \
	    src/token.c \
	    -o $@

.PHONY: test-semantic-pipeline

test-semantic-pipeline: tests/test_semantic_pipeline
	@echo "Pipeline compilado correctamente."
	@echo "Ejecuta el programa con un archivo BAL y su resultado esperado."

# ===========================================
# ETAPA 2N-F: integracion de pruebas de modulos
# ===========================================

MODULOS_SRC := Analisis_Semantico/semantic_modules.c \
               Analisis_Semantico/semantic.c \
               Analisis_Semantico/symbol_table.c \
               $(PARSER_SRC) src/lexer.c $(LEXER_GEN) src/token.c

# Compilacion independiente: el CLI y el backend no participan.
tests/test_semantic_modules: tests/test_semantic_modules.c $(MODULOS_SRC) \
                             include/semantic_modules.h include/semantic.h \
                             include/symbol_table.h include/ast.h \
                             include/sintactico.h Analisis_Sintactico/parser.h
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/test_semantic_modules.c $(MODULOS_SRC) -o $@

# Casos positivos, negativos, transitivos, compartidos y rutas equivalentes.
# El ejecutable compara el codigo real (0, 1 o 2) con el esperado.
test-semantic-modules: tests/test_semantic_modules
	./tests/test_semantic_modules tests/modulos/principal_valido.bal 0
	./tests/test_semantic_modules tests/modulos/principal_inexistente.bal 1
	./tests/test_semantic_modules tests/modulos/principal_ciclo.bal 1
	./tests/test_semantic_modules tests/modulos/principal_modulo_invalido.bal 1
	./tests/test_semantic_modules tests/modulos/principal_llamada.bal 0
	./tests/test_semantic_modules tests/modulos/principal_funcion_inexistente.bal 1
	./tests/test_semantic_modules tests/modulos/principal_argumentos_error.bal 1
	./tests/test_semantic_modules tests/modulos/principal_tipo_error.bal 1
	./tests/test_semantic_modules tests/modulos/principal_doble_importacion.bal 0
	./tests/test_semantic_modules tests/modulos/principal_alias_duplicado.bal 1
	./tests/test_semantic_modules tests/modulos/principal_constante.bal 0
	./tests/test_semantic_modules tests/modulos/principal_alias_inexistente.bal 1
	./tests/test_semantic_modules tests/modulos/principal_anidado.bal 0
	./tests/test_semantic_modules tests/modulos/principal_compartido.bal 0
	./tests/test_semantic_modules tests/modulos/principal_dependencia_error.bal 1
	./tests/test_semantic_modules tests/modulos/principal_privacidad.bal 1
	./tests/test_semantic_modules tests/modulos/principal_arreglo_importado.bal 0
	./tests/test_semantic_modules tests/modulos/principal_arreglo_error.bal 1
	./tests/test_semantic_modules tests/modulos/./principal_llamada.bal 0
	./tests/test_semantic_modules tests/modulos/./principal_ciclo.bal 1
	@echo "OK: 20 escenarios de modulos BAL (sin offsets)"
