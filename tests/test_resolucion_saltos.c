#include "sintactico.h"
#include "semantic.h"
#include "memory.h"
#include "generacion_adaptador.h"
#include "referencias_integracion.h"
#include "resolucion_saltos.h"
#include "enlace_funciones.h"
#include "modelo_control.h"

#include <stdlib.h>

#define RETORNO_FINAL 0xFFF0 /* ra inicial: fuera del programa. */

static int memoria[16384]; /* 64 KiB de datos */

/* Ejecuta la salida en bundles siguiendo los desplazamientos numericos,
 * desde la direccion entrada (programa ubicado desde 0).
 * Mismas reglas que modelo_control.h, pero el PC avanza por bundles: asi se
 * comprueba que cada salto resuelto llega al mismo lugar que su etiqueta.
 * Las llamadas usan las direcciones absolutas cargadas en x13. Termina
 * cuando se retorna a RETORNO_FINAL o en el salto estable "sye x0,0" del
 * arranque; la memoria queda en memoria[]. */
static int ejecutar_desde(const char *salida, uint32_t entrada)
{
    char *copia = malloc(strlen(salida) + 1);
    assert(copia);
    strcpy(copia, salida);
    char *bundles[4096];
    size_t cantidad = 0;
    for (char *p = strtok(copia, "\n"); p; p = strtok(NULL, "\n")) {
        if (*p == '#') continue;
        char *comentario = strstr(p, "  #");
        if (comentario) *comentario = '\0';
        assert(cantidad < 4096);
        bundles[cantidad++] = p;
    }
    int registros[32] = {0};
    memset(memoria, 0, sizeof(memoria));
    registros[2] = 512;
    registros[1] = RETORNO_FINAL;
    assert(entrada % 16 == 0);
    size_t pc = entrada / 16;
    for (size_t pasos = 0; pasos < 100000; ++pasos) {
        assert(pc < cantidad);
        char slots[4][128] = {{0}};
        if (strcmp(bundles[pc], "nop") != 0)
            assert(sscanf(bundles[pc], "%127[^|]| %127[^|]| %127[^|]| %127s",
                          slots[0], slots[1], slots[2], slots[3]) == 4);
        const char *instr = NULL;
        for (int s = 0; s < 4; ++s) {
            char *t = slots[s];
            size_t n = strlen(t);
            while (n && t[n - 1] == ' ') t[--n] = '\0';
            if (n && strcmp(t, "nop") != 0) { assert(!instr); instr = t; }
        }
        size_t siguiente = pc + 1;
        if (instr) {
            char op[16];
            unsigned a, b, d;
            int k;
            assert(sscanf(instr, "%15s", op) == 1);
            const char *args = instr + strlen(op);
            if (!strcmp(op, "syer")) {
                assert(sscanf(args, " x%u,x%u,%d", &a, &b, &k) == 3);
                /* El hardware pone en 0 los 4 bits bajos del destino. */
                unsigned destino = (unsigned)(registros[b] + k) & ~15u;
                if (a) registros[a] = (int)((pc + 1) * 16);
                if (destino == RETORNO_FINAL) {
                    assert(registros[2] == 512);
                    free(copia);
                    return registros[8];
                }
                siguiente = destino / 16;
            } else if (!strcmp(op, "sye")) {
                assert(sscanf(args, " x0,%d", &k) == 1 && k % 16 == 0);
                assert(k >= -32768 && k <= 32767);
                if (k == 0) {
                    free(copia);
                    return registros[8];
                }
                siguiente = (size_t)((long)pc + k / 16);
            } else if (!strcmp(op, "igualsi") || !strcmp(op, "igualno") ||
                       !strcmp(op, "menora") || !strcmp(op, "mayoroigual")) {
                assert(sscanf(args, " x%u,x%u,%d", &a, &b, &k) == 3 && k % 16 == 0);
                assert(k >= -1024 && k <= 1023);
                int x = registros[a], y = registros[b];
                int salta = !strcmp(op, "igualsi") ? x == y :
                            !strcmp(op, "igualno") ? x != y :
                            !strcmp(op, "menora") ? x < y : x >= y;
                if (salta) siguiente = (size_t)((long)pc + k / 16);
            } else if (!strcmp(op, "cargai") || !strcmp(op, "guardap")) {
                assert(sscanf(args, " x%u,x%u,%d", &a, &b, &k) == 3);
                int direccion = registros[op[0] == 'c' ? b : a] + k;
                assert(direccion >= 0 && direccion < 65536 && direccion % 4 == 0);
                if (op[0] == 'c') registros[a] = memoria[direccion / 4];
                else memoria[direccion / 4] = registros[b];
            } else if (sscanf(args, " x%u,x%u,x%u", &a, &b, &d) == 3) {
                int x = registros[b], y = registros[d];
                registros[a] = !strcmp(op, "suma") ? x + y :
                               !strcmp(op, "resta") ? x - y :
                               !strcmp(op, "mrq") ? x < y :
                               !strcmp(op, "myq") ? x > y :
                               !strcmp(op, "and") ? (x & y) :
                               !strcmp(op, "or") ? (x | y) :
                               !strcmp(op, "xor") ? (x ^ y) : (assert(!"op"), 0);
            } else {
                assert(sscanf(args, " x%u,x%u,%d", &a, &b, &k) == 3);
                int x = registros[b];
                registros[a] = !strcmp(op, "sumai") ? x + k :
                               !strcmp(op, "restai") ? x - k :
                               !strcmp(op, "xori") ? (x ^ k) :
                               !strcmp(op, "ori") ? (x | k) :
                               !strcmp(op, "andi") ? (x & k) :
                               !strcmp(op, "cizqi") ? (int)((unsigned)x << (k & 31)) :
                               !strcmp(op, "cderi") ? (int)((unsigned)x >> (k & 31)) :
                               (assert(!"opi"), 0);
            }
        }
        registros[0] = 0;
        pc = siguiente;
    }
    assert(!"la funcion no retorno");
    return -1;
}

