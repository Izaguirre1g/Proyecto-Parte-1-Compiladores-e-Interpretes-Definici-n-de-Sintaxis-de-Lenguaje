#ifndef MEIBEL_SINTACTICO_H
#define MEIBEL_SINTACTICO_H

#include "lexer.h"
#include "ast.h"
#include <stdio.h>

/* Estado de una invocacion del parser. Las posiciones empiezan en 1. */
typedef struct {
    Lexer lexer;
    Ast *arbol; /* Propietario de todos los nodos durante el analisis. */
    FILE *diagnosticos; /*Indica donde escribir los errores*/
    size_t linea; /*Ubicacion del ultimo token*/
    size_t columna; /*Ubicacion del ultimo token*/
    int estado; /*0: sin errores; 1: error lexico/sintactico; 2: sin memoria*/
} ContextoSintactico;

/* Toma prestados fuente, nombre y diagnosticos durante esta llamada.
 * Retorna 0: sintaxis correcta; 1: error lexico/sintactico; 2: sin memoria.
 * Se detiene en el primer error. Construye y libera el AST para solo validar.
 */
int analizar_sintaxis(const char *fuente, size_t longitud,
                     const char *nombre, FILE *diagnosticos);

/* Recibe un Ast inicializado a {0} (o ya liberado). En exito el llamador
 * conserva el arbol y debe usar ast_liberar. En error se libera todo y queda
 * vacio. Los lexemas se copian: el AST no depende de la vida del fuente.
 */
int construir_ast(const char *fuente, size_t longitud, const char *nombre,
                  FILE *diagnosticos, Ast *arbol);

#endif
