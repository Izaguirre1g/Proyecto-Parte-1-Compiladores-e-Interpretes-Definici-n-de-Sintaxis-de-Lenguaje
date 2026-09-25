#include "lexer.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    TokenType type;
    TokenError error;
    const char *lexeme;
    size_t length;
    size_t line;
    size_t column;
} Expected;

#define E(type, str, ln, col) {type, TOKEN_NO_ERROR, str, sizeof(str)-1, ln, col}
#define X(error, str, ln, col) {TOKEN_ERROR, error, str, sizeof(str)-1, ln, col}
#define CHECK(name, source, ...) do { \
    const Expected want[] = {__VA_ARGS__}; \
    check(name, source, sizeof(source)-1, want, sizeof(want)/sizeof(want[0])); \
} while (0)

static size_t cases_run;
static size_t failures;

static void check(const char *name, const char *source, size_t length,
                  const Expected *want, size_t count) {
    Lexer l;
    lexer_init(&l, source, length, name);
    ++cases_run;
    for (size_t i = 0; i < count; ++i) {
        Token got = lexer_next(&l);
        if (!got.lexeme || got.type != want[i].type ||
            got.error != want[i].error || got.length != want[i].length ||
            got.line != want[i].line || got.column != want[i].column ||
            (got.lexeme && memcmp(got.lexeme, want[i].lexeme, got.length) != 0)) {
            fprintf(stderr, "FAIL %s token %zu: esperado %s en %zu:%zu; recibido %s en %zu:%zu\n",
                    name, i, token_type_name(want[i].type), want[i].line,
                    want[i].column, token_type_name(got.type), got.line, got.column);
            ++failures;
            token_dispose(&got);
            return;
        }
        token_dispose(&got);
    }
    /* The expectations must include EOF; subsequent reads return EOF as well. */
    if (want[count-1].type != TOKEN_EOF) {
        fprintf(stderr, "FAIL %s: caso sin EOF esperado\n", name);
        ++failures;
    } else {
        Token again = lexer_next(&l);
        if (again.type != TOKEN_EOF || again.line != want[count-1].line ||
            again.column != want[count-1].column) {
            fprintf(stderr, "FAIL %s: EOF repetido inconsistente\n", name);
            ++failures;
        }
        token_dispose(&again);
    }
}

typedef struct { const char *text; TokenType type; } Word;

/* This table follows the language specification, independently of lexer.c. */
static const Word words[] = {
    {"main",TOKEN_MAIN}, {"declare_const",TOKEN_DECLARE_CONST},
    {"create_funk",TOKEN_CREATE_FUNK}, {"give",TOKEN_GIVE},
    {"whether",TOKEN_WHETHER}, {"alif",TOKEN_ALIF}, {"also",TOKEN_ALSO},
    {"whale",TOKEN_WHALE}, {"stop",TOKEN_STOP}, {"cycle",TOKEN_CYCLE},
    {"let",TOKEN_LET}, {"until",TOKEN_UNTIL}, {"endgame",TOKEN_ENDGAME},
    {"bring",TOKEN_BRING}, {"aka",TOKEN_AKA}, {"seek",TOKEN_SEEK},
    {"seize",TOKEN_SEIZE}, {"show",TOKEN_SHOW}, {"step",TOKEN_STEP},
    {"declare_list",TOKEN_DECLARE_LIST}, {"add",TOKEN_ADD},
    {"remove",TOKEN_REMOVE}, {"size",TOKEN_SIZE},
    {"declare_int",TOKEN_DECLARE_INT},
    {"declare_boolean",TOKEN_DECLARE_BOOLEAN},
    {"declare_text",TOKEN_DECLARE_TEXT},
    {"declare_char",TOKEN_DECLARE_CHAR},
    {"declare_infinite_void",TOKEN_DECLARE_INFINITE_VOID},
    {"declare_false",TOKEN_DECLARE_FALSE}, {"declare_true",TOKEN_DECLARE_TRUE},
    {"gauss",TOKEN_GAUSS}, {"neumann",TOKEN_NEUMANN},
    {"pitagoras",TOKEN_PITAGORAS}, {"euclides",TOKEN_EUCLIDES},
    {"euler",TOKEN_EULER}, {"descartes",TOKEN_DESCARTES}
};

static void check_words(void) {
    char source[1024] = {0};
    size_t used = 0;
    for (size_t i = 0; i < sizeof(words)/sizeof(words[0]); ++i) {
        size_t n = strlen(words[i].text);
        if (used + n + 2 > sizeof(source)) abort();
        memcpy(source + used, words[i].text, n);
        used += n;
        source[used++] = ' ';
    }
    Lexer l;
    lexer_init(&l, source, used, "reservadas");
    ++cases_run;
    size_t column = 1;
    for (size_t i = 0; i < sizeof(words)/sizeof(words[0]); ++i) {
        Token t = lexer_next(&l);
        size_t n = strlen(words[i].text);
        if (t.type != words[i].type || t.length != n || t.column != column ||
            !t.lexeme || memcmp(t.lexeme, words[i].text, n) != 0) {
            fprintf(stderr, "FAIL reservadas: %s\n", words[i].text);
            ++failures;
        }
        token_dispose(&t);
        column += n + 1;
    }
    Token end = lexer_next(&l);
    if (end.type != TOKEN_EOF || end.column != column) {
        fputs("FAIL reservadas: EOF\n", stderr);
        ++failures;
    }
    token_dispose(&end);
}

