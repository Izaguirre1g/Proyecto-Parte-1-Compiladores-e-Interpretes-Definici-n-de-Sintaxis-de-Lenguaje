#ifndef GENERACION_FUNCIONES_H
#define GENERACION_FUNCIONES_H

#include "generacion_basica.h"

/* Obtiene la direccion ABSOLUTA de una funcion; debe ser alineada a 16.
 * La resolucion y comprobacion semantica de la firma ocurren antes. */
typedef int (*GfResolverDestino)(void *datos, const AstNodo *identificador,
                                  unsigned *direccion);

/* 3B-D2: AST_FUNCION -> codigo lineal de una funcion escalar.
 * Cuerpo: declaraciones inicializadas/asignaciones basicas, llamadas como
 * sentencia, un retorno final (give expresion; o give; en void).
 * Llamadas: maximo cuatro argumentos, cada uno una expresion basica sin
 * llamadas internas. Ahora permite llamadas anidadas en expresiones de
 * suma/resta y argumentos, salvando resultados intermedios en pila.
 * Las firmas de las llamadas deben estar validadas por el analizador
 * semantico; este subconjunto asume retornos escalares enteros.
 * x13 se carga con destino; retorno escalar en x8.
 * gb_generar permanece intacta; todo fallo de validacion deja salida intacta.
 * Devuelve 0 en exito y 1 en error. No emite bundles ni relocalizaciones. */
int gf_generar_funcion(const AstNodo *funcion, size_t bytes_marco,
                       GbResolver resolver, GfResolverDestino destino,
                       void *datos, FILE *salida, FILE *diagnosticos);

#endif
