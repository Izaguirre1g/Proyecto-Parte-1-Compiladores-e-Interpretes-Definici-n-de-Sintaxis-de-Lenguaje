#include "parser.h"

/* Traduccion explicita: las enumeraciones no tienen los mismos numeros. */
static int token_bison(TokenType tipo) {
    switch (tipo) {
        case TOKEN_EOF: return YYEOF; /* EOF es 0 en Bison. Informa que Bison ha terminado. */
        case TOKEN_IDENTIFIER: return IDENTIFIER;
        case TOKEN_INTEGER: return INTEGER;
        case TOKEN_STRING: return STRING;
        case TOKEN_CHARACTER: return CHARACTER;
        case TOKEN_DECLARE_INT: return DECLARE_INT;
        case TOKEN_DECLARE_BOOLEAN: return DECLARE_BOOLEAN;
        case TOKEN_DECLARE_TEXT: return DECLARE_TEXT;
        case TOKEN_DECLARE_CHAR: return DECLARE_CHAR;
        case TOKEN_DECLARE_TRUE: return DECLARE_TRUE;
        case TOKEN_DECLARE_FALSE: return DECLARE_FALSE;
        case TOKEN_STAR: return STAR;
        case TOKEN_ASSIGN: return ASSIGN;
        case TOKEN_SEMICOLON: return SEMICOLON;
        case TOKEN_COMMA: return COMMA;
        case TOKEN_GAUSS: return GAUSS;
        case TOKEN_NEUMANN: return NEUMANN;
        case TOKEN_PITAGORAS: return PITAGORAS;
        case TOKEN_EUCLIDES: return EUCLIDES;
        case TOKEN_LPAREN: return LPAREN;
        case TOKEN_RPAREN: return RPAREN;
        case TOKEN_LBRACE: return LBRACE;
        case TOKEN_RBRACE: return RBRACE;
        case TOKEN_WHETHER: return WHETHER;
        case TOKEN_ALIF: return ALIF;
        case TOKEN_ALSO: return ALSO;
        case TOKEN_WHALE: return WHALE;
        case TOKEN_STOP: return STOP;
        case TOKEN_GIVE: return GIVE;
        case TOKEN_CREATE_FUNK: return CREATE_FUNK;
        case TOKEN_HASH: return HASH;
        case TOKEN_MAIN: return MAIN;
        case TOKEN_DECLARE_INFINITE_VOID: return DECLARE_INFINITE_VOID;
        case TOKEN_EQUAL: return EQUAL;
        case TOKEN_NOT_EQUAL: return NOT_EQUAL;
        case TOKEN_LESS: return LESS;
        case TOKEN_GREATER: return GREATER;
        case TOKEN_LESS_EQUAL: return LESS_EQUAL;
        case TOKEN_GREATER_EQUAL: return GREATER_EQUAL;
        case TOKEN_EULER: return EULER;
        case TOKEN_DESCARTES: return DESCARTES;
        case TOKEN_AND: return AND;
        case TOKEN_OR: return OR;
        case TOKEN_NOT: return NOT;
        case TOKEN_XOR: return XOR;
        case TOKEN_CYCLE: return CYCLE;
        case TOKEN_LET: return LET;
        case TOKEN_UNTIL: return UNTIL;
        case TOKEN_STEP: return STEP;
        case TOKEN_ENDGAME: return ENDGAME;
        case TOKEN_LBRACKET: return LBRACKET;
        case TOKEN_RBRACKET: return RBRACKET;
        case TOKEN_DOT: return DOT;
        case TOKEN_DECLARE_LIST: return DECLARE_LIST;
        case TOKEN_ADD: return ADD;
        case TOKEN_REMOVE: return REMOVE;
        case TOKEN_SIZE: return SIZE;
        case TOKEN_BRING: return BRING;
        case TOKEN_AKA: return AKA;
        case TOKEN_DECLARE_CONST: return DECLARE_CONST;
        case TOKEN_SEEK: return SEEK;
        case TOKEN_SEIZE: return SEIZE;
        default: return YYUNDEF; /*indica que el token no está definido en la gramática*/
    }
}

/* Solo los tokens con datos crean hojas. Puntuacion y operadores se usan
 * en las reglas de Bison; no se guardan como nodos independientes.
 */
