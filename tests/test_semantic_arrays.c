#include "semantic.h"

#include <assert.h>
#include <stdio.h>

static AstNodo *crear(
    Ast *arbol,
    AstTipo tipo,
    const char *texto
)
{
    AstUbicacion u = {1, 1, 1, 1};

    AstNodo *n = ast_crear(
        arbol,
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
    ast_agregar_hijo(
        padre,
        hijo_n
    );
}

/*
 * Construye un programa con main.
 */
static AstNodo *crear_main(Ast *arbol)
{
    arbol->raiz = crear(
        arbol,
        AST_PROGRAMA,
        NULL
    );

    AstNodo *f = crear(
        arbol,
        AST_FUNCION,
        NULL
    );

    agregar(arbol->raiz, f);

    agregar(
        f,
        crear(arbol, AST_IDENTIFICADOR, "main")
    );

    agregar(
        f,
        crear(
            arbol,
            AST_TIPO,
            "declare_infinite_void"
        )
    );

    agregar(
        f,
        crear(arbol, AST_PARAMETROS, NULL)
    );

    AstNodo *cuerpo = crear(
        arbol,
        AST_BLOQUE,
        NULL
    );

    agregar(f, cuerpo);

    return cuerpo;
}

/*
 * Declara:
 *
 * numeros * declare_int[tamano];
 */
static void declarar_arreglo(
    Ast *arbol,
    AstNodo *cuerpo,
    const char *tamano
)
{
    AstNodo *decl = crear(
        arbol,
        AST_DECLARACION,
        NULL
    );

    agregar(cuerpo, decl);

    agregar(
        decl,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            "numeros"
        )
    );

    AstNodo *tipo = crear(
        arbol,
        AST_TIPO_ARREGLO,
        NULL
    );

    agregar(decl, tipo);

    agregar(
        tipo,
        crear(
            arbol,
            AST_TIPO,
            "declare_int"
        )
    );

    AstNodo *dims = crear(
        arbol,
        AST_DIMENSIONES,
        NULL
    );

    agregar(tipo, dims);

    AstNodo *dimension = crear(
        arbol,
        AST_DIMENSION,
        NULL
    );

    agregar(dims, dimension);

    agregar(
        dimension,
        crear(
            arbol,
            AST_ENTERO,
            tamano
        )
    );
}

/*
 * Construye:
 *
 * numeros[indice] : 7;
 */
static void asignar_indice(
    Ast *arbol,
    AstNodo *cuerpo,
    AstTipo tipo_indice,
    const char *indice
)
{
    AstNodo *asignacion = crear(
        arbol,
        AST_ASIGNACION,
        NULL
    );

    agregar(cuerpo, asignacion);

    AstNodo *acceso = crear(
        arbol,
        AST_INDICE,
        NULL
    );

    agregar(asignacion, acceso);

    agregar(
        acceso,
        crear(
            arbol,
            AST_IDENTIFICADOR,
            "numeros"
        )
    );

    agregar(
        acceso,
        crear(
            arbol,
            tipo_indice,
            indice
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
 * Construye distintos programas.
 *
 * Caso 0: arreglo válido.
 * Caso 1: índice fuera de rango.
 * Caso 2: índice booleano.
 * Caso 3: dimensión cero.
 */
static Ast construir(int caso)
{
    Ast arbol = {0};

    AstNodo *cuerpo = crear_main(&arbol);

    declarar_arreglo(
        &arbol,
        cuerpo,
        caso == 3 ? "0" : "5"
    );

    if (caso != 3) {

        asignar_indice(
            &arbol,
            cuerpo,
            caso == 2
                ? AST_BOOLEANO
                : AST_ENTERO,
            caso == 1
                ? "8"
                : caso == 2
                    ? "declare_true"
                    : "2"
        );
    }

    return arbol;
}

/*
 * Ejecuta las pruebas.
 */
int main(void)
{
    for (int caso = 0; caso < 4; ++caso) {

        Ast arbol = construir(caso);

        SimTabla tabla = {0};

        assert(
            sim_inicializar(&tabla) == SIM_OK
        );

        int resultado = analizar_semantica(
            &arbol,
            &tabla,
            "arreglos.bal",
            stderr
        );

        fprintf(
            stderr,
            "CASO %d: esperado=%d, obtenido=%d\n",
            caso,
            caso == 0 ? 0 : 1,
            resultado
        );

        assert(
            resultado == (caso == 0 ? 0 : 1)
        );

        sim_liberar(&tabla);
        ast_liberar(&arbol);
    }

    puts(
        "OK: dimensiones, indices "
        "y limites de arreglos"
    );

    return 0;
}