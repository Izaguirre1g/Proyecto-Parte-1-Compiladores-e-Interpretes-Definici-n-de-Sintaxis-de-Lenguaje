#ifndef ENLACE_FUNCIONES_H
#define ENLACE_FUNCIONES_H

#include "ast.h"
#include "resolucion_saltos.h"
#include "symbol_table.h"

/* Genera todas las funciones del programa y fija sus direcciones.
 *
 * Las llamadas cargan en x13 la direccion absoluta de la funcion, pero esa
 * direccion depende del layout de bundles, que a su vez depende del codigo
 * generado. Se itera: generar con direcciones supuestas, armar bundles con
 * rs_resolver_etiquetas, leer el PC real de cada funcion y repetir hasta que
 * coinciden. Se parte de "todas en inicio", el valor minimo: las direcciones
 * y el tamano del codigo solo crecen entre vueltas, asi que converge.
 *
 * Las funciones quedan en orden del fuente desde inicio; cada una empieza en
 * la etiqueta "__fn_<nombre>". El marco de cada funcion se calcula con los
 * desplazamientos ya asignados por mem_asignar_proyecto.
 * No genera arranque (gp, sp, llamada a main).
 *
 * funciones: si no es NULL (tabla inicializada y vacia), recibe la direccion
 * final de cada funcion como RR_FUNCION.
 * Devuelve 0 en exito y 1 en error; en error no escribe nada en salida. */
typedef struct {
    RsResumen saltos; /* De la ultima vuelta. */
    size_t funciones;
    size_t vueltas;
} EfResumen;

int ef_enlazar(const Ast *ast, const SimTabla *tabla, uint32_t inicio,
               RrTabla *funciones, FILE *salida, FILE *diagnosticos,
               EfResumen *resumen);

#endif
