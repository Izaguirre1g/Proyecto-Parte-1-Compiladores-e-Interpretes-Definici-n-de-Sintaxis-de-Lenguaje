#include "generacion_basica.h"

#include <stdint.h>
#include <limits.h>
#include <string.h>

typedef struct {
    GbResolver resolver;
    void *datos;
    FILE *salida;
    FILE *diagnosticos;
    int error;
    unsigned profundidad;
} Generador;

static int fallar(Generador *g, const AstNodo *n, const char *mensaje) {
    if (!g->error && g->diagnosticos)
        fprintf(g->diagnosticos, "%zu:%zu: generación básica: %s\n",
                n ? n->ubicacion.first_line : 0,
                n ? n->ubicacion.first_column : 0, mensaje);
    g->error = 1;
    return 0;
}

static int hijos(const AstNodo *n, unsigned cantidad) {
    const AstNodo *h = n->primer_hijo;
    while (h && cantidad) { h = h->siguiente; --cantidad; }
    return !h && cantidad == 0;
}

static int referencia(Generador *g, const AstNodo *n, GbReferencia *ref) {
    if (!n || n->tipo != AST_IDENTIFICADOR || !n->texto || !g->resolver ||
        !g->resolver(g->datos, n, ref))
        return fallar(g, n, "identificador sin referencia semántica");
    if ((ref->tipo != GB_ENTERO && ref->tipo != GB_BOOLEANO) ||
        (ref->base != 0 && ref->base != 2 && ref->base != 3) ||
        ref->desplazamiento < -1024 || ref->desplazamiento > 1023 ||
        ref->desplazamiento % 4 != 0)
        return fallar(g, n, "tipo, base o desplazamiento no soportado");
    return 1;
}

static int magnitud(const char *s, uint32_t *valor) {
    uint32_t v = 0;
    if (!s || !*s) return 0;
    for (; *s; ++s) {
        if (*s < '0' || *s > '9') return 0;
        unsigned d = (unsigned)(*s - '0');
        if (v > (UINT32_C(2147483648) - d) / 10) return 0;
        v = v * 10 + d;
    }
    *valor = v;
    return 1;
}

/* Cada fragmento cabe en el inmediato con signo de 11 bits. */
static void constante(Generador *g, unsigned r, uint32_t v) {
    if (v <= 1023) {
        fprintf(g->salida, "sumai x%u, x0, %u\n", r, (unsigned)v);
        return;
    }
    fprintf(g->salida, "sumai x%u, x0, %u\n", r, (unsigned)(v >> 24));
    for (int turno = 16; turno >= 0; turno -= 8) {
        fprintf(g->salida, "cizqi x%u, x%u, 8\n", r, r);
        fprintf(g->salida, "ori x%u, x%u, %u\n", r, r,
                (unsigned)((v >> turno) & 255));
    }
}

/* Producto de 32 bits por sumas y desplazamientos; no necesita MUL.
 * x12-x15 son temporales y se reutilizan después de evaluar ambos operandos. */
static void multiplicar(Generador *g, unsigned r) {
    fprintf(g->salida, "sumai x12, x%u, 0\n"
                       "sumai x13, x%u, 0\n"
                       "sumai x14, x0, 0\n", r, r + 1);
    for (unsigned bit = 0; bit < 32; ++bit) {
        fprintf(g->salida, "andi x15, x13, 1\n"
                           "resta x15, x0, x15\n"
                           "and x15, x12, x15\n"
                           "suma x14, x14, x15\n");
        if (bit != 31)
            fprintf(g->salida, "cizqi x12, x12, 1\n"
                               "cderi x13, x13, 1\n");
    }
    fprintf(g->salida, "sumai x%u, x14, 0\n", r);
}

/* Cada invocación reserva nombres distintos, también dentro de funciones.
 * El compilador actual genera código en un solo hilo. */
static unsigned long siguiente_aritmetica;

