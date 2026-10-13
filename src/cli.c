#include "cli.h"
#include "lexer.h"

/*Se incluye interfaz del parser sintáctico, permite utilizar la funcion analizar_sintaxis() desde cli.c*/
#include "sintactico.h"
#include "semantic_modules.h"
#include "memory.h"
#include "enlace_funciones.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *input;
    const char *pending_option;
    int verbose;

    /*Bandera de sintaxis guarda si se pidio analizar sintaxis: 0 es desactivo y 1 es activado*/
    int syntax;
    int symbols;
    int assembly;
} Options;

static void usage(FILE *out) {
    fputs("Uso: balc [-v] [-t] [-m] [-s] <archivo_fuente>\n"
          "-t valida sintaxis e imprime el AST. Sin -t, solo analiza tokens.\n"
          "-m valida semantica, asigna memoria y muestra simbolos.\n"
          "-s genera el ensamblador en bundles (<fuente>.asm).\n"
          "Opciones del proyecto reservadas para integración: -o, -x, -p.\n", out);
}

static int parse_args(int argc, char **argv, Options *opt) {
    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!positional && strcmp(arg, "--") == 0) { positional = 1; continue; }
        if (!positional && strcmp(arg, "-v") == 0) { opt->verbose = 1; continue; }
        /*Se agrega la opcion -t para validar sintaxis */
        if (!positional && strcmp(arg, "-t") == 0) { opt->syntax = 1; continue; }
        if (!positional && strcmp(arg, "-m") == 0) { opt->symbols = 1; continue; }
        if (!positional && strcmp(arg, "-h") == 0) { usage(stdout); return 1; }
        if (!positional && strcmp(arg, "-o") == 0) {
            if (++i >= argc) { fputs("-o necesita un archivo de salida\n", stderr); return -1; }
            opt->pending_option = "-o";
            continue;
        }
        if (!positional && strcmp(arg, "-s") == 0) { opt->assembly = 1; continue; }
        if (!positional && (strcmp(arg, "-x") == 0 ||
                            strcmp(arg, "-p") == 0)) {
            opt->pending_option = arg;
            continue;
        }
        if (!positional && arg[0] == '-') {
            fprintf(stderr, "Opción desconocida: %s\n", arg);
            return -1;
        }
        if (opt->input) {
            fputs("Se admite un único archivo fuente\n", stderr);
            return -1;
        }
        opt->input = arg;
    }
    if (opt->pending_option) {
        fprintf(stderr, "La opción %s requiere una fase aún no integrada\n",
                opt->pending_option);
        return -1;
    }
    if (!opt->input) { usage(stderr); return -1; }
    if (opt->symbols && (opt->syntax || opt->verbose)) {
        fputs("-m no se combina con -t ni -v\n", stderr);
        return -1;
    }
    if (opt->assembly && (opt->symbols || opt->syntax || opt->verbose)) {
        fputs("-s no se combina con -t, -v ni -m\n", stderr);
        return -1;
    }
    return 0;
}

static char *read_source(const char *path, size_t *size) {
    FILE *f = fopen(path, "rb");
    if (!f) { perror(path); return NULL; }
    size_t cap = 4096, used = 0;
    char *buffer = malloc(cap);
    if (!buffer) { fclose(f); fputs("Memoria insuficiente\n", stderr); return NULL; }
    for (;;) {
        if (used == cap) {
            if (cap > SIZE_MAX / 2) {
                fputs("Archivo demasiado grande\n", stderr);
                free(buffer); fclose(f); return NULL;
            }
            size_t next = cap * 2;
            char *grown = realloc(buffer, next);
            if (!grown) {
                fputs("Memoria insuficiente\n", stderr);
                free(buffer); fclose(f); return NULL;
            }
            buffer = grown;
            cap = next;
        }
        size_t n = fread(buffer + used, 1, cap - used, f);
        used += n;
        if (!n) {
            if (ferror(f)) {
                perror(path); free(buffer); fclose(f); return NULL;
            }
            break;
        }
    }
    fclose(f);
    *size = used;
    return buffer;
}

static void print_escaped(const Token *t) {
    putchar('"');
    for (size_t i = 0; i < t->length; ++i) {
        unsigned char c = (unsigned char)t->lexeme[i];
        if (c == '\n') fputs("\\n", stdout);
        else if (c == '\r') fputs("\\r", stdout);
        else if (c == '\t') fputs("\\t", stdout);
        else if (c == '"') fputs("\\\"", stdout);
        else if (c == '\\') fputs("\\\\", stdout);
        else if (c < 0x20 || c == 0x7f) printf("\\x%02X", c);
        else putchar(c);
    }
    putchar('"');
}

