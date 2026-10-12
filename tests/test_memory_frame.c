#include "memory.h"
#include <assert.h>
#include <stdio.h>

static SimSimbolo *agregar(SimTabla *t, const char *n, SimClase c, SimTipo ty)
{
    SimSimbolo *s = NULL;
    assert(sim_insertar(t, n, c, ty, 1, 1, &s) == SIM_OK);
    assert(s != NULL);
    return s;
}

int main(void)
{
    SimTabla t = {0};
    assert(sim_inicializar(&t) == SIM_OK);
    (void)agregar(&t, "f", SIM_FUNCION, SIM_TIPO_VOID);
    (void)agregar(&t, "otra", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&t, "f") == SIM_OK);
    SimSimbolo *a = agregar(&t, "a", SIM_VARIABLE, SIM_TIPO_CHAR);
    assert(sim_entrar_ambito(&t, "bloque_1") == SIM_OK);
    SimSimbolo *i = agregar(&t, "i", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    assert(sim_salir_ambito(&t));
    assert(sim_entrar_ambito(&t, "otra") == SIM_OK);
    SimSimbolo *z = agregar(&t, "z", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    MemMarco m = {0};
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(mem_asignar_marco(&t,"f",8,&m)==MEM_MARCO_SIN_ESPACIO);
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(mem_asignar_marco(&t,"f",64,&m)==MEM_MARCO_OK);
    assert(a->desplazamiento==4 && a->bytes==1 && a->almacen==SIM_ALMACEN_LOCAL);
    assert(i->desplazamiento==8 && i->bytes==4 && i->almacen==SIM_ALMACEN_LOCAL);
    assert(z->almacen==SIM_ALMACEN_SIN_ASIGNAR);
    assert(m.offset_ra==0 && m.bytes_marco==12 && m.bytes_locales==8);
    assert(!sim_verificar_pre_memoria(&t, NULL));
    sim_liberar(&t);
    puts("OK: marcos locales, bloques, alineacion, reserva ra y rollback");
    return 0;
}
