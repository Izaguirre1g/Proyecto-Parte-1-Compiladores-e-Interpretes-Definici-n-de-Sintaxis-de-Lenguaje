#ifndef JAVIER_LEXER_H
#define JAVIER_LEXER_H

#include <stddef.h>
#include "token.h"

typedef struct {
    const char *source;       /* Borrowed: keep alive until the last lexer_next. */
    const char *filename;     /* Borrowed: only for the caller's diagnostics. */
    size_t length;
    size_t offset;
    size_t line;
    size_t column;            /* 1-based Unicode character column. */
} Lexer;

/* Explicit length permits embedded NUL bytes to be reported as errors. */
void lexer_init(Lexer *lexer, const char *source, size_t length,
                const char *filename);
/* Comments and whitespace are skipped. Returns an owned lexeme each time. */
Token lexer_next(Lexer *lexer);

#endif
