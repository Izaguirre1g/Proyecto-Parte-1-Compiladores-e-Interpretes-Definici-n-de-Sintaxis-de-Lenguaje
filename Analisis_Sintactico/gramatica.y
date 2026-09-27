/* Primera etapa: declaraciones, asignaciones, expresiones, bloques, condicionales y ciclos.
 * Incluye funciones sin parametros. Pendientes: parametros, llamadas, give y AST.
 * puente_lexer.c traduce TOKEN_* a los tokens de Bison mediante yylex.
 */

/* Cada analisis recibe su propio contexto; no se comparte un lexer global. */
%define api.pure full
%define parse.error detailed
%define parse.lac full
%parse-param { ContextoSintactico *ctx }
%lex-param { ContextoSintactico *ctx }

%code requires {
#include "sintactico.h"
}

%code provides {
int yylex(YYSTYPE *valor, ContextoSintactico *ctx);
void yyerror(ContextoSintactico *ctx, const char *mensaje);
}

/* Tokens: las piezas que recibe el parser desde el lexer de Javier. */
%token IDENTIFIER INTEGER STRING CHARACTER
%token DECLARE_INT DECLARE_BOOLEAN DECLARE_TEXT DECLARE_CHAR
%token DECLARE_TRUE DECLARE_FALSE
%token STAR ASSIGN SEMICOLON
%token GAUSS NEUMANN PITAGORAS EUCLIDES
%token LPAREN RPAREN
%token LBRACE RBRACE WHETHER ALSO
%token WHALE STOP
%token CREATE_FUNK HASH MAIN DECLARE_INFINITE_VOID
%token EQUAL NOT_EQUAL LESS GREATER LESS_EQUAL GREATER_EQUAL

/* Precedencia propuesta, de menor a mayor. %left asocia a la izquierda.
 * GAUSS: suma; NEUMANN: resta; PITAGORAS: multiplicacion; EUCLIDES: division.
 * NEGATIVO es una marca interna de precedencia, no un token del lexer.
 */
/* Comparaciones: menor prioridad que la aritmetica.
 * %nonassoc rechaza cadenas sin parentesis como a < b < c.
 * EQUAL: ==; NOT_EQUAL: =/=; LESS: <; GREATER: >;
 * LESS_EQUAL: <=; GREATER_EQUAL: >=.
 */
%nonassoc EQUAL NOT_EQUAL LESS GREATER LESS_EQUAL GREATER_EQUAL
%left GAUSS NEUMANN
%left PITAGORAS EUCLIDES
%precedence NEGATIVO

/* Primera forma de programa: una o mas funciones sin parametros.
 * Las sentencias quedan dentro de los cuerpos de las funciones.
 */
%start programa

/* Reglas de gramatica entre separadores %% */
%%

/* Las funciones se declaran al nivel del programa, no dentro de sentencias.
 * La existencia y unicidad de main se verificaran en la etapa semantica.
 */
programa:
    funcion
  | programa funcion
;

/* Ejemplo: create_funk #declare_infinite_void# main() { ... } */
funcion:
    CREATE_FUNK HASH tipo_retorno HASH nombre_funcion LPAREN RPAREN bloque
;

tipo_retorno:
    tipo
  | DECLARE_INFINITE_VOID
;

/* main tiene su propio token en el lexer; no es un IDENTIFIER. */
nombre_funcion:
    MAIN
  | IDENTIFIER
;

/* Una sentencia, o una lista seguida por otra sentencia. */
sentencias:
    sentencia
  | sentencias sentencia
;

/* Los bloques, condicionales y ciclos son sentencias: pueden anidarse. */
sentencia:
    declaracion
  | asignacion
  | bloque
  | condicional
  | ciclo
;

/* Un bloque tiene llaves obligatorias y puede estar vacio. */
bloque:
    LBRACE contenido_bloque RBRACE
;

contenido_bloque:
    %empty
  | sentencias
;

/* whether (condicion) { ... } con una rama also opcional.
 * Las llaves delimitan cada rama, incluso cuando hay otro whether dentro.
 * No se escribe punto y coma despues de estas estructuras.
 */
condicional:
    WHETHER LPAREN expresion RPAREN bloque alternativa
;

alternativa:
    %empty
  | ALSO bloque
;

/* Sigue el ejemplo de Javier: whale (condicion) { ... } stop;
 * En esta regla STOP cierra el ciclo; no es una sentencia break independiente.
 * Las llaves, stop y el punto y coma son obligatorios.
 */
ciclo:
    WHALE LPAREN expresion RPAREN bloque STOP SEMICOLON
;

/* Regla principal. Ejemplo: n * declare_int : 5; */
declaracion:
    IDENTIFIER STAR tipo ASSIGN expresion SEMICOLON
;

/* Regla para asignar valores a variables; Dylan verificara su declaracion.
 * Ejemplo: resultado : 1;
 */
asignacion:
    IDENTIFIER ASSIGN expresion SEMICOLON
;

tipo:
    DECLARE_INT
  | DECLARE_BOOLEAN
  | DECLARE_TEXT
  | DECLARE_CHAR
;

/* Valores, variables, operaciones, comparaciones y parentesis.
 * La compatibilidad de tipos se verifica en el analisis semantico.
 */
expresion:
    INTEGER
  | STRING
  | CHARACTER
  | DECLARE_TRUE
  | DECLARE_FALSE
  | IDENTIFIER
  | expresion EQUAL expresion
  | expresion NOT_EQUAL expresion
  | expresion LESS expresion
  | expresion GREATER expresion
  | expresion LESS_EQUAL expresion
  | expresion GREATER_EQUAL expresion
  | expresion GAUSS expresion
  | expresion NEUMANN expresion
  | expresion PITAGORAS expresion
  | expresion EUCLIDES expresion
  | NEUMANN expresion %prec NEGATIVO
  | LPAREN expresion RPAREN
;

%%

/* yylex y yyerror estan implementadas en puente_lexer.c.
 * Despues de este segundo separador solo va codigo C, no reglas.
 * Ejemplo de entrada:
 * create_funk #declare_infinite_void# main() {
 *     resultado * declare_int : 0;
 *     resultado : 2 gauss 3 pitagoras 4;
 *     cumple * declare_boolean : resultado <= 14;
 * }
 */
