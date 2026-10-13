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
    /* Errores de invocacion. */
    {"ayuda", {"./balc", "-h"}, 0, "Uso: balc", "[AST]"},
    {"sin fuente", {"./balc"}, 2, "Uso: balc", "[AST]"},
    {"opcion desconocida", {"./balc", "-z"}, 2, "Opción desconocida", "[AST]"},
    {"dos fuentes", {"./balc", "a.bal", "b.bal"}, 2, "único archivo", "[AST]"},
    {"salida sin argumento", {"./balc", "-o"}, 2, "necesita un archivo", "[AST]"},
    /* Compilacion segun la especificacion: siempre genera el binario. */
    {"compilar por defecto", {"./balc", "examples/whale.bal"}, 0, "[binario] examples/whale.bin y examples/whale.mem: 144 bundles", "[AST]"},
    {"binario con -o", {"./balc", "-o", "tests/cli_salida.bin", "examples/whale.bal"}, 0, "[binario] tests/cli_salida.bin y tests/cli_salida.mem: 144 bundles", "[ensamblador]"},
    {"-s ademas del binario", {"./balc", "-s", "-o", "tests/cli_s.bin", "examples/whale.bal"}, 0, "[ensamblador] examples/whale.asm", "error"},
    {"-t imprime y compila", {"./balc", "-t", "-o", "tests/cli_t.bin", "examples/whale.bal"}, 0, "[AST]\nPrograma", "error"},
    {"-m imprime y compila", {"./balc", "-m", "-o", "tests/cli_m.bin", "tests/fixtures/3bd3b_escalar.bal"}, 0, "| codigo | 0 | pendiente | 0x", "error"},
    {"-v fases", {"./balc", "-v", "-o", "tests/cli_v.bin", "examples/whale.bal"}, 0, "[fase 6] binario", "no se pudo"},
    {"-p paquetes", {"./balc", "-p", "-o", "tests/cli_p.bin", "examples/whale.bal"}, 0, "           ALU         1000000 0000 00000 00011 00000000000\n", "error"},
    {"no soportado", {"./balc", "-s", "-o", "tests/cli_matriz.bin", "examples/expresiones_basicas.bal"}, 1, "no se pudo generar", "[ensamblador]"},
    /* -x: ensamblador -> binario. */
    {"-x ensamblar", {"./balc", "-x", "-o", "tests/cli_isa.bin", "tests/fixtures/isa_todas.asm"}, 0, "[binario] tests/cli_isa.bin", "[ensamblador]"},
    {"-x invalido", {"./balc", "-x", "-o", "tests/cli_invalido.bin", "tests/fixtures/asm_invalido.asm"}, 1, "RAW en x4", "[binario]"},
    {".asm sin -x", {"./balc", "tests/fixtures/isa_todas.asm"}, 2, "use -x", "[binario]"},
    {"-x con -s", {"./balc", "-x", "-s", "tests/fixtures/isa_todas.asm"}, 2, "no se combina", "[binario]"},
    /* Modo interno --frontend (pruebas de lexico y sintaxis). */
    {"lexico", {"./balc", "--frontend", "examples/factorial.bal"}, 0, "Análisis léxico correcto", "[AST]"},
    {"tokens", {"./balc", "--frontend", "-v", "examples/factorial.bal"}, 0, "64 tokens, 0 errores", "[AST]"},
    {"AST", {"./balc", "--frontend", "-t", "examples/factorial.bal"}, 0, "[AST]", "[léxico]"},
    {"ambas fases", {"./balc", "--frontend", "-v", "-t", "examples/factorial.bal"}, 0, "[sintáctico]", "[binario]"},
    {"opciones despues", {"./balc", "--frontend", "examples/factorial.bal", "-t", "-v"}, 0, "[AST]", "[binario]"},
    {"fin de opciones", {"./balc", "--frontend", "-t", "--", "examples/factorial.bal"}, 0, "[AST]", "Opción desconocida"},
    {"expresiones", {"./balc", "--frontend", "-t", "examples/expresiones_basicas.bal"}, 0, "Análisis sintáctico correcto", "error del parser"},
    {"frontend sin -o", {"./balc", "--frontend", "-o", "x.bin", "examples/factorial.bal"}, 2, "Opción desconocida", "[binario]"}
};

