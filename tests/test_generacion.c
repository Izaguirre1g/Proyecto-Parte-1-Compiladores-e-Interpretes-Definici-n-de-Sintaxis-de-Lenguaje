/* Comprueba el ensamblador con un modelo secuencial, no con el hardware. */
#include "generacion_basica.h"
#include "sintactico.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static void exigir(int condicion, const char *mensaje) {
    if (!condicion) {
        fprintf(stderr, "FALLO generación: %s\n", mensaje);
        exit(EXIT_FAILURE);
    }
}

typedef struct {
    GbTipo operandos, resultado;
    unsigned base;
    int offset, escribible;
} Entorno;

/* Referencias de prueba. No reemplazan la tabla de simbolos del compilador. */
static int resolver(void *datos, const AstNodo *n, GbReferencia *ref) {
    Entorno *e = datos;
    int indice;
    if (!strcmp(n->texto, "a")) indice = 0;
    else if (!strcmp(n->texto, "b")) indice = 1;
    else if (!strcmp(n->texto, "r")) indice = 2;
    else return 0;
    *ref = (GbReferencia){indice == 2 ? e->resultado : e->operandos,
                          e->base, e->offset + indice * 4, e->escribible};
    return 1;
}

static const AstNodo *sentencia(Ast *ast, const char *texto) {
    char fuente[8192];
    int n = snprintf(fuente, sizeof(fuente),
        "create_funk #declare_infinite_void# main() { %s }", texto);
    exigir(n > 0 && (size_t)n < sizeof(fuente), "fuente demasiado grande");
    exigir(construir_ast(fuente, (size_t)n, "prueba.bal", stderr, ast) == 0,
           "el parser debe aceptar la prueba");
    const AstNodo *h = ast->raiz->primer_hijo->primer_hijo;
    for (int i = 0; i < 3; ++i) h = h->siguiente;
    return h->primer_hijo;
}

static int64_t con_signo(uint32_t n) {
    return n <= INT32_MAX ? (int64_t)n : (int64_t)n - INT64_C(4294967296);
}

static void ejecutar(FILE *asmfile, uint32_t registros[32], uint32_t memoria[512]) {
    rewind(asmfile);
    char linea[128], op[16];
    unsigned d, a, b;
    int inmediato;
    while (fgets(linea, sizeof(linea), asmfile)) {
        uint32_t resultado = 0;
        if (sscanf(linea, "%15s x%u, x%u, x%u", op, &d, &a, &b) == 4) {
            exigir(d >= 4 && d <= 7 && a < 32 && b < 32, "registro R");
            uint32_t x = registros[a], y = registros[b];
            if (!strcmp(op, "suma")) resultado = x + y;
            else if (!strcmp(op, "resta")) resultado = x - y;
            else if (!strcmp(op, "and")) resultado = x & y;
            else if (!strcmp(op, "or")) resultado = x | y;
            else if (!strcmp(op, "xor")) resultado = x ^ y;
            else if (!strcmp(op, "mrq")) resultado = con_signo(x) < con_signo(y);
            else if (!strcmp(op, "myq")) resultado = con_signo(x) > con_signo(y);
            else exigir(0, "instrucción R desconocida");
        } else {
            exigir(sscanf(linea, "%15s x%u, x%u, %d", op, &d, &a, &inmediato) == 4,
                   "formato de instrucción");
            exigir(d < 32 && a < 32 && inmediato >= -1024 && inmediato <= 1023,
                   "registro o inmediato fuera de rango");
            if (!strcmp(op, "guardap") || !strcmp(op, "cargai")) {
                unsigned base = !strcmp(op, "guardap") ? d : a;
                int64_t dir = (int64_t)registros[base] + inmediato;
                exigir(dir >= 0 && dir < 2048 && dir % 4 == 0, "dirección");
                if (!strcmp(op, "guardap")) {
                    memoria[dir / 4] = registros[a];
                    continue;
                }
                resultado = memoria[dir / 4];
            } else if (!strcmp(op, "sumai")) resultado = registros[a] + (uint32_t)inmediato;
            else if (!strcmp(op, "cizqi")) resultado = registros[a] << (inmediato & 31);
            else if (!strcmp(op, "ori")) resultado = registros[a] | (uint32_t)inmediato;
            else if (!strcmp(op, "xori")) resultado = registros[a] ^ (uint32_t)inmediato;
            else exigir(0, "instrucción inmediata desconocida");
            exigir(d >= 4 && d <= 7, "destino fuera de los temporales");
        }
        registros[d] = resultado;
    }
    exigir(!ferror(asmfile), "lectura de ensamblador");
}

