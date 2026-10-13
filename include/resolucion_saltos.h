#ifndef RESOLUCION_SALTOS_H
#define RESOLUCION_SALTOS_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Fase 5: ensamblador lineal con etiquetas -> bundles con saltos resueltos.
 *
 * Entrada: una instruccion o una etiqueta ("nombre:") por linea, como la
 * emiten los generadores. Se ignoran lineas vacias y comentarios (#, ;, //).
 *
 * Layout conservador: cada instruccion ocupa un bundle en su slot
 * (ALU | LSU | BRU | CRIPTO) seguido de dos bundles nop. Asi todo valor se
 * lee desde N+3 por cualquier ruta, incluidos los destinos de salto.
 * Cada etiqueta toma el PC del bundle siguiente; si una etiqueta queda al
 * final se agrega un bundle nop para que el destino exista.
 *
 * Saltos: igualsi, igualno, menora y mayoroigual (11 bits) y sye (16 bits)
 * hacia etiquetas se reescriben con el desplazamiento en bytes desde el PC
 * del bundle del salto (rr_desplazamiento). Un condicional fuera de rango se
 * reescribe como condicional invertido corto + sye, y se repite el layout
 * hasta que todos los saltos entran. Destinos numericos y syer no se tocan.
 *
 * Salida: una linea por bundle; las etiquetas quedan como comentarios
 * "# nombre:" y cada salto lleva "# -> etiqueta".
 * inicio: PC del primer bundle, alineado a 16. El ultimo bundle de la IMEM
 * (0x3FF0) esta reservado para el manejador de excepciones.
 * Devuelve 0 en exito y 1 en error; en error no escribe nada en salida. */
typedef struct {
    size_t bundles;   /* Bundles emitidos. */
    size_t saltos;    /* Saltos resueltos contra etiquetas. */
    size_t relajados; /* Condicionales reescritos como condicional + sye. */
} RsResumen;

int rs_resolver(const char *lineal, uint32_t inicio, FILE *salida,
                FILE *diagnosticos, RsResumen *resumen);

#endif
