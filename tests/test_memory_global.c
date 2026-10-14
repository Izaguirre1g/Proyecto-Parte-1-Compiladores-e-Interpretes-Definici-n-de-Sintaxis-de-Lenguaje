#include "memory.h"
#include <assert.h>
#include <stdio.h>

static SimSimbolo *nuevo(SimTabla *t, const char *nombre, SimTipo tipo)
{
    SimSimbolo *s = NULL;
    assert(sim_insertar(t, nombre, SIM_VARIABLE, tipo, 1, 1, &s) == SIM_OK);
    return s;
}

int main(void)
{
    SimTabla t = {0};
    MemGlobalMapa m = {0};
    assert(sim_inicializar(&t) == SIM_OK);
    SimSimbolo *a = nuevo(&t, "a", SIM_TIPO_CHAR);
    SimSimbolo *b = nuevo(&t, "b", SIM_TIPO_INT);
    SimSimbolo *c = nuevo(&t, "c", SIM_TIPO_BOOLEAN);
    assert(mem_asignar_globales(&t, 0x4000, 65536, &m) == MEM_GLOBAL_OK);
    assert(a->desplazamiento == 0 && a->bytes == 1);
    assert(b->desplazamiento == 4 && b->bytes == 4);
    assert(c->desplazamiento == 8 && c->bytes == 1);
    assert(m.bytes_globales == 9);
    assert(mem_offset_directo(1023) && !mem_offset_directo(1024));
    assert(!sim_verificar_pre_memoria(&t, NULL));
    sim_liberar(&t);

    t = (SimTabla){0};
    assert(sim_inicializar(&t) == SIM_OK);
    a = nuevo(&t, "a", SIM_TIPO_CHAR);
    b = nuevo(&t, "b", SIM_TIPO_TEXT);
    assert(mem_asignar_globales(&t, 0x4000, 65536, &m) == MEM_GLOBAL_TAMANO_INDEFINIDO);
    assert(a->almacen == SIM_ALMACEN_SIN_ASIGNAR && a->bytes == 0);
    assert(b->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    sim_liberar(&t);

    t = (SimTabla){0};
    assert(sim_inicializar(&t) == SIM_OK);
    a = nuevo(&t, "a", SIM_TIPO_INT);
    assert(mem_asignar_globales(&t, 65534, 65536, &m) == MEM_GLOBAL_SIN_ESPACIO);
    assert(a->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    sim_liberar(&t);
    puts("OK: offsets globales, alineacion, rollback y capacidad");
    return 0;
}
