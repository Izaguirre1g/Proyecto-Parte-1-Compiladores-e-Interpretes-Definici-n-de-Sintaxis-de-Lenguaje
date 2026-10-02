/* Comprueba la estructura real del AST, no solo que el parser acepte el texto. */
#include "sintactico.h"

#include <stdlib.h>
#include <string.h>

#define COMPROBAR(condicion) do { \
    if (!(condicion)) { \
        fprintf(stderr, "FALLO AST, linea %d: %s\n", __LINE__, #condicion); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

static Ast analizar(const char *fuente) {
    Ast arbol = {0};
    COMPROBAR(construir_ast(fuente, strlen(fuente), "prueba.bal", stderr, &arbol) == 0);
    COMPROBAR(arbol.raiz && arbol.raiz->tipo == AST_PROGRAMA);
    return arbol;
}

static AstNodo *hijo(AstNodo *padre, size_t indice) {
    COMPROBAR(padre != NULL);
    AstNodo *nodo = padre->primer_hijo;
    while (nodo && indice--) nodo = nodo->siguiente;
    COMPROBAR(nodo != NULL);
    COMPROBAR(nodo->padre == padre);
    return nodo;
}

static void es(AstNodo *nodo, AstTipo tipo, const char *texto) {
    COMPROBAR(nodo && nodo->tipo == tipo);
    if (texto) COMPROBAR(nodo->texto && strcmp(nodo->texto, texto) == 0);
    else COMPROBAR(nodo->texto == NULL);
}

static AstNodo *cuerpo(Ast *arbol) { return hijo(hijo(arbol->raiz, 0), 3); }

static void precedencia(void) {
    Ast arbol = analizar("create_funk #declare_int# f() {"
        "give 2 gauss 3 pitagoras 4; give (2 gauss 3) pitagoras 4;"
        "give 10 neumann 3 neumann 2; give neumann 2 pitagoras 3;"
        "give 1 gauss 2 <= 4; }");
    AstNodo *bloque = cuerpo(&arbol);
    AstNodo *suma = hijo(hijo(bloque, 0), 0);
    es(suma, AST_BINARIO, "gauss");
    es(hijo(suma, 0), AST_ENTERO, "2");
    es(hijo(suma, 1), AST_BINARIO, "pitagoras");
    es(hijo(hijo(suma, 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(suma, 1), 1), AST_ENTERO, "4");
    AstNodo *producto = hijo(hijo(bloque, 1), 0);
    es(producto, AST_BINARIO, "pitagoras");
    es(hijo(producto, 0), AST_BINARIO, "gauss");
    es(hijo(producto, 1), AST_ENTERO, "4");
    AstNodo *resta = hijo(hijo(bloque, 2), 0);
    es(resta, AST_BINARIO, "neumann");
    es(hijo(resta, 0), AST_BINARIO, "neumann");
    es(hijo(resta, 1), AST_ENTERO, "2");
    AstNodo *negativo = hijo(hijo(bloque, 3), 0);
    es(negativo, AST_BINARIO, "pitagoras");
    es(hijo(negativo, 0), AST_UNARIO, "neumann");
    AstNodo *comparacion = hijo(hijo(bloque, 4), 0);
    es(comparacion, AST_BINARIO, "<=");
    es(hijo(comparacion, 0), AST_BINARIO, "gauss");
    ast_liberar(&arbol);
    puts("[OK AST] precedencia, parentesis, asociatividad y negacion");
}

static void funciones(void) {
    Ast arbol = analizar("create_funk #declare_int# sumar(a * declare_int, b * declare_int) {"
        "give a gauss b; } create_funk #declare_infinite_void# main() {"
        "x * declare_int : sumar(2, sumar(3, 4)); x : sumar(x, 5); saludar(); give; }");
    AstNodo *f = hijo(arbol.raiz, 0);
    es(f, AST_FUNCION, NULL);
    es(hijo(f, 0), AST_IDENTIFICADOR, "sumar");
    es(hijo(f, 1), AST_TIPO, "declare_int");
    AstNodo *params = hijo(f, 2);
    es(params, AST_PARAMETROS, NULL);
    es(hijo(hijo(params, 0), 0), AST_IDENTIFICADOR, "a");
    es(hijo(hijo(params, 1), 0), AST_IDENTIFICADOR, "b");
    COMPROBAR(hijo(params, 1)->siguiente == NULL);
    AstNodo *main = hijo(arbol.raiz, 1);
    COMPROBAR(hijo(main, 2)->primer_hijo == NULL);
    AstNodo *bloque = hijo(main, 3);
    AstNodo *decl = hijo(bloque, 0);
    es(decl, AST_DECLARACION, NULL);
    es(hijo(decl, 0), AST_IDENTIFICADOR, "x");
    es(hijo(decl, 1), AST_TIPO, "declare_int");
    AstNodo *llamada = hijo(decl, 2);
    es(llamada, AST_LLAMADA, NULL);
    es(hijo(llamada, 0), AST_IDENTIFICADOR, "sumar");
    AstNodo *args = hijo(llamada, 1);
    es(args, AST_ARGUMENTOS, NULL);
    es(hijo(args, 0), AST_ENTERO, "2");
    es(hijo(args, 1), AST_LLAMADA, NULL);
    es(hijo(hijo(hijo(args, 1), 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(hijo(args, 1), 1), 1), AST_ENTERO, "4");
    es(hijo(bloque, 1), AST_ASIGNACION, NULL);
    es(hijo(hijo(bloque, 1), 1), AST_LLAMADA, NULL);
    COMPROBAR(hijo(hijo(bloque, 2), 1)->primer_hijo == NULL);
    es(hijo(bloque, 3), AST_RETORNO, NULL);
    COMPROBAR(hijo(bloque, 3)->primer_hijo == NULL);
    ast_liberar(&arbol);
    puts("[OK AST] funciones, parametros, argumentos, llamadas anidadas y retornos");
}

static void control(void) {
    Ast arbol = analizar("create_funk #declare_int# f(n * declare_int) {"
        "whether (n < 0) { give 1; } alif (n == 0) { give 2; }"
        "alif (n < 10) { whether (n == 1) {} also {} } also { give 3; }"
        "whale (n > 0) { n : n neumann 1; } stop; whether (n == 0) {} }");
    AstNodo *si = hijo(cuerpo(&arbol), 0);
    es(si, AST_SI, "whether");
    es(hijo(si, 0), AST_BINARIO, "<");
    es(hijo(si, 1), AST_BLOQUE, NULL);
    AstNodo *alif = hijo(si, 2);
    es(alif, AST_SI, "alif");
    es(hijo(alif, 0), AST_BINARIO, "==");
    AstNodo *otro = hijo(alif, 2);
    es(otro, AST_SI, "alif");
    es(hijo(hijo(otro, 1), 0), AST_SI, "whether");
    es(hijo(otro, 2), AST_BLOQUE, NULL); /* also */
    COMPROBAR(hijo(otro, 2)->siguiente == NULL);
    AstNodo *ciclo = hijo(cuerpo(&arbol), 1);
    es(ciclo, AST_MIENTRAS, NULL);
    es(hijo(ciclo, 0), AST_BINARIO, ">");
    es(hijo(hijo(ciclo, 1), 0), AST_ASIGNACION, NULL);
    COMPROBAR(hijo(hijo(cuerpo(&arbol), 2), 1)->siguiente == NULL); /* sin else */
    ast_liberar(&arbol);
    puts("[OK AST] condiciones, cadena alif/also, anidamiento y ciclo");
}

static void literales_y_posiciones(void) {
    char fuente[] = "create_funk #declare_infinite_void# main() {\r\n"
        "t * declare_text : \"ñ\"; n * declare_int : 999999999999999999999999999999;\r\n"
        "c * declare_char : $ñ$; b * declare_boolean : declare_false;\r\n}";
    Ast arbol = analizar(fuente);
    memset(fuente, 'X', sizeof(fuente) - 1); /* Los textos deben ser copias propias. */
    AstNodo *bloque = cuerpo(&arbol);
    es(hijo(hijo(bloque, 0), 2), AST_CADENA, "\"ñ\"");
    AstNodo *n = hijo(bloque, 1);
    COMPROBAR(n->ubicacion.first_line == 2 && n->ubicacion.first_column == 25);
    es(hijo(n, 2), AST_ENTERO, "999999999999999999999999999999");
    es(hijo(hijo(bloque, 2), 2), AST_CARACTER, "$ñ$");
    es(hijo(hijo(bloque, 3), 2), AST_BOOLEANO, "declare_false");
    FILE *salida = tmpfile();
    COMPROBAR(salida != NULL);
    ast_imprimir(&arbol, salida);
    rewind(salida);
    char texto[4096];
    size_t usados = fread(texto, 1, sizeof(texto) - 1, salida);
    COMPROBAR(!ferror(salida));
    texto[usados] = '\0';
    COMPROBAR(strstr(texto, "Programa @1:1\n  Funcion @1:1") != NULL);
    COMPROBAR(strstr(texto, "Cadena \"\\\"ñ\\\"\"") != NULL);
    fclose(salida);
    ast_liberar(&arbol);
    COMPROBAR(arbol.raiz == NULL && arbol.reservados == NULL);
    puts("[OK AST] lexemas propios, literales, posiciones UTF-8/CRLF e impresion");
}

static void errores_y_reutilizacion(void) {
    const char *invalidos[] = {
        "bring a aka b; create_funk #declare_int# f() { a * declare_int[2] : [[1,2],]; }",
        "create_funk #declare_int# f() { seek { give 1; } seize (Error e) { give ; } seize ( }",
        "create_funk #declare_int# f() { cycle i let 0 until 5 step {} endgame; }",
        "", "create_funk #declare_int# f() { give 1 gauss; }",
        "create_funk #declare_int# f() { give 1; } create_funk #declare_int# g() { @ }",
        "create_funk #declare_int# f() { give 1; } basura",
        "create_funk #declare_int# f() { show(); }"
    };
    Ast arbol = {0};
    FILE *diagnosticos = tmpfile();
    COMPROBAR(diagnosticos != NULL);
    for (size_t i = 0; i < sizeof(invalidos) / sizeof(invalidos[0]); ++i) {
        COMPROBAR(construir_ast(invalidos[i], strlen(invalidos[i]), "error.bal", diagnosticos, &arbol) == 1);
        COMPROBAR(arbol.raiz == NULL && arbol.reservados == NULL);
        const char valido[] = "create_funk #declare_infinite_void# main() {}";
        COMPROBAR(construir_ast(valido, sizeof(valido) - 1, "ok.bal", diagnosticos, &arbol) == 0);
        es(arbol.raiz, AST_PROGRAMA, NULL);
        ast_liberar(&arbol);
    }
    fclose(diagnosticos);
    puts("[OK AST] errores limpian el arbol parcial y permiten un nuevo analisis");
}

static void operadores_ampliados(void) {
    Ast arbol = analizar("create_funk #declare_int# f() {"
        "give a || b ^ c && ~d; give n >= 18 && n < 65;"
        "give 2 descartes 3 descartes 2; give neumann 2 descartes 2;"
        "give 2 descartes neumann 3; give 8 euler 3 pitagoras 2; }");
    AstNodo *bloque = cuerpo(&arbol);
    AstNodo *op = hijo(hijo(bloque, 0), 0);
    es(op, AST_BINARIO, "||");
    es(hijo(op, 1), AST_BINARIO, "^");
    es(hijo(hijo(op, 1), 1), AST_BINARIO, "&&");
    es(hijo(hijo(hijo(op, 1), 1), 1), AST_UNARIO, "~");
    op = hijo(hijo(bloque, 1), 0);
    es(op, AST_BINARIO, "&&");
    es(hijo(op, 0), AST_BINARIO, ">=");
    es(hijo(op, 1), AST_BINARIO, "<");
    op = hijo(hijo(bloque, 2), 0);
    es(op, AST_BINARIO, "descartes");
    es(hijo(op, 1), AST_BINARIO, "descartes");
    op = hijo(hijo(bloque, 3), 0);
    es(op, AST_UNARIO, "neumann");
    es(hijo(op, 0), AST_BINARIO, "descartes");
    op = hijo(hijo(bloque, 4), 0);
    es(op, AST_BINARIO, "descartes");
    es(hijo(op, 1), AST_UNARIO, "neumann");
    op = hijo(hijo(bloque, 5), 0);
    es(op, AST_BINARIO, "pitagoras");
    es(hijo(op, 0), AST_BINARIO, "euler");
    ast_liberar(&arbol);
    puts("[OK AST] logica, residuo, potencia y precedencia de negacion");
}

static void colecciones(void) {
    Ast arbol = analizar("create_funk #declare_int# f(a * declare_int[][cols], l * declare_list declare_int) {"
        "n * declare_int; m * declare_int[2][2] : [[1,2],[3,4]];"
        "m[1][0] : a[0][1]; declare_list xs * declare_int : [], ys * declare_int : [1];"
        "add(xs, m[1][0]); remove(xs,0); give size(xs); }");
    AstNodo *params = hijo(hijo(arbol.raiz, 0), 2);
    AstNodo *tipo = hijo(hijo(params, 0), 1);
    es(tipo, AST_TIPO_ARREGLO, NULL);
    es(hijo(tipo, 0), AST_TIPO, "declare_int");
    AstNodo *dims = hijo(tipo, 1);
    es(dims, AST_DIMENSIONES, NULL);
    COMPROBAR(hijo(dims, 0)->primer_hijo == NULL);
    es(hijo(hijo(dims, 1), 0), AST_IDENTIFICADOR, "cols");
    es(hijo(hijo(params, 1), 1), AST_TIPO_LISTA, NULL);
    AstNodo *bloque = cuerpo(&arbol);
    AstNodo *n = hijo(bloque, 0);
    es(n, AST_DECLARACION, NULL);
    COMPROBAR(hijo(n, 1)->siguiente == NULL); /* Sin inicializador, no inventar cero. */
    AstNodo *m = hijo(bloque, 1);
    es(hijo(m, 1), AST_TIPO_ARREGLO, NULL);
    AstNodo *literal = hijo(m, 2);
    es(literal, AST_LITERAL_COLECCION, NULL);
    es(hijo(literal, 0), AST_LITERAL_COLECCION, NULL);
    es(hijo(hijo(literal, 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(literal, 1), 1), AST_ENTERO, "4");
    AstNodo *asig = hijo(bloque, 2);
    AstNodo *destino = hijo(asig, 0);
    es(destino, AST_INDICE, NULL);
    es(hijo(destino, 0), AST_INDICE, NULL);
    es(hijo(hijo(destino, 0), 0), AST_IDENTIFICADOR, "m");
    es(hijo(hijo(destino, 0), 1), AST_ENTERO, "1");
    es(hijo(destino, 1), AST_ENTERO, "0");
    AstNodo *listas = hijo(bloque, 3);
    es(listas, AST_DECLARACIONES, NULL);
    es(hijo(listas, 0), AST_LISTA, NULL);
    COMPROBAR(hijo(hijo(listas, 0), 2)->primer_hijo == NULL);
    es(hijo(hijo(listas, 1), 0), AST_IDENTIFICADOR, "ys");
    es(hijo(bloque, 4), AST_AGREGAR, NULL);
    es(hijo(hijo(bloque, 4), 1), AST_INDICE, NULL);
    es(hijo(bloque, 5), AST_ELIMINAR, NULL);
    es(hijo(hijo(bloque, 6), 0), AST_TAMANO, NULL);
    ast_liberar(&arbol);
    puts("[OK AST] inicializacion opcional, matrices, parametros, indices y listas");
}

static void modulos_y_control_ampliado(void) {
    Ast arbol = analizar("bring matematicas aka math; declare_const limite * declare_int : 5;"
        "create_funk #declare_infinite_void# main() {"
        "cycle i let 0 until limite { n : math.suma(i,1); } endgame;"
        "cycle j let 5 until 0 step neumann 2 {} endgame;"
        "seek { math.procesar(); } seize (ErrorTipo e) { n : 0; } seize (ErrorIndice otro) {} }");
    AstNodo *imp = hijo(arbol.raiz, 0);
    es(imp, AST_IMPORTACION, NULL);
    es(hijo(imp, 0), AST_IDENTIFICADOR, "matematicas");
    es(hijo(imp, 1), AST_IDENTIFICADOR, "math");
    AstNodo *constante = hijo(arbol.raiz, 1);
    es(constante, AST_CONSTANTE, NULL);
    es(hijo(constante, 2), AST_ENTERO, "5");
    AstNodo *bloque = hijo(hijo(arbol.raiz, 2), 3);
    AstNodo *ciclo = hijo(bloque, 0);
    es(ciclo, AST_RECORRIDO, NULL);
    es(hijo(ciclo, 0), AST_IDENTIFICADOR, "i");
    es(hijo(ciclo, 1), AST_ENTERO, "0");
    es(hijo(ciclo, 2), AST_IDENTIFICADOR, "limite");
    es(hijo(ciclo, 3), AST_ENTERO, "1"); /* Paso por defecto. */
    AstNodo *llamada = hijo(hijo(hijo(ciclo, 4), 0), 1);
    es(llamada, AST_LLAMADA, NULL);
    es(hijo(llamada, 0), AST_ACCESO_MIEMBRO, NULL);
    es(hijo(hijo(llamada, 0), 0), AST_IDENTIFICADOR, "math");
    es(hijo(hijo(llamada, 0), 1), AST_IDENTIFICADOR, "suma");
    es(hijo(hijo(bloque, 1), 3), AST_UNARIO, "neumann");
    AstNodo *intento = hijo(bloque, 2);
    es(intento, AST_INTENTAR, NULL);
    es(hijo(intento, 0), AST_BLOQUE, NULL);
    AstNodo *capturas = hijo(intento, 1);
    es(capturas, AST_CAPTURAS, NULL);
    es(hijo(capturas, 0), AST_CAPTURA, NULL);
    es(hijo(hijo(capturas, 0), 0), AST_IDENTIFICADOR, "ErrorTipo");
    es(hijo(hijo(capturas, 0), 1), AST_IDENTIFICADOR, "e");
    es(hijo(hijo(capturas, 1), 0), AST_IDENTIFICADOR, "ErrorIndice");
    COMPROBAR(hijo(capturas, 1)->siguiente == NULL);
    ast_liberar(&arbol);
    puts("[OK AST] modulos, constantes, cycle y capturas ordenadas");
}

int main(void) {
    operadores_ampliados();
    colecciones();
    modulos_y_control_ampliado();
    precedencia();
    funciones();
    control();
    literales_y_posiciones();
    errores_y_reutilizacion();
    puts("OK: 8 grupos de pruebas estructurales del AST");
    return 0;
}
