#ifndef MEIBEL_SINTACTICO_H
#define MEIBEL_SINTACTICO_H

#include "lexer.h"
#include <stdio.h>

/* Estado de una invocacion del parser. Las posiciones empiezan en 1. */
typedef struct {
    Lexer lexer;
    FILE *diagnosticos; /*Indica donde escribir los errores*/
    size_t linea; /*Ubicacion del ultimo token*/
    size_t columna; /*Ubicacion del ultimo token*/
    int estado; /*0: sin errores; 1: error lexico/sintactico; 2: sin memoria*/
} ContextoSintactico;

/* Toma prestados fuente, nombre y diagnosticos durante esta llamada.
 * Retorna 0: sintaxis correcta; 1: error lexico/sintactico; 2: sin memoria.
 * Se detiene en el primer error. Por ahora no construye un AST.
 */
int analizar_sintaxis(const char *fuente, size_t longitud,
                     const char *nombre, FILE *diagnosticos);

#endif