typedef struct {
    const char *texto;
    uint32_t a, b, esperado;
    GbTipo operandos, resultado;
} Caso;

static const Caso casos[] = {
    {"r : a gauss b;", 8, 3, 11, GB_ENTERO, GB_ENTERO},
    {"r : a gauss (b gauss (a gauss b));", 8, 3, 22, GB_ENTERO, GB_ENTERO},
    {"r : a neumann b;", 3, 8, UINT32_C(4294967291), GB_ENTERO, GB_ENTERO},
    {"r : a gauss b;", INT32_MAX, 1, UINT32_C(2147483648), GB_ENTERO, GB_ENTERO},
    {"r : a neumann b;", UINT32_C(2147483648), 1, INT32_MAX, GB_ENTERO, GB_ENTERO},
    {"r : neumann a;", 5, 0, UINT32_C(4294967291), GB_ENTERO, GB_ENTERO},
    {"r : (a gauss b) neumann (a neumann b);", 8, 3, 6, GB_ENTERO, GB_ENTERO},
    {"r : a < b;", UINT32_C(2147483648), 0, 1, GB_ENTERO, GB_BOOLEANO},
    {"r : a > b;", 3, 8, 0, GB_ENTERO, GB_BOOLEANO},
    {"r : a <= b;", 8, 8, 1, GB_ENTERO, GB_BOOLEANO},
    {"r : a >= b;", 7, 8, 0, GB_ENTERO, GB_BOOLEANO},
    {"r : a == b;", UINT32_C(2147483648), 0, 0, GB_ENTERO, GB_BOOLEANO},
    {"r : a =/= b;", UINT32_C(2147483648), 0, 1, GB_ENTERO, GB_BOOLEANO},
    {"r : a == b;", UINT32_C(2147483648), UINT32_C(2147483648), 1, GB_ENTERO, GB_BOOLEANO},
    {"r : a =/= b;", 8, 8, 0, GB_ENTERO, GB_BOOLEANO},
    {"r : a && b;", 1, 0, 0, GB_BOOLEANO, GB_BOOLEANO},
    {"r : a || b;", 0, 1, 1, GB_BOOLEANO, GB_BOOLEANO},
    {"r : a ^ b;", 1, 1, 0, GB_BOOLEANO, GB_BOOLEANO},
    {"r : ~a;", 0, 0, 1, GB_BOOLEANO, GB_BOOLEANO},
    {"r : ~a;", 1, 0, 0, GB_BOOLEANO, GB_BOOLEANO},
    {"r : a == b;", 1, 1, 1, GB_BOOLEANO, GB_BOOLEANO},
    {"r : declare_true && ~declare_false;", 0, 0, 1, GB_BOOLEANO, GB_BOOLEANO},
    {"r : 0;", 0, 0, 0, GB_ENTERO, GB_ENTERO},
    {"r : 1023;", 0, 0, 1023, GB_ENTERO, GB_ENTERO},
    {"r : 1024;", 0, 0, 1024, GB_ENTERO, GB_ENTERO},
    {"r : 2147483647;", 0, 0, INT32_MAX, GB_ENTERO, GB_ENTERO},
    {"r : neumann 2147483648;", 0, 0, UINT32_C(2147483648), GB_ENTERO, GB_ENTERO},
    {"r * declare_int : a gauss b;", 8, 3, 11, GB_ENTERO, GB_ENTERO},
    {"r * declare_boolean : a < b;", 3, 8, 1, GB_ENTERO, GB_BOOLEANO}
};

static void positivos(void) {
    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); ++i) {
        const Caso *c = &casos[i];
        Ast ast = {0};
        const AstNodo *n = sentencia(&ast, c->texto);
        FILE *salida = tmpfile();
        exigir(salida != NULL, "tmpfile");
        Entorno e = {c->operandos, c->resultado, 3, 0, 1};
        exigir(gb_generar(n, resolver, &e, salida, stderr) == 0, c->texto);
        uint32_t registros[32] = {0}, memoria[512] = {0};
        registros[3] = 256;
        memoria[64] = c->a; memoria[65] = c->b;
        ejecutar(salida, registros, memoria);
        exigir(memoria[66] == c->esperado, c->texto);
        exigir(memoria[64] == c->a && memoria[65] == c->b, "no alterar operandos");
        fclose(salida);
        ast_liberar(&ast);
    }
    printf("OK: %zu casos de traducción y resultado\n", sizeof(casos) / sizeof(casos[0]));
}