static int ejecutar_bundles(const char *salida)
{
    return ejecutar_desde(salida, 0);
}

/* Cada salto debe llegar exactamente a su etiqueta y respetar su rango. */
static size_t verificar_saltos(const char *salida, uint32_t inicio)
{
    char nombres[512][96];
    uint32_t pcs[512];
    size_t etiquetas = 0, saltos = 0;
    uint32_t pc = inicio;
    for (const char *l = salida; *l; l = strchr(l, '\n') + 1) {
        if (l[0] == '#') {
            assert(etiquetas < 512);
            assert(sscanf(l, "# %95[^:]:", nombres[etiquetas]) == 1);
            pcs[etiquetas++] = pc;
        } else pc += 16;
    }
    pc = inicio;
    for (const char *l = salida; *l; l = strchr(l, '\n') + 1) {
        if (l[0] == '#') continue;
        char linea[512];
        size_t largo = (size_t)(strchr(l, '\n') - l);
        assert(largo < sizeof(linea));
        memcpy(linea, l, largo);
        linea[largo] = '\0';
        const char *flecha = strstr(linea, "# -> ");
        if (flecha) {
            char destino[96];
            assert(sscanf(flecha, "# -> %95s", destino) == 1);
            const char *coma = flecha;
            while (*coma != ',') --coma;
            long d = strtol(coma + 1, NULL, 10);
            size_t i = 0;
            while (i < etiquetas && strcmp(nombres[i], destino)) ++i;
            assert(i < etiquetas);
            assert((long)pc + d == (long)pcs[i]);
            int condicional = strstr(linea, "| sye ") == NULL;
            assert(condicional ? (d >= -1024 && d <= 1008) : (d >= -32768 && d <= 32752));
            ++saltos;
        }
        pc += 16;
    }
    return saltos;
}

static char *resolver(const char *lineal, uint32_t inicio, RsResumen *r)
{
    FILE *f = tmpfile();
    assert(f);
    assert(rs_resolver(lineal, inicio, f, stderr, r) == 0);
    long n = ftell(f);
    rewind(f);
    char *texto = malloc((size_t)n + 1);
    assert(texto && fread(texto, 1, (size_t)n, f) == (size_t)n);
    texto[n] = '\0';
    fclose(f);
    assert(verificar_saltos(texto, inicio) == r->saltos);
    return texto;
}

