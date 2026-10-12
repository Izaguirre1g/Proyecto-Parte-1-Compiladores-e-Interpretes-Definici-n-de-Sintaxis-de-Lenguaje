#include "generacion_funciones.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static AstNodo nodo(AstTipo tipo, char *texto)
{
    AstNodo n = {0};
    n.tipo = tipo;
    n.texto = texto;
    return n;
}

static void hijos(AstNodo *padre, AstNodo *primero)
{
    padre->primer_hijo = primero;
}

static int referencia(void *datos, const AstNodo *id, GbReferencia *ref)
{
    (void)datos;
    if (strcmp(id->texto, "a") == 0)
        *ref = (GbReferencia){GB_ENTERO, 2, 4, 1};
    else if (strcmp(id->texto, "b") == 0)
        *ref = (GbReferencia){GB_ENTERO, 2, 8, 1};
    else if (strcmp(id->texto, "r") == 0)
        *ref = (GbReferencia){GB_ENTERO, 2, 12, 1};
    else return 0;
    return 1;
}

static int destino(void *datos, const AstNodo *id, unsigned *dir)
{
    (void)datos;
    if (strcmp(id->texto, "sumar") != 0) return 0;
    *dir = 4096;
    return 1;
}

static void leer(FILE *f, char *buffer, size_t capacidad)
{
    rewind(f);
    size_t n = fread(buffer, 1, capacidad - 1, f);
    assert(!ferror(f));
    buffer[n] = 0;
}

static void cuerpo_simple(void)
{
    AstNodo fn = nodo(AST_FUNCION, NULL), id = nodo(AST_IDENTIFICADOR, "identidad");
    AstNodo tipo = nodo(AST_TIPO, "declare_int"), ps = nodo(AST_PARAMETROS, NULL);
    AstNodo par = nodo(AST_PARAMETRO, NULL), pid = nodo(AST_IDENTIFICADOR, "a");
    AstNodo pt = nodo(AST_TIPO, "declare_int"), bloque = nodo(AST_BLOQUE, NULL);
    AstNodo ret = nodo(AST_RETORNO, NULL), valor = nodo(AST_IDENTIFICADOR, "a");
    hijos(&fn, &id); id.siguiente = &tipo; tipo.siguiente = &ps;
    ps.siguiente = &bloque; hijos(&ps, &par);
    hijos(&par, &pid); pid.siguiente = &pt;
    hijos(&bloque, &ret); hijos(&ret, &valor);
    FILE *f = tmpfile(); assert(f);
    assert(gf_generar_funcion(&fn, 8, referencia, destino, NULL, f, stderr) == 0);
    char b[2048]; leer(f, b, sizeof(b));
    assert(strcmp(b, "restai x2, x2, 8\n"
                     "guardap x2, x1, 0\n"
                     "guardap x2, x8, 4\n"
                     "cargai x4, x2, 4\n"
                     "sumai x8, x4, 0\n"
                     "cargai x1, x2, 0\n"
                     "sumai x2, x2, 8\n"
                     "syer x0, x1, 0\n") == 0);
    fclose(f);
}

static void llamada_con_pila(void)
{
    AstNodo fn = nodo(AST_FUNCION, NULL), id = nodo(AST_IDENTIFICADOR, "principal");
    AstNodo tipo = nodo(AST_TIPO, "declare_int"), ps = nodo(AST_PARAMETROS, NULL);
    AstNodo bloque = nodo(AST_BLOQUE, NULL), ret = nodo(AST_RETORNO, NULL);
    AstNodo call = nodo(AST_LLAMADA, NULL), target = nodo(AST_IDENTIFICADOR, "sumar");
    AstNodo args = nodo(AST_ARGUMENTOS, NULL), a = nodo(AST_IDENTIFICADOR, "a");
    AstNodo b = nodo(AST_IDENTIFICADOR, "b");
    hijos(&fn, &id); id.siguiente = &tipo; tipo.siguiente = &ps;
    ps.siguiente = &bloque; hijos(&bloque, &ret); hijos(&ret, &call);
    hijos(&call, &target); target.siguiente = &args;
    hijos(&args, &a); a.siguiente = &b;
    FILE *f = tmpfile(); assert(f);
    assert(gf_generar_funcion(&fn, 16, referencia, destino, NULL, f, stderr) == 0);
    char texto[4096]; leer(f, texto, sizeof(texto));
    assert(strstr(texto, "cargai x4, x2, 4\nrestai x2, x2, 4\n") != NULL);
    assert(strstr(texto, "cargai x4, x2, 12\nrestai x2, x2, 4\n") != NULL);
    assert(strstr(texto, "cargai x4, x2, 4\ncargai x5, x2, 0\n") != NULL);
    assert(strstr(texto, "syer x1, x13, 0\n") != NULL);
    assert(strstr(texto, "sumai x2, x2, 8\ncargai x1, x2, 0\n") != NULL);
    fclose(f);

    /* Rechazo atomico: cinco argumentos no entran en el subconjunto D1. */
    AstNodo c = nodo(AST_ENTERO, "3"), d = nodo(AST_ENTERO, "4");
    AstNodo e = nodo(AST_ENTERO, "5");
    b.siguiente = &c; c.siguiente = &d; d.siguiente = &e;
    f = tmpfile(); assert(f);
    fputs("previo\n", f);
    assert(gf_generar_funcion(&fn, 16, referencia, destino, NULL, f, NULL) == 1);
    leer(f, texto, sizeof(texto));
    assert(strcmp(texto, "previo\n") == 0);
    fclose(f);
}

