#include "semantic.h"

#include <assert.h>
#include <stdio.h>

/*
 * Crear un nodo del AST.
 */
static AstNodo *crear(
    Ast *arbol,
    AstTipo tipo,
    const char *texto
)
{
    AstUbicacion ubicacion = {1, 1, 1, 1};

    AstNodo *n = ast_crear(
        arbol,
        tipo,
        texto,
        ubicacion
    );

    assert(n != NULL);

    return n;
}

/*
 * Agregar un hijo a un nodo.
 */
static void agregar(
    AstNodo *padre,
    AstNodo *hijo_n
)
{
    ast_agregar_hijo(padre, hijo_n);
}

/*
 * Crear un programa con una función main.
 *
 * Devuelve el bloque de main.
 */
static AstNodo *crear_main(Ast *arbol)
{
    arbol->raiz = crear(
        arbol,
        AST_PROGRAMA,
        NULL
    );

    AstNodo *funcion = crear(
        arbol,
        AST_FUNCION,
        NULL
    );

    agregar(arbol->raiz, funcion);

    agregar(
        funcion,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            "main"
        )
    );

    agregar(
        funcion,
        crear(
            arbol,
            AST_TIPO,
            "declare_infinite_void"
        )
    );

    agregar(
        funcion,
        crear(
            arbol,
            AST_PARAMETROS,
            NULL
        )
    );

    AstNodo *bloque = crear(
        arbol,
        AST_BLOQUE,
        NULL
    );

    agregar(funcion, bloque);

    return bloque;
}

/*
 * Declarar una variable entera.
 *
 * Ejemplo:
 *
 * resultado * declare_int;
 */
static void declarar_entero(
    Ast *arbol,
    AstNodo *bloque,
    const char *nombre
)
{
    AstNodo *declaracion = crear(
        arbol,
        AST_DECLARACION,
        NULL
    );

    agregar(bloque, declaracion);

    agregar(
        declaracion,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            nombre
        )
    );

    agregar(
        declaracion,
        crear(
            arbol,
            AST_TIPO,
            "declare_int"
        )
    );
}

/*
 * Declarar una matriz de enteros.
 *
 * Ejemplo:
 *
 * matriz * declare_int[2][3];
 */
static void declarar_matriz(
    Ast *arbol,
    AstNodo *bloque
)
{
    AstNodo *declaracion = crear(
        arbol,
        AST_DECLARACION,
        NULL
    );

    agregar(bloque, declaracion);

    agregar(
        declaracion,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            "matriz"
        )
    );

    AstNodo *tipo_arreglo = crear(
        arbol,
        AST_TIPO_ARREGLO,
        NULL
    );

    agregar(declaracion, tipo_arreglo);

    agregar(
        tipo_arreglo,
        crear(
            arbol,
            AST_TIPO,
            "declare_int"
        )
    );

    AstNodo *dimensiones = crear(
        arbol,
        AST_DIMENSIONES,
        NULL
    );

    agregar(tipo_arreglo, dimensiones);

    /*
     * Primera dimensión: 2.
     */
    AstNodo *dimension1 = crear(
        arbol,
        AST_DIMENSION,
        NULL
    );

    agregar(dimensiones, dimension1);

    agregar(
        dimension1,
        crear(
            arbol,
            AST_ENTERO,
            "2"
        )
    );

    /*
     * Segunda dimensión: 3.
     */
    AstNodo *dimension2 = crear(
        arbol,
        AST_DIMENSION,
        NULL
    );

    agregar(dimensiones, dimension2);

    agregar(
        dimension2,
        crear(
            arbol,
            AST_ENTERO,
            "3"
        )
    );
}

/*
 * Construir un acceso a matriz.
 *
 * Ejemplo:
 *
 * matriz[1][2]
 *
 * cantidad = 1 -> matriz[1]
 * cantidad = 2 -> matriz[1][2]
 * cantidad = 3 -> matriz[1][2][0]
 */
static AstNodo *acceso_matriz(
    Ast *arbol,
    const char *indice1,
    const char *indice2,
    unsigned cantidad
)
{
    AstNodo *actual = crear(
        arbol,
        AST_IDENTIFICADOR,
        "matriz"
    );

    const char *indices[3] = {
        indice1,
        indice2,
        "0"
    };

    for (unsigned i = 0; i < cantidad; ++i) {

        AstNodo *indice = crear(
            arbol,
            AST_INDICE,
            NULL
        );

        agregar(indice, actual);

        agregar(
            indice,
            crear(
                arbol,
                AST_ENTERO,
                indices[i]
            )
        );

        actual = indice;
    }

    return actual;
}

/*
 * Construir una asignación a la matriz.
 *
 * Ejemplo:
 *
 * matriz[1][2] : 7;
 */
static void asignar_matriz(
    Ast *arbol,
    AstNodo *bloque,
    const char *indice1,
    const char *indice2,
    unsigned cantidad
)
{
    AstNodo *asignacion = crear(
        arbol,
        AST_ASIGNACION,
        NULL
    );

    agregar(bloque, asignacion);

    agregar(
        asignacion,
        acceso_matriz(
            arbol,
            indice1,
            indice2,
            cantidad
        )
    );

    agregar(
        asignacion,
        crear(
            arbol,
            AST_ENTERO,
            "7"
        )
    );
}

/*
 * Crear un ciclo cycle.
 *
 * Ejemplo:
 *
 * cycle i let 0 until 5 step 1 {
 * }
 * endgame;
 *
 * Devuelve el bloque del ciclo.
 */
