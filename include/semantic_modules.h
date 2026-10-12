#ifndef SEMANTIC_MODULES_H
#define SEMANTIC_MODULES_H
#include <stdio.h>
#include "ast.h"
#include "symbol_table.h"
/* 0: proyecto valido; 1: errores; 2: fallo de recursos o parametros. */
int sem_analizar_proyecto(const char *archivo_principal, FILE *diagnosticos);
/* Vista prestada: AST y simbolos pertenecen al cargador y solo son validos
 * mientras se ejecuta el callback. No se deben liberar ni conservar punteros.
 * Invocado UNA vez para el proyecto principal tras validar todos sus modulos.
 * No se invoca ante errores semanticos. Retorno: 0, 1 o 2.
 * El callback puede asignar memoria sobre la tabla principal.
 */
typedef int (*SemProyectoVisitante)(void *contexto, const Ast *arbol,
                                    SimTabla *tabla, FILE *diagnosticos);
int sem_visitar_proyecto(const char *archivo_principal, FILE *diagnosticos,
                        SemProyectoVisitante visitante, void *contexto);
#endif
