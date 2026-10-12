#include "funciones_llamada.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void leer(FILE *f, char *buf, size_t capacidad)
{
    rewind(f);
    size_t n = fread(buf, 1, capacidad - 1, f);
    assert(!ferror(f));
    buf[n] = 0;
}

static void caso_cero(void)
{
    FILE *f = tmpfile(); assert(f);
    assert(fl_emitir_llamada(f, NULL, 0) == FL_OK);
    char b[256]; leer(f, b, sizeof(b));
    assert(strcmp(b, "syer x1, x13, 0\n") == 0);
    fclose(f);
}

static void caso_permutacion(void)
{
    const unsigned fuentes[] = {9, 8, 11, 10};
    FILE *f = tmpfile(); assert(f);
    assert(fl_emitir_llamada(f, fuentes, 4) == FL_OK);
    char b[1024]; leer(f, b, sizeof(b));
    assert(strcmp(b,
        "restai x2, x2, 16\n"
        "guardap x2, x9, 0\n"
        "guardap x2, x8, 4\n"
        "guardap x2, x11, 8\n"
        "guardap x2, x10, 12\n"
        "cargai x8, x2, 0\n"
        "cargai x9, x2, 4\n"
        "cargai x10, x2, 8\n"
        "cargai x11, x2, 12\n"
        "syer x1, x13, 0\n"
        "sumai x2, x2, 16\n") == 0);
    fclose(f);
}

static void caso_extras(void)
{
    const unsigned fuentes[] = {4, 5, 6, 7, 14, 15};
    FILE *f = tmpfile(); assert(f);
    assert(fl_emitir_llamada(f, fuentes, 6) == FL_OK);
    char b[2048]; leer(f, b, sizeof(b));
    assert(strstr(b, "restai x2, x2, 24\n") == b);
    assert(strstr(b, "guardap x2, x14, 16\n") != NULL);
    assert(strstr(b, "guardap x2, x15, 20\n") != NULL);
    assert(strstr(b, "cargai x12, x2, 16\nguardap x2, x12, 0\n") != NULL);
    assert(strstr(b, "cargai x12, x2, 20\nguardap x2, x12, 4\n") != NULL);
    assert(strstr(b, "syer x1, x13, 0\nsumai x2, x2, 24\n") != NULL);
    fclose(f);
}

static void caso_limites(void)
{
    FILE *f = tmpfile(); assert(f);
    fputs("previo\n", f);
    unsigned invalido[] = {4, 2};
    assert(fl_emitir_llamada(f, NULL, 1) == FL_ARGUMENTO_INVALIDO);
    assert(fl_emitir_llamada(f, invalido, 2) == FL_ARGUMENTO_INVALIDO);
    invalido[1] = 32;
    assert(fl_emitir_llamada(f, invalido, 2) == FL_ARGUMENTO_INVALIDO);
    assert(fl_emitir_llamada(f, invalido, 256) == FL_DEMASIADOS_ARGUMENTOS);
    char b[256]; leer(f, b, sizeof(b));
    assert(strcmp(b, "previo\n") == 0);
    fclose(f);

    unsigned muchos[255];
    for (size_t i = 0; i < 255; ++i) muchos[i] = 4;
    f = tmpfile(); assert(f);
    assert(fl_emitir_llamada(f, muchos, 255) == FL_OK);
    char linea[128], ultima[128] = "";
    size_t cuenta = 0;
    rewind(f);
    while (fgets(linea, sizeof(linea), f)) {
        if (strstr(linea, "guardap x2, x4, ")) ++cuenta;
        memcpy(ultima, linea, strlen(linea) + 1);
    }
    assert(cuenta == 255);
    assert(strcmp(ultima, "sumai x2, x2, 1020\n") == 0);
    fclose(f);
}

int main(void)
{
    caso_cero();
    caso_permutacion();
    caso_extras();
    caso_limites();
    puts("OK: 3B-B llamada escalar, permutacion, argumentos por pila y limites");
    return EXIT_SUCCESS;
}