static void generar(const char *fuente, char *texto, size_t capacidad)
{
    Ast ast = {0}; SimTabla tabla = {0}; MemProyectoMapa mapa;
    assert(construir_ast(fuente, strlen(fuente), "saltos.bal", stderr, &ast) == 0);
    assert(sim_inicializar(&tabla) == SIM_OK);
    assert(analizar_semantica_modo(&ast, &tabla, "saltos.bal", stderr,
                                 SEM_MODULO_IMPORTADO) == 0);
    assert(mem_asignar_proyecto(&tabla, 0x4000, 65536, 32768, &mapa) == MEM_PROYECTO_OK);
    FILE *f = tmpfile(); assert(f);
    assert(gd_generar_funcion(ast.raiz->primer_hijo, &tabla, mapa.marco_maximo_bytes,
                             NULL, NULL, f, stderr) == 0);
    rewind(f);
    size_t n = fread(texto, 1, capacidad - 1, f);
    assert(!ferror(f) && feof(f)); texto[n] = 0;
    fclose(f); sim_liberar(&tabla); ast_liberar(&ast);
}

/* Linea y bundles deben dar el mismo resultado. */
static char *comparar(const char *fuente, int esperado, RsResumen *r)
{
    static char lineal[16384];
    char copia[16384];
    generar(fuente, lineal, sizeof(lineal));
    strcpy(copia, lineal);
    assert(ejecutar_control(copia) == esperado);
    char *bundles = resolver(lineal, 0, r);
    assert(ejecutar_bundles(bundles) == esperado);
    return bundles;
}

static void caso_manual(void)
{
    RsResumen r;
    char *s = resolver("L:\n  sumai x4, x0, 1\n  igualsi x4, x0, L\nsye x0, F\nF:\n", 0, &r);
    assert(!strcmp(s,
        "# L:\n"
        "sumai x4,x0,1 | nop | nop | nop\nnop\nnop\n"
        "nop | nop | igualsi x4,x0,-48 | nop  # -> L\nnop\nnop\n"
        "nop | nop | sye x0,48 | nop  # -> F\nnop\nnop\n"
        "# F:\n"
        "nop\n"));
    assert(r.bundles == 10 && r.saltos == 2 && r.relajados == 0);
    free(s);

    /* Destinos numericos, syer y lineas de comentario pasan sin cambios. */
    s = resolver("# comentario\nigualsi x4, x0, 32\nsyer x0, x1, 0\n", 0x100, &r);
    assert(strstr(s, "nop | nop | igualsi x4,x0,32 | nop\n"));
    assert(strstr(s, "nop | nop | syer x0,x1,0 | nop\n"));
    assert(r.saltos == 0 && r.bundles == 6);
    free(s);
}

static void caso_errores(void)
{
    const char *invalidos[] = {
        "sye x0, nada\n",              /* etiqueta no definida */
        "A:\nnop\nA:\n",               /* etiqueta duplicada */
        "foo x1, x2\n",                /* instruccion desconocida */
        "igualsi x4, L\nL:\n",         /* faltan operandos */
        "1abc:\n",                     /* nombre de etiqueta invalido */
    };
    for (size_t i = 0; i < sizeof(invalidos) / sizeof(*invalidos); ++i) {
        FILE *f = tmpfile(); assert(f);
        assert(rs_resolver(invalidos[i], 0, f, NULL, NULL) == 1);
        assert(ftell(f) == 0);
        fclose(f);
    }
    FILE *f = tmpfile(); assert(f);
    assert(rs_resolver("nop\n", 8, f, NULL, NULL) == 1);   /* PC no alineado */
    assert(rs_resolver("nop\n", 0x3FF0, f, NULL, NULL) == 1); /* vector reservado */
    fclose(f);

    /* 342 instrucciones * 3 bundles no caben en 1023 bundles. */
    size_t largo = 342 * 16 + 1;
    char *grande = malloc(largo);
    assert(grande);
    grande[0] = '\0';
    for (int i = 0; i < 342; ++i) strcat(grande, "sumai x4, x4, 1\n");
    f = tmpfile(); assert(f);
    assert(rs_resolver(grande, 0, f, NULL, NULL) == 1);
    fclose(f);
    free(grande);
}

