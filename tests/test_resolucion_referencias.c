#include "resolucion_referencias.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    RrTabla t;
    rr_inicializar(&t);
    assert(rr_definir(&t, "inicio", RR_FUNCION, 0x1000) == RR_OK);
    assert(rr_definir(&t, "aux", RR_FUNCION, 0x1010) == RR_OK);
    assert(rr_definir(&t, "loop", RR_ETIQUETA, 0x1020) == RR_OK);
    assert(rr_definir(&t, "inicio", RR_FUNCION, 0x2000) == RR_DUPLICADO);
    assert(rr_definir(&t, "mal", RR_FUNCION, 0x1004) == RR_DESALINEADO);
    assert(rr_definir(&t, "fin", RR_FUNCION, 65536) == RR_DESALINEADO);
    assert(t.cantidad == 3);

    uint32_t dir = 5;
    assert(rr_buscar(&t, "aux", RR_FUNCION, &dir) == RR_OK && dir == 0x1010);
    assert(rr_buscar(&t, "loop", RR_FUNCION, &dir) == RR_NO_ENCONTRADO);
    assert(dir == 0x1010);

    int32_t desplazamiento = 99;
    assert(rr_desplazamiento(&t, "loop", RR_ETIQUETA, 0x1000,
                            RR_CONDICIONAL, &desplazamiento) == RR_OK);
    assert(desplazamiento == 32); /* PC del propio bundle, no PC+16 */
    assert(rr_desplazamiento(&t, "inicio", RR_FUNCION, 0x1010,
                            RR_SYE, &desplazamiento) == RR_OK);
    assert(desplazamiento == -16);
    assert(rr_definir(&t, "near_fwd", RR_ETIQUETA, 1008) == RR_OK);
    assert(rr_desplazamiento(&t, "near_fwd", RR_ETIQUETA, 0,
                            RR_CONDICIONAL, &desplazamiento) == RR_OK);
    assert(desplazamiento == 1008);
    assert(rr_definir(&t, "too_far", RR_ETIQUETA, 1024) == RR_OK);
    assert(rr_desplazamiento(&t, "too_far", RR_ETIQUETA, 0,
                            RR_CONDICIONAL, &desplazamiento) == RR_FUERA_DE_RANGO);
    assert(desplazamiento == 1008);
    assert(rr_definir(&t, "low", RR_ETIQUETA, 0) == RR_OK);
    assert(rr_desplazamiento(&t, "low", RR_ETIQUETA, 1024,
                            RR_CONDICIONAL, &desplazamiento) == RR_OK);
    assert(desplazamiento == -1024);
    assert(rr_desplazamiento(&t, "low", RR_ETIQUETA, 1040,
                            RR_CONDICIONAL, &desplazamiento) == RR_FUERA_DE_RANGO);
    assert(rr_definir(&t, "sye_edge", RR_ETIQUETA, 32752) == RR_OK);
    assert(rr_desplazamiento(&t, "sye_edge", RR_ETIQUETA, 0,
                            RR_SYE, &desplazamiento) == RR_OK);
    assert(desplazamiento == 32752);
    assert(rr_definir(&t, "sye_far", RR_ETIQUETA, 32768) == RR_OK);
    assert(rr_desplazamiento(&t, "sye_far", RR_ETIQUETA, 0,
                            RR_SYE, &desplazamiento) == RR_FUERA_DE_RANGO);
    assert(rr_desplazamiento(&t, "low", RR_ETIQUETA, 32768,
                            RR_SYE, &desplazamiento) == RR_OK);
    assert(desplazamiento == -32768);
    assert(rr_desplazamiento(&t, "low", RR_ETIQUETA, 32784,
                            RR_SYE, &desplazamiento) == RR_FUERA_DE_RANGO);

    AstNodo id = {0};
    id.tipo = AST_IDENTIFICADOR;
    id.texto = "aux";
    unsigned d = 0;
    assert(rr_resolver_funcion(&t, &id, &d) && d == 0x1010);
    id.texto = "loop";
    assert(!rr_resolver_funcion(&t, &id, &d));
    id.texto = "fantasma";
    assert(!rr_resolver_funcion(&t, &id, &d));
    assert(strcmp(rr_nombre_estado(RR_FUERA_DE_RANGO), "salto fuera de rango") == 0);
    rr_liberar(&t);
    assert(t.cantidad == 0 && t.entradas == NULL);
    puts("OK: 3B-D3D etiquetas, referencias, sye y condicionales (PC de bundle)");
    return 0;
}
