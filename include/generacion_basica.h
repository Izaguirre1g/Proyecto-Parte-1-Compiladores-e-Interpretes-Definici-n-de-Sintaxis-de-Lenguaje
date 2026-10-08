#ifndef GENERACION_BASICA_H
#define GENERACION_BASICA_H

#include "ast.h"

typedef enum { GB_ENTERO, GB_BOOLEANO } GbTipo;
typedef struct {
    GbTipo tipo;
    unsigned base; /* x0, x2 (sp) o x3 (gp); la integracion los inicializa. */
    int desplazamiento; /* Bytes, multiplo de 4, entre -1024 y 1023. */
    int escribible;
} GbReferencia;

/* El nodo permite resolver el nombre en su ambito. Devuelve 1 si existe.
 * La tabla de simbolos conserva la propiedad de sus datos.
 */
typedef int (*GbResolver)(void *datos, const AstNodo *identificador,
                          GbReferencia *referencia);

/* Recibe una expresion pura, asignacion o declaracion escalar inicializada.
 * Emite instrucciones lineales; NO bundles ni un programa ejecutable.
 * Usa x4-x7; una expresion deja el resultado en x4.
 * Retorna 0 en exito y 1 en error, con diagnostico opcional.
 * No escribe instrucciones si falla la validacion/generacion. Un error de E/S
 * al copiar al destino puede dejar una salida parcial, que debe descartarse.
 */
int gb_generar(const AstNodo *nodo, GbResolver resolver, void *datos,
               FILE *salida, FILE *diagnosticos);

#endif
