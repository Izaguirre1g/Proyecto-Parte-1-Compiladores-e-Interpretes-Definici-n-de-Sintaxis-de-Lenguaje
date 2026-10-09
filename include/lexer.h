#ifndef JAVIER_LEXER_H
#define JAVIER_LEXER_H

// Librerías y definiciones que necesita el lexer.
#include <stddef.h>
#include "token.h"

// Guarda la información necesaria para recorrer el código fuente.
typedef struct {
    const char *source;       // Código que vamos a analizar.
    const char *filename;     // Nombre del archivo.
    size_t length;            // Tamaño del código en bytes.
    size_t offset;            // Posición actual dentro del código.
    size_t line;              // Línea actual.
    size_t column;            // Columna actual, contando caracteres Unicode.
} Lexer;

// Inicializa el lexer con el código fuente y sus datos.
// La longitud permite detectar incluso caracteres nulos (\0).
void lexer_init(Lexer *lexer, const char *source, size_t length,
                const char *filename);

// Obtiene el siguiente token, ignorando espacios y comentarios.
// El token devuelto debe liberarse después de utilizarlo.
Token lexer_next(Lexer *lexer);

#endif
