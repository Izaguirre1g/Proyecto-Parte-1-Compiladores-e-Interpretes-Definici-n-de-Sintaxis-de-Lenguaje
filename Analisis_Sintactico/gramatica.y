/* Primera etapa: declaraciones, asignaciones, expresiones, bloques, condicionales y ciclos.
 * Incluye funciones, parametros, give y llamadas. Bison construye el AST usando las funciones de ast.c.
 * puente_lexer.c traduce TOKEN_* a los tokens de Bison mediante yylex.
 */

/* Cada analisis recibe su propio contexto; no se comparte un lexer global. */
%define api.pure full
%define parse.error detailed
%define parse.lac full
/* Cada simbolo transporta un puntero a un nodo. Ast es dueno de su memoria. */
%define api.value.type {AstNodo *}
%locations
%define api.location.type {AstUbicacion}
%initial-action { @$ = (AstUbicacion){1, 1, 1, 1}; }
%parse-param { ContextoSintactico *ctx }
%lex-param { ContextoSintactico *ctx }

%code requires {
#include "sintactico.h"
}

%code provides {
int yylex(YYSTYPE *valor, YYLTYPE *ubicacion, ContextoSintactico *ctx);
void yyerror(YYLTYPE *ubicacion, ContextoSintactico *ctx, const char *mensaje);
}

/* Tokens: las piezas que recibe el parser desde el lexer de Javier. */
%token IDENTIFIER INTEGER STRING CHARACTER
%token DECLARE_INT DECLARE_BOOLEAN DECLARE_TEXT DECLARE_CHAR
%token DECLARE_TRUE DECLARE_FALSE
%token STAR ASSIGN SEMICOLON COMMA
%token GAUSS NEUMANN PITAGORAS EUCLIDES
%token LPAREN RPAREN
%token LBRACE RBRACE WHETHER ALIF ALSO
%token WHALE STOP GIVE
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

/* Primera forma de programa: una o mas funciones con parametros opcionales.
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
      {
        $$ = ast_crear(ctx->arbol, AST_PROGRAMA, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ctx->arbol->raiz = $$;
      }
  | programa funcion
      { $$ = $1; $$->ubicacion = @$; ast_agregar_hijo($$, $2); }
;

/* Ejemplo: create_funk #declare_infinite_void# main() { ... } */
funcion:
    CREATE_FUNK HASH tipo_retorno HASH nombre_funcion LPAREN parametros_opcionales RPAREN bloque
      {
        $$ = ast_crear(ctx->arbol, AST_FUNCION, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $5);
        ast_agregar_hijo($$, $3);
        ast_agregar_hijo($$, $7);
        ast_agregar_hijo($$, $9);
      }
;

/* Los parentesis pueden estar vacios o contener parametros separados por comas. */
parametros_opcionales:
    %empty
      {
        $$ = ast_crear(ctx->arbol, AST_PARAMETROS, NULL, @$);
        if (!$$) YYNOMEM;
      }
  | parametros
      { $$ = $1; }
;

