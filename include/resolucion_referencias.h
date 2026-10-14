#ifndef RESOLUCION_REFERENCIAS_H
#define RESOLUCION_REFERENCIAS_H

#include <stddef.h>
#include <stdint.h>
#include "generacion_funciones.h"

/* Direcciones de PC de BUNDLE, asignadas externamente por quien haga el layout.
 * Esta API NO infiere direcciones de lineas de ensamblador ni empaqueta VLIW.
 * El llamador mantiene la tabla hasta terminar de generar las funciones. */
typedef enum {
    RR_OK = 0,
    RR_INVALIDO,
    RR_DUPLICADO,
    RR_NO_ENCONTRADO,
    RR_DESALINEADO,
    RR_FUERA_DE_RANGO,
    RR_SIN_MEMORIA
} RrEstado;

typedef enum { RR_FUNCION, RR_ETIQUETA } RrClase;
typedef enum { RR_SYE, RR_CONDICIONAL } RrSalto;

typedef struct RrEntrada RrEntrada;
typedef struct {
    RrEntrada *entradas;
    size_t cantidad;
    size_t capacidad;
} RrTabla;

void rr_inicializar(RrTabla *tabla);
void rr_liberar(RrTabla *tabla);
RrEstado rr_definir(RrTabla *tabla, const char *nombre, RrClase clase,
                    uint32_t direccion);
RrEstado rr_buscar(const RrTabla *tabla, const char *nombre, RrClase clase,
                   uint32_t *direccion);
/* PC = direccion del bundle QUE CONTIENE el salto. Se devuelve offset en bytes,
 * NO diferencia respecto a PC+16. SYE: [-32768,32752] multiplos de 16.
 * Condicional: [-1024,1008] multiplos de 16. */
RrEstado rr_desplazamiento(const RrTabla *tabla, const char *nombre,
                          RrClase clase, uint32_t pc, RrSalto tipo,
                          int32_t *desplazamiento);
/* Puente compatible con GfResolverDestino. Retorna 1 solo si la funcion existe. */
int rr_resolver_funcion(void *contexto, const AstNodo *id, unsigned *direccion);
const char *rr_nombre_estado(RrEstado estado);

#endif
