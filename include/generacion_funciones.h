#ifndef GENERACION_FUNCIONES_H
#define GENERACION_FUNCIONES_H

#include "generacion_basica.h"

/* Obtiene la direccion ABSOLUTA de una funcion; debe ser alineada a 16.
 * La resolucion y comprobacion semantica de la firma ocurren antes. */
typedef int (*GfResolverDestino)(void *datos, const AstNodo *identificador,
                                  unsigned *direccion);

/* AST_FUNCION -> instrucciones lineales con etiquetas simbolicas.
 * Requiere AST validado semanticamente (tipos, condiciones y firmas).
 * Cuerpo: declaraciones inicializadas, asignaciones, llamadas, whether/alif/also
 * anidados y give. No admite ciclos, condiciones con llamadas ni declaraciones
 * dentro de ramas (pendiente el resolvedor de ambitos anidados).
 * Cada give emite su epilogo; una funcion escalar debe retornar en todos los
 * caminos. Rechaza sentencias tras un retorno incondicional.
 * Etiquetas reservadas: __gf_<funcion>_else_<n> / __gf_<funcion>_fin_<n>.
 * Generar una sola vez cada funcion por unidad de salida evita colisiones.
 * Llamadas: hasta cuatro argumentos; permite llamadas anidadas en argumentos,
 * sumas/restas y negacion. Este subconjunto asume retornos de llamadas enteros.
 * x13 contiene el destino de llamada; x8 contiene el retorno escalar.
 * No calcula PC, no empaqueta bundles, no inserta NOP ni genera binario.
 * El backend debe fijar bundles/latencias antes de resolver etiquetas; las
 * direcciones absolutas de llamadas tambien deben corresponder a ese layout.
 * Devuelve 0 en exito y 1 en error. Un fallo de generacion no publica salida;
 * un fallo de E/S durante la copia final puede dejar salida parcial.
 */
int gf_generar_funcion(const AstNodo *funcion, size_t bytes_marco,
                       GbResolver resolver, GfResolverDestino destino,
                       void *datos, FILE *salida, FILE *diagnosticos);

#endif
