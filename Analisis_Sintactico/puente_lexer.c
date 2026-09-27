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
        case TOKEN_GAUSS: return GAUSS;
        case TOKEN_NEUMANN: return NEUMANN;
        case TOKEN_PITAGORAS: return PITAGORAS;
        case TOKEN_EUCLIDES: return EUCLIDES;
        case TOKEN_LPAREN: return LPAREN;
        case TOKEN_RPAREN: return RPAREN;
        case TOKEN_LBRACE: return LBRACE;
        case TOKEN_RBRACE: return RBRACE;
        case TOKEN_WHETHER: return WHETHER;
        case TOKEN_ALSO: return ALSO;
        case TOKEN_WHALE: return WHALE;
        case TOKEN_STOP: return STOP;
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
        default: return YYUNDEF; /*indica que el token no está definido en la gramática*/
    }
}

/* Bison llama a esta funcion cada vez que necesita otro token. */
int yylex(YYSTYPE *valor, ContextoSintactico *ctx) {
    *valor = 0; /* Los valores del AST se incorporaran en una etapa posterior. */
    Token token = lexer_next(&ctx->lexer); /* Se obtiene el siguiente token del analizador léxico. */
    
    /*Se traduce el tipo*/
    ctx->linea = token.line; /**/
    ctx->columna = token.column;

    /* Se traduce el tipo de token a la enumeración de Bison. */
    int tipo = token_bison(token.type);
    if (token.type == TOKEN_ERROR) { /* Se informa el error léxico y se establece el estado del contexto. */
        ctx->estado = token.error == TOKEN_OUT_OF_MEMORY ? 2 : 1;
        fprintf(ctx->diagnosticos, "%s:%zu:%zu: error léxico: %s\n",
                ctx->lexer.filename, ctx->linea, ctx->columna,
                token_error_message(token.error));
        tipo = YYerror; /* Ya se informo el error: no duplicar el diagnostico. */
    } else if (tipo == YYUNDEF) { /*Caso en que puede ser una palabra valida del lexer pero no esta admitida por la gramática*/
        ctx->estado = 1;
        fprintf(ctx->diagnosticos,
                "%s:%zu:%zu: token %s aún no admitido por la gramática\n",
                ctx->lexer.filename, ctx->linea, ctx->columna,
                token_type_name(token.type));
        tipo = YYerror;
    }
    /* Por ahora solo validamos sintaxis; no retenemos el lexema para un AST. */
    /*Se libera la memoria del token obtenido*/
    token_dispose(&token);
    return tipo;
}

/* Bison llama a esta funcion cada vez que detecta un error de sintaxis. */
void yyerror(ContextoSintactico *ctx, const char *mensaje) {
    fprintf(ctx->diagnosticos, "%s:%zu:%zu: error del parser: %s\n",
            ctx->lexer.filename, ctx->linea, ctx->columna, mensaje);
    ctx->estado = 1;
}
/* Esta funcion es llamada desde cli.c para analizar sintaxis, recibe el texto del programa, su longitud , el nombre del archivo y donde escribir errores. */
int analizar_sintaxis(const char *fuente, size_t longitud,
                     const char *nombre, FILE *diagnosticos) {

    /* Inicializa el contexto sintáctico y llama a yyparse, que es la función generada por Bison. */
    ContextoSintactico ctx = {0};
    lexer_init(&ctx.lexer, fuente, longitud, nombre);
    /* Inicializa el flujo de salida de errores, la posición inicial, la salida de errores y el resultado. */
    ctx.diagnosticos = diagnosticos;
    ctx.linea = ctx.columna = 1;
    /*yyparse() es la función generada por Bison a partir de gramatica.y. Mientras analiza, pide tokens llamando a yylex().*/
    int resultado = yyparse(&ctx);
    if (resultado == 2 || ctx.estado == 2) return 2;
    return resultado != 0 || ctx.estado != 0 ? 1 : 0;
}
