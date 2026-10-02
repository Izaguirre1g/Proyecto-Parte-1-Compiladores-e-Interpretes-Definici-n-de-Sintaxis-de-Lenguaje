/* Pruebas de integracion: archivo -> balc -> lexer -> Bison.
 * Usa procesos y archivos temporales POSIX (Ubuntu/WSL), sin Python.
 * Ejecutar desde la raiz del proyecto con make test-parser.
 */
#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

typedef struct {
    const char *nombre;
    const char *fuente;
    size_t longitud;
    int codigo;
    const char *mensaje;
} Caso;

/* sizeof incluye los bytes posteriores a un NUL incrustado; strlen no. */
#define CASO(nombre, fuente, codigo, mensaje) \
    {nombre, fuente, sizeof(fuente) - 1, codigo, mensaje}

static const Caso casos[] = {
    CASO("bloque vacio", "create_funk #declare_infinite_void# main() {\n\n}\n", 0, ""),
    CASO("tipos y literales", "create_funk #declare_infinite_void# main() {\nn * declare_int : 1; t * declare_text : \"café\"; c * declare_char : $ñ$; b * declare_boolean : declare_true;\n}\n", 0, ""),
    CASO("aritmetica y comparacion", "create_funk #declare_infinite_void# main() {\nn * declare_int : neumann (2 gauss 3) pitagoras 4 euclides 2; b * declare_boolean : n =/= 0;\n}\n", 0, ""),
    CASO("operadores de comparacion", "create_funk #declare_infinite_void# main() {\nb * declare_boolean : 1 == 1; b : 1 < 2; b : 2 > 1; b : 1 <= 2; b : 2 >= 1;\n}\n", 0, ""),
    CASO("anidamiento", "create_funk #declare_infinite_void# main() {\nwhether (n <= 1) { whale (i < n) { whether (i == 2) {} also { i : i gauss 1; } } stop; } also {}\n}\n", 0, ""),
    CASO("varias funciones", "create_funk #declare_infinite_void# auxiliar() {}\ncreate_funk #declare_infinite_void# main() {\n\n}\n", 0, ""),
    CASO("un parametro", "create_funk #declare_infinite_void# procesar(n * declare_int) { n : n gauss 1; }\ncreate_funk #declare_infinite_void# main() {\n\n}\n", 0, ""),
    CASO("varios parametros", "create_funk #declare_infinite_void# procesar(n * declare_int, t * declare_text, c * declare_char, b * declare_boolean) { whether (b) { n : n gauss 1; } }\ncreate_funk #declare_infinite_void# main() {\n\n}\n", 0, ""),
    CASO("parametro sin tipo", "create_funk #declare_int# f(a *) {}", 1, "error del parser"),
    CASO("parametro sin nombre", "create_funk #declare_int# f(* declare_int) {}", 1, "error del parser"),
    CASO("parametro sin estrella", "create_funk #declare_int# f(a declare_int) {}", 1, "error del parser"),
    CASO("parametro void", "create_funk #declare_int# f(a * declare_infinite_void) {}", 1, "error del parser"),
    CASO("parametro inicializado", "create_funk #declare_int# f(a * declare_int : 1) {}", 1, "error del parser"),
    CASO("coma inicial", "create_funk #declare_int# f(,a * declare_int) {}", 1, "error del parser"),
    CASO("coma final", "create_funk #declare_int# f(a * declare_int,) {}", 1, "error del parser"),
    CASO("falta coma", "create_funk #declare_int# f(a * declare_int b * declare_int) {}", 1, "error del parser"),
    CASO("comentarios", "create_funk #declare_infinite_void# main() {\n%% comentario\n%%// bloque //%%\nn * declare_int : 5;\n}\n", 0, ""),
    CASO("falta punto y coma", "create_funk #declare_infinite_void# main() {\nn * declare_int : 5\n}\n", 1, "error del parser"),
    CASO("falta stop", "create_funk #declare_infinite_void# main() {\nwhale (n > 0) {}\n}\n", 1, "error del parser"),
    CASO("also suelto", "create_funk #declare_infinite_void# main() {\nalso {}\n}\n", 1, "error del parser"),
    CASO("comparacion encadenada", "create_funk #declare_infinite_void# main() {\nb * declare_boolean : 1 < 2 < 3;\n}\n", 1, "error del parser"),
    CASO("falta llave", "create_funk #declare_infinite_void# main() {", 1, "error del parser"),
    CASO("vacio", "", 1, "error del parser"),
    CASO("sentencia fuera de funcion", "n * declare_int : 5;", 1, "error del parser"),
    CASO("retorno con valor", "create_funk #declare_int# uno() { give 1; }", 0, ""),
    CASO("retorno sin valor", "create_funk #declare_infinite_void# main() { give; }", 0, ""),
    CASO("retorno de expresion", "create_funk #declare_int# sumar(a * declare_int, b * declare_int) { give a gauss b; }", 0, ""),
    CASO("retornos en ramas", "create_funk #declare_int# f(n * declare_int) { whether (n <= 1) { give 1; } also { give n pitagoras 2; } }", 0, ""),
    CASO("retorno dentro de ciclo", "create_funk #declare_int# f(n * declare_int) { whale (n > 0) { give n; } stop; give 0; }", 0, ""),
    CASO("retorno sin punto y coma", "create_funk #declare_int# f() { give 1 }", 1, "error del parser"),
    CASO("retorno vacio sin punto y coma", "create_funk #declare_infinite_void# main() { give }", 1, "error del parser"),
    CASO("retorno con expresion incompleta", "create_funk #declare_int# f() { give 1 gauss; }", 1, "error del parser"),
    CASO("retorno fuera de funcion", "give 1;", 1, "error del parser"),
    CASO("retorno no es expresion", "create_funk #declare_int# f() { n * declare_int : give 1; }", 1, "error del parser"),
    CASO("llamada sin argumentos", "create_funk #declare_infinite_void# saludar() { give; } create_funk #declare_infinite_void# main() { saludar(); }", 0, ""),
    CASO("llamada con argumentos", "create_funk #declare_int# sumar(a * declare_int, b * declare_int) { give a gauss b; } create_funk #declare_infinite_void# main() { resultado * declare_int : sumar(2, 3); resultado : sumar(resultado, 1); }", 0, ""),
    CASO("llamadas anidadas y aritmetica", "create_funk #declare_int# f(a * declare_int) { give a; } create_funk #declare_infinite_void# main() { n * declare_int : f(f(2 gauss 3)) pitagoras f(4); }", 0, ""),
    CASO("llamadas en retorno y condiciones", "create_funk #declare_int# f(n * declare_int) { whether (f(n) > 0) { give f(n neumann 1); } whale (f(n) <= 2) { f(n); } stop; give 0; }", 0, ""),
    CASO("argumentos de varios tipos", "create_funk #declare_infinite_void# f(n * declare_int, t * declare_text, c * declare_char, b * declare_boolean) {} create_funk #declare_infinite_void# main() { f(1, \"hola\", $ñ$, 2 < 3); }", 0, ""),
    CASO("llamada coma inicial", "create_funk #declare_infinite_void# main() { f(,1); }", 1, "error del parser"),
    CASO("llamada coma final", "create_funk #declare_infinite_void# main() { f(1,); }", 1, "error del parser"),
    CASO("llamada argumento ausente", "create_funk #declare_infinite_void# main() { f(1,,2); }", 1, "error del parser"),
    CASO("llamada falta coma", "create_funk #declare_infinite_void# main() { f(1 2); }", 1, "error del parser"),
    CASO("llamada falta parentesis", "create_funk #declare_infinite_void# main() { f(1; }", 1, "error del parser"),
    CASO("llamada falta punto y coma", "create_funk #declare_infinite_void# main() { f() }", 1, "error del parser"),
    CASO("llamada fuera de funcion", "f();", 1, "error del parser"),
    CASO("argumento expresion incompleta", "create_funk #declare_infinite_void# main() { f(1 gauss); }", 1, "error del parser"),
    CASO("argumento no es parametro", "create_funk #declare_infinite_void# main() { f(a * declare_int); }", 1, "error del parser"),
    CASO("alif sin also", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif (n == 0) {} }", 0, ""),
    CASO("alif con also", "create_funk #declare_infinite_void# main() { whether (n < 0) { give 1; } alif (n == 0) { give 2; } also { give 3; } }", 0, ""),
    CASO("varios alif", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif (n == 0) {} alif (n < 10) {} alif (n < 20) {} }", 0, ""),
    CASO("varios alif con also", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif (n == 0) {} alif (n < 10) {} also {} }", 0, ""),
    CASO("alif anidado", "create_funk #declare_infinite_void# main() { whether (n < 0) { whether (m < 0) {} alif (m == 0) {} also {} } alif (n == 0) { whether (m > 0) {} alif (m == 0) {} } also {} }", 0, ""),
    CASO("alif dentro de ciclo", "create_funk #declare_infinite_void# main() { whale (n > 0) { whether (n == 1) {} alif (n == 2) { n : n neumann 1; } also {} } stop; }", 0, ""),
    CASO("alif con llamada y aritmetica", "create_funk #declare_infinite_void# main() { whether (f(n) < 0) {} alif (f(n gauss 1) == 0) { f(n); } also {} }", 0, ""),
    CASO("alif suelto", "create_funk #declare_infinite_void# main() { alif (n == 0) {} }", 1, "error del parser"),
    CASO("alif despues de also", "create_funk #declare_infinite_void# main() { whether (n < 0) {} also {} alif (n == 0) {} }", 1, "error del parser"),
    CASO("also duplicado", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif (n == 0) {} also {} also {} }", 1, "error del parser"),
    CASO("alif sin condicion", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif () {} }", 1, "error del parser"),
    CASO("alif sin parentesis", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif n == 0 {} }", 1, "error del parser"),
    CASO("alif sin bloque", "create_funk #declare_infinite_void# main() { whether (n < 0) {} alif (n == 0) give; }", 1, "error del parser"),
    CASO("alif separado por sentencia", "create_funk #declare_infinite_void# main() { whether (n < 0) {} n : 1; alif (n == 0) {} }", 1, "error del parser"),
    CASO("alif con punto y coma previo", "create_funk #declare_infinite_void# main() { whether (n < 0) {}; alif (n == 0) {} }", 1, "error del parser"),
    CASO("declaracion sin inicializar", "create_funk #declare_infinite_void# main() { n * declare_int; n : 5; }", 0, ""),
    CASO("declaraciones multiples", "create_funk #declare_infinite_void# main() { a * declare_int, b * declare_int : 2; }", 0, ""),
    CASO("tipos sin inicializar", "create_funk #declare_infinite_void# main() { b * declare_boolean; t * declare_text; c * declare_char; }", 0, ""),
    CASO("inicializacion incompleta", "create_funk #declare_infinite_void# main() { n * declare_int : ; }", 1, "error del parser"),
    CASO("declaracion sin punto y coma", "create_funk #declare_infinite_void# main() { n * declare_int }", 1, "error del parser"),
    CASO("declaracion coma final", "create_funk #declare_infinite_void# main() { a * declare_int,; }", 1, "error del parser"),
    CASO("logica completa", "create_funk #declare_infinite_void# main() { b * declare_boolean : ~declare_false && declare_true || declare_false ^ declare_true; }", 0, ""),
    CASO("condicion compuesta", "create_funk #declare_infinite_void# main() { whether (edad >= 18 && edad < 65) {} alif (~(edad == 0) || edad > 90) {} }", 0, ""),
    CASO("residuo y potencia", "create_funk #declare_infinite_void# main() { n * declare_int : 2 descartes 3 descartes 2 euler 5; n : neumann 2 descartes 2; }", 0, ""),
    CASO("and sin operando", "create_funk #declare_infinite_void# main() { b * declare_boolean : declare_true &&; }", 1, "error del parser"),
    CASO("not sin operando", "create_funk #declare_infinite_void# main() { b * declare_boolean : ~; }", 1, "error del parser"),
    CASO("potencia sin operando", "create_funk #declare_infinite_void# main() { n * declare_int : 2 descartes; }", 1, "error del parser"),
    CASO("cycle paso por defecto", "create_funk #declare_infinite_void# main() { cycle i let 0 until 5 {} endgame; }", 0, ""),
    CASO("cycle paso explicito", "create_funk #declare_infinite_void# main() { cycle i let 0 until 5 step 2 { n : i; } endgame; }", 0, ""),
    CASO("cycle descendente", "create_funk #declare_infinite_void# main() { cycle i let 5 until 0 step neumann 1 {} endgame; }", 0, ""),
    CASO("cycle con expresiones", "create_funk #declare_infinite_void# main() { cycle i let f(0) until limite gauss 1 step paso {} endgame; }", 0, ""),
    CASO("cycle anidado", "create_funk #declare_infinite_void# main() { cycle i let 0 until 2 { cycle j let 0 until 3 {} endgame; } endgame; }", 0, ""),
    CASO("cycle con parentesis rechazado", "create_funk #declare_infinite_void# main() { cycle(i let 0 until 5) {} endgame; }", 1, "error del parser"),
    CASO("cycle formato anterior rechazado", "create_funk #declare_infinite_void# main() { cycle(let i:0; until 5) {} endgame; }", 1, "error del parser"),
    CASO("cycle sin endgame", "create_funk #declare_infinite_void# main() { cycle i let 0 until 5 {} }", 1, "error del parser"),
    CASO("cycle sin punto y coma", "create_funk #declare_infinite_void# main() { cycle i let 0 until 5 {} endgame }", 1, "error del parser"),
    CASO("cycle step sin valor", "create_funk #declare_infinite_void# main() { cycle i let 0 until 5 step {} endgame; }", 1, "error del parser"),
    CASO("arreglo sin inicializar", "create_funk #declare_infinite_void# main() { a * declare_int[5]; a[0] : 2; n * declare_int : a[0]; }", 0, ""),
    CASO("matriz inicializada", "create_funk #declare_infinite_void# main() { a * declare_int[2][3] : [[1,2,3],[4,5,6]]; a[1][2] : a[0][0] gauss 1; }", 0, ""),
    CASO("dimensiones dinamicas", "create_funk #declare_infinite_void# main() { a * declare_int[filas][columnas gauss 1]; }", 0, ""),
    CASO("multiples matrices", "create_funk #declare_infinite_void# main() { a * declare_int[1][1] : [[1]], b * declare_int[1][1] : [[2]]; }", 0, ""),
    CASO("parametro arreglo", "create_funk #declare_int# f(a * declare_int[]) { give a[0]; }", 0, ""),
    CASO("parametro matriz", "create_funk #declare_int# f(a * declare_int[filas][cols], b * declare_int[][]) { give a[1][2] gauss b[0][0]; }", 0, ""),
    CASO("parametro lista", "create_funk #declare_int# f(a * declare_list declare_int) { give size(a); }", 0, ""),
    CASO("dimension vacia en variable", "create_funk #declare_infinite_void# main() { a * declare_int[]; }", 1, "error del parser"),
    CASO("indice vacio", "create_funk #declare_infinite_void# main() { a[] : 1; }", 1, "error del parser"),
    CASO("matriz falta corchete", "create_funk #declare_infinite_void# main() { a * declare_int[2][3; }", 1, "error del parser"),
    CASO("literal coma final", "create_funk #declare_infinite_void# main() { a * declare_int[2] : [1,]; }", 1, "error del parser"),
    CASO("literal falta separador", "create_funk #declare_infinite_void# main() { a * declare_int[2] : [1 2]; }", 1, "error del parser"),
    CASO("literal no asignable", "create_funk #declare_infinite_void# main() { [1,2] : 3; }", 1, "error del parser"),
    CASO("listas y operaciones", "create_funk #declare_infinite_void# main() { declare_list a * declare_int : [1,2], b * declare_int : []; add(a,3); remove(a,0); n * declare_int : size(a); }", 0, ""),
    CASO("lista sin inicializador", "create_funk #declare_infinite_void# main() { declare_list a * declare_text; }", 0, ""),
    CASO("lista y recorrido exclusivo", "create_funk #declare_infinite_void# main() { declare_list a * declare_int : [1,2]; cycle i let 0 until size(a) { a[i] : a[i] gauss 1; } endgame; }", 0, ""),
    CASO("add falta argumento", "create_funk #declare_infinite_void# main() { add(a); }", 1, "error del parser"),
    CASO("remove argumento extra", "create_funk #declare_infinite_void# main() { remove(a,1,2); }", 1, "error del parser"),
    CASO("size sin argumento", "create_funk #declare_infinite_void# main() { size(); }", 1, "error del parser"),
    CASO("size varios argumentos", "create_funk #declare_infinite_void# main() { size(a,b); }", 1, "error del parser"),
    CASO("lista sin tipo", "create_funk #declare_infinite_void# main() { declare_list a : []; }", 1, "error del parser"),
    CASO("importacion con alias", "bring matematicas aka math; create_funk #declare_infinite_void# main() { n * declare_int : math.suma(2,3); math.procesar(n); }", 0, ""),
    CASO("importacion sin alias", "bring matematicas; create_funk #declare_int# f() { give matematicas.suma(1,2); }", 0, ""),
    CASO("acceso modulo y matriz", "bring datos aka d; create_funk #declare_infinite_void# main() { d.matriz[0][1] : d.valor; }", 0, ""),
    CASO("alias ausente", "bring datos aka; create_funk #declare_infinite_void# main() {}", 1, "error del parser"),
    CASO("importacion sin punto y coma", "bring datos create_funk #declare_infinite_void# main() {}", 1, "error del parser"),
    CASO("importacion dentro de funcion", "create_funk #declare_infinite_void# main() { bring datos; }", 1, "error del parser"),
    CASO("miembro ausente", "create_funk #declare_infinite_void# main() { math.(1); }", 1, "error del parser"),
    CASO("constante local", "create_funk #declare_infinite_void# main() { declare_const limite * declare_int : 5; }", 0, ""),
    CASO("constante global", "declare_const limite * declare_int : 5; create_funk #declare_int# f() { give limite; }", 0, ""),
    CASO("constante sin valor", "create_funk #declare_infinite_void# main() { declare_const limite * declare_int; }", 1, "error del parser"),
    CASO("constante sin tipo", "create_funk #declare_infinite_void# main() { declare_const limite : 5; }", 1, "error del parser"),
    CASO("seek seize", "create_funk #declare_infinite_void# main() { seek { n : f(); } seize (ErrorType e) { n : 0; } }", 0, ""),
    CASO("varias capturas", "create_funk #declare_infinite_void# main() { seek {} seize (ErrorNumero e) {} seize (ErrorIndice otro) {} }", 0, ""),
    CASO("seek anidado", "create_funk #declare_infinite_void# main() { seek { seek {} seize (ErrorType interno) {} } seize (ErrorType externo) {} }", 0, ""),
    CASO("seek sin captura", "create_funk #declare_infinite_void# main() { seek {} }", 1, "error del parser"),
    CASO("seize sin seek", "create_funk #declare_infinite_void# main() { seize (ErrorType e) {} }", 1, "error del parser"),
    CASO("seize sin variable", "create_funk #declare_infinite_void# main() { seek {} seize (ErrorType) {} }", 1, "error del parser"),
    CASO("seize sin bloque", "create_funk #declare_infinite_void# main() { seek {} seize (ErrorType e) give; }", 1, "error del parser"),
    CASO("alif sin condicion sigue rechazado", "create_funk #declare_infinite_void# main() { whether (declare_true) {} alif {} }", 1, "error del parser"),
    CASO("declare_float sigue rechazado", "create_funk #declare_infinite_void# main() { n * declare_float : 1; }", 1, "error del parser"),
    CASO("token aun no soportado", "create_funk #declare_infinite_void# main() { show(); }", 1, "token SHOW aún no admitido"),
    CASO("error lexico", "create_funk #declare_infinite_void# main() {\n@\n}\n", 1, ":2:1: error léxico"),
    CASO("posicion tras Unicode", "create_funk #declare_infinite_void# main() {\nt * declare_text : \"ñ\"; @\n}\n", 1, ":2:25: error léxico"),
    CASO("decimal invalido", "create_funk #declare_infinite_void# main() {\nn * declare_int : 1.5;\n}\n", 1, "error léxico"),
    CASO("NUL", "create_funk #declare_infinite_void# main() {\n\000\n}\n", 1, "error léxico"),
    CASO("texto sobrante", "create_funk #declare_infinite_void# main() {\n\n}\nn", 1, "error del parser"),
};