static int dividir(Generador *g, const AstNodo *n, unsigned r, int residuo) {
    if (siguiente_aritmetica == ULONG_MAX)
        return fallar(g, n, "etiquetas de division agotadas");
    unsigned long id = siguiente_aritmetica++;
    FILE *f = g->salida;
    /* Un divisor cero detiene el programa antes de escribir el resultado. */
    fprintf(f, "igualno x%u, x0, __gb_div_%lu_inicio\n"
               "__gb_div_%lu_cero:\n"
               "sye x0, __gb_div_%lu_cero\n"
               "__gb_div_%lu_inicio:\n", r + 1, id, id, id, id);
    fprintf(f, "mrq x8, x%u, x0\nresta x8, x0, x8\n"
               "mrq x9, x%u, x0\nresta x9, x0, x9\n"
               "xor x12, x%u, x8\nresta x12, x12, x8\n"
               "xor x13, x%u, x9\nresta x13, x13, x9\n"
               "xor x9, x8, x9\n"
               "sumai x10, x0, 1\ncizqi x10, x10, 31\n"
               "xor x11, x13, x10\n"
               "sumai x14, x0, 0\nsumai x%u, x0, 32\n"
               "__gb_div_%lu_paso:\n", r, r + 1, r, r + 1, r, id);
    /* División binaria de magnitudes. XOR con el bit de signo permite
     * comparar como unsigned usando la comparación con signo del ISA. */
    fprintf(f, "cderi x15, x12, 31\ncizqi x14, x14, 1\n"
               "or x14, x14, x15\ncizqi x12, x12, 1\n"
               "xor x15, x14, x10\nmrq x15, x15, x11\n"
               "xori x15, x15, 1\nor x12, x12, x15\n"
               "resta x15, x0, x15\nand x15, x13, x15\n"
               "resta x14, x14, x15\nrestai x%u, x%u, 1\n"
               "igualno x%u, x0, __gb_div_%lu_paso\n", r, r, r, id);
    fprintf(f, "xor x%u, x%u, x%u\nresta x%u, x%u, x%u\n",
            r, residuo ? 14u : 12u, residuo ? 8u : 9u,
            r, r, residuo ? 8u : 9u);
    return 1;
}

static void producto_potencia(FILE *f, unsigned a, unsigned b,
                              unsigned destino, unsigned long id, unsigned paso) {
    fprintf(f, "sumai x12, x%u, 0\nsumai x13, x%u, 0\n"
               "sumai x14, x0, 0\nsumai x11, x0, 32\n"
               "__gb_pow_%lu_mul_%u:\n"
               "andi x15, x13, 1\nresta x15, x0, x15\n"
               "and x15, x12, x15\nsuma x14, x14, x15\n"
               "cizqi x12, x12, 1\ncderi x13, x13, 1\n"
               "restai x11, x11, 1\n"
               "igualno x11, x0, __gb_pow_%lu_mul_%u\n"
               "sumai x%u, x14, 0\n", a, b, id, paso, id, paso, destino);
}

/* Exponenciación por cuadrados: procesa como máximo 31 bits del exponente. */
static int potencia(Generador *g, const AstNodo *n, unsigned r) {
    if (siguiente_aritmetica == ULONG_MAX)
        return fallar(g, n, "etiquetas de potencia agotadas");
    unsigned long id = siguiente_aritmetica++;
    FILE *f = g->salida;
    fprintf(f, "mayoroigual x%u, x0, __gb_pow_%lu_inicio\n"
               "__gb_pow_%lu_negativo:\n"
               "sye x0, __gb_pow_%lu_negativo\n"
               "__gb_pow_%lu_inicio:\n"
               "sumai x8, x0, 1\nsumai x9, x%u, 0\nsumai x10, x%u, 0\n"
               "__gb_pow_%lu_paso:\n"
               "igualno x10, x0, __gb_pow_%lu_iterar\n"
               "sye x0, __gb_pow_%lu_fin\n"
               "__gb_pow_%lu_iterar:\n"
               "andi x%u, x10, 1\n"
               "igualno x%u, x0, __gb_pow_%lu_producto\n"
               "sye x0, __gb_pow_%lu_cuadrado\n"
               "__gb_pow_%lu_producto:\n",
            r + 1, id, id, id, id, r, r + 1, id, id, id, id, r, r, id, id, id);
    producto_potencia(f, 8, 9, 8, id, 0);
    fprintf(f, "__gb_pow_%lu_cuadrado:\n", id);
    producto_potencia(f, 9, 9, 9, id, 1);
    fprintf(f, "cderi x10, x10, 1\nsye x0, __gb_pow_%lu_paso\n"
               "__gb_pow_%lu_fin:\nsumai x%u, x8, 0\n", id, id, r);
    return 1;
}

static int negativo_literal(const AstNodo *n) {
    uint32_t v;
    return n && n->tipo == AST_UNARIO && n->texto &&
           !strcmp(n->texto, "neumann") && hijos(n, 1) &&
           n->primer_hijo->tipo == AST_ENTERO &&
           magnitud(n->primer_hijo->texto, &v) && v != 0;
}

