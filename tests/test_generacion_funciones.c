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

/* Modelo secuencial de las instrucciones usadas por estos casos.
 * Comprueba que se ejecuta la rama correcta y que se restaura sp/ra.
 * No modela bundles, latencias ni reemplaza la simulacion del procesador. */
static int ejecutar_condicional(char *texto)
{
    char *lineas[256];
    size_t cantidad = 0;
    for (char *p = strtok(texto, "\n"); p; p = strtok(NULL, "\n")) {
        assert(cantidad < 256);
        lineas[cantidad++] = p;
    }
    int registros[32] = {0}, memoria[256] = {0};
    registros[2] = 512;
    registros[1] = 1232;
    for (size_t pc = 0, pasos = 0; pc < cantidad && pasos < 1000; ++pc, ++pasos) {
        char *linea = lineas[pc];
        unsigned a, b;
        int inmediato;
        char etiqueta[128];
        if (linea[strlen(linea) - 1] == ':') continue;
        if (sscanf(linea, "sumai x%u, x%u, %d", &a, &b, &inmediato) == 3)
            registros[a] = registros[b] + inmediato;
        else if (sscanf(linea, "restai x%u, x%u, %d", &a, &b, &inmediato) == 3)
            registros[a] = registros[b] - inmediato;
        else if (sscanf(linea, "guardap x%u, x%u, %d", &a, &b, &inmediato) == 3) {
            int direccion = registros[a] + inmediato;
            assert(direccion >= 0 && direccion < 1024 && direccion % 4 == 0);
            memoria[direccion / 4] = registros[b];
        } else if (sscanf(linea, "cargai x%u, x%u, %d", &a, &b, &inmediato) == 3) {
            int direccion = registros[b] + inmediato;
            assert(direccion >= 0 && direccion < 1024 && direccion % 4 == 0);
            registros[a] = memoria[direccion / 4];
        } else if (!strcmp(linea, "syer x0, x1, 0")) {
            assert(registros[2] == 512 && registros[1] == 1232);
            return registros[8];
        } else {
            int saltar = 0;
            if (sscanf(linea, "igualsi x%u, x%u, %127s", &a, &b, etiqueta) == 3)
                saltar = registros[a] == registros[b];
            else {
                assert(sscanf(linea, "sye x0, %127s", etiqueta) == 1);
                saltar = 1;
            }
            if (saltar) {
                size_t destino;
                for (destino = 0; destino < cantidad; ++destino)
                    if (strlen(lineas[destino]) == strlen(etiqueta) + 1 &&
                        !strncmp(lineas[destino], etiqueta, strlen(etiqueta)) &&
                        lineas[destino][strlen(etiqueta)] == ':') break;
                assert(destino < cantidad);
                pc = destino;
            }
        }
        registros[0] = 0;
    }
    assert(!"la funcion no retorno");
    return -1;
}