typedef struct {
    int codigo;
    char salida[32768];
    char errores[32768];
} Resultado;

static void fallo_sistema(const char *operacion) {
    perror(operacion);
    exit(EXIT_FAILURE);
}

static void leer_salida(FILE *archivo, char *destino, size_t capacidad) {
    if (fseek(archivo, 0, SEEK_SET) != 0) fallo_sistema("fseek");
    size_t n = fread(destino, 1, capacidad - 1, archivo);
    if (ferror(archivo)) fallo_sistema("fread");
    destino[n] = '\0';
    if (n == capacidad - 1 && fgetc(archivo) != EOF) {
        fputs("La salida de la prueba excede el buffer\n", stderr);
        exit(EXIT_FAILURE);
    }
}

/* Ejecuta el binario real sin pasar comandos por un shell.
 * Los temporales capturan stdout/stderr sin bloquear el proceso hijo.
 */
static Resultado ejecutar(const char *archivo, int sintaxis, int verbose) {
    FILE *salida = tmpfile();
    FILE *errores = tmpfile();
    if (!salida || !errores) fallo_sistema("tmpfile");
    /* Vaciar la salida de las pruebas antes de crear el proceso hijo. */
    if (fflush(stdout) != 0) fallo_sistema("fflush");
    pid_t hijo = fork();
    if (hijo < 0) fallo_sistema("fork");
    if (hijo == 0) {
        if (dup2(fileno(salida), STDOUT_FILENO) < 0 ||
            dup2(fileno(errores), STDERR_FILENO) < 0) _exit(126);
        fclose(salida);
        fclose(errores);
        char *argumentos[5];
        int n = 0;
        argumentos[n++] = "./balc";
        if (verbose) argumentos[n++] = "-v";
        if (sintaxis) argumentos[n++] = "-t";
        argumentos[n++] = (char *)archivo;
        argumentos[n] = NULL;
        execv(argumentos[0], argumentos);
        perror("execv balc");
        _exit(127);
    }
    int estado;
    while (waitpid(hijo, &estado, 0) < 0) {
        if (errno != EINTR) fallo_sistema("waitpid");
    }
    Resultado r = {0};
    r.codigo = WIFEXITED(estado) ? WEXITSTATUS(estado) : -1;
    leer_salida(salida, r.salida, sizeof(r.salida));
    leer_salida(errores, r.errores, sizeof(r.errores));
    fclose(salida);
    fclose(errores);
    return r;
}

