#include "generacion_adaptador.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNodo nodo(AstTipo t, char *texto)
{
    AstNodo n = {0}; n.tipo = t; n.texto = texto; return n;
}

static int direccion(void *datos, const AstNodo *id, unsigned *out)
{
    (void)datos;
    if (strcmp(id->texto, "identidad") != 0) return 0;
    *out = 0x1000; /* Direccion definida exclusivamente por esta prueba. */
    return 1;
}

int main(void)
{
    SimTabla tabla = {0};
    assert(sim_inicializar(&tabla) == SIM_OK);
    SimSimbolo *s = NULL;
    assert(sim_insertar(&tabla, "identidad", SIM_FUNCION, SIM_TIPO_INT,
                       1, 1, &s) == SIM_OK);
    assert(sim_entrar_ambito(&tabla, "identidad") == SIM_OK);
    assert(sim_insertar(&tabla, "a", SIM_PARAMETRO, SIM_TIPO_INT,
                       1, 1, &s) == SIM_OK);
    sim_definir_ubicacion(s, SIM_ALMACEN_PARAMETRO, 4, 4);
    assert(sim_salir_ambito(&tabla));

    AstNodo fn = nodo(AST_FUNCION, NULL);
    AstNodo id = nodo(AST_IDENTIFICADOR, "identidad");
    AstNodo tipo = nodo(AST_TIPO, "declare_int");
    AstNodo params = nodo(AST_PARAMETROS, NULL);
    AstNodo par = nodo(AST_PARAMETRO, NULL);
    AstNodo pid = nodo(AST_IDENTIFICADOR, "a");
    AstNodo ptip = nodo(AST_TIPO, "declare_int");
    AstNodo bloque = nodo(AST_BLOQUE, NULL);
    AstNodo ret = nodo(AST_RETORNO, NULL);
    AstNodo valor = nodo(AST_IDENTIFICADOR, "a");
    fn.primer_hijo = &id; id.siguiente = &tipo;
    tipo.siguiente = &params; params.siguiente = &bloque;
    params.primer_hijo = &par; par.primer_hijo = &pid;
    pid.siguiente = &ptip; bloque.primer_hijo = &ret;
    ret.primer_hijo = &valor;

    FILE *f = tmpfile(); assert(f);
    assert(gd_generar_funcion(&fn, &tabla, 8, direccion, NULL, f, stderr) == 0);
    char contenido[1024]; rewind(f);
    size_t leidos = fread(contenido, 1, sizeof(contenido) - 1, f);
    contenido[leidos] = '\0';
    assert(strstr(contenido, "cargai x4, x2, 4\n") != NULL);
    assert(strstr(contenido, "guardap x2, x8, 4\n") != NULL);
    fclose(f);

    /* Slot fuera del marco: rechazo atomico. */
    s->desplazamiento = 20;
    f = tmpfile(); assert(f);
    assert(fputs("prefijo\n", f) >= 0);
    assert(gd_generar_funcion(&fn, &tabla, 8, direccion, NULL, f, NULL) == 1);
    rewind(f); char prefijo[32];
    assert(fgets(prefijo, sizeof(prefijo), f));
    assert(strcmp(prefijo, "prefijo\n") == 0);
    assert(fgetc(f) == EOF);
    fclose(f);
    sim_liberar(&tabla);
    puts("OK: 3B-D3C referencias por ambito, offsets reales y rechazo");
    return EXIT_SUCCESS;
}
