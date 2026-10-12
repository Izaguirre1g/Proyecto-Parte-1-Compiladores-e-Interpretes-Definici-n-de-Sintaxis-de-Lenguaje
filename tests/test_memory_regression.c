/* 3A-F: regresion de limites del ABI y aislamiento de marcos. */
#include "memory.h"
#include <assert.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>

static SimSimbolo *add(SimTabla *t, const char *n, SimClase c, SimTipo ty)
{
    SimSimbolo *s = NULL;
    assert(sim_insertar(t, n, c, ty, 1, 1, &s) == SIM_OK);
    return s;
}

static void abi_limites(void)
{
    MemAbiArgumento a = {7, 19, 123};
    assert(mem_abi_argumento(4, 28, &a) == MEM_ABI_OK);
    assert(a.en_registro == 0 && a.registro == 0 && a.offset_entrada == 28);
    assert(mem_abi_argumento(5, 28, &a) == MEM_ABI_OK);
    assert(a.offset_entrada == 32);
    for (size_t i = 0; i < 4; ++i) {
        assert(mem_abi_argumento(i, 28, &a) == MEM_ABI_OK);
        assert(a.en_registro && a.registro == 8 + i);
        assert(a.offset_entrada == 0);
    }
    a = (MemAbiArgumento){7, 19, 123};
    assert(mem_abi_argumento(4, 0, &a) == MEM_ABI_ARGUMENTO_INVALIDO);
    assert(a.registro == 19 && a.offset_entrada == 123);
    assert(mem_abi_argumento(4, 6, &a) == MEM_ABI_ARGUMENTO_INVALIDO);
    assert(a.registro == 19 && a.offset_entrada == 123);
    assert(mem_abi_argumento(SIZE_MAX, 28, &a) == MEM_ABI_DESBORDAMIENTO);
    assert(a.registro == 19 && a.offset_entrada == 123);
    assert(mem_abi_argumento(4, 28, NULL) == MEM_ABI_ARGUMENTO_INVALIDO);
}

/* Modelo de direcciones: espaciado del llamador debajo de su sp actual.
 * El nuevo sp del llamado se ubica debajo de dichos argumentos. */
static void no_solapamiento(void)
{
    const size_t marco_llamado = 28;
    const size_t extras = 3, bytes_extras = extras * 4;
    const size_t sp_llamador = 4096;
    const size_t sp_llamado = sp_llamador - bytes_extras - marco_llamado;
    assert(sp_llamado + marco_llamado == sp_llamador - bytes_extras);
    for (size_t i = 4; i < 4 + extras; ++i) {
        MemAbiArgumento arg = {0};
        assert(mem_abi_argumento(i, marco_llamado, &arg) == MEM_ABI_OK);
        size_t direccion = sp_llamado + arg.offset_entrada;
        assert(direccion >= sp_llamador - bytes_extras);
        assert(direccion + 4 <= sp_llamador);
        assert(direccion >= sp_llamado + marco_llamado);
    }
}

static void proyecto_rechazo_tardio(void)
{
    SimTabla t = {0};
    assert(sim_inicializar(&t) == SIM_OK);
    SimSimbolo *global = add(&t, "g", SIM_CONSTANTE, SIM_TIPO_INT);
    add(&t, "f", SIM_FUNCION, SIM_TIPO_VOID);
    add(&t, "h", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&t, "f") == SIM_OK);
    SimSimbolo *p = add(&t, "p", SIM_PARAMETRO, SIM_TIPO_INT);
    p->es_parametro = 1;
    SimSimbolo *local = add(&t, "local", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    assert(sim_entrar_ambito(&t, "h") == SIM_OK);
    SimSimbolo *invalido = add(&t, "texto", SIM_PARAMETRO, SIM_TIPO_TEXT);
    invalido->es_parametro = 1;
    assert(sim_salir_ambito(&t));
    MemProyectoMapa mapa = {0};
    mapa.funciones = 777;
    assert(mem_asignar_proyecto(&t, 4096, 8192, 128, &mapa)
           == MEM_PROYECTO_PARAMETROS_PENDIENTES);
    assert(mapa.funciones == 777);
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(global->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    assert(p->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    assert(local->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    assert(invalido->almacen == SIM_ALMACEN_SIN_ASIGNAR);
    sim_liberar(&t);
}

static void capacidad_exacta(void)
{
    SimTabla t = {0};
    assert(sim_inicializar(&t) == SIM_OK);
    add(&t, "f", SIM_FUNCION, SIM_TIPO_VOID);
    assert(sim_entrar_ambito(&t, "f") == SIM_OK);
    SimSimbolo *p = add(&t, "a", SIM_PARAMETRO, SIM_TIPO_CHAR);
    p->es_parametro = 1;
    SimSimbolo *v = add(&t, "b", SIM_VARIABLE, SIM_TIPO_INT);
    assert(sim_salir_ambito(&t));
    MemMarco marco = {0};
    assert(mem_asignar_marco(&t, "f", 11, &marco) == MEM_MARCO_SIN_ESPACIO);
    assert(sim_verificar_pre_memoria(&t, NULL));
    assert(mem_asignar_marco(&t, "f", 12, &marco) == MEM_MARCO_OK);
    assert(marco.bytes_marco == 12 && marco.offset_ra == 0);
    assert(p->desplazamiento == 4 && p->bytes == 4);
    assert(v->desplazamiento == 8 && v->bytes == 4);
    assert(!sim_verificar_pre_memoria(&t, NULL));
    sim_liberar(&t);
}

int main(void)
{
    abi_limites();
    no_solapamiento();
    proyecto_rechazo_tardio();
    capacidad_exacta();
    puts("OK: 3A-F limites ABI, pila sin solapamiento, rollback y capacidad");
    return 0;
}