static void escribir(const char *ruta, const char *fuente, size_t longitud) {
    FILE *archivo = fopen(ruta, "wb");
    if (!archivo) fallo_sistema("fopen");
    if (fwrite(fuente, 1, longitud, archivo) != longitud) fallo_sistema("fwrite");
    if (fclose(archivo) != 0) fallo_sistema("fclose");
}

/* Mostrar incluso bytes invisibles, sin truncar el caso que contiene NUL. */
static void mostrar_fuente(const char *fuente, size_t longitud) {
    puts("Código de entrada:");
    if (!longitud) {
        puts("(archivo vacío)");
        return;
    }
    for (size_t i = 0; i < longitud; ++i) {
        unsigned char c = (unsigned char)fuente[i];
        if (c == 0) fputs("\\0", stdout);
        else if (c == '\r') fputs("\\r", stdout);
        else if (c < 0x20 && c != '\n' && c != '\t') printf("\\x%02X", c);
        else putchar(c);
    }
    if (fuente[longitud - 1] != '\n') putchar('\n');
}

static int comprobar(const char *nombre, const Resultado *r, int codigo,
                     const char *mensaje, int sintaxis) {
    int correcto = r->codigo == codigo && strstr(r->errores, mensaje) != NULL;
    if (sintaxis) {
        int anuncia_exito = strstr(r->salida, "Análisis sintáctico correcto") != NULL;
        correcto = correcto && anuncia_exito == (codigo == 0);
        int imprime_ast = strstr(r->salida, "[AST]\nPrograma") != NULL;
        correcto = correcto && imprime_ast == (codigo == 0);
    }
    printf("[%s] %s — código esperado: %d; obtenido: %d\n",
           correcto ? "OK" : "FALLO", nombre, codigo, r->codigo);
    if (codigo == 1) puts("Se esperaba rechazar la entrada: ese rechazo cuenta como prueba correcta.");
    if (*mensaje) printf("Diagnóstico esperado: %s\n", mensaje);
    if (*r->salida) printf("Salida:\n%s", r->salida);
    if (*r->errores) printf("Diagnósticos:\n%s", r->errores);
    putchar('\n');
    if (!correcto) {
        fprintf(stderr, "FALLO: %s (esperado %d, obtenido %d)\nstdout: %s\nstderr: %s\n",
                nombre, codigo, r->codigo, r->salida, r->errores);
    }
    return correcto ? 0 : 1;
}

