#include "semantic_modules.h"
#include "memory.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { unsigned llamadas; int memoria; } Estado;

static int visitar(void *datos, const Ast *ast, SimTabla *tabla, FILE *diag)
{
    Estado *e = datos;
    (void)diag;
    ++e->llamadas;
    assert(ast && ast->raiz && ast->raiz->tipo == AST_PROGRAMA);
    assert(tabla && tabla->global && tabla->actual == tabla->global);
    const SimSimbolo *id = NULL;
    const SimSimbolo *main_fn = NULL;
    for (const SimSimbolo *s = tabla->primero; s; s = s->siguiente_orden) {
        if (s->ambito != tabla->global || s->clase != SIM_FUNCION) continue;
        if (strcmp(s->nombre, "identidad") == 0) id = s;
        if (strcmp(s->nombre, "main") == 0) main_fn = s;
    }
    assert(id && id->cantidad_parametros == 1);
    assert(main_fn && main_fn->cantidad_parametros == 0);
    assert(id->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    MemProyectoMapa mapa = {0};
    assert(mem_asignar_proyecto(tabla, 0, 4096, 1020, &mapa) == MEM_PROYECTO_OK);
    assert(mapa.funciones == 2);
    int parametro = 0, local = 0;
    for (const SimSimbolo *s = tabla->primero; s; s = s->siguiente_orden) {
        if (strcmp(s->nombre, "a") == 0) {
            assert(s->almacen == SIM_ALMACEN_PARAMETRO && s->desplazamiento == 4);
            parametro = 1;
        }
        if (strcmp(s->nombre, "n") == 0) {
            assert(s->almacen == SIM_ALMACEN_LOCAL && s->desplazamiento == 4);
            local = 1;
        }
    }
    assert(parametro && local);
    e->memoria = 1;
    return 0;
}

static int rechazar(void *datos, const Ast *ast, SimTabla *tabla, FILE *diag)
{
    Estado *e = datos;
    (void)ast; (void)tabla; (void)diag;
    ++e->llamadas;
    return 1;
}

int main(void)
{
    const char *valido = "tests/fixtures/3bd3b_escalar.bal";
    Estado e = {0};
    assert(sem_visitar_proyecto(valido, stderr, visitar, &e) == 0);
    assert(e.llamadas == 1 && e.memoria == 1);
    e.llamadas = 0;
    assert(sem_visitar_proyecto(valido, NULL, rechazar, &e) == 1);
    assert(e.llamadas == 1);
    e.llamadas = 0;
    assert(sem_visitar_proyecto("tests/modulos/principal_inexistente.bal",
                                NULL, visitar, &e) == 1);
    assert(e.llamadas == 0);
    assert(sem_visitar_proyecto(valido, NULL, NULL, &e) == 2);
    assert(sem_analizar_proyecto(valido, NULL) == 0);
    puts("OK: 3B-D3B callback, vida del AST, memoria y errores");
    return EXIT_SUCCESS;
}