static void check_factorial(void) {
    FILE *f = fopen("examples/factorial.bal", "rb");
    if (!f) { perror("examples/factorial.bal"); ++failures; return; }
    char source[4096];
    size_t n = fread(source, 1, sizeof(source), f);
    if (ferror(f) || n == sizeof(source)) {
        fputs("FAIL factorial: archivo ilegible o demasiado grande\n", stderr);
        ++failures;
        fclose(f);
        return;
    }
    fclose(f);
    Lexer l;
    lexer_init(&l, source, n, "factorial.bal");
    ++cases_run;
    unsigned saw_main=0, saw_if=0, saw_else=0, saw_loop=0;
    unsigned saw_mul=0, saw_add=0, saw_stop=0;
    size_t count=0;
    for (;;) {
        Token t = lexer_next(&l);
        if (t.type == TOKEN_ERROR) {
            fprintf(stderr, "FAIL factorial %zu:%zu: %s\n",
                    t.line, t.column, token_error_message(t.error));
            ++failures;
        }
        saw_main += t.type == TOKEN_MAIN;
        saw_if += t.type == TOKEN_WHETHER;
        saw_else += t.type == TOKEN_ALSO;
        saw_loop += t.type == TOKEN_WHALE;
        saw_mul += t.type == TOKEN_PITAGORAS;
        saw_add += t.type == TOKEN_GAUSS;
        saw_stop += t.type == TOKEN_STOP;
        TokenType type = t.type;
        token_dispose(&t);
        if (type == TOKEN_EOF) break;
        if (++count > sizeof(source)) { fputs("FAIL factorial: sin EOF\n",stderr); ++failures; break; }
    }
    if (saw_main != 1 || saw_if != 1 || saw_else != 1 || saw_loop != 1 ||
        saw_mul != 1 || saw_add != 1 || saw_stop != 1) {
        fputs("FAIL factorial: faltan tokens estructurales\n", stderr);
        ++failures;
    }
}

