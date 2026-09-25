#include "lexer.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct { const char *word; TokenType type; } Keyword;

static const Keyword keywords[] = {
    {"main", TOKEN_MAIN}, {"declare_const", TOKEN_DECLARE_CONST},
    {"create_funk", TOKEN_CREATE_FUNK}, {"give", TOKEN_GIVE},
    {"whether", TOKEN_WHETHER}, {"alif", TOKEN_ALIF}, {"also", TOKEN_ALSO},
    {"whale", TOKEN_WHALE}, {"stop", TOKEN_STOP},
    {"cycle", TOKEN_CYCLE}, {"let", TOKEN_LET},
    {"until", TOKEN_UNTIL}, {"endgame", TOKEN_ENDGAME},
    {"step", TOKEN_STEP}, {"bring", TOKEN_BRING}, {"aka", TOKEN_AKA},
    {"seek", TOKEN_SEEK}, {"seize", TOKEN_SEIZE}, {"show", TOKEN_SHOW},
    {"declare_list", TOKEN_DECLARE_LIST}, {"add", TOKEN_ADD},
    {"remove", TOKEN_REMOVE}, {"size", TOKEN_SIZE},
    {"declare_int", TOKEN_DECLARE_INT},
    {"declare_boolean", TOKEN_DECLARE_BOOLEAN},
    {"declare_text", TOKEN_DECLARE_TEXT},
    {"declare_char", TOKEN_DECLARE_CHAR},
    {"declare_infinite_void", TOKEN_DECLARE_INFINITE_VOID},
    {"declare_false", TOKEN_DECLARE_FALSE},
    {"declare_true", TOKEN_DECLARE_TRUE},
    {"gauss", TOKEN_GAUSS}, {"neumann", TOKEN_NEUMANN},
    {"pitagoras", TOKEN_PITAGORAS}, {"euclides", TOKEN_EUCLIDES},
    {"euler", TOKEN_EULER}, {"descartes", TOKEN_DESCARTES}
};