int main(void) {
    char carpeta[] = "/tmp/test_parser_XXXXXX";
    if (!mkdtemp(carpeta)) fallo_sistema("mkdtemp");
    char entrada[256], inexistente[256];
    snprintf(entrada, sizeof(entrada), "%s/prueba.bal", carpeta);
    snprintf(inexistente, sizeof(inexistente), "%s/no_existe.bal", carpeta);
    int fallos = 0;
    size_t cantidad = sizeof(casos) / sizeof(casos[0]);
    for (size_t i = 0; i < cantidad; ++i) {
        const Caso *c = &casos[i];
        printf("\n--- Caso %zu/%zu: %s ---\n", i + 1, cantidad, c->nombre);
        mostrar_fuente(c->fuente, c->longitud);
        escribir(entrada, c->fuente, c->longitud);
        Resultado r = ejecutar(entrada, 1, 0);
        fallos += comprobar(c->nombre, &r, c->codigo, c->mensaje, 1);
    }

    /* Tokens validos no garantizan sintaxis valida. */
    const char fuente[] = "n * declare_int : ;";
    puts("\n--- Comparación entre modo léxico y sintáctico ---");
    mostrar_fuente(fuente, sizeof(fuente) - 1);
    escribir(entrada, fuente, sizeof(fuente) - 1);
    Resultado r = ejecutar(entrada, 0, 0);
    fallos += comprobar("modo lexico", &r, 0, "", 0);
    r = ejecutar(entrada, 1, 0);
    fallos += comprobar("modo sintactico", &r, 1, "", 1);
    puts("\n--- Archivo inexistente ---");
    r = ejecutar(inexistente, 1, 0);
    fallos += comprobar("archivo inexistente", &r, 2, "", 1);

    for (int verbose = 0; verbose <= 1; ++verbose) {
        printf("\n--- examples/factorial.bal (%s) ---\n", verbose ? "-v -t" : "-t");
        r = ejecutar("examples/factorial.bal", 1, verbose);
        fallos += comprobar("factorial", &r, 0, "", 1);
        if (verbose && !strstr(r.salida, "64 tokens, 0 errores")) {
            fputs("FALLO: recuento de tokens del factorial\n", stderr);
            ++fallos;
        }
    }
    if (remove(entrada) != 0) fallo_sistema("remove");
    if (rmdir(carpeta) != 0) fallo_sistema("rmdir");
    if (fallos) {
        fprintf(stderr, "%d comprobaciones fallaron\n", fallos);
        return EXIT_FAILURE;
    }
    printf("OK: %zu casos del parser, modos de CLI y factorial con/sin -v\n", cantidad);
    return EXIT_SUCCESS;
}
