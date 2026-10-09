/* Comprueba la estructura real del AST, no solo que el parser acepte el texto. */
#include "sintactico.h" /* construir_ast y los tipos y funciones del árbol. */

#include <stdlib.h> /* exit y EXIT_FAILURE para detener una prueba fallida. */
#include <string.h> /* Medir, comparar, modificar y buscar cadenas. */

/* Verifica una condición y termina el programa si no se cumple.
 * __LINE__ indica la línea de esta prueba que falló; #condicion convierte
 * la expresión comprobada en texto para mostrarla en el diagnóstico.
 * Las barras al final continúan la macro en la siguiente línea.
 * do/while (0) hace que la macro se use como una sola instrucción.
 */
#define COMPROBAR(condicion) do { \
    if (!(condicion)) { \
        fprintf(stderr, "FALLO AST, linea %d: %s\n", __LINE__, #condicion); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

/* Analiza un programa guardado como cadena en este archivo, no en un .bal.
 * "prueba.bal" es solo el nombre que aparecería en los diagnósticos.
 * static limita el uso de estas funciones auxiliares a este archivo.
 */
static Ast analizar(const char *fuente) {
    /* Empieza sin raíz ni reservas de memoria. */
    Ast arbol = {0};
    /* Exige que el análisis termine correctamente (código 0). */
    COMPROBAR(construir_ast(fuente, strlen(fuente), "prueba.bal", stderr, &arbol) == 0);
    /* Además de aceptar el texto, debe haber creado una raíz Programa. */
    COMPROBAR(arbol.raiz && arbol.raiz->tipo == AST_PROGRAMA);
    /* El llamador recibe el árbol y se encarga de liberarlo. */
    return arbol;
}

/* Obtiene un hijo por posición: 0 es el primero, 1 el segundo, etc. */
static AstNodo *hijo(AstNodo *padre, size_t indice) {
    /* Evita acceder a los campos de un puntero nulo. */
    COMPROBAR(padre != NULL);
    AstNodo *nodo = padre->primer_hijo;
    /* Avanza por los hermanos tantas posiciones como indique el índice. */
    while (nodo && indice--) nodo = nodo->siguiente;
    /* Comprueba que exista el hijo pedido y que su enlace al padre sea correcto. */
    COMPROBAR(nodo != NULL);
    COMPROBAR(nodo->padre == padre);
    return nodo;
}

/* Comprueba el tipo y el texto esperado de un nodo.
 * Pasar NULL como texto exige que el nodo no tenga texto propio.
 */
static void es(AstNodo *nodo, AstTipo tipo, const char *texto) {
    COMPROBAR(nodo && nodo->tipo == tipo);
    /* strcmp devuelve 0 cuando ambas cadenas tienen el mismo contenido. */
    if (texto) COMPROBAR(nodo->texto && strcmp(nodo->texto, texto) == 0);
    else COMPROBAR(nodo->texto == NULL);
}

/* Obtiene el bloque de la primera función del programa.
 * Hijos de una función: 0 nombre, 1 tipo, 2 parámetros, 3 bloque.
 * Esta ayuda solo se usa cuando el primer elemento del programa es una función.
 */
static AstNodo *cuerpo(Ast *arbol) { return hijo(hijo(arbol->raiz, 0), 3); }

/* Verifica cómo se agrupan los operadores, sin calcular resultados numéricos. */
static void precedencia(void) {
    /* C concatena estas cadenas contiguas en un único programa de prueba. */
    Ast arbol = analizar("create_funk #declare_int# f() {"
        "give 2 gauss 3 pitagoras 4; give (2 gauss 3) pitagoras 4;"
        "give 10 neumann 3 neumann 2; give neumann 2 pitagoras 3;"
        "give 1 gauss 2 <= 4; }");
    AstNodo *bloque = cuerpo(&arbol);
    /* Primer give, hijo 0: debe representar 2 + (3 * 4). */
    AstNodo *suma = hijo(hijo(bloque, 0), 0);
    es(suma, AST_BINARIO, "gauss");
    es(hijo(suma, 0), AST_ENTERO, "2");
    es(hijo(suma, 1), AST_BINARIO, "pitagoras");
    es(hijo(hijo(suma, 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(suma, 1), 1), AST_ENTERO, "4");
    /* Los paréntesis cambian la agrupación a (2 + 3) * 4. */
    AstNodo *producto = hijo(hijo(bloque, 1), 0);
    es(producto, AST_BINARIO, "pitagoras");
    es(hijo(producto, 0), AST_BINARIO, "gauss");
    es(hijo(producto, 1), AST_ENTERO, "4");
    /* La resta se asocia a la izquierda: (10 - 3) - 2. */
    AstNodo *resta = hijo(hijo(bloque, 2), 0);
    es(resta, AST_BINARIO, "neumann");
    es(hijo(resta, 0), AST_BINARIO, "neumann");
    es(hijo(resta, 1), AST_ENTERO, "2");
    /* En -2 * 3, el signo negativo es un nodo unario dentro del producto. */
    AstNodo *negativo = hijo(hijo(bloque, 3), 0);
    es(negativo, AST_BINARIO, "pitagoras");
    es(hijo(negativo, 0), AST_UNARIO, "neumann");
    /* En 1 + 2 <= 4, la suma queda como operando izquierdo de la comparación. */
    AstNodo *comparacion = hijo(hijo(bloque, 4), 0);
    es(comparacion, AST_BINARIO, "<=");
    es(hijo(comparacion, 0), AST_BINARIO, "gauss");
    ast_liberar(&arbol);
    puts("[OK AST] precedencia, parentesis, asociatividad y negacion");
}

/* Revisa declaraciones de funciones, parámetros, llamadas y retornos. */
static void funciones(void) {
    Ast arbol = analizar("create_funk #declare_int# sumar(a * declare_int, b * declare_int) {"
        "give a gauss b; } create_funk #declare_infinite_void# main() {"
        "x * declare_int : sumar(2, sumar(3, 4)); x : sumar(x, 5); saludar(); give; }");
    /* La primera función debe llamarse sumar y devolver declare_int. */
    AstNodo *f = hijo(arbol.raiz, 0);
    es(f, AST_FUNCION, NULL);
    es(hijo(f, 0), AST_IDENTIFICADOR, "sumar");
    es(hijo(f, 1), AST_TIPO, "declare_int");
    /* Sus parámetros deben aparecer en orden: a y después b. */
    AstNodo *params = hijo(f, 2);
    es(params, AST_PARAMETROS, NULL);
    es(hijo(hijo(params, 0), 0), AST_IDENTIFICADOR, "a");
    es(hijo(hijo(params, 1), 0), AST_IDENTIFICADOR, "b");
    COMPROBAR(hijo(params, 1)->siguiente == NULL); /* No debe haber un tercero. */
    /* La segunda función, main, tiene un contenedor de parámetros vacío. */
    AstNodo *main = hijo(arbol.raiz, 1);
    COMPROBAR(hijo(main, 2)->primer_hijo == NULL);
    AstNodo *bloque = hijo(main, 3);
    /* La primera instrucción de main declara x con tipo declare_int. */
    AstNodo *decl = hijo(bloque, 0);
    es(decl, AST_DECLARACION, NULL);
    es(hijo(decl, 0), AST_IDENTIFICADOR, "x");
    es(hijo(decl, 1), AST_TIPO, "declare_int");
    /* Su inicializador es sumar(2, sumar(3, 4)). */
    AstNodo *llamada = hijo(decl, 2);
    es(llamada, AST_LLAMADA, NULL);
    es(hijo(llamada, 0), AST_IDENTIFICADOR, "sumar");
    /* La llamada externa contiene el entero 2 y otra llamada como argumentos. */
    AstNodo *args = hijo(llamada, 1);
    es(args, AST_ARGUMENTOS, NULL);
    es(hijo(args, 0), AST_ENTERO, "2");
    es(hijo(args, 1), AST_LLAMADA, NULL);
    /* Entra a los argumentos de la llamada interna y verifica 3 y 4. */
    es(hijo(hijo(hijo(args, 1), 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(hijo(args, 1), 1), 1), AST_ENTERO, "4");
    /* La siguiente instrucción asigna a x el resultado de otra llamada. */
    es(hijo(bloque, 1), AST_ASIGNACION, NULL);
    es(hijo(hijo(bloque, 1), 1), AST_LLAMADA, NULL);
    /* saludar() debe tener argumentos vacíos y give; no debe llevar expresión. */
    COMPROBAR(hijo(hijo(bloque, 2), 1)->primer_hijo == NULL);
    es(hijo(bloque, 3), AST_RETORNO, NULL);
    COMPROBAR(hijo(bloque, 3)->primer_hijo == NULL);
    ast_liberar(&arbol);
    puts("[OK AST] funciones, parametros, argumentos, llamadas anidadas y retornos");
}

/* Comprueba condicionales encadenados/anidados y un ciclo whale. */
static void control(void) {
    Ast arbol = analizar("create_funk #declare_int# f(n * declare_int) {"
        "whether (n < 0) { give 1; } alif (n == 0) { give 2; }"
        "alif (n < 10) { whether (n == 1) {} also {} } also { give 3; }"
        "whale (n > 0) { n : n neumann 1; } stop; whether (n == 0) {} }");
    /* Un condicional tiene condición, bloque y, si existe, rama alternativa. */
    AstNodo *si = hijo(cuerpo(&arbol), 0);
    es(si, AST_SI, "whether");
    es(hijo(si, 0), AST_BINARIO, "<");
    es(hijo(si, 1), AST_BLOQUE, NULL);
    /* Cada alif se guarda como otro nodo Si en la rama alternativa anterior. */
    AstNodo *alif = hijo(si, 2);
    es(alif, AST_SI, "alif");
    es(hijo(alif, 0), AST_BINARIO, "==");
    AstNodo *otro = hijo(alif, 2);
    es(otro, AST_SI, "alif");
    /* Dentro del segundo alif debe conservarse el whether anidado. */
    es(hijo(hijo(otro, 1), 0), AST_SI, "whether");
    es(hijo(otro, 2), AST_BLOQUE, NULL); /* also */
    COMPROBAR(hijo(otro, 2)->siguiente == NULL);
    /* whale contiene la condición n > 0 y un bloque con la asignación a n. */
    AstNodo *ciclo = hijo(cuerpo(&arbol), 1);
    es(ciclo, AST_MIENTRAS, NULL);
    es(hijo(ciclo, 0), AST_BINARIO, ">");
    es(hijo(hijo(ciclo, 1), 0), AST_ASIGNACION, NULL);
    COMPROBAR(hijo(hijo(cuerpo(&arbol), 2), 1)->siguiente == NULL); /* sin else */
    ast_liberar(&arbol);
    puts("[OK AST] condiciones, cadena alif/also, anidamiento y ciclo");
}

/* Revisa copias de lexemas, posiciones Unicode/CRLF e impresión del árbol. */
static void literales_y_posiciones(void) {
    /* Usa ñ, finales de línea CRLF y un entero muy largo para probar su texto. */
    char fuente[] = "create_funk #declare_infinite_void# main() {\r\n"
        "t * declare_text : \"ñ\"; n * declare_int : 999999999999999999999999999999;\r\n"
        "c * declare_char : $ñ$; b * declare_boolean : declare_false;\r\n}";
    Ast arbol = analizar(fuente);
    memset(fuente, 'X', sizeof(fuente) - 1); /* Los textos deben ser copias propias. */
    AstNodo *bloque = cuerpo(&arbol);
    es(hijo(hijo(bloque, 0), 2), AST_CADENA, "\"ñ\"");
    AstNodo *n = hijo(bloque, 1);
    /* La ñ anterior ocupa una columna aunque UTF-8 use varios bytes. */
    COMPROBAR(n->ubicacion.first_line == 2 && n->ubicacion.first_column == 25);
    es(hijo(n, 2), AST_ENTERO, "999999999999999999999999999999");
    es(hijo(hijo(bloque, 2), 2), AST_CARACTER, "$ñ$");
    es(hijo(hijo(bloque, 3), 2), AST_BOOLEANO, "declare_false");
    /* Captura la impresión en un archivo temporal en vez de mostrarla en pantalla. */
    FILE *salida = tmpfile();
    COMPROBAR(salida != NULL);
    ast_imprimir(&arbol, salida);
    rewind(salida); /* Vuelve al inicio para leer lo que acaba de imprimir. */
    char texto[4096];
    /* Reserva un byte del búfer para el terminador de cadena. */
    size_t usados = fread(texto, 1, sizeof(texto) - 1, salida);
    COMPROBAR(!ferror(salida));
    texto[usados] = '\0'; /* Permite buscar fragmentos con strstr. */
    /* Comprueba sangría, posiciones y comillas escapadas del literal. */
    COMPROBAR(strstr(texto, "Programa @1:1\n  Funcion @1:1") != NULL);
    COMPROBAR(strstr(texto, "Cadena \"\\\"ñ\\\"\"") != NULL);
    fclose(salida); /* Cierra y elimina el archivo temporal. */
    ast_liberar(&arbol);
    COMPROBAR(arbol.raiz == NULL && arbol.reservados == NULL); /* Árbol vacío al liberar. */
    puts("[OK AST] lexemas propios, literales, posiciones UTF-8/CRLF e impresion");
}

/* Cada entrada inválida debe limpiar el árbol y permitir analizar otra válida. */
static void errores_y_reutilizacion(void) {
    /* Casos con colecciones/capturas/ciclos incompletos, entrada vacía,
     * expresiones incompletas, caracteres o texto sobrantes y show sin argumento.
     */
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
    /* Guarda los errores esperados fuera de la salida normal de las pruebas. */
    FILE *diagnosticos = tmpfile();
    COMPROBAR(diagnosticos != NULL);
    /* La división de tamaños calcula el número de programas inválidos. */
    for (size_t i = 0; i < sizeof(invalidos) / sizeof(invalidos[0]); ++i) {
        /* El código 1 indica rechazo; no deben quedar nodos parciales registrados. */
        COMPROBAR(construir_ast(invalidos[i], strlen(invalidos[i]), "error.bal", diagnosticos, &arbol) == 1);
        COMPROBAR(arbol.raiz == NULL && arbol.reservados == NULL);
        /* Reutiliza el mismo contenedor inmediatamente después del error. */
        const char valido[] = "create_funk #declare_infinite_void# main() {}";
        COMPROBAR(construir_ast(valido, sizeof(valido) - 1, "ok.bal", diagnosticos, &arbol) == 0);
        es(arbol.raiz, AST_PROGRAMA, NULL);
        ast_liberar(&arbol); /* Lo deja listo para la siguiente vuelta. */
    }
    fclose(diagnosticos);
    puts("[OK AST] errores limpian el arbol parcial y permiten un nuevo analisis");
}

/* Revisa la estructura de operadores lógicos, potencia, residuo y negación. */
static void operadores_ampliados(void) {
    Ast arbol = analizar("create_funk #declare_int# f() {"
        "give a || b ^ c && ~d; give n >= 18 && n < 65;"
        "give 2 descartes 3 descartes 2; give neumann 2 descartes 2;"
        "give 2 descartes neumann 3; give 8 euler 3 pitagoras 2; }");
    AstNodo *bloque = cuerpo(&arbol);
    /* Debe agruparse como a || (b ^ (c && (~d))). */
    AstNodo *op = hijo(hijo(bloque, 0), 0);
    es(op, AST_BINARIO, "||");
    es(hijo(op, 1), AST_BINARIO, "^");
    es(hijo(hijo(op, 1), 1), AST_BINARIO, "&&");
    es(hijo(hijo(hijo(op, 1), 1), 1), AST_UNARIO, "~");
    /* Las comparaciones quedan a ambos lados de &&: (n >= 18) && (n < 65). */
    op = hijo(hijo(bloque, 1), 0);
    es(op, AST_BINARIO, "&&");
    es(hijo(op, 0), AST_BINARIO, ">=");
    es(hijo(op, 1), AST_BINARIO, "<");
    /* Potencia asociativa a la derecha: 2 elevado a (3 elevado a 2). */
    op = hijo(hijo(bloque, 2), 0);
    es(op, AST_BINARIO, "descartes");
    es(hijo(op, 1), AST_BINARIO, "descartes");
    /* En -2 elevado a 2, la negación contiene la potencia: -(2 elevado a 2). */
    op = hijo(hijo(bloque, 3), 0);
    es(op, AST_UNARIO, "neumann");
    es(hijo(op, 0), AST_BINARIO, "descartes");
    /* También admite la negación en el exponente: 2 elevado a (-3). */
    op = hijo(hijo(bloque, 4), 0);
    es(op, AST_BINARIO, "descartes");
    es(hijo(op, 1), AST_UNARIO, "neumann");
    /* Residuo y multiplicación se agrupan aquí como (8 % 3) * 2. */
    op = hijo(hijo(bloque, 5), 0);
    es(op, AST_BINARIO, "pitagoras");
    es(hijo(op, 0), AST_BINARIO, "euler");
    ast_liberar(&arbol);
    puts("[OK AST] logica, residuo, potencia y precedencia de negacion");
}

/* Comprueba arreglos, listas, dimensiones, índices e inicialización opcional. */
static void colecciones(void) {
    Ast arbol = analizar("create_funk #declare_int# f(a * declare_int[][cols], l * declare_list declare_int) {"
        "n * declare_int; m * declare_int[2][2] : [[1,2],[3,4]];"
        "m[1][0] : a[0][1]; declare_list xs * declare_int : [], ys * declare_int : [1];"
        "add(xs, m[1][0]); remove(xs,0); give size(xs); }");
    /* Primer parámetro: arreglo de enteros con dimensiones [] y [cols]. */
    AstNodo *params = hijo(hijo(arbol.raiz, 0), 2);
    AstNodo *tipo = hijo(hijo(params, 0), 1);
    es(tipo, AST_TIPO_ARREGLO, NULL);
    es(hijo(tipo, 0), AST_TIPO, "declare_int");
    AstNodo *dims = hijo(tipo, 1);
    es(dims, AST_DIMENSIONES, NULL);
    COMPROBAR(hijo(dims, 0)->primer_hijo == NULL); /* [] no tiene expresión de tamaño. */
    es(hijo(hijo(dims, 1), 0), AST_IDENTIFICADOR, "cols");
    /* El segundo parámetro debe tener tipo lista. */
    es(hijo(hijo(params, 1), 1), AST_TIPO_LISTA, NULL);
    AstNodo *bloque = cuerpo(&arbol);
    AstNodo *n = hijo(bloque, 0);
    es(n, AST_DECLARACION, NULL);
    COMPROBAR(hijo(n, 1)->siguiente == NULL); /* Sin inicializador, no inventar cero. */
    /* La matriz m tiene un literal que contiene otros literales: sus filas. */
    AstNodo *m = hijo(bloque, 1);
    es(hijo(m, 1), AST_TIPO_ARREGLO, NULL);
    AstNodo *literal = hijo(m, 2);
    es(literal, AST_LITERAL_COLECCION, NULL);
    es(hijo(literal, 0), AST_LITERAL_COLECCION, NULL);
    es(hijo(hijo(literal, 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(literal, 1), 1), AST_ENTERO, "4");
    /* El destino m[1][0] se representa como dos nodos Indice anidados. */
    AstNodo *asig = hijo(bloque, 2);
    AstNodo *destino = hijo(asig, 0);
    es(destino, AST_INDICE, NULL);
    es(hijo(destino, 0), AST_INDICE, NULL);
    es(hijo(hijo(destino, 0), 0), AST_IDENTIFICADOR, "m");
    es(hijo(hijo(destino, 0), 1), AST_ENTERO, "1");
    es(hijo(destino, 1), AST_ENTERO, "0");
    /* La instrucción de listas agrupa las declaraciones de xs y ys en orden. */
    AstNodo *listas = hijo(bloque, 3);
    es(listas, AST_DECLARACIONES, NULL);
    es(hijo(listas, 0), AST_LISTA, NULL);
    COMPROBAR(hijo(hijo(listas, 0), 2)->primer_hijo == NULL); /* xs inicia con []. */
    es(hijo(hijo(listas, 1), 0), AST_IDENTIFICADOR, "ys");
    /* add, remove y size deben producir sus respectivos tipos de nodo. */
    es(hijo(bloque, 4), AST_AGREGAR, NULL);
    es(hijo(hijo(bloque, 4), 1), AST_INDICE, NULL);
    es(hijo(bloque, 5), AST_ELIMINAR, NULL);
    es(hijo(hijo(bloque, 6), 0), AST_TAMANO, NULL);
    ast_liberar(&arbol);
    puts("[OK AST] inicializacion opcional, matrices, parametros, indices y listas");
}

/* Verifica importaciones, constantes, cycle y cláusulas seek/seize. */
static void modulos_y_control_ampliado(void) {
    Ast arbol = analizar("bring matematicas aka math; declare_const limite * declare_int : 5;"
        "create_funk #declare_infinite_void# main() {"
        "cycle i let 0 until limite { n : math.suma(i,1); } endgame;"
        "cycle j let 5 until 0 step neumann 2 {} endgame;"
        "seek { math.procesar(); } seize (ErrorTipo e) { n : 0; } seize (ErrorIndice otro) {} }");
    /* La importación conserva tanto el nombre del módulo como su alias. */
    AstNodo *imp = hijo(arbol.raiz, 0);
    es(imp, AST_IMPORTACION, NULL);
    es(hijo(imp, 0), AST_IDENTIFICADOR, "matematicas");
    es(hijo(imp, 1), AST_IDENTIFICADOR, "math");
    /* La constante tiene el literal 5 como inicializador (hijo 2). */
    AstNodo *constante = hijo(arbol.raiz, 1);
    es(constante, AST_CONSTANTE, NULL);
    es(hijo(constante, 2), AST_ENTERO, "5");
    /* main es el tercer elemento, después de la importación y la constante. */
    AstNodo *bloque = hijo(hijo(arbol.raiz, 2), 3);
    /* Hijos de cycle: variable, inicio, límite, paso y bloque. */
    AstNodo *ciclo = hijo(bloque, 0);
    es(ciclo, AST_RECORRIDO, NULL);
    es(hijo(ciclo, 0), AST_IDENTIFICADOR, "i");
    es(hijo(ciclo, 1), AST_ENTERO, "0");
    es(hijo(ciclo, 2), AST_IDENTIFICADOR, "limite");
    es(hijo(ciclo, 3), AST_ENTERO, "1"); /* Paso por defecto. */
    /* Entra al bloque del ciclo, a la asignación y a su expresión math.suma(i,1). */
    AstNodo *llamada = hijo(hijo(hijo(ciclo, 4), 0), 1);
    es(llamada, AST_LLAMADA, NULL);
    es(hijo(llamada, 0), AST_ACCESO_MIEMBRO, NULL);
    es(hijo(hijo(llamada, 0), 0), AST_IDENTIFICADOR, "math");
    es(hijo(hijo(llamada, 0), 1), AST_IDENTIFICADOR, "suma");
    /* El segundo ciclo conserva el signo negativo de su paso explícito. */
    es(hijo(hijo(bloque, 1), 3), AST_UNARIO, "neumann");
    /* seek contiene un bloque protegido y una agrupación de capturas. */
    AstNodo *intento = hijo(bloque, 2);
    es(intento, AST_INTENTAR, NULL);
    es(hijo(intento, 0), AST_BLOQUE, NULL);
    /* Las capturas deben conservar el orden ErrorTipo y luego ErrorIndice. */
    AstNodo *capturas = hijo(intento, 1);
    es(capturas, AST_CAPTURAS, NULL);
    es(hijo(capturas, 0), AST_CAPTURA, NULL);
    es(hijo(hijo(capturas, 0), 0), AST_IDENTIFICADOR, "ErrorTipo");
    es(hijo(hijo(capturas, 0), 1), AST_IDENTIFICADOR, "e");
    es(hijo(hijo(capturas, 1), 0), AST_IDENTIFICADOR, "ErrorIndice");
    COMPROBAR(hijo(capturas, 1)->siguiente == NULL); /* No hay una tercera captura. */
    ast_liberar(&arbol);
    puts("[OK AST] modulos, constantes, cycle y capturas ordenadas");
}

/* Punto de entrada de ./tests/test_ast (también ejecutado por make test-ast).
 * Cada grupo libera sus árboles e imprime [OK AST] al terminar.
 * Si alguna comprobación falla, COMPROBAR termina el programa antes del resumen.
 */
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
    return 0; /* Informa al sistema y a make que todas las pruebas pasaron. */
}