/* El AST se muestra solo cuando el programa completo es sintacticamente valido. */
static int mostrar_ast(const char *fuente, size_t longitud, const char *nombre) {
    Ast arbol = {0};
    int estado = construir_ast(fuente, longitud, nombre, stderr, &arbol);
    if (estado == 0) {
        puts("[AST]");
        ast_imprimir(&arbol, stdout);
        puts("Análisis sintáctico correcto. Generación de código pendiente.");
    }
    ast_liberar(&arbol);
    return estado;
}

/* Etapa 3B-D3F: reporte de datos, no mapa de codigo. */
static const char *clase_sim(SimClase c)
{
    switch (c) {
        case SIM_VARIABLE: return "variable";
        case SIM_CONSTANTE: return "constante";
        case SIM_FUNCION: return "funcion";
        case SIM_PARAMETRO: return "parametro";
        case SIM_LISTA: return "lista";
        case SIM_MODULO: return "modulo";
        case SIM_ETIQUETA: return "etiqueta";
    }
    return "desconocida";
}

static const char *tipo_sim(SimTipo t)
{
    switch (t) {
        case SIM_TIPO_INT: return "declare_int";
        case SIM_TIPO_BOOLEAN: return "declare_boolean";
        case SIM_TIPO_TEXT: return "declare_text";
        case SIM_TIPO_CHAR: return "declare_char";
        case SIM_TIPO_VOID: return "declare_infinite_void";
        case SIM_TIPO_ERROR: return "error";
    }
    return "desconocido";
}

static const char *almacen_sim(SimAlmacen a)
{
    switch (a) {
        case SIM_ALMACEN_SIN_ASIGNAR: return "sin_asignar";
        case SIM_ALMACEN_GLOBAL: return "global(gp)";
        case SIM_ALMACEN_LOCAL: return "local(sp)";
        case SIM_ALMACEN_PARAMETRO: return "parametro(sp)";
        case SIM_ALMACEN_CODIGO: return "codigo";
    }
    return "desconocido";
}

static int reporte_simbolos(void *ctx, const Ast *arbol,
                           SimTabla *tabla, FILE *diagnosticos)
{
    (void)ctx;
    (void)arbol;
    MemProyectoMapa mapa = {0};
    MemProyectoResultado r = mem_asignar_proyecto(tabla, 0, 4096,
                                                  1020, &mapa);
    if (r != MEM_PROYECTO_OK) {
        fprintf(diagnosticos, "-m: asignacion de memoria no disponible (codigo %d)\n", (int)r);
        return 1;
    }
    /* Imprimir solo tras terminar la validacion y la asignacion. */
    puts("[SIMBOLOS]");
    puts("nombre | clase | tipo | ambito | nivel | almacen | bytes | desplazamiento | direccion_codigo");
    for (const SimSimbolo *s = tabla->primero; s; s = s->siguiente_orden) {
        const SimAmbito *a = s->ambito;
        printf("%s | %s | %s | %s | %u | %s | %zu | ",
               s->nombre ? s->nombre : "?", clase_sim(s->clase),
               tipo_sim(s->tipo), a && a->nombre ? a->nombre : "?",
               a ? a->nivel : 0, almacen_sim(s->almacen), s->bytes);
        if (s->almacen == SIM_ALMACEN_GLOBAL ||
            s->almacen == SIM_ALMACEN_LOCAL ||
            s->almacen == SIM_ALMACEN_PARAMETRO)
            printf("%ld", s->desplazamiento);
        else
            fputs("pendiente", stdout);
        puts(" | pendiente");
    }
    printf("[MEMORIA] funciones=%zu marco_maximo_bytes=%zu\n",
           mapa.funciones, mapa.marco_maximo_bytes);
    return ferror(stdout) ? 2 : 0;
}

/* Mapa de memoria del contrato: globales desde 0x4000 hasta 0x7FFF y pila
 * desde 0x10000 hacia abajo (0x8000-0xFFFF). El codigo empieza en 0. */
#define CLI_GP_BASE 0x4000u
#define CLI_FIN_GLOBALES 0x8000u
#define CLI_TOPE_PILA 0x10000u

