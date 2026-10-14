/* Comprueba la estructura real del AST, no solo que el parser acepte el texto. */
/* Interfaz de construcción del AST; proporciona los tipos y las funciones usados por las pruebas. */
#include "sintactico.h"

/* Proporciona exit y EXIT_FAILURE para detener la ejecución cuando falla una comprobación. */
#include <stdlib.h>
/* Proporciona strlen, strcmp, memset y strstr para preparar y comprobar textos. */
#include <string.h>

/* Evalúa una condición y termina la prueba si es falsa. El diagnóstico muestra la línea
 * y la expresión comprobada (#condicion la convierte en texto). do/while (0) permite
 * usar la macro como una sola sentencia; las barras finales continúan su definición. */
#define COMPROBAR(condicion) do { \
    if (!(condicion)) { \
        fprintf(stderr, "FALLO AST, linea %d: %s\n", __LINE__, #condicion); \
        exit(EXIT_FAILURE); \
    } \
} while (0)

/* Construye un AST de un programa que debe ser válido y comprueba que tenga una raíz
 * de tipo programa. Devuelve el árbol por valor; quien lo recibe debe liberar sus nodos. */
static Ast analizar(const char *fuente) {
    Ast arbol = {0};
    COMPROBAR(construir_ast(fuente, strlen(fuente), "prueba.bal", stderr, &arbol) == 0);
    COMPROBAR(arbol.raiz && arbol.raiz->tipo == AST_PROGRAMA);
    return arbol;
}

/* Obtiene un hijo por su posición, contando desde cero. Recorre los enlaces de hermanos
 * y comprueba tanto su existencia como el enlace de vuelta al padre. */
static AstNodo *hijo(AstNodo *padre, size_t indice) {
    COMPROBAR(padre != NULL);
    AstNodo *nodo = padre->primer_hijo;
    while (nodo && indice--) nodo = nodo->siguiente;
    COMPROBAR(nodo != NULL);
    COMPROBAR(nodo->padre == padre);
    return nodo;
}

/* Comprueba el tipo y el texto de un nodo. Si texto es NULL, exige que el nodo tampoco
 * tenga texto; de lo contrario, compara el contenido de las cadenas. */
static void es(AstNodo *nodo, AstTipo tipo, const char *texto) {
    COMPROBAR(nodo && nodo->tipo == tipo);
    if (texto) COMPROBAR(nodo->texto && strcmp(nodo->texto, texto) == 0);
    else COMPROBAR(nodo->texto == NULL);
}

/* Devuelve el bloque de la primera función del programa. Sus hijos se ordenan como
 * nombre, tipo de retorno, parámetros y cuerpo; por eso el cuerpo tiene índice 3. */
static AstNodo *cuerpo(Ast *arbol) { return hijo(hijo(arbol->raiz, 0), 3); }

/* Verifica la forma del árbol para prioridades de operadores, agrupación con paréntesis,
 * resta asociativa a la izquierda, negación y comparación. No evalúa las expresiones. */
