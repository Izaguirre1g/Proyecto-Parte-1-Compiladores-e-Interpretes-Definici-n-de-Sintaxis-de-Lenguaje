#include "semantic.h"

#include <assert.h>
#include <stdio.h>

/*
 * Crea un nodo del AST para las pruebas.
 */
static AstNodo *n(
    Ast *a,
    AstTipo t,
    const char *s
) {
    AstUbicacion u = {1, 1, 1, 1};

    AstNodo *r = ast_crear(
        a,
        t,
        s,
        u
    );

    assert(r);

    return r;
}

/*
 * Agrega un hijo a un nodo.
 */
static void agregar(
    AstNodo *padre,
    AstNodo *nuevo
) {
    ast_agregar_hijo(
        padre,
        nuevo
    );
}

/*
 * Construye cuatro casos:
 *
 * 0: programa válido.
 * 1: variable no declarada.
 * 2: asignación con tipos incompatibles.
 * 3: variable duplicada.
 */
static Ast construir(int caso)
{
    Ast a = {0};

    /*
     * Crear raíz del programa.
     */
    a.raiz = n(
        &a,
        AST_PROGRAMA,
        NULL
    );

    /*
     * Crear función main.
     */
    AstNodo *f = n(
        &a,
        AST_FUNCION,
        NULL
    );

    agregar(a.raiz, f);

    agregar(
        f,
        n(&a, AST_IDENTIFICADOR, "main")
    );

    agregar(
        f,
        n(
            &a,
            AST_TIPO,
            "declare_infinite_void"
        )
    );

    agregar(
        f,
        n(&a, AST_PARAMETROS, NULL)
    );

    /*
     * Crear cuerpo.
     */
    AstNodo *bloque = n(
        &a,
        AST_BLOQUE,
        NULL
    );

    agregar(f, bloque);

    /*
     * Declarar:
     *
     * x * declare_int : 3;
     */
    AstNodo *decl = n(
        &a,
        AST_DECLARACION,
        NULL
    );

    agregar(bloque, decl);

    agregar(
        decl,
        n(&a, AST_IDENTIFICADOR, "x")
    );

    agregar(
        decl,
        n(&a, AST_TIPO, "declare_int")
    );

    agregar(
        decl,
        n(&a, AST_ENTERO, "3")
    );

    /*
     * Construir asignación.
     *
     * Caso 0: x : 4;
     * Caso 1: y : 4;
     * Caso 2: x : declare_true;
     * Caso 3: x : 4; + redeclaración.
     */
    AstNodo *as = n(
        &a,
        AST_ASIGNACION,
        NULL
    );

    agregar(bloque, as);

    agregar(
        as,
        n(
            &a,
            AST_IDENTIFICADOR,
            caso == 1 ? "y" : "x"
        )
    );

    agregar(
        as,
        n(
            &a,
            caso == 2
                ? AST_BOOLEANO
                : AST_ENTERO,
            caso == 2
                ? "declare_true"
                : "4"
        )
    );

    /*
     * Caso 3:
     * Declarar x nuevamente.
     */
    if (caso == 3) {

        AstNodo *d2 = n(
            &a,
            AST_DECLARACION,
            NULL
        );

        agregar(bloque, d2);

        agregar(
            d2,
            n(&a, AST_IDENTIFICADOR, "x")
        );

        agregar(
            d2,
            n(&a, AST_TIPO, "declare_int")
        );
    }

    return a;
}

/*
 * Ejecutar pruebas.
 */
int main(void)
{
    for (int i = 0; i < 4; ++i) {

        Ast arbol = construir(i);

        SimTabla tabla = {0};

        /*
         * Inicializar tabla de símbolos.
         */
        assert(
            sim_inicializar(&tabla) == SIM_OK
        );

        /*
         * Ejecutar análisis semántico.
         */
        int resultado = analizar_semantica(
            &arbol,
            &tabla,
            "prueba.bal",
            NULL
        );

        /*
         * El caso 0 debe aceptarse.
         * Los otros tres deben rechazarse.
         */
        assert(
            resultado == (i == 0 ? 0 : 1)
        );

        /*
         * Deben registrarse al menos
         * main y la variable x.
         */
        assert(tabla.cantidad >= 2);

        /*
         * Liberar recursos.
         */
        sim_liberar(&tabla);
        ast_liberar(&arbol);
    }

    puts(
        "OK: semantic AST, variable no declarada, "
        "tipos y duplicados"
    );

    return 0;
}