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

    AstNodo *n = ast_crear(a, tipo, texto, u);
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
 * Programa con main.
 */
static AstNodo *crear_main(Ast *a)
{
    a->raiz = crear(a, AST_PROGRAMA, NULL);

    AstNodo *f = crear(a, AST_FUNCION, NULL);
    agregar(a->raiz, f);

    agregar(
        f,
        crear(a, AST_IDENTIFICADOR, "main")
    );

    agregar(
        f,
        crear(a, AST_TIPO, "declare_infinite_void")
    );

    agregar(
        f,
        crear(a, AST_PARAMETROS, NULL)
    );

    AstNodo *b = crear(a, AST_BLOQUE, NULL);
    agregar(f, b);

    return b;
}

/*
 * Declarar:
 *
 * filas * declare_int : 3;
 *
 * o:
 *
 * bandera * declare_boolean : declare_true;
 */
static void declarar_variable(
    Ast *a,
    AstNodo *bloque,
    const char *nombre,
    int booleano
)
{
    AstNodo *d = crear(a, AST_DECLARACION, NULL);
    agregar(bloque, d);

    agregar(
        d,
        crear(a, AST_IDENTIFICADOR, nombre)
    );

    agregar(
        d,
        crear(
            a,
            AST_TIPO,
            booleano
                ? "declare_boolean"
                : "declare_int"
        )
    );

    agregar(
        d,
        crear(
            a,
            booleano
                ? AST_BOOLEANO
                : AST_ENTERO,
            booleano
                ? "declare_true"
                : "3"
        )
    );
}

/*
 * Agregar una dimension cuyo tamano
 * es una expresion del AST.
 */
static void dimension(
    Ast *a,
    AstNodo *dims,
    AstNodo *exp
)
{
    AstNodo *d = crear(a, AST_DIMENSION, NULL);
    agregar(dims, d);
    agregar(d, exp);
}

/*
 * Crear:
 *
 * matriz * declare_int[...];
 */
static AstNodo *crear_tipo_matriz(
    Ast *a,
    AstNodo *bloque
)
{
    AstNodo *decl = crear(a, AST_DECLARACION, NULL);
    agregar(bloque, decl);

    agregar(
        decl,
        crear(a, AST_IDENTIFICADOR, "matriz")
    );

    AstNodo *tipo = crear(
        a,
        AST_TIPO_ARREGLO,
        NULL
    );

    agregar(decl, tipo);

    agregar(
        tipo,
        crear(a, AST_TIPO, "declare_int")
    );

    AstNodo *dims = crear(
        a,
        AST_DIMENSIONES,
        NULL
    );

    agregar(tipo, dims);

    return dims;
}

/*
 * Crear expresion:
 *
 * filas gauss 1
 */
static AstNodo *crear_suma(Ast *a)
{
    AstNodo *suma = crear(
        a,
        AST_BINARIO,
        "gauss"
    );

    agregar(
        suma,
        crear(a, AST_IDENTIFICADOR, "filas")
    );

    agregar(
        suma,
        crear(a, AST_ENTERO, "1")
    );

    return suma;
}

/*
 * Crear expresion:
 *
 * neumann 2
 */
static AstNodo *crear_negativo(Ast *a)
{
    AstNodo *negativo = crear(
        a,
        AST_UNARIO,
        "neumann"
    );

    agregar(
        negativo,
        crear(a, AST_ENTERO, "2")
    );

    return negativo;
}

/*
 * Casos:
 *
 * 0 -> dimension estatica valida
 * 1 -> dimension dinamica valida
 * 2 -> expresion dinamica valida
 * 3 -> matriz mixta valida
 * 4 -> dimension booleana
 * 5 -> variable no declarada
 * 6 -> dimension cero
 * 7 -> dimension negativa
 * 8 -> expresion booleana como dimension
 */
static Ast construir(int caso)
{
    Ast a = {0};
    AstNodo *bloque = crear_main(&a);

    if (caso == 1 ||
        caso == 2 ||
        caso == 3) {

        declarar_variable(
            &a,
            bloque,
            "filas",
            0
        );
    }

    if (caso == 4 ||
        caso == 8) {

        declarar_variable(
            &a,
            bloque,
            "bandera",
            1
        );
    }

    AstNodo *dims = crear_tipo_matriz(
        &a,
        bloque
    );

    switch (caso) {

        case 0:
            dimension(
                &a,
                dims,
                crear(&a, AST_ENTERO, "5")
            );
            break;

        case 1:
            dimension(
                &a,
                dims,
                crear(
                    &a,
                    AST_IDENTIFICADOR,
                    "filas"
                )
            );
            break;

        case 2:
            dimension(
                &a,
                dims,
                crear_suma(&a)
            );
            break;

        case 3:
            dimension(
                &a,
                dims,
                crear(&a, AST_ENTERO, "2")
            );

            dimension(
                &a,
                dims,
                crear(
                    &a,
                    AST_IDENTIFICADOR,
                    "filas"
                )
            );
            break;

        case 4:
            dimension(
                &a,
                dims,
                crear(
                    &a,
                    AST_BOOLEANO,
                    "declare_true"
                )
            );
            break;

        case 5:
            dimension(
                &a,
                dims,
                crear(
                    &a,
                    AST_IDENTIFICADOR,
                    "desconocida"
                )
            );
            break;

        case 6:
            dimension(
                &a,
                dims,
                crear(&a, AST_ENTERO, "0")
            );
            break;

        case 7:
            dimension(
                &a,
                dims,
                crear_negativo(&a)
            );
            break;

        case 8:
            dimension(
                &a,
                dims,
                crear(
                    &a,
                    AST_IDENTIFICADOR,
                    "bandera"
                )
            );
            break;
    }

    return a;
}

int main(void)
{
    for (int caso = 0; caso < 9; ++caso) {

        Ast arbol = construir(caso);
        SimTabla tabla = {0};

        assert(
            sim_inicializar(&tabla) == SIM_OK
        );

        int resultado = analizar_semantica(
            &arbol,
            &tabla,
            "dimensiones_dinamicas.bal",
            stderr
        );

        /*
         * Casos validos: 0, 1, 2 y 3.
         * Casos invalidos: 4, 5, 6, 7 y 8.
         */
        int esperado = caso < 4 ? 0 : 1;

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
        "OK: dimensiones estaticas y dinamicas"
    );

    return 0;
}