#ifndef GENERACION_ADAPTADOR_H
#define GENERACION_ADAPTADOR_H

#include "generacion_funciones.h"
#include "symbol_table.h"

/* 3B-D3C: integra simbolos ya ubicados con el backend 3B-D2.
 * La tabla debe haber pasado por mem_asignar_proyecto().
 * bytes_marco corresponde a la funcion seleccionada, NO al maximo global.
 * El mapa de codigo pertenece al llamador: destinos no se inventan.
 * Solo variables/parametros enteros de la funcion (sin bloques anidados)
 * y constantes enteras globales. No produce bundles ni binarios.
 * 0 exito, 1 generacion o referencias no soportadas.
 */
int gd_generar_funcion(const AstNodo *funcion, const SimTabla *tabla,
                       size_t bytes_marco, GfResolverDestino destino,
                       void *contexto_destinos, FILE *salida,
                       FILE *diagnosticos);

#endif
