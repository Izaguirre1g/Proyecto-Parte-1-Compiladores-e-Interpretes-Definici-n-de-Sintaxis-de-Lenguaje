#include "generacion_interna.h"

#include <limits.h>

/* Recorrido estructural y etiquetas simbolicas. Los PC se fijan despues
 * de empaquetar las instrucciones e insertar las separaciones necesarias. */
static int condicional(GfContexto *c, const AstNodo *n, int es_void, int *retorna)
{
    const AstNodo *cond = n->primer_hijo;
    const AstNodo *si = cond ? cond->siguiente : NULL;
    const AstNodo *no = si ? si->siguiente : NULL;
    if (!cond || !si || si->tipo != AST_BLOQUE ||
        (no && (no->siguiente ||
                (no->tipo != AST_BLOQUE && no->tipo != AST_SI))))
        return gf_fallo(c, n, "estructura de whether/alif invalida");
    if (c->bloques >= 256 || c->etiquetas == UINT_MAX)
        return gf_fallo(c, n, "demasiados condicionales anidados");
    unsigned etiqueta = c->etiquetas++;
    if (!gf_condicion(c, cond)) return 0;
    if (fprintf(c->salida, "igualsi x4, x0, __gf_%s_else_%u\n",
                c->funcion, etiqueta) < 0) return 0;
    int retorna_si = 0, retorna_no = 0;
    ++c->bloques;
    int ok = gc_bloque(c, si, es_void, &retorna_si);
    if (ok && no && !retorna_si)
        ok = fprintf(c->salida, "sye x0, __gf_%s_fin_%u\n",
                     c->funcion, etiqueta) >= 0;
    if (ok) ok = fprintf(c->salida, "__gf_%s_else_%u:\n",
                         c->funcion, etiqueta) >= 0;
    if (ok && no) {
        if (no->tipo == AST_SI)
            ok = condicional(c, no, es_void, &retorna_no);
        else
            ok = gc_bloque(c, no, es_void, &retorna_no);
    }
    if (ok && no && !retorna_si)
        ok = fprintf(c->salida, "__gf_%s_fin_%u:\n",
                     c->funcion, etiqueta) >= 0;
    --c->bloques;
    *retorna = retorna_si && retorna_no;
    return ok;
}

int gc_bloque(GfContexto *c, const AstNodo *bloque, int es_void, int *retorna)
{
    if (!bloque || bloque->tipo != AST_BLOQUE)
        return gf_fallo(c, bloque, "bloque ausente");
    int retorno = 0;
    for (const AstNodo *n = bloque->primer_hijo; n; n = n->siguiente) {
        if (retorno) return gf_fallo(c, n, "sentencias despues del retorno final");
        if (n->tipo == AST_DECLARACION && c->bloques)
            return gf_fallo(c, n, "declaracion en bloque anidado requiere resolvedor de ambitos");
        if (n->tipo == AST_SI) {
            if (!condicional(c, n, es_void, &retorno)) return 0;
        } else if (!gf_sentencia(c, n, es_void, &retorno)) return 0;
    }
    *retorna = retorno;
    return 1;
}
