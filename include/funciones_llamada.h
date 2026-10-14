#ifndef FUNCIONES_LLAMADA_H
#define FUNCIONES_LLAMADA_H

#include <stddef.h>
#include <stdio.h>

/* 3B-B: emision aislada del lado LLAMADOR del ABI escalar BAL.
 * Antes de llamar, x13 debe contener una direccion de funcion alineada a 16.
 * Los registros_fuente contienen valores de argumentos ya evaluados; el
 * emisor los guarda antes de sobrescribir x8..x11 y usa x12 como auxiliar.
 * Puede modificar x1, x2, x8..x12 y el estado del callee mediante syer;
 * los temporales vivos y x13 se gestionaran en la futura integracion AST.
 * Al volver, sp recupera su valor y x8 contiene el retorno escalar.
 * No soporta aun evaluacion de AST ni llamadas anidadas como expresiones.
 */
typedef enum {
    FL_OK = 0,
    FL_ARGUMENTO_INVALIDO,
    FL_DEMASIADOS_ARGUMENTOS,
    FL_ERROR_ESCRITURA
} FlResultado;

/* Solo se aceptan registros fuente x0 y x4..x31; se excluyen x1..x3
 * (ra/sp/gp). x13 puede ser fuente, ya que se lee antes del salto.
 * Limite: 255 argumentos, consistente con los home slots de 3B-A.
 * Fallas de validacion no escriben nada en salida. */
FlResultado fl_emitir_llamada(FILE *salida,
                             const unsigned *registros_fuente,
                             size_t numero_argumentos);

#endif
