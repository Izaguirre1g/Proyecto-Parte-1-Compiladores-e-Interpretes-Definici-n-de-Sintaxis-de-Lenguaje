/* Primera etapa: declaraciones con valor inicial y asignaciones.
 * Basada en los tokens del lexer de Javier y en examples/factorial.bal.
 * Todavia no incluye funciones, operaciones, bloques ni construccion del AST.
 * Los nombres de Bison se mapearan a TOKEN_* mediante un adaptador yylex.
 */

/* Tokens: elementos que entrega el analizador lexico. */
%token IDENTIFIER INTEGER STRING CHARACTER
%token DECLARE_INT DECLARE_BOOLEAN DECLARE_TEXT DECLARE_CHAR
%token DECLARE_TRUE DECLARE_FALSE
%token STAR ASSIGN SEMICOLON

/* Entrada provisional: una o mas sentencias, no un programa completo. */
%start sentencias

%%

/* Una sentencia, o una lista seguida por otra sentencia. */
sentencias:
    sentencia
  | sentencias sentencia
;

/* Permite mezclar declaraciones y asignaciones. */
sentencia:
    declaracion
  | asignacion
;

/* Ejemplo: n * declare_int : 5; */
declaracion:
    IDENTIFIER STAR tipo ASSIGN expresion SEMICOLON
;

/* Ejemplo: resultado : 1; */
asignacion:
    IDENTIFIER ASSIGN expresion SEMICOLON
;

tipo:
    DECLARE_INT
  | DECLARE_BOOLEAN
  | DECLARE_TEXT
  | DECLARE_CHAR
;

/* Por ahora, solo valores simples o referencias a variables. */
expresion:
    INTEGER
  | STRING
  | CHARACTER
  | DECLARE_TRUE
  | DECLARE_FALSE
  | IDENTIFIER
;

%%

/* Pendiente: conectar yylex y yyerror con el proyecto existente. */