static void precedencia(void) {
    Ast arbol = analizar("create_funk #declare_int# f() {"
        "give 2 gauss 3 pitagoras 4; give (2 gauss 3) pitagoras 4;"
        "give 10 neumann 3 neumann 2; give neumann 2 pitagoras 3;"
        "give 1 gauss 2 <= 4; }");
    AstNodo *bloque = cuerpo(&arbol);
    /* El primer hijo del retorno es su expresión: en 2 + 3 * 4, la raíz debe ser la suma
     * y su operando derecho debe contener el producto de 3 y 4. */
    AstNodo *suma = hijo(hijo(bloque, 0), 0);
    es(suma, AST_BINARIO, "gauss");
    es(hijo(suma, 0), AST_ENTERO, "2");
    es(hijo(suma, 1), AST_BINARIO, "pitagoras");
    es(hijo(hijo(suma, 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(suma, 1), 1), AST_ENTERO, "4");
    /* Los paréntesis cambian la agrupación: en (2 + 3) * 4, el producto contiene la suma. */
    AstNodo *producto = hijo(hijo(bloque, 1), 0);
    es(producto, AST_BINARIO, "pitagoras");
    es(hijo(producto, 0), AST_BINARIO, "gauss");
    es(hijo(producto, 1), AST_ENTERO, "4");
    /* La resta se agrupa a la izquierda: (10 - 3) - 2 se representa con otra resta como hijo izquierdo. */
    AstNodo *resta = hijo(hijo(bloque, 2), 0);
    es(resta, AST_BINARIO, "neumann");
    es(hijo(resta, 0), AST_BINARIO, "neumann");
    es(hijo(resta, 1), AST_ENTERO, "2");
    /* En -2 * 3, la negación es una operación unaria dentro del operando izquierdo del producto. */
    AstNodo *negativo = hijo(hijo(bloque, 3), 0);
    es(negativo, AST_BINARIO, "pitagoras");
    es(hijo(negativo, 0), AST_UNARIO, "neumann");
    /* En 1 + 2 <= 4, la comparación queda por encima de la suma en el árbol. */
    AstNodo *comparacion = hijo(hijo(bloque, 4), 0);
    es(comparacion, AST_BINARIO, "<=");
    es(hijo(comparacion, 0), AST_BINARIO, "gauss");
    ast_liberar(&arbol);
    puts("[OK AST] precedencia, parentesis, asociatividad y negacion");
}

/* Comprueba la organización de funciones, parámetros y argumentos, además de llamadas
 * anidadas, declaraciones inicializadas, asignaciones y retornos sin expresión. */
static void funciones(void) {
    Ast arbol = analizar("create_funk #declare_int# sumar(a * declare_int, b * declare_int) {"
        "give a gauss b; } create_funk #declare_infinite_void# main() {"
        "x * declare_int : sumar(2, sumar(3, 4)); x : sumar(x, 5); saludar(); give; }");
    /* Inspecciona la función sumar: nombre, tipo de retorno y nombres de sus dos parámetros. */
    AstNodo *f = hijo(arbol.raiz, 0);
    es(f, AST_FUNCION, NULL);
    es(hijo(f, 0), AST_IDENTIFICADOR, "sumar");
    es(hijo(f, 1), AST_TIPO, "declare_int");
    AstNodo *params = hijo(f, 2);
    es(params, AST_PARAMETROS, NULL);
    es(hijo(hijo(params, 0), 0), AST_IDENTIFICADOR, "a");
    es(hijo(hijo(params, 1), 0), AST_IDENTIFICADOR, "b");
    COMPROBAR(hijo(params, 1)->siguiente == NULL);
    /* La segunda función es main; su contenedor de parámetros debe estar vacío. */
    AstNodo *main = hijo(arbol.raiz, 1);
    COMPROBAR(hijo(main, 2)->primer_hijo == NULL);
    AstNodo *bloque = hijo(main, 3);
    AstNodo *decl = hijo(bloque, 0);
    es(decl, AST_DECLARACION, NULL);
    es(hijo(decl, 0), AST_IDENTIFICADOR, "x");
    es(hijo(decl, 1), AST_TIPO, "declare_int");
    /* El tercer hijo de la declaración es el inicializador: sumar(2, sumar(3, 4)).
     * La llamada contiene el nombre invocado y un contenedor de argumentos. */
    AstNodo *llamada = hijo(decl, 2);
    es(llamada, AST_LLAMADA, NULL);
    es(hijo(llamada, 0), AST_IDENTIFICADOR, "sumar");
    AstNodo *args = hijo(llamada, 1);
    es(args, AST_ARGUMENTOS, NULL);
    es(hijo(args, 0), AST_ENTERO, "2");
    es(hijo(args, 1), AST_LLAMADA, NULL);
    es(hijo(hijo(hijo(args, 1), 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(hijo(args, 1), 1), 1), AST_ENTERO, "4");
    /* Las siguientes sentencias incluyen una asignación con llamada, una llamada sin argumentos
     * y un retorno sin expresión; se comprueba que los contenedores correspondientes estén vacíos. */
    es(hijo(bloque, 1), AST_ASIGNACION, NULL);
    es(hijo(hijo(bloque, 1), 1), AST_LLAMADA, NULL);
    COMPROBAR(hijo(hijo(bloque, 2), 1)->primer_hijo == NULL);
    es(hijo(bloque, 3), AST_RETORNO, NULL);
    COMPROBAR(hijo(bloque, 3)->primer_hijo == NULL);
    ast_liberar(&arbol);
    puts("[OK AST] funciones, parametros, argumentos, llamadas anidadas y retornos");
}

/* Comprueba condiciones whether, ramas alif/also, un condicional anidado y un ciclo whale.
 * Los hijos del condicional representan condición, bloque principal y rama alternativa opcional. */
static void control(void) {
    Ast arbol = analizar("create_funk #declare_int# f(n * declare_int) {"
        "whether (n < 0) { give 1; } alif (n == 0) { give 2; }"
        "alif (n < 10) { whether (n == 1) {} also {} } also { give 3; }"
        "whale (n > 0) { n : n neumann 1; } stop; whether (n == 0) {} }");
    AstNodo *si = hijo(cuerpo(&arbol), 0);
    es(si, AST_SI, "whether");
    es(hijo(si, 0), AST_BINARIO, "<");
    es(hijo(si, 1), AST_BLOQUE, NULL);
    /* Cada alif se representa como otro condicional en la rama alternativa del anterior. */
    AstNodo *alif = hijo(si, 2);
    es(alif, AST_SI, "alif");
    es(hijo(alif, 0), AST_BINARIO, "==");
    AstNodo *otro = hijo(alif, 2);
    es(otro, AST_SI, "alif");
    es(hijo(hijo(otro, 1), 0), AST_SI, "whether");
    es(hijo(otro, 2), AST_BLOQUE, NULL); /* also */
    COMPROBAR(hijo(otro, 2)->siguiente == NULL);
    /* Inspecciona la condición del ciclo y la asignación que forma parte de su cuerpo. */
    AstNodo *ciclo = hijo(cuerpo(&arbol), 1);
    es(ciclo, AST_MIENTRAS, NULL);
    es(hijo(ciclo, 0), AST_BINARIO, ">");
    es(hijo(hijo(ciclo, 1), 0), AST_ASIGNACION, NULL);
    COMPROBAR(hijo(hijo(cuerpo(&arbol), 2), 1)->siguiente == NULL); /* sin else */
    ast_liberar(&arbol);
    puts("[OK AST] condiciones, cadena alif/also, anidamiento y ciclo");
}

/* Prueba la conservación de lexemas, las columnas Unicode con finales de línea CRLF,
 * la impresión del AST y el restablecimiento de sus punteros después de liberarlo. */
static void literales_y_posiciones(void) {
    char fuente[] = "create_funk #declare_infinite_void# main() {\r\n"
        "t * declare_text : \"ñ\"; n * declare_int : 999999999999999999999999999999;\r\n"
        "c * declare_char : $ñ$; b * declare_boolean : declare_false;\r\n}";
    Ast arbol = analizar(fuente);
    /* Sobrescribe la fuente, conservando su terminador nulo, para demostrar que los nodos
     * guardan copias independientes y no dependen de la memoria del texto original. */
    memset(fuente, 'X', sizeof(fuente) - 1); /* Los textos deben ser copias propias. */
    AstNodo *bloque = cuerpo(&arbol);
    es(hijo(hijo(bloque, 0), 2), AST_CADENA, "\"ñ\"");
    AstNodo *n = hijo(bloque, 1);
    /* La ñ anterior ocupa varios bytes en UTF-8, pero una sola columna; la declaración n comienza en la columna 25. */
    COMPROBAR(n->ubicacion.first_line == 2 && n->ubicacion.first_column == 25);
    /* El entero se conserva como texto, sin exigir aquí que quepa en un tipo numérico de C. */
    es(hijo(n, 2), AST_ENTERO, "999999999999999999999999999999");
    es(hijo(hijo(bloque, 2), 2), AST_CARACTER, "$ñ$");
    es(hijo(hijo(bloque, 3), 2), AST_BOOLEANO, "declare_false");
    /* Crea un archivo temporal para capturar y examinar la impresión del árbol. */
    FILE *salida = tmpfile();
    COMPROBAR(salida != NULL);
    ast_imprimir(&arbol, salida);
    /* Vuelve al inicio del archivo para leer lo que acaba de escribir ast_imprimir. */
    rewind(salida);
    /* Reserva un búfer de lectura; se deja un byte disponible para añadir el terminador nulo. */
    char texto[4096];
    size_t usados = fread(texto, 1, sizeof(texto) - 1, salida);
    COMPROBAR(!ferror(salida));
    texto[usados] = '\0';
    /* Comprueba la sangría y las ubicaciones iniciales de programa y función en la salida. */
    COMPROBAR(strstr(texto, "Programa @1:1\n  Funcion @1:1") != NULL);
    COMPROBAR(strstr(texto, "Cadena \"\\\"ñ\\\"\"") != NULL);
    /* Cierra el archivo temporal después de inspeccionar la salida. */
    fclose(salida);
    ast_liberar(&arbol);
    COMPROBAR(arbol.raiz == NULL && arbol.reservados == NULL);
    puts("[OK AST] lexemas propios, literales, posiciones UTF-8/CRLF e impresion");
}

/* Comprueba que las entradas inválidas devuelvan error y liberen el AST parcial. Después
 * de cada fallo, reutiliza la misma estructura con un programa válido para verificar su recuperación. */
static void errores_y_reutilizacion(void) {
    /* Casos rechazados: colección mal formada, captura incompleta, paso de ciclo ausente,
     * fuente vacía, expresión incompleta, carácter inválido, texto sobrante y uso de show(). */
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
    /* Captura los diagnósticos esperados de los casos inválidos en un archivo temporal. */
    FILE *diagnosticos = tmpfile();
    COMPROBAR(diagnosticos != NULL);
    /* Recorre todas las entradas inválidas; el cociente de tamaños calcula la cantidad de elementos. */
    for (size_t i = 0; i < sizeof(invalidos) / sizeof(invalidos[0]); ++i) {
        COMPROBAR(construir_ast(invalidos[i], strlen(invalidos[i]), "error.bal", diagnosticos, &arbol) == 1);
        COMPROBAR(arbol.raiz == NULL && arbol.reservados == NULL);
        /* Usa un programa mínimo válido para comprobar que el error anterior no impide un nuevo análisis. */
        const char valido[] = "create_funk #declare_infinite_void# main() {}";
        COMPROBAR(construir_ast(valido, sizeof(valido) - 1, "ok.bal", diagnosticos, &arbol) == 0);
        es(arbol.raiz, AST_PROGRAMA, NULL);
        ast_liberar(&arbol);
    }
    fclose(diagnosticos);
    puts("[OK AST] errores limpian el arbol parcial y permiten un nuevo analisis");
}

/* Verifica las prioridades de OR, XOR, AND y negación lógica, así como comparaciones,
 * potencia, negación aritmética y residuo, inspeccionando la estructura de las expresiones. */
static void operadores_ampliados(void) {
    Ast arbol = analizar("create_funk #declare_int# f() {"
        "give a || b ^ c && ~d; give n >= 18 && n < 65;"
        "give 2 descartes 3 descartes 2; give neumann 2 descartes 2;"
        "give 2 descartes neumann 3; give 8 euler 3 pitagoras 2; }");
    AstNodo *bloque = cuerpo(&arbol);
    /* La expresión lógica debe anidarse como a || (b ^ (c && (~d))). */
    AstNodo *op = hijo(hijo(bloque, 0), 0);
    es(op, AST_BINARIO, "||");
    es(hijo(op, 1), AST_BINARIO, "^");
    es(hijo(hijo(op, 1), 1), AST_BINARIO, "&&");
    es(hijo(hijo(hijo(op, 1), 1), 1), AST_UNARIO, "~");
    /* AND combina las dos comparaciones n >= 18 y n < 65. */
    op = hijo(hijo(bloque, 1), 0);
    es(op, AST_BINARIO, "&&");
    es(hijo(op, 0), AST_BINARIO, ">=");
    es(hijo(op, 1), AST_BINARIO, "<");
    /* La potencia se agrupa a la derecha: 2 elevado a (3 elevado a 2). */
    op = hijo(hijo(bloque, 2), 0);
    es(op, AST_BINARIO, "descartes");
    es(hijo(op, 1), AST_BINARIO, "descartes");
    /* En -2 elevado a 2, la negación queda fuera de la potencia. */
    op = hijo(hijo(bloque, 3), 0);
    es(op, AST_UNARIO, "neumann");
    es(hijo(op, 0), AST_BINARIO, "descartes");
    /* Una potencia también puede tener un exponente negativo, representado por un nodo unario. */
    op = hijo(hijo(bloque, 4), 0);
    es(op, AST_BINARIO, "descartes");
    es(hijo(op, 1), AST_UNARIO, "neumann");
    /* Residuo y multiplicación se agrupan aquí a la izquierda: (8 euler 3) pitagoras 2. */
    op = hijo(hijo(bloque, 5), 0);
    es(op, AST_BINARIO, "pitagoras");
    es(hijo(op, 0), AST_BINARIO, "euler");
    ast_liberar(&arbol);
    puts("[OK AST] logica, residuo, potencia y precedencia de negacion");
}

/* Comprueba tipos de arreglos y listas, dimensiones opcionales, literales anidados,
 * accesos por índice, declaraciones sin inicializador y operaciones add, remove y size. */
static void colecciones(void) {
    Ast arbol = analizar("create_funk #declare_int# f(a * declare_int[][cols], l * declare_list declare_int) {"
        "n * declare_int; m * declare_int[2][2] : [[1,2],[3,4]];"
        "m[1][0] : a[0][1]; declare_list xs * declare_int : [], ys * declare_int : [1];"
        "add(xs, m[1][0]); remove(xs,0); give size(xs); }");
    AstNodo *params = hijo(hijo(arbol.raiz, 0), 2);
    /* El primer parámetro es un arreglo cuyo tipo contiene el tipo base y las dimensiones. */
    AstNodo *tipo = hijo(hijo(params, 0), 1);
    es(tipo, AST_TIPO_ARREGLO, NULL);
    es(hijo(tipo, 0), AST_TIPO, "declare_int");
    AstNodo *dims = hijo(tipo, 1);
    es(dims, AST_DIMENSIONES, NULL);
    /* La primera dimensión no tiene tamaño explícito; la segunda usa el identificador cols. */
    COMPROBAR(hijo(dims, 0)->primer_hijo == NULL);
    es(hijo(hijo(dims, 1), 0), AST_IDENTIFICADOR, "cols");
    es(hijo(hijo(params, 1), 1), AST_TIPO_LISTA, NULL);
    AstNodo *bloque = cuerpo(&arbol);
    AstNodo *n = hijo(bloque, 0);
    es(n, AST_DECLARACION, NULL);
    COMPROBAR(hijo(n, 1)->siguiente == NULL); /* Sin inicializador, no inventar cero. */
    AstNodo *m = hijo(bloque, 1);
    es(hijo(m, 1), AST_TIPO_ARREGLO, NULL);
    /* La matriz se inicializa con una colección de colecciones; se revisan valores de su segunda fila. */
    AstNodo *literal = hijo(m, 2);
    es(literal, AST_LITERAL_COLECCION, NULL);
    es(hijo(literal, 0), AST_LITERAL_COLECCION, NULL);
    es(hijo(hijo(literal, 1), 0), AST_ENTERO, "3");
    es(hijo(hijo(literal, 1), 1), AST_ENTERO, "4");
    AstNodo *asig = hijo(bloque, 2);
    /* El destino m[1][0] se representa mediante dos nodos de índice anidados. */
    AstNodo *destino = hijo(asig, 0);
    es(destino, AST_INDICE, NULL);
    es(hijo(destino, 0), AST_INDICE, NULL);
    es(hijo(hijo(destino, 0), 0), AST_IDENTIFICADOR, "m");
    es(hijo(hijo(destino, 0), 1), AST_ENTERO, "1");
    es(hijo(destino, 1), AST_ENTERO, "0");
    /* La declaración conjunta agrupa las listas xs e ys; xs debe conservar su inicializador vacío. */
    AstNodo *listas = hijo(bloque, 3);
    es(listas, AST_DECLARACIONES, NULL);
    es(hijo(listas, 0), AST_LISTA, NULL);
    COMPROBAR(hijo(hijo(listas, 0), 2)->primer_hijo == NULL);
    es(hijo(hijo(listas, 1), 0), AST_IDENTIFICADOR, "ys");
    /* Verifica los nodos de agregar, eliminar y consultar tamaño, incluido el acceso por índice usado en add. */
    es(hijo(bloque, 4), AST_AGREGAR, NULL);
    es(hijo(hijo(bloque, 4), 1), AST_INDICE, NULL);
    es(hijo(bloque, 5), AST_ELIMINAR, NULL);
    es(hijo(hijo(bloque, 6), 0), AST_TAMANO, NULL);
    ast_liberar(&arbol);
    puts("[OK AST] inicializacion opcional, matrices, parametros, indices y listas");
}

/* Comprueba importaciones con alias, constantes, recorridos cycle, acceso a miembros
 * de módulos y bloques seek/seize con varias capturas en el orden del código fuente. */
static void modulos_y_control_ampliado(void) {
    Ast arbol = analizar("bring matematicas aka math; declare_const limite * declare_int : 5;"
        "create_funk #declare_infinite_void# main() {"
        "cycle i let 0 until limite { n : math.suma(i,1); } endgame;"
        "cycle j let 5 until 0 step neumann 2 {} endgame;"
        "seek { math.procesar(); } seize (ErrorTipo e) { n : 0; } seize (ErrorIndice otro) {} }");
    /* La importación guarda tanto el nombre del módulo como su alias. */
    AstNodo *imp = hijo(arbol.raiz, 0);
    es(imp, AST_IMPORTACION, NULL);
    es(hijo(imp, 0), AST_IDENTIFICADOR, "matematicas");
    es(hijo(imp, 1), AST_IDENTIFICADOR, "math");
    /* La constante es el segundo elemento del programa y tiene el valor inicial 5. */
    AstNodo *constante = hijo(arbol.raiz, 1);
    es(constante, AST_CONSTANTE, NULL);
    es(hijo(constante, 2), AST_ENTERO, "5");
    AstNodo *bloque = hijo(hijo(arbol.raiz, 2), 3);
    /* El recorrido contiene variable, inicio, límite, paso y bloque; al omitir step se espera el paso 1. */
    AstNodo *ciclo = hijo(bloque, 0);
    es(ciclo, AST_RECORRIDO, NULL);
    es(hijo(ciclo, 0), AST_IDENTIFICADOR, "i");
    es(hijo(ciclo, 1), AST_ENTERO, "0");
    es(hijo(ciclo, 2), AST_IDENTIFICADOR, "limite");
    es(hijo(ciclo, 3), AST_ENTERO, "1"); /* Paso por defecto. */
    /* La asignación del ciclo llama a math.suma; el destino de la llamada es un acceso a miembro. */
    AstNodo *llamada = hijo(hijo(hijo(ciclo, 4), 0), 1);
    es(llamada, AST_LLAMADA, NULL);
    es(hijo(llamada, 0), AST_ACCESO_MIEMBRO, NULL);
    es(hijo(hijo(llamada, 0), 0), AST_IDENTIFICADOR, "math");
    es(hijo(hijo(llamada, 0), 1), AST_IDENTIFICADOR, "suma");
    es(hijo(hijo(bloque, 1), 3), AST_UNARIO, "neumann");
    /* El bloque seek contiene su cuerpo y un contenedor con las capturas seize. */
    AstNodo *intento = hijo(bloque, 2);
    es(intento, AST_INTENTAR, NULL);
    es(hijo(intento, 0), AST_BLOQUE, NULL);
    /* Comprueba los tipos de excepción, el nombre de la primera variable y que haya exactamente dos capturas. */
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

/* Ejecuta los ocho grupos de pruebas. Cualquier comprobación fallida termina el proceso;
 * el mensaje final y el retorno cero solo se alcanzan si todas las comprobaciones pasan. */
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
