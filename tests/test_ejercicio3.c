#include "sintactico.h"
#include "semantic.h"
#include "memory.h"
#include "referencias_integracion.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Emite funciones para el modelo de prueba, NO direcciones de bundles.
 * Los destinos 16,32,... son identificadores alineados del arnes. */
int main(void)
{
    FILE *entrada = fopen("examples/ejercicio3_puntajes.bal", "r");
    assert(entrada);
    char fuente[8192];
    size_t longitud = fread(fuente, 1, sizeof(fuente)-1, entrada);
    assert(!ferror(entrada) && feof(entrada)); fuente[longitud] = 0;
    fclose(entrada);
    Ast ast = {0}; SimTabla tabla = {0}; MemProyectoMapa mapa;
    assert(construir_ast(fuente, longitud, "ejercicio3", stderr, &ast) == 0);
    assert(sim_inicializar(&tabla) == SIM_OK);
    assert(analizar_semantica(&ast, &tabla, "ejercicio3", stderr) == 0);
    assert(mem_asignar_proyecto(&tabla, 0x4000, 65536, 32768, &mapa) == MEM_PROYECTO_OK);
    assert(mapa.funciones == 4);
    RrTabla destinos; rr_inicializar(&destinos);
    unsigned direccion = 16;
    for (const AstNodo *f = ast.raiz->primer_hijo; f; f = f->siguiente) {
        assert(f->tipo == AST_FUNCION);
        assert(rr_definir(&destinos, f->primer_hijo->texto, RR_FUNCION, direccion) == RR_OK);
        direccion += 16;
    }
    puts("# Solo modelo secuencial: destinos de prueba, no layout VLIW");
    direccion = 16;
    for (const AstNodo *f = ast.raiz->primer_hijo; f; f = f->siguiente) {
        const char *nombre = f->primer_hijo->texto;
        const SimAmbito *ambito = NULL;
        for (const SimAmbito *a = tabla.global->siguiente_creacion; a; a = a->siguiente_creacion)
            if (a->padre == tabla.global && !strcmp(a->nombre, nombre)) ambito = a;
        assert(ambito);
        /* Enteros de 4 bytes: el ultimo slot asignado determina el marco.
         * No usar marco_maximo_bytes para todas las funciones. */
        size_t marco = 4;
        for (const SimSimbolo *s = tabla.primero; s; s = s->siguiente_orden) {
            const SimAmbito *a = s->ambito;
            while (a && a != ambito) a = a->padre;
            if (!a) continue;
            assert(s->bytes == 4 && s->desplazamiento >= 4);
            size_t fin = (size_t)s->desplazamiento + s->bytes;
            if (fin > marco) marco = fin;
        }
        printf("# funcion %s %u %zu\n", nombre, direccion, marco);
        if (!strcmp(nombre, "main"))
            for (const SimSimbolo *s = tabla.primero; s; s = s->siguiente_orden)
                if (s->ambito == ambito)
                    printf("# local %s %ld\n", s->nombre, s->desplazamiento);
        assert(ri_generar_funcion(f, &tabla, marco, &destinos, stdout, stderr) == 0);
        direccion += 16;
    }
    rr_liberar(&destinos); sim_liberar(&tabla); ast_liberar(&ast);
    return 0;
}