static int tipo_hoja(TokenType token, AstTipo *tipo) {
    switch (token) {
        case TOKEN_IDENTIFIER: case TOKEN_MAIN: *tipo = AST_IDENTIFICADOR; return 1;
        case TOKEN_INTEGER: *tipo = AST_ENTERO; return 1;
        case TOKEN_STRING: *tipo = AST_CADENA; return 1;
        case TOKEN_CHARACTER: *tipo = AST_CARACTER; return 1;
        case TOKEN_DECLARE_TRUE: case TOKEN_DECLARE_FALSE: *tipo = AST_BOOLEANO; return 1;
        case TOKEN_DECLARE_INT: case TOKEN_DECLARE_BOOLEAN:
        case TOKEN_DECLARE_TEXT: case TOKEN_DECLARE_CHAR:
        case TOKEN_DECLARE_INFINITE_VOID: *tipo = AST_TIPO; return 1;
        default: return 0;
    }
}

/* Bison llama a esta funcion cada vez que necesita otro token. */
int yylex(YYSTYPE *valor, YYLTYPE *ubicacion, ContextoSintactico *ctx) {
    *valor = NULL;
    Token token = lexer_next(&ctx->lexer);
    ctx->linea = token.line;
    ctx->columna = token.column;
    *ubicacion = (AstUbicacion){token.line, token.column,
                               ctx->lexer.line, ctx->lexer.column};
    int tipo = token_bison(token.type);
    if (token.type == TOKEN_ERROR) {
        ctx->estado = token.error == TOKEN_OUT_OF_MEMORY ? 2 : 1;
        fprintf(ctx->diagnosticos, "%s:%zu:%zu: error léxico: %s\n",
                ctx->lexer.filename, ctx->linea, ctx->columna,
                token_error_message(token.error));
        tipo = YYerror; /* El error ya fue informado. */
    } else if (tipo == YYUNDEF) {
        ctx->estado = 1;
        fprintf(ctx->diagnosticos,
                "%s:%zu:%zu: token %s aún no admitido por la gramática\n",
                ctx->lexer.filename, ctx->linea, ctx->columna,
                token_type_name(token.type));
        tipo = YYerror;
    } else {
        AstTipo hoja;
        if (tipo_hoja(token.type, &hoja)) {
            *valor = ast_crear(ctx->arbol, hoja, token.lexeme, *ubicacion);
            if (!*valor) {
                ctx->estado = 2;
                fprintf(ctx->diagnosticos, "%s:%zu:%zu: memoria insuficiente para el AST\n",
                        ctx->lexer.filename, ctx->linea, ctx->columna);
                tipo = YYerror;
            }
        }
    }
    /* ast_crear copio el lexema; ya podemos liberar el token de Javier. */
    token_dispose(&token);
    return tipo;
}

/* Bison llama a esta funcion cuando detecta un error de sintaxis o memoria. */
void yyerror(YYLTYPE *ubicacion, ContextoSintactico *ctx, const char *mensaje) {
    fprintf(ctx->diagnosticos, "%s:%zu:%zu: error del parser: %s\n",
            ctx->lexer.filename, ubicacion->first_line, ubicacion->first_column, mensaje);
    if (ctx->estado != 2) ctx->estado = 1;
}

int construir_ast(const char *fuente, size_t longitud, const char *nombre,
                  FILE *diagnosticos, Ast *arbol) {
    ContextoSintactico ctx = {0};
    lexer_init(&ctx.lexer, fuente, longitud, nombre);
    ctx.arbol = arbol;
    ctx.diagnosticos = diagnosticos;
    ctx.linea = ctx.columna = 1;
    int resultado = yyparse(&ctx);
    int estado = resultado == 2 || ctx.estado == 2 ? 2 :
                 resultado != 0 || ctx.estado != 0 ? 1 : 0;
    /* Incluye nodos parciales y hojas que Bison descarto al abortar. */
    if (estado != 0) ast_liberar(arbol);
    return estado;
}

/* Conserva la interfaz anterior para quien solo necesite validar. */
int analizar_sintaxis(const char *fuente, size_t longitud,
                     const char *nombre, FILE *diagnosticos) {
    Ast arbol = {0};
    int estado = construir_ast(fuente, longitud, nombre, diagnosticos, &arbol);
    ast_liberar(&arbol);
    return estado;
}