static void rechazo_no_final(void)
{
    AstNodo fn = nodo(AST_FUNCION, NULL), id = nodo(AST_IDENTIFICADOR, "f");
    AstNodo tipo = nodo(AST_TIPO, "declare_int"), ps = nodo(AST_PARAMETROS, NULL);
    AstNodo bloque = nodo(AST_BLOQUE, NULL), ret = nodo(AST_RETORNO, NULL);
    AstNodo valor = nodo(AST_ENTERO, "7"), despues = nodo(AST_RETORNO, NULL);
    hijos(&fn, &id); id.siguiente = &tipo; tipo.siguiente = &ps;
    ps.siguiente = &bloque; hijos(&bloque, &ret);
    hijos(&ret, &valor); ret.siguiente = &despues;
    FILE *f = tmpfile(); assert(f);
    assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, NULL) == 1);
    assert(ftell(f) == 0);
    fclose(f);
}

static void llamadas_anidadas(void)
{
    /* give sumar(sumar(2 gauss 3)) gauss sumar(4); */
    AstNodo fn = nodo(AST_FUNCION, NULL), id = nodo(AST_IDENTIFICADOR, "f");
    AstNodo tipo = nodo(AST_TIPO, "declare_int"), ps = nodo(AST_PARAMETROS, NULL);
    AstNodo bloque = nodo(AST_BLOQUE, NULL), ret = nodo(AST_RETORNO, NULL);
    AstNodo suma = nodo(AST_BINARIO, "gauss");
    AstNodo externa = nodo(AST_LLAMADA, NULL), e_id = nodo(AST_IDENTIFICADOR, "sumar");
    AstNodo e_args = nodo(AST_ARGUMENTOS, NULL);
    AstNodo interna = nodo(AST_LLAMADA, NULL), i_id = nodo(AST_IDENTIFICADOR, "sumar");
    AstNodo i_args = nodo(AST_ARGUMENTOS, NULL), arit = nodo(AST_BINARIO, "gauss");
    AstNodo dos = nodo(AST_ENTERO, "2"), tres = nodo(AST_ENTERO, "3");
    AstNodo otra = nodo(AST_LLAMADA, NULL), o_id = nodo(AST_IDENTIFICADOR, "sumar");
    AstNodo o_args = nodo(AST_ARGUMENTOS, NULL), cuatro = nodo(AST_ENTERO, "4");
    hijos(&fn, &id); id.siguiente = &tipo; tipo.siguiente = &ps;
    ps.siguiente = &bloque; hijos(&bloque, &ret); hijos(&ret, &suma);
    hijos(&suma, &externa); externa.siguiente = &otra;
    hijos(&externa, &e_id); e_id.siguiente = &e_args; hijos(&e_args, &interna);
    hijos(&interna, &i_id); i_id.siguiente = &i_args; hijos(&i_args, &arit);
    hijos(&arit, &dos); dos.siguiente = &tres;
    hijos(&otra, &o_id); o_id.siguiente = &o_args; hijos(&o_args, &cuatro);
    FILE *f = tmpfile(); assert(f);
    assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, stderr) == 0);
    char texto[8192]; leer(f, texto, sizeof(texto));
    unsigned llamadas = 0;
    const char *cursor = texto;
    while ((cursor = strstr(cursor, "syer x1, x13, 0\n")) != NULL) {
        ++llamadas;
        cursor += strlen("syer x1, x13, 0\n");
    }
    assert(llamadas == 3);
    assert(strstr(texto, "guardap x2, x4, 0\n") != NULL);
    assert(strstr(texto, "cargai x5, x2, 0\nsuma x4, x5, x4\nsumai x2, x2, 4\n") != NULL);
    assert(strstr(texto, "sumai x8, x4, 0\n") != NULL);
    fclose(f);

    /* Un operador no integrado debe conservar la salida preexistente. */
    suma.texto = "pitagoras";
    f = tmpfile(); assert(f); fputs("previo\n", f);
    assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, NULL) == 1);
    leer(f, texto, sizeof(texto));
    assert(strcmp(texto, "previo\n") == 0);
    fclose(f);
}

int main(void)
{
    cuerpo_simple();
    llamada_con_pila();
    rechazo_no_final();
    llamadas_anidadas();
    puts("OK: 3B-D2 AST llamadas anidadas, temporales en pila y rollback");
    return EXIT_SUCCESS;
}