static void caso_anidados(void)
{
    /* cycle dentro de cycle con whether/alif/also, y whale con whether.
     * i<j: 3*1, i==j: 3*10, i>j: 3*100; el whale suma 1000 una vez. */
    RsResumen r;
    char *s = comparar(
        "create_funk #declare_int# f() { r * declare_int : 0; n * declare_int : 3;"
        "cycle i let 0 until n { cycle j let 0 until n {"
        "whether (i < j) { r : r gauss 1; } alif (i == j) { r : r gauss 10; }"
        "also { r : r gauss 100; } } endgame; } endgame;"
        "whale (n > 0) { n : n neumann 1;"
        "whether (n == 1) { r : r gauss 1000; } } stop; give r; }", 1333, &r);
    /* Con 2 nop por instruccion un condicional alcanza ~21 instrucciones:
     * las salidas de los cycle anidados ya necesitan la forma larga. */
    assert(r.relajados >= 1 && r.saltos >= 11);
    free(s);
}

static void caso_relajacion(void)
{
    /* Cuerpos largos: los saltos de salida del whale y del whether no
     * alcanzan con 11 bits y se reescriben; el resultado no cambia.
     * i=0: 12 + 24; i=1 e i=2: 12 + 100 cada uno; cycle: 0 + 1. */
    char fuente[4096] = "create_funk #declare_int# f() { r * declare_int : 0;"
                        "i * declare_int : 0; whale (i < 3) {";
    for (int k = 0; k < 12; ++k) strcat(fuente, "r : r gauss 1;");
    strcat(fuente, "whether (i < 1) {");
    for (int k = 0; k < 12; ++k) strcat(fuente, "r : r gauss 2;");
    strcat(fuente, "} also { r : r gauss 100; } i : i gauss 1; } stop;"
                   "cycle k let 0 until 2 { r : r gauss k; } endgame; give r; }");
    RsResumen r;
    char *s = comparar(fuente, 36 + 112 + 112 + 1, &r);
    assert(r.relajados == 2);
    assert(strstr(s, "# __rs_cerca_0:\n") && strstr(s, "# __rs_cerca_1:\n"));
    assert(strstr(s, "igualno x4,x0,96 | nop  # -> __rs_cerca_"));
    free(s);
}

/* El whale de acumular en el ejercicio 3 es el caso que no alcanzaba. */
static void caso_ejercicio3(int emitir)
{
    FILE *entrada = fopen("examples/ejercicio3_puntajes.bal", "r");
    assert(entrada);
    char fuente[8192];
    size_t n = fread(fuente, 1, sizeof(fuente) - 1, entrada);
    assert(!ferror(entrada) && feof(entrada)); fuente[n] = 0;
    fclose(entrada);
    Ast ast = {0}; SimTabla tabla = {0}; MemProyectoMapa mapa;
    assert(construir_ast(fuente, n, "ejercicio3", stderr, &ast) == 0);
    assert(sim_inicializar(&tabla) == SIM_OK);
    assert(analizar_semantica(&ast, &tabla, "ejercicio3", stderr) == 0);
    assert(mem_asignar_proyecto(&tabla, 0x4000, 65536, 32768, &mapa) == MEM_PROYECTO_OK);
    RrTabla destinos; rr_inicializar(&destinos);
    unsigned direccion = 16; /* Direcciones de prueba: las llamadas no se resuelven aqui. */
    for (const AstNodo *f = ast.raiz->primer_hijo; f; f = f->siguiente, direccion += 16)
        assert(rr_definir(&destinos, f->primer_hijo->texto, RR_FUNCION, direccion) == RR_OK);
    FILE *f = tmpfile(); assert(f);
    for (const AstNodo *fn = ast.raiz->primer_hijo; fn; fn = fn->siguiente)
        assert(ri_generar_funcion(fn, &tabla, mapa.marco_maximo_bytes, &destinos,
                                  f, stderr) == 0);
    long largo = ftell(f);
    rewind(f);
    char *lineal = malloc((size_t)largo + 1);
    assert(lineal && fread(lineal, 1, (size_t)largo, f) == (size_t)largo);
    lineal[largo] = '\0';
    fclose(f);
    RsResumen r;
    char *s = resolver(lineal, 0, &r);
    assert(r.relajados == 1);
    assert(strstr(s, "# -> __gf_acumular_while_fin_0"));
    if (emitir) assert(fputs(s, stdout) >= 0);
    free(s); free(lineal);
    rr_liberar(&destinos); sim_liberar(&tabla); ast_liberar(&ast);
}