static void exigir(int condicion, const char *mensaje) {
    if (!condicion) {
        fprintf(stderr, "FALLO CLI: %s\n", mensaje);
        exit(EXIT_FAILURE);
    }
}

static long tamano(const char *ruta) {
    FILE *f = fopen(ruta, "rb");
    if (!f) return -1;
    long n = fseek(f, 0, SEEK_END) == 0 ? ftell(f) : -1;
    fclose(f);
    return n;
}

static int mismos(const char *a, const char *b) {
    FILE *x = fopen(a, "rb"), *y = fopen(b, "rb");
    int iguales = x && y;
    while (iguales) {
        int c = fgetc(x), d = fgetc(y);
        if (c != d) iguales = 0;
        if (c == EOF || d == EOF) break;
    }
    if (x) fclose(x);
    if (y) fclose(y);
    return iguales;
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
    /* -s escribe el .asm junto al fuente y empieza con el arranque. */
    FILE *asm_whale = fopen("examples/whale.asm", "r");
    exigir(asm_whale != NULL, "-s no creo examples/whale.asm");
    char primera[64] = "";
    exigir(fgets(primera, sizeof(primera), asm_whale) != NULL, "whale.asm vacio");
    fclose(asm_whale);
    exigir(strcmp(primera, "# __arranque:\n") == 0, "whale.asm sin arranque");
    /* .bin de 16 bytes por bundle y .mem de 33 caracteres por bundle. */
    exigir(tamano("tests/cli_salida.bin") == 144 * 16, "tamano de cli_salida.bin");
    exigir(tamano("tests/cli_salida.mem") == 144 * 33, "tamano de cli_salida.mem");
    /* Todas las opciones generan el mismo binario. */
    const char *iguales[] = {"examples/whale.bin", "tests/cli_s.bin", "tests/cli_t.bin",
                             "tests/cli_v.bin", "tests/cli_p.bin"};
    for (size_t i = 0; i < sizeof(iguales) / sizeof(*iguales); ++i)
        exigir(mismos(iguales[i], "tests/cli_salida.bin"), iguales[i]);
    /* -x sobre un .asm da el mismo binario que tools/assembler.py. */
    exigir(mismos("tests/cli_isa.bin", "tests/fixtures/isa_todas.bin"), "cli_isa.bin distinto");
    exigir(mismos("tests/cli_isa.mem", "tests/fixtures/isa_todas.mem"), "cli_isa.mem distinto");
    const char *generados[] = {"examples/whale.asm", "examples/whale.bin", "examples/whale.mem",
                               "tests/cli_salida.bin", "tests/cli_salida.mem",
                               "tests/cli_s.bin", "tests/cli_s.mem", "tests/cli_t.bin",
                               "tests/cli_t.mem", "tests/cli_m.bin", "tests/cli_m.mem",
                               "tests/cli_v.bin", "tests/cli_v.mem", "tests/cli_p.bin",
                               "tests/cli_p.mem", "tests/cli_isa.bin", "tests/cli_isa.mem"};
    for (size_t i = 0; i < sizeof(generados) / sizeof(*generados); ++i)
        exigir(remove(generados[i]) == 0, generados[i]);
    /* Un error no deja archivos; --frontend nunca escribe. */
    const char *no_generados[] = {"tests/cli_invalido.bin", "tests/cli_invalido.mem",
                                  "tests/cli_matriz.bin", "tests/cli_matriz.mem",
                                  "examples/expresiones_basicas.asm",
                                  "examples/factorial.bin", "examples/factorial.mem",
                                  "examples/expresiones_basicas.bin", "x.bin"};
    for (size_t i = 0; i < sizeof(no_generados) / sizeof(*no_generados); ++i)
        exigir(access(no_generados[i], F_OK) != 0, no_generados[i]);
    printf("OK: %zu casos de CLI\n", cantidad);
    return EXIT_SUCCESS;
}