static AstNodo *crear_cycle(
    Ast *arbol,
    AstNodo *bloque,
    const char *variable,
    const char *inicio,
    const char *limite,
    const char *paso
)
{
    AstNodo *ciclo = crear(
        arbol,
        AST_RECORRIDO,
        NULL
    );

    agregar(bloque, ciclo);

    /*
     * Hijo 0: variable de control.
     */
    agregar(
        ciclo,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            variable
        )
    );

    /*
     * Hijo 1: inicio.
     */
    agregar(
        ciclo,
        crear(
            arbol,
            AST_ENTERO,
            inicio
        )
    );

    /*
     * Hijo 2: límite.
     */
    agregar(
        ciclo,
        crear(
            arbol,
            AST_ENTERO,
            limite
        )
    );

    /*
     * Hijo 3: paso.
     */
    agregar(
        ciclo,
        crear(
            arbol,
            AST_ENTERO,
            paso
        )
    );

    /*
     * Hijo 4: cuerpo.
     */
    AstNodo *cuerpo = crear(
        arbol,
        AST_BLOQUE,
        NULL
    );

    agregar(ciclo, cuerpo);

    return cuerpo;
}

/*
 * Asignar una variable a otra.
 *
 * Ejemplo:
 *
 * resultado : i;
 */
static void asignar_variable(
    Ast *arbol,
    AstNodo *bloque,
    const char *destino,
    const char *origen
)
{
    AstNodo *asignacion = crear(
        arbol,
        AST_ASIGNACION,
        NULL
    );

    agregar(bloque, asignacion);

    agregar(
        asignacion,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            destino
        )
    );

    agregar(
        asignacion,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            origen
        )
    );
}

/*
 * Construir los casos de prueba.
 *
 * Caso 0:
 *     Acceso correcto a matriz[1][2].
 *
 * Caso 1:
 *     Primera dimensión fuera de rango.
 *
 * Caso 2:
 *     Segunda dimensión fuera de rango.
 *
 * Caso 3:
 *     Cantidad insuficiente de índices.
 *
 * Caso 4:
 *     Demasiados índices.
 *
 * Caso 5:
 *     Cycle válido.
 *
 * Caso 6:
 *     Cycle con paso cero.
 *
 * Caso 7:
 *     Cycles anidados.
 *
 * Caso 8:
 *     Uso de variable de cycle
 *     fuera de su ámbito.
 */
static Ast construir_programa(int caso)
{
    Ast arbol = {0};

    AstNodo *main_bloque = crear_main(
        &arbol
    );

    /*
     * Casos relacionados con matrices.
     */
    if (caso >= 0 && caso <= 4) {

        declarar_matriz(
            &arbol,
            main_bloque
        );

        switch (caso) {

            case 0:
                asignar_matriz(
                    &arbol,
                    main_bloque,
                    "1",
                    "2",
                    2
                );
                break;

            case 1:
                asignar_matriz(
                    &arbol,
                    main_bloque,
                    "2",
                    "1",
                    2
                );
                break;

            case 2:
                asignar_matriz(
                    &arbol,
                    main_bloque,
                    "1",
                    "3",
                    2
                );
                break;

            case 3:
                asignar_matriz(
                    &arbol,
                    main_bloque,
                    "1",
                    "0",
                    1
                );
                break;

            case 4:
                asignar_matriz(
                    &arbol,
                    main_bloque,
                    "1",
                    "2",
                    3
                );
                break;
        }
    }

    /*
     * Casos relacionados con cycle.
     */
    if (caso >= 5 && caso <= 8) {

        declarar_entero(
            &arbol,
            main_bloque,
            "resultado"
        );

        if (caso == 5) {

            /*
             * Cycle válido.
             */
            AstNodo *cuerpo = crear_cycle(
                &arbol,
                main_bloque,
                "i",
                "0",
                "5",
                "1"
            );

            asignar_variable(
                &arbol,
                cuerpo,
                "resultado",
                "i"
            );

        } else if (caso == 6) {

            /*
             * Paso cero: debe fallar.
             */
            (void)crear_cycle(
                &arbol,
                main_bloque,
                "i",
                "0",
                "5",
                "0"
            );

        } else if (caso == 7) {

            /*
             * Cycles anidados.
             *
             * Las variables i y j están
             * disponibles dentro del
             * ciclo interno.
             */
            AstNodo *externo = crear_cycle(
                &arbol,
                main_bloque,
                "i",
                "0",
                "2",
                "1"
            );

            AstNodo *interno = crear_cycle(
                &arbol,
                externo,
                "j",
                "0",
                "3",
                "1"
            );

            asignar_variable(
                &arbol,
                interno,
                "resultado",
                "i"
            );

            asignar_variable(
                &arbol,
                interno,
                "resultado",
                "j"
            );

        } else if (caso == 8) {

            /*
             * La variable i solo existe
             * dentro del ciclo.
             */
            (void)crear_cycle(
                &arbol,
                main_bloque,
                "i",
                "0",
                "5",
                "1"
            );

            /*
             * Debe producir un error:
             * i está fuera de ámbito.
             */
            asignar_variable(
                &arbol,
                main_bloque,
                "resultado",
                "i"
            );
        }
    }

    return arbol;
}

/*
 * Ejecutar las nueve pruebas.
 */
int main(void)
{
    for (int caso = 0; caso < 9; ++caso) {

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
            "matrix_cycle.bal",
            stderr
        );

        /*
         * Casos válidos:
         *
         * 0, 5, 7
         *
         * Casos inválidos:
         *
         * 1, 2, 3, 4, 6, 8
         */
        int esperado = (
            caso == 0 ||
            caso == 5 ||
            caso == 7
        ) ? 0 : 1;

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
        "OK: matrices bidimensionales, "
        "cycle y ambitos"
    );

    return 0;
}