/* Ejercicio 3 completo con direcciones reales: main -> acumular -> puntaje
 * y sumar. resultado = 24, vacio = 0, combinado = 13, total = 37. */
static void caso_enlace_ejercicio3(int emitir)
{
    FILE *entrada = fopen("examples/ejercicio3_puntajes.bal", "r");
    assert(entrada);
    char fuente[8192];
    size_t n = fread(fuente, 1, sizeof(fuente) - 1, entrada);
    assert(!ferror(entrada) && feof(entrada)); fuente[n] = 0;
    fclose(entrada);
    Ast ast = {0}; SimTabla tabla = {0}; MemProyectoMapa mapa;
    assert(construir_ast(fuente, n, "ejercicio3", stderr, &ast) == 0);
    assert(sim_inicializar(&tabla) == SIM_OK);
    assert(analizar_semantica(&ast, &tabla, "ejercicio3", stderr) == 0);
    assert(mem_asignar_proyecto(&tabla, 0x4000, 65536, 32768, &mapa) == MEM_PROYECTO_OK);

    RrTabla funciones; rr_inicializar(&funciones);
    EfResumen r;
    FILE *f = tmpfile(); assert(f);
    assert(ef_enlazar(&ast, &tabla, 0, NULL, &funciones, f, stderr, &r) == 0);
    long largo = ftell(f);
    rewind(f);
    char *s = malloc((size_t)largo + 1);
    assert(s && fread(s, 1, (size_t)largo, f) == (size_t)largo);
    s[largo] = '\0';
    fclose(f);
    assert(r.funciones == 4 && r.vueltas >= 2 && r.saltos.relajados >= 1);
    assert(verificar_saltos(s, 0) == r.saltos.saltos);

    /* Cada funcion empieza en su etiqueta y en orden del fuente. */
    const char *nombres[] = {"sumar", "puntaje", "acumular", "main"};
    uint32_t anterior = 0;
    for (int i = 0; i < 4; ++i) {
        uint32_t d;
        char etiqueta[64];
        assert(rr_buscar(&funciones, nombres[i], RR_FUNCION, &d) == RR_OK);
        assert(i == 0 ? d == 0 : d > anterior);
        snprintf(etiqueta, sizeof(etiqueta), "# __fn_%s:\n", nombres[i]);
        assert(strstr(s, etiqueta));
        anterior = d;
    }
    /* main empieza despues de 1023 bytes: su constante usa cizqi/ori. */
    assert(anterior > 1023 && strstr(s, "cizqi x13,x13,8"));

    /* Ejecutar desde main. Locales en sp+4..sp+16 de un marco de 20 bytes. */
    ejecutar_desde(s, anterior);
    int base = (512 - 20) / 4;
    assert(memoria[base + 1] == 24 && memoria[base + 2] == 0);
    assert(memoria[base + 3] == 13 && memoria[base + 4] == 37);
    if (emitir) assert(fputs(s, stdout) >= 0);
    free(s);

    /* Sin funciones no hay nada que enlazar; tampoco escribe salida. */
    Ast vacio = {0};
    f = tmpfile(); assert(f);
    assert(ef_enlazar(&vacio, &tabla, 0, NULL, NULL, f, NULL, NULL) == 1);
    assert(ftell(f) == 0);
    fclose(f);
    rr_liberar(&funciones); sim_liberar(&tabla); ast_liberar(&ast);
}