int main(void) {
    check_words();
    CHECK("vacío", "", E(TOKEN_EOF,"",1,1));
    CHECK("identificadores", "main2 _cont x9 Main declare_float",
          E(TOKEN_IDENTIFIER,"main2",1,1), E(TOKEN_IDENTIFIER,"_cont",1,7),
          E(TOKEN_IDENTIFIER,"x9",1,13), E(TOKEN_IDENTIFIER,"Main",1,16),
          E(TOKEN_IDENTIFIER,"declare_float",1,21), E(TOKEN_EOF,"",1,34));
    CHECK("operadores", "== =/= < > <= >= && || ~ ^ : .",
          E(TOKEN_EQUAL,"==",1,1), E(TOKEN_NOT_EQUAL,"=/=",1,4),
          E(TOKEN_LESS,"<",1,8), E(TOKEN_GREATER,">",1,10),
          E(TOKEN_LESS_EQUAL,"<=",1,12), E(TOKEN_GREATER_EQUAL,">=",1,15),
          E(TOKEN_AND,"&&",1,18), E(TOKEN_OR,"||",1,21),
          E(TOKEN_NOT,"~",1,24), E(TOKEN_XOR,"^",1,26),
          E(TOKEN_ASSIGN,":",1,28), E(TOKEN_DOT,".",1,30),
          E(TOKEN_EOF,"",1,31));
    CHECK("delimitadores", "()[]{} ,;*#",
          E(TOKEN_LPAREN,"(",1,1), E(TOKEN_RPAREN,")",1,2),
          E(TOKEN_LBRACKET,"[",1,3), E(TOKEN_RBRACKET,"]",1,4),
          E(TOKEN_LBRACE,"{",1,5), E(TOKEN_RBRACE,"}",1,6),
          E(TOKEN_COMMA,",",1,8), E(TOKEN_SEMICOLON,";",1,9),
          E(TOKEN_STAR,"*",1,10), E(TOKEN_HASH,"#",1,11),
          E(TOKEN_EOF,"",1,12));
    CHECK("literales", "0 123 \"José\" $B$ $ñ$ declare_true declare_false",
          E(TOKEN_INTEGER,"0",1,1), E(TOKEN_INTEGER,"123",1,3),
          E(TOKEN_STRING,"\"José\"",1,7), E(TOKEN_CHARACTER,"$B$",1,14),
          E(TOKEN_CHARACTER,"$ñ$",1,18), E(TOKEN_DECLARE_TRUE,"declare_true",1,22),
          E(TOKEN_DECLARE_FALSE,"declare_false",1,35), E(TOKEN_EOF,"",1,48));
    CHECK("comentarios", "x%% línea\n%%// bloque\n á //%%y",
          E(TOKEN_IDENTIFIER,"x",1,1), E(TOKEN_IDENTIFIER,"y",3,8),
          E(TOKEN_EOF,"",3,9));
    CHECK("multilínea CRLF", "a:1;\r\nb:2;",
          E(TOKEN_IDENTIFIER,"a",1,1), E(TOKEN_ASSIGN,":",1,2),
          E(TOKEN_INTEGER,"1",1,3), E(TOKEN_SEMICOLON,";",1,4),
          E(TOKEN_IDENTIFIER,"b",2,1), E(TOKEN_ASSIGN,":",2,2),
          E(TOKEN_INTEGER,"2",2,3), E(TOKEN_SEMICOLON,";",2,4),
          E(TOKEN_EOF,"",2,5));
    CHECK("real descartado", "1.85 12abc",
          X(TOKEN_INVALID_NUMBER,"1.85",1,1),
          X(TOKEN_INVALID_NUMBER,"12abc",1,6), E(TOKEN_EOF,"",1,11));
    CHECK("identificador con acento", "tamaño:1;",
          E(TOKEN_IDENTIFIER,"tama",1,1), X(TOKEN_INVALID_CHARACTER,"ñ",1,5),
          E(TOKEN_IDENTIFIER,"o",1,6), E(TOKEN_ASSIGN,":",1,7),
          E(TOKEN_INTEGER,"1",1,8), E(TOKEN_SEMICOLON,";",1,9),
          E(TOKEN_EOF,"",1,10));
    CHECK("cadena sin cerrar", "\"texto\nx",
          X(TOKEN_UNTERMINATED_STRING,"\"texto",1,1),
          E(TOKEN_IDENTIFIER,"x",2,1), E(TOKEN_EOF,"",2,2));
    CHECK("comentario sin cerrar", "%%// comentario\nx",
          X(TOKEN_UNTERMINATED_COMMENT,"%%// comentario\nx",1,1),
          E(TOKEN_EOF,"",2,2));
    CHECK("carácter sin cerrar", "$a\nx",
          X(TOKEN_UNTERMINATED_CHARACTER,"$a",1,1),
          E(TOKEN_IDENTIFIER,"x",2,1), E(TOKEN_EOF,"",2,2));
    CHECK("caracteres mal formados", "$$ $ab$",
          X(TOKEN_INVALID_CHARACTER_LITERAL,"$$",1,1),
          X(TOKEN_INVALID_CHARACTER_LITERAL,"$ab$",1,4), E(TOKEN_EOF,"",1,8));
    CHECK("símbolos ajenos", "@ !!! !? %?",
          X(TOKEN_INVALID_CHARACTER,"@",1,1),
          X(TOKEN_INVALID_CHARACTER,"!",1,3),
          X(TOKEN_INVALID_CHARACTER,"!",1,4),
          X(TOKEN_INVALID_CHARACTER,"!",1,5),
          X(TOKEN_INVALID_CHARACTER,"!",1,7),
          X(TOKEN_INVALID_CHARACTER,"?",1,8),
          X(TOKEN_INVALID_CHARACTER,"%",1,10),
          X(TOKEN_INVALID_CHARACTER,"?",1,11), E(TOKEN_EOF,"",1,12));
    CHECK("ejemplo de módulo", "bring matematicas aka math; math.suma(3,4);",
          E(TOKEN_BRING,"bring",1,1), E(TOKEN_IDENTIFIER,"matematicas",1,7),
          E(TOKEN_AKA,"aka",1,19), E(TOKEN_IDENTIFIER,"math",1,23),
          E(TOKEN_SEMICOLON,";",1,27), E(TOKEN_IDENTIFIER,"math",1,29),
          E(TOKEN_DOT,".",1,33), E(TOKEN_IDENTIFIER,"suma",1,34),
          E(TOKEN_LPAREN,"(",1,38), E(TOKEN_INTEGER,"3",1,39),
          E(TOKEN_COMMA,",",1,40), E(TOKEN_INTEGER,"4",1,41),
          E(TOKEN_RPAREN,")",1,42), E(TOKEN_SEMICOLON,";",1,43),
          E(TOKEN_EOF,"",1,44));
    {
        const char data[] = {'a','\0','b'};
        const Expected want[] = {E(TOKEN_IDENTIFIER,"a",1,1),
                                 X(TOKEN_INVALID_CHARACTER,"\0",1,2),
                                 E(TOKEN_IDENTIFIER,"b",1,3), E(TOKEN_EOF,"",1,4)};
        check("NUL en fuente", data, sizeof(data), want, sizeof(want)/sizeof(want[0]));
    }
    check_factorial();
    if (failures) {
        fprintf(stderr, "%zu de %zu casos fallaron\n", failures, cases_run);
        return 1;
    }
    printf("OK: %zu casos del lexer, incluido factorial.bal\n", cases_run);
    return 0;
}
