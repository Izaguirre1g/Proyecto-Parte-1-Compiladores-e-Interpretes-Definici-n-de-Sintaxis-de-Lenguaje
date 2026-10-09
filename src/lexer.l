%option noyywrap nounput noinput never-interactive nodefault prefix="ballex_"
%x LINE_COMMENT BLOCK_COMMENT

%{
#include <limits.h>
#include <stddef.h>
#include <string.h>

#include "token.h"
#include "lexer_flex.h"

typedef struct {
    const char *word;
    TokenType type;
} Keyword;

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

static LexerFlexMatch *current_match;
static size_t consumed;
static size_t block_start;

static TokenType keyword_type(const char *text, size_t length) {
    for (size_t i = 0; i < sizeof(keywords) / sizeof(keywords[0]); ++i) {
        size_t n = strlen(keywords[i].word);
        if (n == length && memcmp(text, keywords[i].word, n) == 0)
            return keywords[i].type;
    }
    return TOKEN_IDENTIFIER;
}

static int one_utf8_scalar(const unsigned char *s, size_t n) {
    if (n == 1) return 1;
    if (n == 2)
        return s[0] >= 0xC2 && s[0] <= 0xDF &&
               (s[1] & 0xC0) == 0x80;
    if (n == 3) {
        if (s[0] < 0xE0 || s[0] > 0xEF ||
            (s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80) return 0;
        if (s[0] == 0xE0 && s[1] < 0xA0) return 0;
        if (s[0] == 0xED && s[1] >= 0xA0) return 0;
        return 1;
    }
    if (n == 4) {
        if (s[0] < 0xF0 || s[0] > 0xF4 ||
            (s[1] & 0xC0) != 0x80 || (s[2] & 0xC0) != 0x80 ||
            (s[3] & 0xC0) != 0x80) return 0;
        if (s[0] == 0xF0 && s[1] < 0x90) return 0;
        if (s[0] == 0xF4 && s[1] >= 0x90) return 0;
        return 1;
    }
    return 0;
}

#define RETURN_TOKEN(kind, cause) do { \
    current_match->skip_length = consumed; \
    current_match->token_length = (size_t)yyleng; \
    current_match->type = (kind); \
    current_match->error = (cause); \
    return 1; \
} while (0)

#define RETURN_EOF() do { \
    current_match->skip_length = consumed; \
    current_match->token_length = 0; \
    current_match->type = TOKEN_EOF; \
    current_match->error = TOKEN_NO_ERROR; \
    return 1; \
} while (0)
%}

ID          [A-Za-z_][A-Za-z0-9_]*
INT         [0-9]+
BADNUM      [0-9]+[A-Za-z_][A-Za-z0-9_]*
BADREAL     [0-9]+\.[0-9]+[A-Za-z_][A-Za-z0-9_]*
REAL        [0-9]+\.[0-9]+
UTF8_2      [\xC2-\xDF][\x80-\xBF]
UTF8_3      (\xE0[\xA0-\xBF][\x80-\xBF]|[\xE1-\xEC\xEE-\xEF][\x80-\xBF]{2}|\xED[\x80-\x9F][\x80-\xBF])
UTF8_4      (\xF0[\x90-\xBF][\x80-\xBF]{2}|[\xF1-\xF3][\x80-\xBF]{3}|\xF4[\x80-\x8F][\x80-\xBF]{2})

%%

<INITIAL>[ \t\v\f\r\n]+          { consumed += (size_t)yyleng; }

<INITIAL>"%%//"                  {
                                      block_start = consumed;
                                      consumed += (size_t)yyleng;
                                      BEGIN(BLOCK_COMMENT);
                                  }
<INITIAL>"%%"                    {
                                      consumed += (size_t)yyleng;
                                      BEGIN(LINE_COMMENT);
                                  }
<LINE_COMMENT>[^\r\n]+           { consumed += (size_t)yyleng; }
<LINE_COMMENT>(\r\n|\r|\n)       {
                                      consumed += (size_t)yyleng;
                                      BEGIN(INITIAL);
                                  }
<LINE_COMMENT><<EOF>>            { BEGIN(INITIAL); RETURN_EOF(); }

<BLOCK_COMMENT>"//%%"            {
                                      consumed += (size_t)yyleng;
                                      BEGIN(INITIAL);
                                  }
<BLOCK_COMMENT>\x00              { consumed += (size_t)yyleng; }
<BLOCK_COMMENT>(\r\n|\r|\n|.)    { consumed += (size_t)yyleng; }
<BLOCK_COMMENT><<EOF>>           {
                                      current_match->skip_length = block_start;
                                      current_match->token_length = consumed - block_start;
                                      current_match->type = TOKEN_ERROR;
                                      current_match->error = TOKEN_UNTERMINATED_COMMENT;
                                      BEGIN(INITIAL);
                                      return 1;
                                  }

<INITIAL>{BADREAL}               { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_NUMBER); }
<INITIAL>{REAL}                  { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_NUMBER); }
<INITIAL>{BADNUM}                { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_NUMBER); }
<INITIAL>{INT}                   { RETURN_TOKEN(TOKEN_INTEGER, TOKEN_NO_ERROR); }

