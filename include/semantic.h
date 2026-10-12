#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"
#include <stdio.h>

/*
 * Realiza el analisis semantico del AST.
 *
 * Requisitos:
 * - El AST debe haberse construido correctamente.
 * - La tabla de simbolos debe estar inicializada.
 * - El ambito actual debe ser global.
 *
 * Valores de retorno:
 * 0: analisis correcto.
 * 1: se encontraron errores semanticos.
 * 2: fallo de memoria o argumentos invalidos.
 *
 * El analizador no modifica ni libera el AST.
 */
int analizar_semantica(
    const Ast *arbol,
    SimTabla *tabla,
    const char *archivo,
    FILE *diagnosticos
);

#endif