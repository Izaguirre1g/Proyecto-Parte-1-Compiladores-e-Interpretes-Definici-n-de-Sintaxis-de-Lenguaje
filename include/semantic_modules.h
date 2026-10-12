#ifndef SEMANTIC_MODULES_H
#define SEMANTIC_MODULES_H
#include <stdio.h>
/* 0: proyecto valido; 1: errores; 2: fallo de recursos o parametros. */
int sem_analizar_proyecto(const char *archivo_principal, FILE *diagnosticos);
#endif
