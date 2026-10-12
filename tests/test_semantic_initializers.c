#include "semantic.h"

#include <assert.h>
#include <stdio.h>

static AstNodo *crear(
    Ast *a,
    AstTipo tipo,
    const char *texto
)
{
    AstUbicacion u = {1, 1, 1, 1};

    AstNodo *n = ast_crear(
        a,
        tipo,
        texto,
        u
    );

    assert(n != NULL);
    return n;
}

static void agregar(
    AstNodo *padre,
    AstNodo *hijo_n
)
{
    ast_agregar_hijo(padre, hijo_n);
}

/*
 * Crear main y devolver su bloque.
 */
static AstNodo *crear_main(Ast *a)
{
    a->raiz = crear(
        a,
        AST_PROGRAMA,
        NULL
    );

    AstNodo *funcion = crear(
        a,
        AST_FUNCION,
        NULL
    );

    agregar(a->raiz, funcion);

    agregar(
        funcion,
        crear(
            a,
            AST_IDENTIFICADOR,
            "main"
        )
    );

    agregar(
        funcion,
        crear(
            a,
            AST_TIPO,
            "declare_infinite_void"
        )
    );

    agregar(
        funcion,
        crear(
            a,
            AST_PARAMETROS,
            NULL
        )
    );

    AstNodo *bloque = crear(
        a,
        AST_BLOQUE,
        NULL
    );

    agregar(funcion, bloque);

    return bloque;
}

/*
 * Agregar una dimension al tipo.
 */
static void agregar_dimension(
    Ast *a,
    AstNodo *dimensiones,
    const char *tamano
)
{
    AstNodo *dimension = crear(
        a,
        AST_DIMENSION,
        NULL
    );

    agregar(dimensiones, dimension);

    agregar(
        dimension,
        crear(
            a,
            AST_ENTERO,
            tamano
        )
    );
}

/*
 * Agregar un elemento a una fila.
 *
 * Un booleano intencionalmente incorrecto
 * se usa para verificar los tipos.
 */
static void agregar_elemento(
    Ast *a,
    AstNodo *fila,
    int booleano
)
{
    agregar(
        fila,
        crear(
            a,
            booleano
                ? AST_BOOLEANO
                : AST_ENTERO,
            booleano
                ? "declare_true"
                : "1"
        )
    );
}

/*
 * Construir:
 *
 * matriz * declare_int[2][3] : [...];
 */
static void crear_matriz(
    Ast *a,
    AstNodo *bloque,
    int caso
)
{
    AstNodo *declaracion = crear(
        a,
        AST_DECLARACION,
        NULL
    );

    agregar(bloque, declaracion);

    agregar(
        declaracion,
        crear(
            a,
            AST_IDENTIFICADOR,
            "matriz"
        )
    );

    AstNodo *tipo_arreglo = crear(
        a,
        AST_TIPO_ARREGLO,
        NULL
    );

    agregar(declaracion, tipo_arreglo);

    agregar(
        tipo_arreglo,
        crear(
            a,
            AST_TIPO,
            "declare_int"
        )
    );

    AstNodo *dimensiones = crear(
        a,
        AST_DIMENSIONES,
        NULL
    );

    agregar(tipo_arreglo, dimensiones);

    agregar_dimension(a, dimensiones, "2");
    agregar_dimension(a, dimensiones, "3");

    /*
     * Inicializador:
     *
     * [
     *     [1, 1, 1],
     *     [1, 1, 1]
     * ]
     */
    AstNodo *literal = crear(
        a,
        AST_LITERAL_COLECCION,
        NULL
    );

    agregar(declaracion, literal);

    /*
     * Caso 1:
     * solamente una fila.
     *
     * Los demas casos tienen dos.
     */
    int filas = caso == 1 ? 1 : 2;

    for (int i = 0; i < filas; ++i) {

        AstNodo *fila = crear(
            a,
            AST_LITERAL_COLECCION,
            NULL
        );

        agregar(literal, fila);

        /*
         * Caso 2:
         * una fila tiene dos columnas.
         */
        int columnas = (
            caso == 2 && i == 1
        ) ? 2 : 3;

        for (int j = 0; j < columnas; ++j) {

            /*
             * Caso 3:
             * un elemento booleano en
             * una matriz de enteros.
             */
            int booleano = (
                caso == 3 &&
                i == 1 &&
                j == 1
            );

            agregar_elemento(
                a,
                fila,
                booleano
            );
        }
    }
}

static Ast construir_programa(int caso)
{
    Ast arbol = {0};

    AstNodo *cuerpo = crear_main(
        &arbol
    );

    crear_matriz(
        &arbol,
        cuerpo,
        caso
    );

    return arbol;
}

int main(void)
{
    /*
     * Caso 0: matriz valida.
     * Caso 1: numero incorrecto de filas.
     * Caso 2: numero incorrecto de columnas.
     * Caso 3: elemento de tipo incorrecto.
     */
    for (int caso = 0; caso < 4; ++caso) {

        Ast arbol = construir_programa(
            caso
        );

        SimTabla tabla = {0};

        assert(
            sim_inicializar(&tabla) == SIM_OK
        );

        int resultado = analizar_semantica(
            &arbol,
            &tabla,
            "inicializadores.bal",
            stderr
        );

        int esperado = caso == 0 ? 0 : 1;

        fprintf(
            stderr,
            "CASO %d: esperado=%d, obtenido=%d\n",
            caso,
            esperado,
            resultado
        );

        assert(resultado == esperado);

        sim_liberar(&tabla);
        ast_liberar(&arbol);
    }

    puts(
        "OK: inicializadores de matrices "
        "y tipos de elementos"
    );

    return 0;
}