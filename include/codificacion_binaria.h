#ifndef CODIFICACION_BINARIA_H
#define CODIFICACION_BINARIA_H

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

/* Fase 6: ensamblador VLIW en texto -> palabras de 32 bits por slot.
 *
 * Acepta la sintaxis del contrato de Arquitectura (docs/arquitectura/assembler.md):
 * una linea por bundle "ALU | LSU | BRU | CRIPTO" o "nop"; comentarios con
 * #, ; o //; etiquetas "nombre:" al inicio de una linea (solas o antes del
 * bundle); registros x0-x31 y alias ABI (cero, ra, sp, gp, t0-t11, s0-s15);
 * inmediatos decimales, 0x y 0b; pseudoinstrucciones mov, not, ret y nop.
 * Los saltos aceptan etiqueta o desplazamiento en bytes desde el PC del bundle.
 *
 * Valida, igual que tools/assembler.py: slot correcto para cada instruccion,
 * rangos de campos, destino de salto dentro del programa y alineado,
 * conflictos WAW/RAW entre slots de un bundle y la latencia del pipeline
 * (un valor escrito en el bundle N se lee desde N+3) por ambas rutas de cada
 * salto. La IMEM admite 1024 bundles. */
typedef struct {
    uint32_t slots[4];   /* 0 ALU, 1 LSU, 2 BRU, 3 CRIPTO */
    char texto[4][64];   /* Texto de cada slot, para el listado (-p). */
    unsigned linea;      /* Linea del fuente. */
} CbBundle;

typedef struct {
    CbBundle *bundles;
    size_t cantidad;
} CbPrograma;

/* Devuelve 0 en exito; 1 con diagnostico "linea N: ..." si hay errores. */
int cb_ensamblar(const char *fuente, CbPrograma *programa, FILE *diagnosticos);
void cb_liberar(CbPrograma *programa);

/* .mem: 32 digitos hexadecimales por bundle (CRIPTO en los bits altos),
 * para $readmemh de la IMEM. */
int cb_escribir_mem(const CbPrograma *programa, FILE *salida);
/* .bin: 16 bytes little-endian por bundle (ALU primero). Sin encabezado:
 * es la imagen que se carga en la IMEM desde la direccion 0. */
int cb_escribir_bin(const CbPrograma *programa, FILE *salida);
/* Listado de paquetes (-p): por bundle, PC y ensamblador; palabra en
 * hexadecimal y el binario de cada slot separado por campos. */
int cb_escribir_listado(const CbPrograma *programa, FILE *salida);

#endif
