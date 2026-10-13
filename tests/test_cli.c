/* Prueba la CLI real; usa los mismos procesos POSIX que test_parser.c. */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    const char *nombre;
    char *args[7];
    int codigo;
    const char *esperado;
    const char *prohibido;
} Caso;

static const Caso casos[] = {
    {"ayuda", {"./balc", "-h"}, 0, "Uso: balc", "[AST]"},
    {"sin fuente", {"./balc"}, 2, "Uso: balc", "[AST]"},
    {"opcion desconocida", {"./balc", "-z"}, 2, "Opción desconocida", "[AST]"},
    {"dos fuentes", {"./balc", "a.bal", "b.bal"}, 2, "único archivo", "[AST]"},
    {"salida sin argumento", {"./balc", "-o"}, 2, "necesita un archivo", "[AST]"},
    {"salida pendiente", {"./balc", "-o", "salida.bin", "examples/factorial.bal"}, 2, "aún no integrada", "[AST]"},
    {"ensamblador", {"./balc", "-s", "examples/whale.bal"}, 0, "[ensamblador] examples/whale.asm", "aún no integrada"},
    {"ensamblador no soportado", {"./balc", "-s", "examples/ejercicio2_busqueda_matriz.bal"}, 1, "no se pudo generar", "[ensamblador]"},
    {"ensamblador con -t", {"./balc", "-s", "-t", "examples/whale.bal"}, 2, "no se combina", "[ensamblador]"},
    {"simbolos habilitados", {"./balc", "-m", "tests/fixtures/3bd3b_escalar.bal"}, 0, "[SIMBOLOS]", "aún no integrada"},
    {"ensamblado pendiente", {"./balc", "-x", "examples/factorial.bal"}, 2, "aún no integrada", "[AST]"},
    {"paquetes pendientes", {"./balc", "-p", "examples/factorial.bal"}, 2, "aún no integrada", "[AST]"},
    {"lexico", {"./balc", "examples/factorial.bal"}, 0, "Análisis léxico correcto", "[AST]"},
    {"tokens", {"./balc", "-v", "examples/factorial.bal"}, 0, "64 tokens, 0 errores", "[AST]"},
    {"AST", {"./balc", "-t", "examples/factorial.bal"}, 0, "[AST]", "[léxico]"},
    {"ambas fases", {"./balc", "-v", "-t", "examples/factorial.bal"}, 0, "[sintáctico]", "fases posteriores pendientes"},
    {"opciones despues", {"./balc", "examples/factorial.bal", "-t", "-v"}, 0, "[AST]", "fases posteriores pendientes"},
    {"fin de opciones", {"./balc", "-t", "--", "examples/factorial.bal"}, 0, "[AST]", "Opción desconocida"},
    {"expresiones", {"./balc", "-t", "examples/expresiones_basicas.bal"}, 0, "Análisis sintáctico correcto", "error del parser"}
};

static void exigir(int condicion, const char *mensaje) {
    if (!condicion) {
        fprintf(stderr, "FALLO CLI: %s\n", mensaje);
        exit(EXIT_FAILURE);
    }
}

static void probar(const Caso *caso) {
    FILE *captura = tmpfile();
    exigir(captura != NULL, "tmpfile");
    exigir(fflush(stdout) == 0, "fflush");
    pid_t hijo = fork();
    exigir(hijo >= 0, "fork");
    if (hijo == 0) {
        if (dup2(fileno(captura), STDOUT_FILENO) < 0 ||
            dup2(fileno(captura), STDERR_FILENO) < 0) _exit(126);
        fclose(captura);
        execv(caso->args[0], caso->args);
        _exit(127);
    }
    int estado;
    while (waitpid(hijo, &estado, 0) < 0) exigir(errno == EINTR, "waitpid");
    exigir(fseek(captura, 0, SEEK_SET) == 0, "fseek");
    char texto[32768];
    size_t n = fread(texto, 1, sizeof(texto) - 1, captura);
    exigir(!ferror(captura) && feof(captura), "salida demasiado larga o ilegible");
    texto[n] = '\0';
    fclose(captura);
    exigir(WIFEXITED(estado) && WEXITSTATUS(estado) == caso->codigo, caso->nombre);
    exigir(strstr(texto, caso->esperado) != NULL, caso->nombre);
    exigir(strstr(texto, caso->prohibido) == NULL, caso->nombre);
    printf("[OK CLI] %s\n", caso->nombre);
}

int main(void) {
    size_t cantidad = sizeof(casos) / sizeof(casos[0]);
    for (size_t i = 0; i < cantidad; ++i) probar(&casos[i]);
    /* -s escribe el archivo junto al fuente; si falla no deja ninguno. */
    FILE *asm_whale = fopen("examples/whale.asm", "r");
    exigir(asm_whale != NULL, "-s no creo examples/whale.asm");
    char primera[64] = "";
    exigir(fgets(primera, sizeof(primera), asm_whale) != NULL, "whale.asm vacio");
    fclose(asm_whale);
    exigir(strcmp(primera, "# __arranque:\n") == 0, "whale.asm sin arranque");
    exigir(remove("examples/whale.asm") == 0, "remove whale.asm");
    exigir(access("examples/ejercicio2_busqueda_matriz.asm", F_OK) != 0,
           "-s dejo un archivo tras fallar");
    printf("OK: %zu casos de CLI\n", cantidad);
    return EXIT_SUCCESS;
}
