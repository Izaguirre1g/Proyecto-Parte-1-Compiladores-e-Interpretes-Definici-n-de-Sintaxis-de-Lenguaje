#include "semantic.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int caso(const char *origen, const char *modulo, const char *esperado)
{
    char *ruta = sem_ruta_modulo(origen, modulo);
    int correcto = ruta && strcmp(ruta, esperado) == 0;
    if (!correcto) {
        fprintf(stderr, "ERROR: %s + %s => %s (esperado: %s)\n",
                origen, modulo, ruta ? ruta : "NULL", esperado);
    }
    free(ruta);
    return correcto;
}

int main(void)
{
    if (!caso("tests/modulos/principal.bal", "matematicas",
              "tests/modulos/matematicas.bal") ||
        !caso("principal.bal", "matematicas", "matematicas.bal") ||
        !caso("./principal.bal", "datos", "./datos.bal") ||
        !caso("proyecto/sub/main.bal", "utils", "proyecto/sub/utils.bal")) {
        return 1;
    }
    char *invalida = sem_ruta_modulo("main.bal", "../fuera");
    if (invalida) {
        free(invalida);
        fprintf(stderr, "ERROR: se acepto un nombre de modulo invalido\n");
        return 1;
    }
    puts("OK: rutas relativas de modulos BAL");
    return 0;
}
