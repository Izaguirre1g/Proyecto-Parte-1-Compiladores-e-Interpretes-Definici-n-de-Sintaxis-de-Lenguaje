#ifndef FUNCIONES_MARCO_H
#define FUNCIONES_MARCO_H

#include <stddef.h>
#include <stdio.h>

/* 3B-A: emision de prologo y epilogo; NO emite una llamada.
 * Supone sp alineado a 4, pila descendente y marco ABI ya calculado.
 * Parametros 0..3 llegan en x8..x11; extras ya estan sobre el marco.
 * Limita los home slots accesibles por un solo offset LSU: 0..1020.
 */
typedef enum {
    FM_OK = 0,
    FM_ARGUMENTO_INVALIDO,
    FM_MARCO_NO_SOPORTADO,
    FM_ERROR_ESCRITURA
} FmResultado;

/* Por seguridad genera primero a un archivo temporal y solo entonces copia.
 * Un error de validacion no escribe nada en salida. */
FmResultado fm_emitir_prologo(FILE *salida, size_t bytes_marco,
                             size_t numero_parametros);
FmResultado fm_emitir_epilogo(FILE *salida, size_t bytes_marco);

#endif
