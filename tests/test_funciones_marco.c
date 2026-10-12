#include "funciones_marco.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void comprobar(const char *texto, const char *esperado)
{
    assert(strcmp(texto, esperado) == 0);
}

static void leer(FILE *f, char *buffer, size_t capacidad)
{
    rewind(f);
    size_t n = fread(buffer, 1, capacidad - 1, f);
    assert(!ferror(f));
    buffer[n] = 0;
}

int main(void)
{
    FILE *f = tmpfile();
    assert(f);
    assert(fm_emitir_prologo(f, 20, 3) == FM_OK);
    char texto[4096];
    leer(f, texto, sizeof(texto));
    comprobar(texto,
        "restai x2, x2, 20\n"
        "guardap x2, x1, 0\n"
        "guardap x2, x8, 4\n"
        "guardap x2, x9, 8\n"
        "guardap x2, x10, 12\n");
    fclose(f);

    f = tmpfile(); assert(f);
    assert(fm_emitir_prologo(f, 28, 5) == FM_OK);
    leer(f, texto, sizeof(texto));
    assert(strstr(texto, "sumai x12, x2, 0\n") != NULL);
    assert(strstr(texto, "sumai x12, x12, 28\n") != NULL);
    assert(strstr(texto, "cargai x12, x12, 0\n") != NULL);
    assert(strstr(texto, "guardap x2, x12, 20\n") != NULL);
    fclose(f);

    f = tmpfile(); assert(f);
    assert(fm_emitir_prologo(f, 2048, 0) == FM_OK);
    leer(f, texto, sizeof(texto));
    assert(strstr(texto, "restai x2, x2, 1020\nrestai x2, x2, 1020\nrestai x2, x2, 8\n") != NULL);
    fclose(f);

    f = tmpfile(); assert(f);
    assert(fm_emitir_epilogo(f, 20) == FM_OK);
    leer(f, texto, sizeof(texto));
    comprobar(texto, "cargai x1, x2, 0\nsumai x2, x2, 20\nsyer x0, x1, 0\n");
    fclose(f);

    f = tmpfile(); assert(f);
    fputs("previo\n", f);
    assert(fm_emitir_prologo(f, 8, 2) == FM_MARCO_NO_SOPORTADO);
    assert(fm_emitir_prologo(f, 7, 0) == FM_ARGUMENTO_INVALIDO);
    assert(fm_emitir_epilogo(f, 0) == FM_ARGUMENTO_INVALIDO);
    leer(f, texto, sizeof(texto));
    comprobar(texto, "previo\n");
    fclose(f);
    puts("OK: 3B-A prologos, epilogos, registros, quinto argumento y limites");
    return EXIT_SUCCESS;
}