<INITIAL>\"[^\"\r\n]*\"        { RETURN_TOKEN(TOKEN_STRING, TOKEN_NO_ERROR); }
<INITIAL>\"[^\"\r\n]*          { RETURN_TOKEN(TOKEN_ERROR, TOKEN_UNTERMINATED_STRING); }

<INITIAL>\$[^$\r\n]*\$          {
                                      size_t inner = (size_t)yyleng - 2;
                                      int valid = one_utf8_scalar(
                                          (const unsigned char *)yytext + 1, inner);
                                      RETURN_TOKEN(valid ? TOKEN_CHARACTER : TOKEN_ERROR,
                                                   valid ? TOKEN_NO_ERROR : TOKEN_INVALID_CHARACTER_LITERAL);
                                  }
<INITIAL>\$[^$\r\n]*            { RETURN_TOKEN(TOKEN_ERROR, TOKEN_UNTERMINATED_CHARACTER); }

<INITIAL>{ID}                    {
                                      RETURN_TOKEN(keyword_type(yytext, (size_t)yyleng),
                                                   TOKEN_NO_ERROR);
                                  }

<INITIAL>"=/="                   { RETURN_TOKEN(TOKEN_NOT_EQUAL, TOKEN_NO_ERROR); }
<INITIAL>"=="                    { RETURN_TOKEN(TOKEN_EQUAL, TOKEN_NO_ERROR); }
<INITIAL>"<="                    { RETURN_TOKEN(TOKEN_LESS_EQUAL, TOKEN_NO_ERROR); }
<INITIAL>">="                    { RETURN_TOKEN(TOKEN_GREATER_EQUAL, TOKEN_NO_ERROR); }
<INITIAL>"&&"                    { RETURN_TOKEN(TOKEN_AND, TOKEN_NO_ERROR); }
<INITIAL>"||"                    { RETURN_TOKEN(TOKEN_OR, TOKEN_NO_ERROR); }
<INITIAL>"<"                     { RETURN_TOKEN(TOKEN_LESS, TOKEN_NO_ERROR); }
<INITIAL>">"                     { RETURN_TOKEN(TOKEN_GREATER, TOKEN_NO_ERROR); }
<INITIAL>"~"                     { RETURN_TOKEN(TOKEN_NOT, TOKEN_NO_ERROR); }
<INITIAL>"^"                     { RETURN_TOKEN(TOKEN_XOR, TOKEN_NO_ERROR); }
<INITIAL>":"                     { RETURN_TOKEN(TOKEN_ASSIGN, TOKEN_NO_ERROR); }
<INITIAL>"."                     { RETURN_TOKEN(TOKEN_DOT, TOKEN_NO_ERROR); }
<INITIAL>"("                     { RETURN_TOKEN(TOKEN_LPAREN, TOKEN_NO_ERROR); }
<INITIAL>")"                     { RETURN_TOKEN(TOKEN_RPAREN, TOKEN_NO_ERROR); }
<INITIAL>"["                     { RETURN_TOKEN(TOKEN_LBRACKET, TOKEN_NO_ERROR); }
<INITIAL>"]"                     { RETURN_TOKEN(TOKEN_RBRACKET, TOKEN_NO_ERROR); }
<INITIAL>"{"                     { RETURN_TOKEN(TOKEN_LBRACE, TOKEN_NO_ERROR); }
<INITIAL>"}"                     { RETURN_TOKEN(TOKEN_RBRACE, TOKEN_NO_ERROR); }
<INITIAL>","                     { RETURN_TOKEN(TOKEN_COMMA, TOKEN_NO_ERROR); }
<INITIAL>";"                     { RETURN_TOKEN(TOKEN_SEMICOLON, TOKEN_NO_ERROR); }
<INITIAL>"*"                     { RETURN_TOKEN(TOKEN_STAR, TOKEN_NO_ERROR); }
<INITIAL>"#"                     { RETURN_TOKEN(TOKEN_HASH, TOKEN_NO_ERROR); }

<INITIAL>{UTF8_2}                { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_CHARACTER); }
<INITIAL>{UTF8_3}                { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_CHARACTER); }
<INITIAL>{UTF8_4}                { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_CHARACTER); }
<INITIAL>\x00                    { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_CHARACTER); }
<INITIAL>.                       { RETURN_TOKEN(TOKEN_ERROR, TOKEN_INVALID_CHARACTER); }
<INITIAL><<EOF>>                 { RETURN_EOF(); }

%%

int lexer_flex_match(const char *source, size_t length, LexerFlexMatch *match) {
    if (!source || !match || length > (size_t)INT_MAX) return 0;

    current_match = match;
    consumed = 0;
    block_start = 0;
    *match = (LexerFlexMatch){0};

    YY_BUFFER_STATE buffer = yy_scan_bytes(source, (int)length);
    if (!buffer) return 0;

    BEGIN(INITIAL);
    int result = yylex();
    yy_delete_buffer(buffer);
    yylex_destroy();
    current_match = NULL;
    return result == 1;
}
