#include "memory.h"
#include <assert.h>
#include <stdio.h>

static SimSimbolo *add(SimTabla *t, const char *n, SimClase c, SimTipo ty)
{
    SimSimbolo *s = NULL;
    assert(sim_insertar(t, n, c, ty, 1, 1, &s) == SIM_OK);
    return s;
}

int main(void)
{
    SimTabla t = {0};
    assert(sim_inicializar(&t) == SIM_OK);
    SimSimbolo *g = add(&t, "global_int", SIM_CONSTANTE, SIM_TIPO_INT);
    (void)add(&t, "f", SIM_FUNCION, SIM_TIPO_VOID);
    (void)add(&t, "h", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&t, "f") == SIM_OK);
    SimSimbolo *a = add(&t, "a", SIM_VARIABLE, SIM_TIPO_CHAR);
    assert(sim_entrar_ambito(&t, "bloque") == SIM_OK);
    SimSimbolo *b = add(&t, "b", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t) && sim_salir_ambito(&t));
    assert(sim_entrar_ambito(&t, "h") == SIM_OK);
    SimSimbolo *c = add(&t, "c", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    MemProyectoMapa mapa = {0};
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(mem_asignar_proyecto(&t, 0x4000, 0x10000, 8, &mapa)
           == MEM_PROYECTO_SIN_ESPACIO);
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(mem_asignar_proyecto(&t, 0x4000, 0x10000, 64, &mapa)
           == MEM_PROYECTO_OK);
    assert(g->almacen == SIM_ALMACEN_GLOBAL && g->desplazamiento == 0 && g->bytes == 4);
    assert(a->almacen == SIM_ALMACEN_LOCAL && a->desplazamiento == 4);
    assert(b->almacen == SIM_ALMACEN_LOCAL && b->desplazamiento == 8);
    assert(c->almacen == SIM_ALMACEN_LOCAL && c->desplazamiento == 4);
    assert(mapa.funciones == 2 && mapa.marco_maximo_bytes == 12);
    assert(mapa.globales.bytes_globales == 4);
    assert(!sim_verificar_pre_memoria(&t, NULL));
    sim_liberar(&t);

    SimTabla pendiente = {0};
    assert(sim_inicializar(&pendiente) == SIM_OK);
    SimSimbolo *pg = add(&pendiente, "g", SIM_CONSTANTE, SIM_TIPO_INT);
    (void)add(&pendiente, "f", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&pendiente, "f") == SIM_OK);
    SimSimbolo *param = add(&pendiente, "x", SIM_PARAMETRO, SIM_TIPO_INT);
    param->es_parametro = 1;
    assert(sim_salir_ambito(&pendiente));
    MemProyectoMapa rechazo = {0};
    assert(mem_asignar_proyecto(&pendiente, 0x4000, 0x10000, 64, &rechazo)
           == MEM_PROYECTO_OK);
    assert(pg->almacen == SIM_ALMACEN_GLOBAL);
    assert(param->almacen == SIM_ALMACEN_PARAMETRO &&
           param->desplazamiento == 4 && param->bytes == 4);
    sim_liberar(&pendiente);

    puts("OK: proyecto global+local, marcos independientes y rollback total");
    return 0;
}
