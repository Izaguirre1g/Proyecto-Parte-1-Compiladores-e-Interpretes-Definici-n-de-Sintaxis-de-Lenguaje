#include "cli.h"
#include "lexer.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    const char *input;
    const char *output;
    const char *pending_option;
    int verbose;
} Options;

static void usage(FILE *out) {
    fputs("Uso: micomp [-v] <archivo_fuente>\n"
          "Fase B: analiza únicamente tokens; todavía no genera binario.\n"
          "Opciones del proyecto reservadas para integración: -o, -s, -t, -m, -x, -p.\n", out);
}

static int parse_args(int argc, char **argv, Options *opt) {
    int positional = 0;
    for (int i = 1; i < argc; ++i) {
        const char *arg = argv[i];
        if (!positional && strcmp(arg, "--") == 0) { positional = 1; continue; }
        if (!positional && strcmp(arg, "-v") == 0) { opt->verbose = 1; continue; }
        if (!positional && strcmp(arg, "-h") == 0) { usage(stdout); return 1; }
        if (!positional && strcmp(arg, "-o") == 0) {
            if (++i >= argc) { fputs("-o necesita un archivo de salida\n", stderr); return -1; }
            opt->output = argv[i];
            opt->pending_option = "-o";
            continue;
        }
        if (!positional && (strcmp(arg, "-s") == 0 || strcmp(arg, "-t") == 0 ||
                            strcmp(arg, "-m") == 0 || strcmp(arg, "-x") == 0 ||
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

int cli_run(int argc, char **argv) {
    Options opt = {0};
    int parsed = parse_args(argc, argv, &opt);
    if (parsed != 0) return parsed == 1 ? 0 : 2;

    size_t size = 0;
    char *source = read_source(opt.input, &size);
    if (!source) return 2;
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
        printf("[léxico] %zu tokens, %zu errores; fases posteriores pendientes.\n",
               count, errors);
    else if (errors == 0)
        fputs("Análisis léxico correcto. Fases posteriores pendientes; no se generó binario.\n",
              stderr);
    free(source);
    return errors ? 1 : 0;
}