static int alpha(unsigned char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

static int digit(unsigned char c) { return c >= '0' && c <= '9'; }
static int ident(unsigned char c) { return alpha(c) || digit(c); }

static unsigned char peek(const Lexer *l, size_t ahead) {
    if (ahead >= l->length - l->offset) return 0;
    return (unsigned char)l->source[l->offset + ahead];
}

static int starts(const Lexer *l, const char *text, size_t n) {
    return n <= l->length - l->offset &&
           memcmp(l->source + l->offset, text, n) == 0;
}

static size_t utf8_width(const Lexer *l);

/* CRLF is one line break. Each valid UTF-8 scalar occupies one column. */
static void advance(Lexer *l) {
    unsigned char c = peek(l, 0);
    if (c == '\r') {
        ++l->offset;
        if (l->offset < l->length && l->source[l->offset] == '\n') ++l->offset;
        ++l->line;
        l->column = 1;
    } else if (c == '\n') {
        ++l->offset;
        ++l->line;
        l->column = 1;
    } else {
        size_t width = c >= 0x80 ? utf8_width(l) : 1;
        l->offset += width ? width : 1;
        ++l->column;
    }
}

static Token make_token(const Lexer *l, TokenType type, TokenError error,
                        size_t begin, size_t line, size_t column) {
    Token t = {type, error, NULL, l->offset - begin, line, column};
    if (t.length == SIZE_MAX) {
        t.type = TOKEN_ERROR;
        t.error = TOKEN_OUT_OF_MEMORY;
        return t;
    }
    t.lexeme = malloc(t.length + 1);
    if (!t.lexeme) {
        t.type = TOKEN_ERROR;
        t.error = TOKEN_OUT_OF_MEMORY;
        return t;
    }
    if (t.length) memcpy(t.lexeme, l->source + begin, t.length);
    t.lexeme[t.length] = '\0';
    return t;
}

void lexer_init(Lexer *l, const char *source, size_t length,
                const char *filename) {
    *l = (Lexer){source, filename, length, 0, 1, 1};
}

/* Recognize one valid UTF-8 scalar; accents are legal in strings/chars only. */
static size_t utf8_width(const Lexer *l) {
    unsigned char a = peek(l, 0);
    if (a < 0x80) return 1;
    size_t n = a >= 0xC2 && a <= 0xDF ? 2 :
               a >= 0xE0 && a <= 0xEF ? 3 :
               a >= 0xF0 && a <= 0xF4 ? 4 : 0;
    if (!n || n > l->length - l->offset) return 0;
    for (size_t i = 1; i < n; ++i)
        if ((peek(l, i) & 0xC0) != 0x80) return 0;
    unsigned char b = peek(l, 1);
    if ((a == 0xE0 && b < 0xA0) || (a == 0xED && b >= 0xA0) ||
        (a == 0xF0 && b < 0x90) || (a == 0xF4 && b >= 0x90)) return 0;
    return n;
}

static Token scan_string(Lexer *l, size_t start, size_t line, size_t column) {
    advance(l); /* opening quote */
    while (l->offset < l->length && peek(l, 0) != '"' &&
           peek(l, 0) != '\n' && peek(l, 0) != '\r') {
        advance(l);
    }
    if (peek(l, 0) != '"' || l->offset == l->length)
        return make_token(l, TOKEN_ERROR, TOKEN_UNTERMINATED_STRING,
                          start, line, column);
    advance(l);
    return make_token(l, TOKEN_STRING, TOKEN_NO_ERROR, start, line, column);
}

static Token scan_character(Lexer *l, size_t start, size_t line, size_t column) {
    advance(l); /* opening $ */
    size_t count = 0;
    int invalid_utf8 = 0;
    while (l->offset < l->length && peek(l, 0) != '$' &&
           peek(l, 0) != '\n' && peek(l, 0) != '\r') {
        size_t n = utf8_width(l);
        if (!n) { invalid_utf8 = 1; n = 1; }
        advance(l);
        ++count;
    }
    if (l->offset == l->length || peek(l, 0) != '$')
        return make_token(l, TOKEN_ERROR, TOKEN_UNTERMINATED_CHARACTER,
                          start, line, column);
    advance(l);
    return make_token(l, count == 1 && !invalid_utf8 ? TOKEN_CHARACTER : TOKEN_ERROR,
                      count == 1 && !invalid_utf8 ? TOKEN_NO_ERROR : TOKEN_INVALID_CHARACTER_LITERAL,
                      start, line, column);
}

Token lexer_next(Lexer *l) {
    for (;;) {
        size_t start = l->offset, line = l->line, column = l->column;
        if (start == l->length)
            return make_token(l, TOKEN_EOF, TOKEN_NO_ERROR, start, line, column);
        unsigned char c = peek(l, 0);
        if (c == ' ' || c == '\t' || c == '\v' || c == '\f' ||
            c == '\n' || c == '\r') {
            advance(l);
            continue;
        }
        if (starts(l, "%%//", 4)) {
            for (size_t i = 0; i < 4; ++i) advance(l);
            while (l->offset < l->length && !starts(l, "//%%", 4)) advance(l);
            if (l->offset == l->length)
                return make_token(l, TOKEN_ERROR, TOKEN_UNTERMINATED_COMMENT,
                                  start, line, column);
            for (size_t i = 0; i < 4; ++i) advance(l);
            continue;
        }
        if (starts(l, "%%", 2)) {
            while (l->offset < l->length && peek(l, 0) != '\r' &&
                   peek(l, 0) != '\n') advance(l);
            continue;
        }
        if (alpha(c)) {
            advance(l);
            while (l->offset < l->length && ident(peek(l, 0))) advance(l);
            size_t n = l->offset - start;
            TokenType type = TOKEN_IDENTIFIER;
            for (size_t i = 0; i < sizeof(keywords)/sizeof(keywords[0]); ++i) {
                if (strlen(keywords[i].word) == n &&
                    memcmp(l->source + start, keywords[i].word, n) == 0) {
                    type = keywords[i].type;
                    break;
                }
            }
            return make_token(l, type, TOKEN_NO_ERROR, start, line, column);
        }
        if (digit(c)) {
            advance(l);
            while (l->offset < l->length && digit(peek(l, 0))) advance(l);
            int bad = 0;
            if (peek(l, 0) == '.' && digit(peek(l, 1))) {
                bad = 1;
                advance(l);
                while (l->offset < l->length && digit(peek(l, 0))) advance(l);
            }
            if (alpha(peek(l, 0))) {
                bad = 1;
                while (l->offset < l->length && ident(peek(l, 0))) advance(l);
            }
            return make_token(l, bad ? TOKEN_ERROR : TOKEN_INTEGER,
                              bad ? TOKEN_INVALID_NUMBER : TOKEN_NO_ERROR,
                              start, line, column);
        }
        if (c == '"') return scan_string(l, start, line, column);
        if (c == '$') return scan_character(l, start, line, column);

        struct { const char *s; size_t n; TokenType type; } ops[] = {
            {"=/=", 3, TOKEN_NOT_EQUAL}, {"==", 2, TOKEN_EQUAL},
            {"<=", 2, TOKEN_LESS_EQUAL}, {">=", 2, TOKEN_GREATER_EQUAL},
            {"&&", 2, TOKEN_AND}, {"||", 2, TOKEN_OR}
        };
        for (size_t i = 0; i < sizeof(ops)/sizeof(ops[0]); ++i) {
            if (starts(l, ops[i].s, ops[i].n)) {
                for (size_t j = 0; j < ops[i].n; ++j) advance(l);
                return make_token(l, ops[i].type, TOKEN_NO_ERROR,
                                  start, line, column);
            }
        }
        TokenType type;
        switch (c) {
            case '<': type = TOKEN_LESS; break;
            case '>': type = TOKEN_GREATER; break;
            case '~': type = TOKEN_NOT; break;
            case '^': type = TOKEN_XOR; break;
            case ':': type = TOKEN_ASSIGN; break;
            case '.': type = TOKEN_DOT; break;
            case '(': type = TOKEN_LPAREN; break;
            case ')': type = TOKEN_RPAREN; break;
            case '[': type = TOKEN_LBRACKET; break;
            case ']': type = TOKEN_RBRACKET; break;
            case '{': type = TOKEN_LBRACE; break;
            case '}': type = TOKEN_RBRACE; break;
            case ',': type = TOKEN_COMMA; break;
            case ';': type = TOKEN_SEMICOLON; break;
            case '*': type = TOKEN_STAR; break;
            case '#': type = TOKEN_HASH; break;
            default: type = TOKEN_ERROR; break;
        }
        /* Report a whole accented UTF-8 scalar instead of one error per byte. */
        advance(l);
        return make_token(l, type,
                          type == TOKEN_ERROR ? TOKEN_INVALID_CHARACTER : TOKEN_NO_ERROR,
                          start, line, column);
    }
}
