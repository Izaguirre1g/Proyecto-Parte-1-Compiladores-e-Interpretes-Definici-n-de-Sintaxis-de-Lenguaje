#include "memory.h"
#include <assert.h>
#include <stdio.h>

static SimSimbolo *add(SimTabla *t, const char *name, SimClase clase, SimTipo tipo)
{
    SimSimbolo *s = NULL;
    assert(sim_insertar(t, name, clase, tipo, 1, 1, &s) == SIM_OK);
    return s;
}

int main(void)
{
    MemAbiArgumento a = {0};
    for (size_t i = 0; i < 4; ++i) {
        assert(mem_abi_argumento(i, 32, &a) == MEM_ABI_OK);
        assert(a.en_registro && a.registro == i + 8);
    }
    assert(mem_abi_argumento(4, 32, &a) == MEM_ABI_OK);
    assert(!a.en_registro && a.offset_entrada == 32);
    assert(mem_abi_argumento(5, 32, &a) == MEM_ABI_OK);
    assert(!a.en_registro && a.offset_entrada == 36);
    assert(mem_abi_argumento((size_t)-1, 32, &a) == MEM_ABI_DESBORDAMIENTO);
    assert(mem_abi_argumento(0, 3, &a) == MEM_ABI_ARGUMENTO_INVALIDO);

    SimTabla t = {0};
    assert(sim_inicializar(&t) == SIM_OK);
    SimSimbolo *g = add(&t, "g", SIM_CONSTANTE, SIM_TIPO_INT);
    add(&t, "f", SIM_FUNCION, SIM_TIPO_VOID);
    add(&t, "main", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&t, "f") == SIM_OK);
    SimSimbolo *p[5];
    for (int i = 0; i < 5; ++i) {
        char name[3] = {'p', (char)('0'+i), 0};
        p[i] = add(&t, name, SIM_PARAMETRO,
                   i == 1 ? SIM_TIPO_BOOLEAN : i == 2 ? SIM_TIPO_CHAR : SIM_TIPO_INT);
        p[i]->es_parametro = 1;
    }
    SimSimbolo *local = add(&t, "local", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    assert(sim_entrar_ambito(&t, "main") == SIM_OK);
    SimSimbolo *mainvar = add(&t, "v", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    MemProyectoMapa mapa = {0};
    assert(mem_asignar_proyecto(&t, 0x4000, 0x10000, 24, &mapa) == MEM_PROYECTO_SIN_ESPACIO);
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(mem_asignar_proyecto(&t, 0x4000, 0x10000, 32, &mapa) == MEM_PROYECTO_OK);
    for (int i = 0; i < 5; ++i) {
        assert(p[i]->almacen == SIM_ALMACEN_PARAMETRO);
        assert(p[i]->desplazamiento == 4 + 4 * i && p[i]->bytes == 4);
    }
    assert(local->desplazamiento == 24 && local->almacen == SIM_ALMACEN_LOCAL);
    assert(mainvar->desplazamiento == 4 && mapa.marco_maximo_bytes == 28);
    assert(g->almacen == SIM_ALMACEN_GLOBAL && g->desplazamiento == 0);
    sim_liberar(&t);

    SimTabla r = {0};
    assert(sim_inicializar(&r) == SIM_OK);
    SimSimbolo *rg = add(&r, "g", SIM_CONSTANTE, SIM_TIPO_INT);
    add(&r, "f", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&r, "f") == SIM_OK);
    SimSimbolo *text = add(&r, "texto", SIM_PARAMETRO, SIM_TIPO_TEXT);
    text->es_parametro = 1;
    assert(sim_salir_ambito(&r));
    assert(mem_asignar_proyecto(&r, 0x4000, 0x10000, 64, &mapa)
           == MEM_PROYECTO_PARAMETROS_PENDIENTES);
    assert(rg->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    assert(sim_verificar_pre_memoria(&r, NULL));
    sim_liberar(&r);
    puts("OK: ABI escalar, 5 argumentos, home slots, locales y rollback");
    return 0;
}
