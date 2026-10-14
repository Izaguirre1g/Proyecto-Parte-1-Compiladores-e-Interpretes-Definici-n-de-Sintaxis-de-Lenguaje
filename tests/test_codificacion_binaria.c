#include "codificacion_binaria.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

static char *leer(const char *ruta, size_t *largo)
{
    FILE *f = fopen(ruta, "rb");
    assert(f);
    assert(fseek(f, 0, SEEK_END) == 0);
    long n = ftell(f);
    rewind(f);
    char *texto = malloc((size_t)n + 1);
    assert(texto && fread(texto, 1, (size_t)n, f) == (size_t)n);
    texto[n] = '\0';
    fclose(f);
    if (largo) *largo = (size_t)n;
    return texto;
}

static char *volcar(const CbPrograma *p, int (*escribir)(const CbPrograma *, FILE *), size_t *largo)
{
    FILE *f = tmpfile();
    assert(f && escribir(p, f) == 0);
    long n = ftell(f);
    rewind(f);
    char *texto = malloc((size_t)n + 1);
    assert(texto && fread(texto, 1, (size_t)n, f) == (size_t)n);
    texto[n] = '\0';
    fclose(f);
    if (largo) *largo = (size_t)n;
    return texto;
}

/* Las 34 instrucciones, pseudo, alias, etiquetas y operandos nombrados deben
 * dar exactamente el .mem y el .bin de tools/assembler.py. */
static void caso_referencia(void)
{
    char *fuente = leer("tests/fixtures/isa_todas.asm", NULL);
    size_t largo_mem, largo_bin, n;
    char *mem = leer("tests/fixtures/isa_todas.mem", &largo_mem);
    char *bin = leer("tests/fixtures/isa_todas.bin", &largo_bin);
    CbPrograma p;
    assert(cb_ensamblar(fuente, &p, stderr) == 0);
    assert(p.cantidad == 43);
    char *nuestro = volcar(&p, cb_escribir_mem, &n);
    assert(n == largo_mem && !memcmp(nuestro, mem, n));
    free(nuestro);
    nuestro = volcar(&p, cb_escribir_bin, &n);
    assert(n == largo_bin && n == 16 * p.cantidad && !memcmp(nuestro, bin, n));
    free(nuestro);
    nuestro = volcar(&p, cb_escribir_listado, &n);
    /* sye ra, inicio en 0x2a0: offset -672 = 0xfd60 en 16 bits. */
    assert(strstr(nuestro, "0042       0x000002a0  nop | nop | sye ra, inicio | nop\n"
                           "           hex         00000000 9601fd60 00000000 00000000\n"
                           "           BRU         1001011 0000 00001 1111110101100000\n"));
    /* Formato registro: tipo id rg rf1 rf2 y 6 bits libres. */
    assert(strstr(nuestro, "           ALU         1101010 1000 00101 00110 00111 000000\n"));
    /* Bundle completo de nop: una sola linea de ensamblador. */
    assert(strstr(nuestro, "0038       0x00000260  nop\n"));
    free(nuestro);
    cb_liberar(&p);
    free(fuente); free(mem); free(bin);
}