parametros:
    parametro
      {
        $$ = ast_crear(ctx->arbol, AST_PARAMETROS, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
      }
  | parametros COMMA parametro
      { $$ = $1; $$->ubicacion = @$; ast_agregar_hijo($$, $3); }
;

/* Sintaxis adoptada: nombre * tipo, sin valor inicial ni punto y coma.
 * Ejemplo: a * declare_int, b * declare_int
 */
parametro:
    IDENTIFIER STAR tipo
      {
        $$ = ast_crear(ctx->arbol, AST_PARAMETRO, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
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
      {
        $$ = ast_crear(ctx->arbol, AST_BLOQUE, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
      }
  | sentencias sentencia
      { $$ = $1; ast_agregar_hijo($$, $2); }
;

/* Los bloques, condicionales y ciclos son sentencias: pueden anidarse. */
sentencia:
    declaracion
  | asignacion
  | bloque
  | condicional
  | ciclo
  | retorno
  | llamada SEMICOLON
;

/* El tipo de retorno y la presencia de valor se verifican semantica. */
retorno:
    GIVE expresion SEMICOLON
      {
        $$ = ast_crear(ctx->arbol, AST_RETORNO, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $2);
      }
  | GIVE SEMICOLON
      {
        $$ = ast_crear(ctx->arbol, AST_RETORNO, NULL, @$);
        if (!$$) YYNOMEM;
      }
;

/* Una llamada puede usarse como expresion o como sentencia seguida de ;.
 * Cada argumento es una expresion: permite operaciones y llamadas anidadas.
 */
llamada:
    nombre_funcion LPAREN argumentos_opcionales RPAREN
      {
        $$ = ast_crear(ctx->arbol, AST_LLAMADA, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
;

argumentos_opcionales:
    %empty
      {
        $$ = ast_crear(ctx->arbol, AST_ARGUMENTOS, NULL, @$);
        if (!$$) YYNOMEM;
      }
  | argumentos
      { $$ = $1; }
;

argumentos:
    expresion
      {
        $$ = ast_crear(ctx->arbol, AST_ARGUMENTOS, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
      }
  | argumentos COMMA expresion
      { $$ = $1; $$->ubicacion = @$; ast_agregar_hijo($$, $3); }
;

/* Un bloque tiene llaves obligatorias y puede estar vacio. */
bloque:
    LBRACE contenido_bloque RBRACE
      { $$ = $2; $$->ubicacion = @$; }
;

contenido_bloque:
    %empty
      {
        $$ = ast_crear(ctx->arbol, AST_BLOQUE, NULL, @$);
        if (!$$) YYNOMEM;
      }
  | sentencias
      { $$ = $1; }
;

/* whether (condicion) { ... }, cero o mas alif y un also final opcional.
 * Las llaves delimitan cada rama, incluso cuando hay otro whether dentro.
 * No se escribe punto y coma despues de estas estructuras.
 */
condicional:
    WHETHER LPAREN expresion RPAREN bloque alternativa
      {
        $$ = ast_crear(ctx->arbol, AST_SI, "whether", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $3);
        ast_agregar_hijo($$, $5);
        ast_agregar_hijo($$, $6);
      }
;

/* La recursion permite varios alif; ALSO cierra la cadena de alternativas. */
alternativa:
    %empty
      { $$ = NULL; }
  | ALIF LPAREN expresion RPAREN bloque alternativa
      {
        $$ = ast_crear(ctx->arbol, AST_SI, "alif", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $3);
        ast_agregar_hijo($$, $5);
        ast_agregar_hijo($$, $6);
      }
  | ALSO bloque
      { $$ = $2; }
;

/* Sigue el ejemplo de Javier: whale (condicion) { ... } stop;
 * En esta regla STOP cierra el ciclo; no es una sentencia break independiente.
 * Las llaves, stop y el punto y coma son obligatorios.
 */
ciclo:
    WHALE LPAREN expresion RPAREN bloque STOP SEMICOLON
      {
        $$ = ast_crear(ctx->arbol, AST_MIENTRAS, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $3);
        ast_agregar_hijo($$, $5);
      }
;

/* Regla principal. Ejemplo: n * declare_int : 5; */
declaracion:
    IDENTIFIER STAR tipo ASSIGN expresion SEMICOLON
      {
        $$ = ast_crear(ctx->arbol, AST_DECLARACION, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
        ast_agregar_hijo($$, $5);
      }
;

/* Regla para asignar valores a variables; Dylan verificara su declaracion.
 * Ejemplo: resultado : 1;
 */
asignacion:
    IDENTIFIER ASSIGN expresion SEMICOLON
      {
        $$ = ast_crear(ctx->arbol, AST_ASIGNACION, NULL, @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
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
  | llamada
  | expresion EQUAL expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "==", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion NOT_EQUAL expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "=/=", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion LESS expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "<", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion GREATER expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, ">", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion LESS_EQUAL expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "<=", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion GREATER_EQUAL expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, ">=", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion GAUSS expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "gauss", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion NEUMANN expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "neumann", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion PITAGORAS expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "pitagoras", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | expresion EUCLIDES expresion
      {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "euclides", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
      }
  | NEUMANN expresion %prec NEGATIVO
      {
        $$ = ast_crear(ctx->arbol, AST_UNARIO, "neumann", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $2);
      }
  | LPAREN expresion RPAREN
      { $$ = $2; } /* Los parentesis ya cumplieron su funcion de agrupacion. */
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