static int cero_literal(const AstNodo *n) {
    if (n && n->tipo == AST_UNARIO && n->texto &&
        !strcmp(n->texto, "neumann") && hijos(n, 1))
        return cero_literal(n->primer_hijo);
    if (!n || n->tipo != AST_ENTERO || !n->texto) return 0;
    uint32_t v;
    return magnitud(n->texto, &v) && v == 0;
}

static int expresion(Generador *g, const AstNodo *n, unsigned r, GbTipo *tipo);

static int expresion_interna(Generador *g, const AstNodo *n, unsigned r, GbTipo *tipo) {
    if (!n) return fallar(g, n, "expresión ausente");
    if (r > 7) return fallar(g, n, "temporales agotados (x4-x7)");
    if (n->tipo == AST_ENTERO && hijos(n, 0)) {
        uint32_t v;
        if (!magnitud(n->texto, &v) || v > INT32_MAX)
            return fallar(g, n, "literal fuera del rango entero de 32 bits");
        constante(g, r, v);
        *tipo = GB_ENTERO;
        return 1;
    }
    if (n->tipo == AST_BOOLEANO && hijos(n, 0) && n->texto) {
        if (strcmp(n->texto, "declare_true") && strcmp(n->texto, "declare_false"))
            return fallar(g, n, "literal booleano inválido");
        constante(g, r, strcmp(n->texto, "declare_true") == 0);
        *tipo = GB_BOOLEANO;
        return 1;
    }
    if (n->tipo == AST_IDENTIFICADOR && hijos(n, 0)) {
        GbReferencia ref;
        if (!referencia(g, n, &ref)) return 0;
        fprintf(g->salida, "cargai x%u, x%u, %d\n", r, ref.base, ref.desplazamiento);
        *tipo = ref.tipo;
        return 1;
    }
    if (n->tipo == AST_UNARIO && n->texto && hijos(n, 1)) {
        const AstNodo *h = n->primer_hijo;
        uint32_t v;
        if (!strcmp(n->texto, "neumann") && h->tipo == AST_ENTERO &&
            hijos(h, 0) && magnitud(h->texto, &v) && v == UINT32_C(2147483648)) {
            constante(g, r, v);
            *tipo = GB_ENTERO;
            return 1;
        }
        if (!expresion(g, h, r, tipo)) return 0;
        if (!strcmp(n->texto, "neumann") && *tipo == GB_ENTERO)
            fprintf(g->salida, "resta x%u, x0, x%u\n", r, r);
        else if (!strcmp(n->texto, "~") && *tipo == GB_BOOLEANO)
            fprintf(g->salida, "xori x%u, x%u, 1\n", r, r);
        else return fallar(g, n, "operador unario o tipo no soportado");
        return 1;
    }
    if (n->tipo != AST_BINARIO || !n->texto || !hijos(n, 2))
        return fallar(g, n, "nodo fuera del subconjunto básico (sin llamadas ni índices)");

    const char *op = n->texto, *instruccion = NULL;
    int categoria = 0, invertir = 0;
    if (!strcmp(op, "gauss")) instruccion = "suma";
    else if (!strcmp(op, "neumann")) instruccion = "resta";
    else if (!strcmp(op, "pitagoras")) instruccion = "producto";
    else if (!strcmp(op, "euclides") || !strcmp(op, "euler")) instruccion = "division";
    else if (!strcmp(op, "descartes")) instruccion = "potencia";
    else if (!strcmp(op, "&&")) { instruccion = "and"; categoria = 1; }
    else if (!strcmp(op, "||")) { instruccion = "or"; categoria = 1; }
    else if (!strcmp(op, "^")) { instruccion = "xor"; categoria = 1; }
    else if (!strcmp(op, "<")) { instruccion = "mrq"; categoria = 2; }
    else if (!strcmp(op, ">")) { instruccion = "myq"; categoria = 2; }
    else if (!strcmp(op, "<=")) { instruccion = "myq"; categoria = 2; invertir = 1; }
    else if (!strcmp(op, ">=")) { instruccion = "mrq"; categoria = 2; invertir = 1; }
    else if (!strcmp(op, "==") || !strcmp(op, "=/=")) { instruccion = "xor"; categoria = 3; }
    else return fallar(g, n, "operador sin traducción acordada en la ISA");

    if ((!strcmp(op, "euclides") || !strcmp(op, "euler")) &&
        cero_literal(n->primer_hijo->siguiente))
        return fallar(g, n, "division por cero literal");

    if (!strcmp(op, "descartes") && negativo_literal(n->primer_hijo->siguiente))
        return fallar(g, n, "exponente negativo literal");

    GbTipo izquierda, derecha;
    if (!expresion(g, n->primer_hijo, r, &izquierda) ||
        !expresion(g, n->primer_hijo->siguiente, r + 1, &derecha)) return 0;
    if (izquierda != derecha || (categoria == 1 && izquierda != GB_BOOLEANO) ||
        ((categoria == 0 || categoria == 2) && izquierda != GB_ENTERO))
        return fallar(g, n, "tipos incompatibles con el operador");
    if (!strcmp(op, "pitagoras")) multiplicar(g, r);
    else if (!strcmp(instruccion, "division")) {
        if (!dividir(g, n, r, !strcmp(op, "euler"))) return 0;
    } else if (!strcmp(instruccion, "potencia")) {
        if (!potencia(g, n, r)) return 0;
    } else fprintf(g->salida, "%s x%u, x%u, x%u\n", instruccion, r, r, r + 1);
    if (categoria == 3) {
        /* XOR puede tener el bit de signo: probar ambos lados de cero. */
        fprintf(g->salida, "mrq x%u, x%u, x0\n", r + 1, r);
        fprintf(g->salida, "myq x%u, x%u, x0\n", r, r);
        fprintf(g->salida, "or x%u, x%u, x%u\n", r, r, r + 1);
        invertir = !strcmp(op, "==");
    }
    if (invertir) fprintf(g->salida, "xori x%u, x%u, 1\n", r, r);
    *tipo = categoria == 0 ? GB_ENTERO : GB_BOOLEANO;
    return 1;
}

