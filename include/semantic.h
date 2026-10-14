#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symbol_table.h"
#include <stdio.h>

/*
 * Determina si el AST corresponde al
 * programa ejecutable o a un modulo.
 */
typedef enum {
    SEM_PROGRAMA_PRINCIPAL,
    SEM_MODULO_IMPORTADO
} SemModo;

/*
 * Analiza un AST segun su contexto.
 *
 * Programa principal:
 *   exige exactamente una funcion main.
 *
 * Modulo importado:
 *   no exige main y rechaza su declaracion.
 */
int analizar_semantica_modo(
    const Ast *arbol,
    SimTabla *tabla,
    const char *archivo,
    FILE *diagnosticos,
    SemModo modo
);
/*
 * Resuelve un modulo importado mediante
 * su alias visible.
 *
 * El AST y la tabla pertenecen al cargador.
 */
typedef int (*SemResolverModulo)(
    void *contexto,
    const char *archivo,
    const char *alias,
    const Ast **arbol,
    const SimTabla **tabla
);

/*
 * Analisis semantico con acceso a modulos.
 *
 * Conserva las reglas del modo principal
 * y del modo modulo importado.
 */
int analizar_semantica_con_modulos(
    const Ast *arbol,
    SimTabla *tabla,
    const char *archivo,
    FILE *diagnosticos,
    SemModo modo,
    SemResolverModulo resolver,
    void *contexto
);

/* Devuelve una ruta reservada con malloc; liberar con free. */
char *sem_ruta_modulo(const char *archivo_origen, const char *nombre_modulo);

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