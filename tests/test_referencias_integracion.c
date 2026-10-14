#include "referencias_integracion.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNodo nodo(AstTipo tipo, char *texto)
{
    AstNodo n = {0}; n.tipo = tipo; n.texto = texto; return n;
}

int main(void)
{
    SimTabla sim = {0};
    SimSimbolo *s = NULL;
    assert(sim_inicializar(&sim) == SIM_OK);
    assert(sim_insertar(&sim, "identidad", SIM_FUNCION, SIM_TIPO_INT,
                       1, 1, &s) == SIM_OK);
    assert(sim_entrar_ambito(&sim, "identidad") == SIM_OK);
    assert(sim_insertar(&sim, "a", SIM_PARAMETRO, SIM_TIPO_INT,
                       1, 1, &s) == SIM_OK);
    sim_definir_ubicacion(s, SIM_ALMACEN_PARAMETRO, 4, 4);
    assert(sim_salir_ambito(&sim));

    /* identidad(a) { give identidad(a); }: llamada recursiva simbolica.
     * Direccion definida EXPLICITAMENTE como dato de prueba del layout. */
    AstNodo fn = nodo(AST_FUNCION, NULL);
    AstNodo id = nodo(AST_IDENTIFICADOR, "identidad");
    AstNodo tipo = nodo(AST_TIPO, "declare_int");
    AstNodo params = nodo(AST_PARAMETROS, NULL);
    AstNodo par = nodo(AST_PARAMETRO, NULL);
    AstNodo pid = nodo(AST_IDENTIFICADOR, "a");
    AstNodo ptip = nodo(AST_TIPO, "declare_int");
    AstNodo bloque = nodo(AST_BLOQUE, NULL);
    AstNodo ret = nodo(AST_RETORNO, NULL);
    AstNodo call = nodo(AST_LLAMADA, NULL);
    AstNodo callee = nodo(AST_IDENTIFICADOR, "identidad");
    AstNodo args = nodo(AST_ARGUMENTOS, NULL);
    AstNodo arg = nodo(AST_IDENTIFICADOR, "a");
    fn.primer_hijo = &id; id.siguiente = &tipo;
    tipo.siguiente = &params; params.siguiente = &bloque;
    params.primer_hijo = &par; par.primer_hijo = &pid;
    pid.siguiente = &ptip; bloque.primer_hijo = &ret;
    ret.primer_hijo = &call; call.primer_hijo = &callee;
    callee.siguiente = &args; args.primer_hijo = &arg;

    RrTabla rr;
    rr_inicializar(&rr);
    FILE *f = tmpfile(); assert(f);
    assert(fputs("prefijo\n", f) >= 0);
    /* Sin direccion de layout: no genera salida. */
    assert(ri_generar_funcion(&fn, &sim, 8, &rr, f, NULL) == 1);
    assert(fflush(f) == 0); rewind(f);
    char buf[8192]; size_t n = fread(buf, 1, sizeof(buf) - 1, f);
    buf[n] = '\0'; assert(strcmp(buf, "prefijo\n") == 0);

    assert(rr_definir(&rr, "identidad", RR_FUNCION, 0x1000) == RR_OK);
    assert(fseek(f, 0, SEEK_END) == 0);
    assert(ri_generar_funcion(&fn, &sim, 8, &rr, f, stderr) == 0);
    assert(fflush(f) == 0); rewind(f);
    n = fread(buf, 1, sizeof(buf) - 1, f); buf[n] = '\0';
    assert(strstr(buf, "prefijo\n") == buf);
    assert(strstr(buf, "sumai x13, x0, 0\n") != NULL ||
           strstr(buf, "sumai x13, x0, 16\n") != NULL ||
           strstr(buf, "cizqi x13") != NULL);
    assert(strstr(buf, "syer x1, x13, 0\n") != NULL);
    fclose(f);
    rr_liberar(&rr);
    sim_liberar(&sim);
    puts("OK: 3B-D3E destinos registrados, llamada recursiva y rechazo sin layout");
    return EXIT_SUCCESS;
}
