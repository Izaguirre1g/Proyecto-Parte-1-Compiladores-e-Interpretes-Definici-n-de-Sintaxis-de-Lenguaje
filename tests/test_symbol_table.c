#include "symbol_table.h"
#include <assert.h>
#include <stdio.h>

int main(void) {

    SimTabla t = {0};
    SimSimbolo *global = NULL;

    // Inicializar tabla
    assert(sim_inicializar(&t) == SIM_OK);

    // Insertar variable
    assert(
        sim_insertar(
            &t,
            "x",
            SIM_VARIABLE,
            SIM_TIPO_INT,
            1,
            1,
            &global
        ) == SIM_OK
    );

    // Comprobar búsqueda
    assert(sim_buscar(&t, "x") == global);

    printf("Prueba exitosa\n");

    sim_liberar(&t);

    return 0;
}