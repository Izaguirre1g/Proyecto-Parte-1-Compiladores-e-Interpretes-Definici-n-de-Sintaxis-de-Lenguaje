#include "sintactico.h"
#include "semantic.h"
#include "memory.h"
#include "generacion_adaptador.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Desde BAL real hasta el ensamblador lineal que consume el backend.
 * --emitir permite inspeccionar la salida del ejemplo reproducible. */
int main(int argc, char **argv)
{
    assert(argc == 1 || (argc == 2 && !strcmp(argv[1], "--emitir")));
    FILE *entrada = fopen("examples/condicional_meibel.bal", "r");
    assert(entrada);
    char fuente[4096];
    size_t n = fread(fuente, 1, sizeof(fuente) - 1, entrada);
    assert(!ferror(entrada) && feof(entrada));
    fuente[n] = 0;
    fclose(entrada);
    Ast ast = {0};
    SimTabla tabla = {0};
    assert(construir_ast(fuente, n, "condicional_meibel.bal", stderr, &ast) == 0);
    assert(sim_inicializar(&tabla) == SIM_OK);
    assert(analizar_semantica(&ast, &tabla, "condicional_meibel.bal", stderr) == 0);
    MemProyectoMapa mapa;
    assert(mem_asignar_proyecto(&tabla, 0x4000, 65536, 32768, &mapa) == MEM_PROYECTO_OK);
    /* Solo hay una funcion: su marco coincide con el maximo del proyecto. */
    assert(mapa.funciones == 1);
    FILE *salida = tmpfile();
    assert(salida);
    assert(gd_generar_funcion(ast.raiz->primer_hijo, &tabla, mapa.marco_maximo_bytes,
                             NULL, NULL, salida, stderr) == 0);
    rewind(salida);
    char texto[8192];
    n = fread(texto, 1, sizeof(texto) - 1, salida);
    assert(!ferror(salida) && feof(salida));
    texto[n] = 0;
    assert(strstr(texto, "igualsi x4, x0, __gf_main_else_0\n"));
    assert(strstr(texto, "igualsi x4, x0, __gf_main_else_1\n"));
    assert(strstr(texto, "__gf_main_fin_0:\n"));
    assert(strstr(texto, "__gf_main_fin_1:\n"));
    assert(strstr(texto, "cargai x4, x2, 4\n"));
    assert(strstr(texto, "guardap x2, x4, 8\n"));
    if (argc == 2) assert(fputs(texto, stdout) >= 0);
    else puts("OK: BAL -> AST -> semantica -> memoria -> condicionales lineales");
    fclose(salida);
    sim_liberar(&tabla);
    ast_liberar(&ast);
    return 0;
}