/* Los mismos rechazos que tools/assembler.py, sin dejar programa. */
static void caso_errores(void)
{
    static const struct { const char *fuente; const char *mensaje; } casos[] = {
        {"sumai x4, x0, 1\n", "4 slots"},
        {"nop | sumai x4, x0, 1 | nop | nop\n", "corresponde al slot 0, no al 1"},
        {"sumai x4, x0, 1024 | nop | nop | nop\n", "fuera de rango"},
        {"cizqi x4, x0, 2048 | nop | nop | nop\n", "fuera de rango"},
        {"cizqi x4, x0, -1 | nop | nop | nop\n", "fuera de rango"},
        {"suma x32, x0, x1 | nop | nop | nop\n", "registro invalido"},
        {"suma x01, x0, x1 | nop | nop | nop\n", "registro invalido"},
        {"suma t12, x0, x1 | nop | nop | nop\n", "registro invalido"},
        {"sumai x4, x0, 0x | nop | nop | nop\n", "inmediato invalido"},
        {"suma x4, x0 | nop | nop | nop\n", "requiere 3 operandos"},
        {"suma x4, , x1 | nop | nop | nop\n", "operando vacio"},
        {"foo x4 | nop | nop | nop\n", "desconocida"},
        {"csi | nop | nop | nop\n", "no existe"},
        {"a:\na:\nnop\n", "etiqueta repetida"},
        {"nop | nop | sye x0, nada | nop\n", "inmediato invalido"},
        {"nop | nop | igualsi x4, x5, 8 | nop\n", "alineado"},
        {"nop | nop | igualsi x4, x5, 1024 | nop\n", "11 bits"},
        {"nop | nop | sye x0, 32768 | nop\n", "16 bits"},
        {"nop | nop | sye x0, 16 | nop\n", "fuera del programa"},
        {"sumai x4, x0, 1 | cargai x4, x2, 0 | nop | nop\n", "WAW"},
        {"sumai x4, x0, 1 | guardap x2, x4, 0 | nop | nop\n", "RAW entre slots"},
        {"sumai x4, x0, 1 | nop | nop | nop\nnop\nsuma x5, x4, x4 | nop | nop | nop\n",
         "RAW en x4"},
        /* Por la ruta del salto tomado: el destino lee x4 a distancia 1. */
        {"sumai x4, x0, 1 | nop | sye x0, 32 | nop\nnop\nsuma x5, x4, x4 | nop | nop | nop\n",
         "RAW en x4"},
        {"nop | nop | nop | fsl x5, x2, 0, 1\n", "pares de registros"},
        {"nop | nop | nop | ell 0, 1, x4, x6\n", "off 0 o 2"},
        {"nop | nop | nop | fsl rd=6, rd=2, LK=0, RK=1\n", "nombrados"},
        {"# solo comentarios\n\n", "no contiene bundles"},
        {"nop |  | nop | nop\n", "slot vacio"},
    };
    for (size_t i = 0; i < sizeof(casos) / sizeof(*casos); ++i) {
        FILE *diag = tmpfile();
        assert(diag);
        CbPrograma p = {(CbBundle *)1, 7};
        int estado = cb_ensamblar(casos[i].fuente, &p, diag);
        char texto[512] = "";
        rewind(diag);
        size_t n = fread(texto, 1, sizeof(texto) - 1, diag);
        texto[n] = '\0';
        fclose(diag);
        if (estado != 1 || !strstr(texto, casos[i].mensaje) || p.bundles || p.cantidad) {
            fprintf(stderr, "caso %zu: %s-> %s", i, casos[i].fuente, texto);
            assert(!"rechazo esperado");
        }
    }

    /* Mas de 1024 bundles. */
    char *grande = malloc(1025 * 4 + 1);
    assert(grande);
    grande[0] = '\0';
    for (int i = 0; i < 1025; ++i) strcat(grande, "nop\n");
    CbPrograma p;
    assert(cb_ensamblar(grande, &p, NULL) == 1);
    free(grande);
}

/* Una escritura justo antes de syer no afecta al destino (2 bundles anulados). */
static void caso_aceptados(void)
{
    CbPrograma p;
    assert(cb_ensamblar("sumai x13, x0, 0 | nop | nop | nop\nnop\nnop\n"
                        "sumai x4, x0, 1 | nop | syer x1, x13, 0 | nop\n"
                        "inicio: nop\nnop | nop | sye x0, inicio | nop\n", &p, stderr) == 0);
    assert(p.cantidad == 6);
    cb_liberar(&p);
}

int main(void)
{
    caso_referencia();
    caso_errores();
    caso_aceptados();
    puts("OK: codificacion binaria identica a tools/assembler.py y validaciones");
    return 0;
}