static void condicionales(void)
{
    AstNodo fn = nodo(AST_FUNCION, NULL), id = nodo(AST_IDENTIFICADOR, "elegir");
    AstNodo tipo = nodo(AST_TIPO, "declare_int"), params = nodo(AST_PARAMETROS, NULL);
    AstNodo bloque = nodo(AST_BLOQUE, NULL), si = nodo(AST_SI, NULL);
    AstNodo cond = nodo(AST_BOOLEANO, "declare_true"), verdadero = nodo(AST_BLOQUE, NULL);
    AstNodo alternativo = nodo(AST_SI, NULL), cond2 = nodo(AST_BOOLEANO, "declare_true");
    AstNodo segundo = nodo(AST_BLOQUE, NULL), ultimo = nodo(AST_BLOQUE, NULL);
    AstNodo ret1 = nodo(AST_RETORNO, NULL), uno = nodo(AST_ENTERO, "10");
    AstNodo ret2 = nodo(AST_RETORNO, NULL), dos = nodo(AST_ENTERO, "20");
    AstNodo ret3 = nodo(AST_RETORNO, NULL), tres = nodo(AST_ENTERO, "30");
    hijos(&fn, &id); id.siguiente = &tipo; tipo.siguiente = &params;
    params.siguiente = &bloque; hijos(&bloque, &si);
    hijos(&si, &cond); cond.siguiente = &verdadero; verdadero.siguiente = &alternativo;
    hijos(&verdadero, &ret1); hijos(&ret1, &uno);
    hijos(&alternativo, &cond2); cond2.siguiente = &segundo; segundo.siguiente = &ultimo;
    hijos(&segundo, &ret2); hijos(&ret2, &dos);
    hijos(&ultimo, &ret3); hijos(&ret3, &tres);
    for (int a = 0; a < 2; ++a) for (int b = 0; b < 2; ++b) {
        cond.texto = a ? "declare_true" : "declare_false";
        cond2.texto = b ? "declare_true" : "declare_false";
        FILE *f = tmpfile(); assert(f);
        assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, stderr) == 0);
        char texto[8192]; leer(f, texto, sizeof(texto)); fclose(f);
        assert(strstr(texto, "__gf_elegir_else_0:") != NULL);
        assert(strstr(texto, "__gf_elegir_else_1:") != NULL);
        assert(ejecutar_condicional(texto) == (a ? 10 : b ? 20 : 30));
    }
    /* El mismo AST con whether anidado en un bloque. */
    AstNodo envoltura = nodo(AST_BLOQUE, NULL);
    hijos(&envoltura, &alternativo); verdadero.siguiente = &envoltura;
    FILE *f = tmpfile(); assert(f);
    assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, stderr) == 0);
    fclose(f);

    /* Ramas que continuan: el salto a fin debe omitir el also. */
    verdadero.siguiente = &ultimo; verdadero.primer_hijo = NULL;
    si.siguiente = &ret2;
    for (int a = 0; a < 2; ++a) {
        cond.texto = a ? "declare_true" : "declare_false";
        f = tmpfile(); assert(f);
        assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, stderr) == 0);
        char texto[8192]; leer(f, texto, sizeof(texto)); fclose(f);
        assert(strstr(texto, "sye x0, __gf_elegir_fin_0\n") != NULL);
        assert(ejecutar_condicional(texto) == (a ? 20 : 30));
    }
    /* Sin also, una rama retorna y la otra continua hasta give 20. */
    verdadero.siguiente = NULL; verdadero.primer_hijo = &ret1;
    for (int a = 0; a < 2; ++a) {
        cond.texto = a ? "declare_true" : "declare_false";
        id.texto = "otra_funcion";
        f = tmpfile(); assert(f);
        assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, stderr) == 0);
        char texto[8192]; leer(f, texto, sizeof(texto)); fclose(f);
        assert(strstr(texto, "__gf_otra_funcion_else_0:") != NULL);
        assert(ejecutar_condicional(texto) == (a ? 10 : 20));
    }
    /* Falta retorno en la ruta falsa: se conserva la salida previa. */
    si.siguiente = NULL;
    f = tmpfile(); assert(f); fputs("previo\n", f);
    assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, NULL) == 1);
    char texto[8192]; leer(f, texto, sizeof(texto)); fclose(f);
    assert(strcmp(texto, "previo\n") == 0);
    /* Estructura incorrecta: tampoco publica el prologo parcial. */
    cond.siguiente = NULL;
    f = tmpfile(); assert(f);
    assert(gf_generar_funcion(&fn, 4, referencia, destino, NULL, f, NULL) == 1);
    assert(ftell(f) == 0); fclose(f);
}

int main(void)
{
    cuerpo_simple();
    llamada_con_pila();
    rechazo_no_final();
    llamadas_anidadas();
    condicionales();
    puts("OK: funciones, llamadas, whether/alif/also, rutas de retorno y rollback");
    return EXIT_SUCCESS;
}