static void rechazar(const char *texto, Entorno e, const char *mensaje) {
    Ast ast = {0};
    const AstNodo *n = sentencia(&ast, texto);
    FILE *salida = tmpfile(), *errores = tmpfile();
    exigir(salida && errores, "tmpfile");
    fputs("previo\n", salida);
    exigir(gb_generar(n, resolver, &e, salida, errores) == 1, texto);
    exigir(ftell(salida) == 7, "no emitir instrucciones parciales");
    rewind(errores);
    char diagnostico[512];
    exigir(fgets(diagnostico, sizeof(diagnostico), errores) != NULL, "diagnóstico ausente");
    exigir(strstr(diagnostico, mensaje) != NULL, texto);
    fclose(salida); fclose(errores); ast_liberar(&ast);
}

static void negativos(void) {
    Entorno e = {GB_ENTERO, GB_ENTERO, 3, 0, 1};
    const char *no_soportados[] = {"r : a pitagoras b;", "r : a euclides b;",
        "r : a euler b;", "r : a descartes b;"};
    for (size_t i = 0; i < 4; ++i) rechazar(no_soportados[i], e, "sin traducción acordada");
    rechazar("r : f();", e, "fuera del subconjunto");
    rechazar("r : a[0];", e, "fuera del subconjunto");
    rechazar("r : \"hola\";", e, "fuera del subconjunto");
    rechazar("r : ausente;", e, "sin referencia semántica");
    rechazar("r : 2147483648;", e, "fuera del rango");
    rechazar("r : 99999999999999999999999;", e, "fuera del rango");
    rechazar("r : neumann 2147483649;", e, "fuera del rango");
    rechazar("r : declare_true;", e, "asignación incompatible");
    rechazar("r : a && b;", e, "tipos incompatibles");
    rechazar("r : ~a;", e, "unario o tipo");
    rechazar("r * declare_int;", e, "declaración inicializada");
    rechazar("r * declare_boolean : declare_true;", e, "tipo declarado distinto");
    rechazar("r : a;", (Entorno){GB_ENTERO, GB_ENTERO, 3, 0, 0}, "no escribible");
    rechazar("r : a;", (Entorno){GB_ENTERO, GB_ENTERO, 4, 0, 1}, "base o desplazamiento");
    rechazar("r : a;", (Entorno){GB_ENTERO, GB_ENTERO, 3, 1024, 1}, "base o desplazamiento");
    rechazar("r : a;", (Entorno){GB_ENTERO, GB_ENTERO, 3, 1, 1}, "base o desplazamiento");
    char profundo[512] = "r : ";
    for (int i = 0; i < 4; ++i) strcat(profundo, "1 gauss (");
    strcat(profundo, "1");
    for (int i = 0; i < 4; ++i) strcat(profundo, ")");
    strcat(profundo, ";");
    rechazar(profundo, e, "temporales agotados");
    char unarios[4096] = "r : ";
    for (int i = 0; i < 257; ++i) strcat(unarios, "neumann ");
    strcat(unarios, "1;");
    rechazar(unarios, e, "demasiado profunda");
    puts("OK: 22 rechazos con diagnóstico y sin salida parcial");
}

static void accesos(void) {
    Ast ast = {0};
    const AstNodo *n = sentencia(&ast, "r : a gauss b;");
    Entorno e = {GB_ENTERO, GB_ENTERO, 2, -12, 1};
    FILE *salida = tmpfile();
    exigir(salida != NULL, "tmpfile");
    exigir(gb_generar(n, resolver, &e, salida, stderr) == 0, "offset negativo");
    uint32_t registros[32] = {0}, memoria[512] = {0};
    registros[2] = 256; memoria[61] = 3; memoria[62] = 4;
    ejecutar(salida, registros, memoria);
    exigir(memoria[63] == 7, "acceso relativo a sp");
    fclose(salida); ast_liberar(&ast);
    puts("OK: cargas y almacenamiento relativos a sp");
}

int main(int argc, char **argv) {
    if (argc == 2 && !strcmp(argv[1], "--ejemplo")) {
        Ast ast = {0};
        const AstNodo *n = sentencia(&ast, "r : (a gauss b) neumann 1;");
        Entorno e = {GB_ENTERO, GB_ENTERO, 3, 0, 1};
        int estado = gb_generar(n, resolver, &e, stdout, stderr);
        ast_liberar(&ast);
        return estado;
    }
    exigir(argc == 1, "uso: test_generacion [--ejemplo]");
    positivos(); negativos(); accesos();
    puts("Modelo secuencial validado; no prueba bundles, binario ni procesador.");
    return EXIT_SUCCESS;
}
