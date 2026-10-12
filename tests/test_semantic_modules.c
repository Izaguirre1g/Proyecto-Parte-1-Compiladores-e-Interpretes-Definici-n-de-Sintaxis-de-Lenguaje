#include "semantic_modules.h"
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
    if (argc != 3) {
        fprintf(stderr, "Uso: %s archivo_principal.bal esperado\n", argv[0]);
        return 2;
    }
    int esperado = atoi(argv[2]);
    int obtenido = sem_analizar_proyecto(argv[1], stderr);
    printf("MODULOS: esperado=%d obtenido=%d\n", esperado, obtenido);
    return esperado == obtenido ? 0 : 1;
}
