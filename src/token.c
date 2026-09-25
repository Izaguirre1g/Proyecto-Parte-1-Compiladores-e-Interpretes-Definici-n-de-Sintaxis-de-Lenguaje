#include "token.h"
#include <stdlib.h>

const char *token_type_name(TokenType type) {
    static const char *const names[] = {
        "EOF", "ERROR", "IDENTIFIER", "INTEGER", "STRING", "CHARACTER",
        "MAIN", "DECLARE_CONST", "CREATE_FUNK", "GIVE",
        "WHETHER", "ALIF", "ALSO", "WHALE", "STOP",
        "CYCLE", "LET", "UNTIL", "ENDGAME", "STEP",
        "BRING", "AKA", "SEEK", "SEIZE", "SHOW",
        "DECLARE_LIST", "ADD", "REMOVE", "SIZE",
        "DECLARE_INT", "DECLARE_BOOLEAN", "DECLARE_TEXT",
        "DECLARE_CHAR", "DECLARE_INFINITE_VOID",
        "DECLARE_FALSE", "DECLARE_TRUE",
        "GAUSS", "NEUMANN", "PITAGORAS", "EUCLIDES",
        "EULER", "DESCARTES",
        "EQUAL", "NOT_EQUAL", "LESS", "GREATER",
        "LESS_EQUAL", "GREATER_EQUAL", "AND", "OR", "NOT", "XOR",
        "ASSIGN", "DOT", "LPAREN", "RPAREN",
        "LBRACKET", "RBRACKET", "LBRACE", "RBRACE",
        "COMMA", "SEMICOLON", "STAR", "HASH"
    };
    return (unsigned)type < sizeof(names) / sizeof(names[0]) ? names[type] : "UNKNOWN";
}

const char *token_error_message(TokenError error) {
    switch (error) {
        case TOKEN_INVALID_CHARACTER: return "carácter no permitido";
        case TOKEN_UNTERMINATED_STRING: return "cadena sin cerrar";
        case TOKEN_UNTERMINATED_CHARACTER: return "literal de carácter sin cerrar";
        case TOKEN_INVALID_CHARACTER_LITERAL: return "el literal debe contener exactamente un carácter";
        case TOKEN_UNTERMINATED_COMMENT: return "comentario de bloque sin cerrar";
        case TOKEN_INVALID_NUMBER: return "literal numérico inválido (solo enteros)";
        case TOKEN_OUT_OF_MEMORY: return "memoria insuficiente";
        case TOKEN_NO_ERROR: return "sin error";
    }
    return "error desconocido";
}

void token_dispose(Token *token) {
    if (!token) return;
    free(token->lexeme);
    token->lexeme = NULL;
    token->length = 0;
}
