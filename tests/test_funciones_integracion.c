#include "funciones_marco.h"
#include "funciones_llamada.h"
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Modelo SECUENCIAL del subconjunto usado por 3B-A/B. No emula bundles VLIW. */
enum { PALABRAS = 16384, SP_INICIAL = 32768, MAX_PROFUNDIDAD = 16 };
typedef struct Sim Sim;
struct Sim {
    uint32_t x[32];
    uint32_t memoria[PALABRAS];
    unsigned profundidad, profundidad_maxima, llamadas, retornos;
    uint32_t siguiente_enlace, enlaces[MAX_PROFUNDIDAD];
};

static char *emitir_marco(size_t bytes, size_t parametros, int prologo)
{
    FILE *f = tmpfile();
    assert(f);
    if (prologo) assert(fm_emitir_prologo(f, bytes, parametros) == FM_OK);
    else assert(fm_emitir_epilogo(f, bytes) == FM_OK);
    assert(fseek(f, 0, SEEK_END) == 0);
    long n = ftell(f);
    assert(n >= 0);
    char *texto = malloc((size_t)n + 1);
    assert(texto);
    rewind(f);
    assert(fread(texto, 1, (size_t)n, f) == (size_t)n);
    texto[n] = '\0';
    fclose(f);
    return texto;
}

static char *emitir_llamada(const unsigned *registros, size_t cuantos)
{
    FILE *f = tmpfile();
    assert(f);
    assert(fl_emitir_llamada(f, registros, cuantos) == FL_OK);
    assert(fseek(f, 0, SEEK_END) == 0);
    long n = ftell(f);
    assert(n >= 0);
    char *texto = malloc((size_t)n + 1);
    assert(texto);
    rewind(f);
    assert(fread(texto, 1, (size_t)n, f) == (size_t)n);
    texto[n] = '\0';
    fclose(f);
    return texto;
}

static uint32_t *celda(Sim *s, uint32_t direccion)
{
    assert(direccion % 4 == 0 && direccion / 4 < PALABRAS);
    return &s->memoria[direccion / 4];
}

static void ejecutar(Sim *s, const char *texto);

/* La logica de estas funciones es un arnes C; SOLO los movimientos,
 * cambios de pila, llamadas y retornos se ejecutan desde el ensamblador. */
static void funcion(Sim *s, uint32_t destino)
{
    assert(s->profundidad < MAX_PROFUNDIDAD);
    unsigned nivel = s->profundidad++;
    s->enlaces[nivel] = s->x[1];
    if (s->profundidad_maxima < s->profundidad)
        s->profundidad_maxima = s->profundidad;
    assert(destino == 0x1000 || destino == 0x2000);
    /* Cinco parametros => ra + 20 bytes de home slots. Una variable local. */
    char *prologo = emitir_marco(28, 5, 1);
    ejecutar(s, prologo);
    free(prologo);
    const uint32_t sp_marco = s->x[2];
    uint32_t argumentos[5];
    for (unsigned i = 0; i < 5; ++i)
        argumentos[i] = *celda(s, sp_marco + 4 * (i + 1));
    assert(argumentos[1] == 101 && argumentos[2] == 102 &&
           argumentos[3] == 103 && argumentos[4] == 104);
    *celda(s, sp_marco + 24) = argumentos[0];
    if (argumentos[0] != 0) {
        /* Solo el primer argumento decrece. Los otros permanecen iguales.
         * x13 se prepara inmediatamente antes de cada salto. */
        s->x[4] = argumentos[0] - 1;
        s->x[5] = argumentos[1];
        s->x[6] = argumentos[2];
        s->x[7] = argumentos[3];
        s->x[14] = argumentos[4];
        s->x[13] = destino == 0x1000 ? 0x2000 : 0x1000;
        const unsigned fuentes[] = {4, 5, 6, 7, 14};
        char *llamada = emitir_llamada(fuentes, 5);
        ejecutar(s, llamada);
        free(llamada);
        assert(s->x[2] == sp_marco);
        /* Se lee de pila, no de caller-saved; el hijo pudo sobrescribirlos. */
        s->x[8] += *celda(s, sp_marco + 24);
    } else {
        s->x[8] = 1;
    }
    char *epilogo = emitir_marco(28, 0, 0);
    ejecutar(s, epilogo);
    free(epilogo);
    assert(s->profundidad == nivel + 1);
    --s->profundidad;
}

static void ejecutar(Sim *s, const char *texto)
{
    const char *p = texto;
    while (*p) {
        char linea[128];
        const char *fin = strchr(p, '\n');
        assert(fin);
        size_t n = (size_t)(fin - p);
        assert(n < sizeof(linea));
        memcpy(linea, p, n);
        linea[n] = '\0';
        p = fin + 1;
        char op[20];
        unsigned a, b, inm;
        assert(sscanf(linea, "%19s x%u, x%u, %u", op, &a, &b, &inm) == 4);
        assert(a < 32 && b < 32);
        if (strcmp(op, "restai") == 0) {
            assert(a == 2 && b == 2 && inm <= 1020);
            s->x[a] = s->x[b] - inm;
        } else if (strcmp(op, "sumai") == 0) {
            assert(inm <= 1020);
            s->x[a] = s->x[b] + inm;
        } else if (strcmp(op, "guardap") == 0) {
            assert(a == 2 && inm <= 1023);
            *celda(s, s->x[a] + inm) = s->x[b];
        } else if (strcmp(op, "cargai") == 0) {
            assert(inm <= 1023);
            s->x[a] = *celda(s, s->x[b] + inm);
        } else if (strcmp(op, "syer") == 0) {
            assert(inm == 0);
            if (a == 1 && b == 13) {
                uint32_t destino = s->x[13];
                s->x[1] = ++s->siguiente_enlace;
                ++s->llamadas;
                funcion(s, destino);
            } else {
                assert(a == 0 && b == 1);
                assert(s->profundidad > 0);
                assert(s->x[1] == s->enlaces[s->profundidad - 1]);
                ++s->retornos;
            }
        } else assert(!"instruccion inesperada");
        s->x[0] = 0;
    }
}

int main(void)
{
    Sim *s = calloc(1, sizeof(*s));
    assert(s);
    s->x[2] = SP_INICIAL;
    s->x[1] = 0xfeed;
    /* Tres niveles recursivos mediante destinos alternantes A -> B -> A. */
    s->x[4] = 3;
    s->x[5] = 101;
    s->x[6] = 102;
    s->x[7] = 103;
    s->x[14] = 104;
    s->x[13] = 0x1000;
    const unsigned fuentes[] = {4, 5, 6, 7, 14};
    char *llamada = emitir_llamada(fuentes, 5);
    ejecutar(s, llamada);
    free(llamada);
    assert(s->x[8] == 7); /* 1 + 1 + 2 + 3 */
    assert(s->x[2] == SP_INICIAL);
    /* x1 es caller-saved: la llamada externa puede modificarlo. */
    assert(s->x[1] != 0xfeed);
    assert(s->llamadas == 4 && s->retornos == 4);
    assert(s->profundidad == 0 && s->profundidad_maxima == 4);
    free(s);
    puts("OK: 3B-C pila, quinto argumento, retornos y anidamiento recursivo (modelo secuencial)");
    return EXIT_SUCCESS;
}