static char *enlazar(const char *fuente, const EfArranque *arranque, EfResumen *r)
{
    Ast ast = {0}; SimTabla tabla = {0}; MemProyectoMapa mapa;
    assert(construir_ast(fuente, strlen(fuente), "arranque.bal", stderr, &ast) == 0);
    assert(sim_inicializar(&tabla) == SIM_OK);
    assert(analizar_semantica(&ast, &tabla, "arranque.bal", stderr) == 0);
    assert(mem_asignar_proyecto(&tabla, 0x4000, 65536, 32768, &mapa) == MEM_PROYECTO_OK);
    assert(mapa.globales.gp_base == arranque->gp);
    FILE *f = tmpfile(); assert(f);
    assert(ef_enlazar(&ast, &tabla, 0, arranque, NULL, f, stderr, r) == 0);
    long largo = ftell(f);
    rewind(f);
    char *s = malloc((size_t)largo + 1);
    assert(s && fread(s, 1, (size_t)largo, f) == (size_t)largo);
    s[largo] = '\0';
    fclose(f);
    assert(verificar_saltos(s, 0) == r->saltos.saltos);
    sim_liberar(&tabla); ast_liberar(&ast);
    return s;
}

/* El arranque fija gp y sp, inicializa constantes globales, llama a main
 * (que no esta primero) y termina en un salto estable. */
static void caso_arranque(int emitir)
{
    const EfArranque arranque = {0x4000, 0x10000};
    EfResumen r;
    char *s = enlazar(
        "declare_const base * declare_int : 5;"
        "declare_const doble * declare_int : base gauss base;"
        "create_funk #declare_int# f(n * declare_int) { give n gauss doble; }"
        "create_funk #declare_infinite_void# main() { r * declare_int : f(base); }",
        &arranque, &r);
    assert(strstr(s, "# __arranque:\nsumai x3,x0,0 | nop | nop | nop\n"));
    assert(strstr(s, "nop | nop | sye x0,0 | nop  # -> __arranque_fin\n"));
    ejecutar_desde(s, 0);
    /* Globales desde gp = 0x4000; r en sp+4 de un marco de 8 bajo 0x10000. */
    assert(memoria[0x4000 / 4] == 5 && memoria[0x4004 / 4] == 10);
    assert(memoria[(0x10000 - 8 + 4) / 4] == 15);
    free(s);

    /* Ejercicio 3 desde su arranque: los locales de main quedan bajo 0x10000. */
    FILE *entrada = fopen("examples/ejercicio3_puntajes.bal", "r");
    assert(entrada);
    char fuente[8192];
    size_t n = fread(fuente, 1, sizeof(fuente) - 1, entrada);
    assert(!ferror(entrada) && feof(entrada)); fuente[n] = 0;
    fclose(entrada);
    s = enlazar(fuente, &arranque, &r);
    ejecutar_desde(s, 0);
    int base = (0x10000 - 20) / 4;
    assert(memoria[base + 1] == 24 && memoria[base + 2] == 0);
    assert(memoria[base + 3] == 13 && memoria[base + 4] == 37);
    if (emitir) assert(fputs(s, stdout) >= 0);
    free(s);

    /* Ejercicio 1, factorial de 5 con la multiplicacion del generador basico.
     * n, resultado, i en sp+4, sp+8, sp+12 de un marco de 16. */
    entrada = fopen("examples/factorial.bal", "r");
    assert(entrada);
    n = fread(fuente, 1, sizeof(fuente) - 1, entrada);
    assert(!ferror(entrada) && feof(entrada)); fuente[n] = 0;
    fclose(entrada);
    s = enlazar(fuente, &arranque, &r);
    ejecutar_desde(s, 0);
    base = (0x10000 - 16) / 4;
    assert(memoria[base + 2] == 120 && memoria[base + 3] == 6);
    free(s);
}

/* --emitir imprime los bundles del ejercicio 3 en lugar del resumen. */
int main(int argc, char **argv)
{
    int emitir = argc > 1 && !strcmp(argv[1], "--emitir");
    caso_manual();
    caso_errores();
    caso_anidados();
    caso_relajacion();
    caso_ejercicio3(0);
    caso_enlace_ejercicio3(0);
    caso_arranque(emitir);
    if (!emitir) puts("OK: layout de bundles, saltos anidados, saltos largos y direcciones de funciones y arranque");
    return 0;
}