static int expresion(Generador *g, const AstNodo *n, unsigned r, GbTipo *tipo) {
    if (g->profundidad >= 256)
        return fallar(g, n, "expresión demasiado profunda (máximo 256 niveles)");
    ++g->profundidad;
    int ok = expresion_interna(g, n, r, tipo);
    --g->profundidad;
    return ok;
}

static int generar(Generador *g, const AstNodo *n) {
    GbTipo tipo;
    if (!n) return fallar(g, n, "nodo ausente");
    if (n->tipo != AST_ASIGNACION && n->tipo != AST_DECLARACION)
        return expresion(g, n, 4, &tipo);
    int declaracion = n->tipo == AST_DECLARACION;
    if (!hijos(n, declaracion ? 3 : 2))
        return fallar(g, n, "se requiere asignación o declaración inicializada");
    const AstNodo *id = n->primer_hijo, *valor = id->siguiente;
    GbReferencia ref;
    if (!referencia(g, id, &ref)) return 0;
    if (!ref.escribible) return fallar(g, id, "destino no escribible");
    if (declaracion) {
        const char *esperado = ref.tipo == GB_ENTERO ? "declare_int" : "declare_boolean";
        if (valor->tipo != AST_TIPO || !valor->texto || strcmp(valor->texto, esperado))
            return fallar(g, valor, "tipo declarado distinto de la referencia semántica");
        valor = valor->siguiente;
    }
    if (!expresion(g, valor, 4, &tipo)) return 0;
    if (tipo != ref.tipo) return fallar(g, valor, "tipo de asignación incompatible");
    fprintf(g->salida, "guardap x%u, x4, %d\n", ref.base, ref.desplazamiento);
    return 1;
}

int gb_generar(const AstNodo *nodo, GbResolver resolver, void *datos,
               FILE *salida, FILE *diagnosticos) {
    Generador g = {resolver, datos, NULL, diagnosticos, 0, 0};
    if (!salida) { fallar(&g, nodo, "salida ausente"); return 1; }
    g.salida = tmpfile();
    if (!g.salida) { fallar(&g, nodo, "no se pudo crear el temporal"); return 1; }
    int ok = generar(&g, nodo);
    if (ok && (fflush(g.salida) != 0 || ferror(g.salida) ||
               fseek(g.salida, 0, SEEK_SET) != 0))
        ok = fallar(&g, nodo, "error de E/S en el temporal");
    if (ok) {
        char bloque[4096];
        size_t n;
        while ((n = fread(bloque, 1, sizeof(bloque), g.salida)) != 0) {
            if (fwrite(bloque, 1, n, salida) != n) {
                ok = fallar(&g, nodo, "error de escritura");
                break;
            }
        }
        if (ferror(g.salida) || fflush(salida) != 0)
            ok = fallar(&g, nodo, "error de E/S al copiar la salida");
    }
    fclose(g.salida);
    return ok ? 0 : 1;
}
