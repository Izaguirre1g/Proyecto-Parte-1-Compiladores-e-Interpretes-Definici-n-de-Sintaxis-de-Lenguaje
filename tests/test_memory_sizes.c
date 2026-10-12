#include "memory.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    size_t n = 99;
    SimSimbolo s = {0};
    assert(mem_tamano_escalar(SIM_TIPO_INT, &n) == MEM_TAM_OK && n == 4);
    assert(mem_tamano_escalar(SIM_TIPO_BOOLEAN, &n) == MEM_TAM_OK && n == 1);
    assert(mem_tamano_escalar(SIM_TIPO_CHAR, &n) == MEM_TAM_OK && n == 1);
    assert(mem_tamano_escalar(SIM_TIPO_TEXT, &n) == MEM_TAM_INDEFINIDO && n == 0);
    assert(mem_tamano_escalar(SIM_TIPO_INT, NULL) == MEM_TAM_ARGUMENTO_INVALIDO);

    s.clase = SIM_VARIABLE;
    s.tipo = SIM_TIPO_INT;
    assert(mem_tamano_simbolo(&s, &n) == MEM_TAM_OK && n == 4);
    size_t dims[] = {2, 3};
    s.dimensiones = 2;
    s.longitudes = dims;
    assert(mem_tamano_simbolo(&s, &n) == MEM_TAM_OK && n == 24);
    assert(s.bytes == 0 && s.desplazamiento == 0 &&
           s.almacen == SIM_ALMACEN_SIN_ASIGNAR);
    dims[1] = 0;
    assert(mem_tamano_simbolo(&s, &n) == MEM_TAM_INDEFINIDO && n == 0);
    dims[1] = SIZE_MAX;
    assert(mem_tamano_simbolo(&s, &n) == MEM_TAM_DESBORDAMIENTO && n == 0);
    s.es_parametro = 1;
    assert(mem_tamano_simbolo(&s, &n) == MEM_TAM_INDEFINIDO);
    s.es_parametro = 0;
    s.clase = SIM_LISTA;
    assert(mem_tamano_simbolo(&s, &n) == MEM_TAM_INDEFINIDO);
    puts("OK: tamanos basicos, matrices, dinamicos y overflow (sin offsets)");
    return 0;
}