static int generar_ensamblador(void *ctx, const Ast *arbol,
                               SimTabla *tabla, FILE *diagnosticos)
{
    const char *destino = ctx;
    MemProyectoMapa mapa = {0};
    MemProyectoResultado r = mem_asignar_proyecto(tabla, CLI_GP_BASE, CLI_FIN_GLOBALES,
                                                  1020, &mapa);
    if (r != MEM_PROYECTO_OK) {
        fprintf(diagnosticos, "-s: asignacion de memoria no disponible (codigo %d)\n", (int)r);
        return 1;
    }
    const EfArranque arranque = {(uint32_t)mapa.globales.gp_base, CLI_TOPE_PILA};
    EfResumen resumen;
    FILE *temporal = tmpfile();
    if (!temporal) { perror("tmpfile"); return 2; }
    if (ef_enlazar(arbol, tabla, 0, &arranque, NULL, temporal,
                   diagnosticos, &resumen) != 0) {
        fclose(temporal);
        fputs("-s: no se pudo generar el ensamblador\n", diagnosticos);
        return 1;
    }
    /* El archivo se crea solo si la generacion completa tuvo exito. */
    FILE *salida = fopen(destino, "w");
    int ok = salida && fseek(temporal, 0, SEEK_SET) == 0;
    char buf[4096];
    size_t n;
    while (ok && (n = fread(buf, 1, sizeof(buf), temporal)) != 0)
        ok = fwrite(buf, 1, n, salida) == n;
    ok = ok && !ferror(temporal);
    if (salida && fclose(salida) != 0) ok = 0;
    fclose(temporal);
    if (!ok) { perror(destino); return 2; }
    printf("[ensamblador] %s: %zu bundles, %zu funciones, %zu saltos (%zu reescritos)\n",
           destino, resumen.saltos.bundles, resumen.funciones,
           resumen.saltos.saltos, resumen.saltos.relajados);
    return ferror(stdout) ? 2 : 0;
}

/* prog.bal -> prog.asm; sin extension .bal se agrega .asm. */
static char *nombre_ensamblador(const char *fuente)
{
    size_t largo = strlen(fuente);
    if (largo >= 4 && strcmp(fuente + largo - 4, ".bal") == 0) largo -= 4;
    char *nombre = malloc(largo + 5);
    if (!nombre) return NULL;
    memcpy(nombre, fuente, largo);
    memcpy(nombre + largo, ".asm", 5);
    return nombre;
}

int cli_run(int argc, char **argv) {
    Options opt = {0};
    int parsed = parse_args(argc, argv, &opt);
    if (parsed != 0) return parsed == 1 ? 0 : 2;

    if (opt.symbols)
        return sem_visitar_proyecto(opt.input, stderr, reporte_simbolos, NULL);
    if (opt.assembly) {
        char *destino = nombre_ensamblador(opt.input);
        if (!destino) { fputs("Memoria insuficiente\n", stderr); return 2; }
        int estado = sem_visitar_proyecto(opt.input, stderr, generar_ensamblador, destino);
        free(destino);
        return estado;
    }

    size_t size = 0;
    char *source = read_source(opt.input, &size);
    if (!source) return 2;

    /*Se agrega la condicion para que si se pidio analizar sintaxis y no verbose, se ejecute el analisis sintactico*/
    if (opt.syntax && !opt.verbose) {
        int estado = mostrar_ast(source, size, opt.input);
        free(source);
        return estado;
    }
    Lexer lexer;
    lexer_init(&lexer, source, size, opt.input);
    if (opt.verbose) fprintf(stdout, "[léxico] Analizando %s\n", opt.input);
    size_t count = 0, errors = 0;
    for (;;) {
        Token t = lexer_next(&lexer);
        if (t.error == TOKEN_OUT_OF_MEMORY) {
            fprintf(stderr, "%s:%zu:%zu: memoria insuficiente\n",
                    opt.input, t.line, t.column);
            token_dispose(&t);
            free(source);
            return 2;
        }
        if (t.type == TOKEN_ERROR) {
            fprintf(stderr, "%s:%zu:%zu: error léxico: %s\n",
                    opt.input, t.line, t.column, token_error_message(t.error));
            ++errors;
        }
        if (opt.verbose) {
            printf("%zu:%zu %-23s ", t.line, t.column, token_type_name(t.type));
            print_escaped(&t);
            putchar('\n');
        }
        TokenType kind = t.type;
        if (kind != TOKEN_EOF) ++count;
        token_dispose(&t);
        if (kind == TOKEN_EOF) break;
    }
    if (opt.verbose)
        printf("[léxico] %zu tokens, %zu errores.\n",
               count, errors);
    else if (errors == 0 && !opt.syntax)
        fputs("Análisis léxico correcto. Fases posteriores pendientes; no se generó binario.\n",
              stderr);

    /*Se agrega la condicion para que si se pidio analizar sintaxis y no hubo errores léxicos, se ejecute el analisis sintactico*/
    int estado = errors ? 1 : 0;
    if (opt.syntax && errors == 0) {
        puts("[sintáctico] Construyendo AST");
        estado = mostrar_ast(source, size, opt.input);
    }
    if (opt.verbose && !opt.syntax && errors == 0)
        puts("[estado] Solo análisis léxico; no se generó ensamblador ni binario.");
    free(source);
    return estado;
}
