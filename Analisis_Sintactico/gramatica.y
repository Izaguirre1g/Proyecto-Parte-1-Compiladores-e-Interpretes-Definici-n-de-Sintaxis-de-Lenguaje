/* Primera etapa: declaraciones, asignaciones, aritmetica y comparaciones.
 * Todavia no incluye funciones, bloques ni construccion del AST.
 * Los nombres de Bison se mapearan a TOKEN_* mediante un adaptador yylex.
 */

/* Tokens: las piezas que recibe el parser desde el lexer de Javier. */
%token IDENTIFIER INTEGER STRING CHARACTER
%token DECLARE_INT DECLARE_BOOLEAN DECLARE_TEXT DECLARE_CHAR
%token DECLARE_TRUE DECLARE_FALSE
%token STAR ASSIGN SEMICOLON
%token GAUSS NEUMANN PITAGORAS EUCLIDES
%token LPAREN RPAREN
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

/* Entrada provisional: una o mas sentencias, no un programa completo. */
%start sentencias

/* Reglas de gramatica entre separadores %% */
%%

/* Una sentencia, o una lista seguida por otra sentencia. */
sentencias:
    sentencia
  | sentencias sentencia
;

/* Cada sentencia puede ser una declaracion o una asignacion. */
sentencia:
    declaracion
  | asignacion
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

/* Pendiente: conectar yylex y yyerror con el proyecto existente.
 * Despues de este segundo separador solo va codigo C, no reglas.
 * Ejemplo de entrada cuando se conecte el lexer:
 * resultado * declare_int : 0;
 * resultado : 2 gauss 3 pitagoras 4;
 * cumple * declare_boolean : n <= 1;
 * cumple : i gauss 1 <= n;
 */
