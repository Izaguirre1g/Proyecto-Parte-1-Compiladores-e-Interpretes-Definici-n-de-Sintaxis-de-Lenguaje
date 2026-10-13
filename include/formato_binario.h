#ifndef FORMATO_BINARIO_H
#define FORMATO_BINARIO_H

#include "ast.h"
#include "symbol_table.h"
#include "codificacion_binaria.h"

#define FB_CABECERA_BYTES 48u
#define FB_MAX_DATOS 16384u

typedef struct {
    uint8_t bytes[FB_MAX_DATOS];
    size_t cantidad;
    uint32_t base;
    uint32_t flags; /* bit 0: el arranque tambien inicializa los datos */
} FbDatos;

/* Obtiene los valores globales antes de que se liberen AST y simbolos. */
int fb_preparar_datos(const Ast *ast, const SimTabla *tabla, uint32_t base,
                     size_t cantidad, FbDatos *datos, FILE *diagnosticos);
/* Contenedor BALB v1: cabecera, codigo crudo y datos; entrada en PC=0. */
int fb_escribir(const CbPrograma *programa, const FbDatos *datos, FILE *salida);

#endif
