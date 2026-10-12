#include "generacion_adaptador.h"

#include <limits.h>
#include <stdint.h>
#include <string.h>

typedef struct {
    const SimTabla *tabla;
    const SimAmbito *ambito;
    GfResolverDestino destino;
    void *contexto_destinos;
    size_t bytes_marco;
} GdContexto;

static const SimAmbito *ambito_funcion(const SimTabla *tabla, const char *nombre)
{
    const SimAmbito *encontrado = NULL;
    for (const SimAmbito *a = tabla->global->siguiente_creacion;
         a; a = a->siguiente_creacion) {
        if (a->padre == tabla->global && strcmp(a->nombre, nombre) == 0) {
            if (encontrado) return NULL;
            encontrado = a;
        }
    }
    return encontrado;
}

static int referencia(void *datos, const AstNodo *id, GbReferencia *r)
{
    GdContexto *c = datos;
    if (!id || id->tipo != AST_IDENTIFICADOR || !id->texto || !r) return 0;
    const SimSimbolo *encontrado = NULL;
    for (const SimSimbolo *s = c->tabla->primero; s; s = s->siguiente_orden) {
        if (strcmp(s->nombre, id->texto) != 0) continue;
        if (s->ambito == c->ambito) { encontrado = s; break; }
        if (s->ambito == c->tabla->global) encontrado = s;
    }
    if (!encontrado || encontrado->tipo != SIM_TIPO_INT ||
        encontrado->dimensiones || encontrado->bytes != 4 ||
        (encontrado->clase != SIM_VARIABLE &&
         encontrado->clase != SIM_PARAMETRO &&
         encontrado->clase != SIM_CONSTANTE)) return 0;
    unsigned base;
    if (encontrado->ambito == c->ambito &&
        (encontrado->almacen == SIM_ALMACEN_LOCAL ||
         encontrado->almacen == SIM_ALMACEN_PARAMETRO)) {
        base = 2;
        if (encontrado->desplazamiento < 4 ||
            (unsigned long)encontrado->desplazamiento > c->bytes_marco - 4)
            return 0;
    } else if (encontrado->ambito == c->tabla->global &&
               encontrado->almacen == SIM_ALMACEN_GLOBAL) {
        base = 3;
    } else return 0;
    if (encontrado->desplazamiento < -1024 ||
        encontrado->desplazamiento > 1023 ||
        encontrado->desplazamiento % 4 != 0) return 0;
    *r = (GbReferencia){GB_ENTERO, base, (int)encontrado->desplazamiento,
                       encontrado->clase != SIM_CONSTANTE};
    return 1;
}

static int resolver_destino(void *datos, const AstNodo *id, unsigned *direccion)
{
    GdContexto *c = datos;
    if (!c->destino || !id || id->tipo != AST_IDENTIFICADOR ||
        !id->texto || !direccion) return 0;
    const SimSimbolo *f = NULL;
    for (const SimSimbolo *s = c->tabla->primero; s; s = s->siguiente_orden)
        if (s->ambito == c->tabla->global && s->clase == SIM_FUNCION &&
            strcmp(s->nombre, id->texto) == 0) { f = s; break; }
    if (!f) return 0;
    return c->destino(c->contexto_destinos, id, direccion) &&
           (*direccion & 15u) == 0;
}

int gd_generar_funcion(const AstNodo *funcion, const SimTabla *tabla,
                       size_t bytes_marco, GfResolverDestino destino,
                       void *contexto_destinos, FILE *salida,
                       FILE *diagnosticos)
{
    if (!funcion || funcion->tipo != AST_FUNCION || !tabla || !tabla->global ||
        !salida || bytes_marco < 4 || bytes_marco % 4 ||
        bytes_marco > (size_t)INT_MAX) return 1;
    const AstNodo *id = funcion->primer_hijo;
    if (!id || id->tipo != AST_IDENTIFICADOR || !id->texto) return 1;
    const SimAmbito *ambito = ambito_funcion(tabla, id->texto);
    int tiene_funcion = 0;
    for (const SimSimbolo *s = tabla->primero; s; s = s->siguiente_orden)
        if (s->ambito == tabla->global && s->clase == SIM_FUNCION &&
            strcmp(s->nombre, id->texto) == 0) { tiene_funcion = 1; break; }
    if (!ambito || !tiene_funcion) return 1;
    /* El generador base solo tiene instrucciones de palabra; no traducir
     * silenciosamente booleanos o caracteres ubicados en un byte. */
    for (const SimSimbolo *s = tabla->primero; s; s = s->siguiente_orden)
        if (s->ambito == ambito &&
            s->almacen != SIM_ALMACEN_PARAMETRO &&
            s->almacen != SIM_ALMACEN_LOCAL) return 1;
    GdContexto c = {tabla, ambito, destino, contexto_destinos, bytes_marco};
    return gf_generar_funcion(funcion, bytes_marco, referencia,
                             resolver_destino, &c, salida, diagnosticos);
}
