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
 * Declarar una lista de enteros.
 *
 * caso 1: incluir un elemento booleano.
 * caso 2: inicializador escalar.
 * resto: inicializador [1, 2].
 */
static void declarar_lista(
    Ast *a,
    AstNodo *bloque,
    int caso
)
{
    AstNodo *lista = crear(a, AST_LISTA, NULL);
    agregar(bloque, lista);

    agregar(
        lista,
        crear(a, AST_IDENTIFICADOR, "numeros")
    );

    agregar(
        lista,
        crear(a, AST_TIPO, "declare_int")
    );

    if (caso == 2) {
        agregar(
            lista,
            crear(a, AST_ENTERO, "5")
        );

        return;
    }

    AstNodo *literal = crear(
        a,
        AST_LITERAL_COLECCION,
        NULL
    );

    agregar(lista, literal);

    agregar(
        literal,
        crear(a, AST_ENTERO, "1")
    );

    agregar(
        literal,
        crear(
            a,
            caso == 1 ? AST_BOOLEANO : AST_ENTERO,
            caso == 1 ? "declare_true" : "2"
        )
    );
}

/*
 * Crear add(lista, valor)
 * o remove(lista, indice).
 */
static void operacion_lista(
    Ast *a,
    AstNodo *bloque,
    AstTipo operacion,
    const char *nombre,
    int booleano
)
{
    AstNodo *n = crear(a, operacion, NULL);
    agregar(bloque, n);

    agregar(
        n,
        crear(a, AST_IDENTIFICADOR, nombre)
    );

    agregar(
        n,
        crear(
            a,
            booleano ? AST_BOOLEANO : AST_ENTERO,
            booleano ? "declare_true" : "1"
        )
    );
}

/*
 * Declarar:
 *
 * cantidad * declare_int : size(nombre);
 */
static void declarar_tamano(
    Ast *a,
    AstNodo *bloque,
    const char *nombre
)
{
    AstNodo *decl = crear(
        a,
        AST_DECLARACION,
        NULL
    );

    agregar(bloque, decl);

    agregar(
        decl,
        crear(a, AST_IDENTIFICADOR, "cantidad")
    );

    agregar(
        decl,
        crear(a, AST_TIPO, "declare_int")
    );

    AstNodo *tamano = crear(
        a,
        AST_TAMANO,
        NULL
    );

    agregar(decl, tamano);

    agregar(
        tamano,
        crear(a, AST_IDENTIFICADOR, nombre)
    );
}

/*
 * Crear una variable escalar.
 */
static void declarar_escalar(
    Ast *a,
    AstNodo *bloque
)
{
    AstNodo *decl = crear(
        a,
        AST_DECLARACION,
        NULL
    );

    agregar(bloque, decl);

    agregar(
        decl,
        crear(a, AST_IDENTIFICADOR, "numero")
    );

    agregar(
        decl,
        crear(a, AST_TIPO, "declare_int")
    );

    agregar(
        decl,
        crear(a, AST_ENTERO, "5")
    );
}

/*
 * Casos:
 *
 * 0: lista valida.
 * 1: inicializador con booleano.
 * 2: inicializador escalar.
 * 3: add con entero.
 * 4: add con booleano.
 * 5: remove con entero.
 * 6: remove con booleano.
 * 7: size(lista).
 * 8: size(escalar).
 * 9: add(lista inexistente).
 */
static Ast construir(int caso)
{
    Ast a = {0};
    AstNodo *bloque = crear_main(&a);

    if (caso == 8) {

        declarar_escalar(&a, bloque);

        declarar_tamano(
            &a,
            bloque,
            "numero"
        );

        return a;
    }

    if (caso != 9) {
        declarar_lista(&a, bloque, caso);
    }

    switch (caso) {

        case 3:
            operacion_lista(
                &a,
                bloque,
                AST_AGREGAR,
                "numeros",
                0
            );
            break;

        case 4:
            operacion_lista(
                &a,
                bloque,
                AST_AGREGAR,
                "numeros",
                1
            );
            break;

        case 5:
            operacion_lista(
                &a,
                bloque,
                AST_ELIMINAR,
                "numeros",
                0
            );
            break;

        case 6:
            operacion_lista(
                &a,
                bloque,
                AST_ELIMINAR,
                "numeros",
                1
            );
            break;

        case 7:
            declarar_tamano(
                &a,
                bloque,
                "numeros"
            );
            break;

        case 9:
            operacion_lista(
                &a,
                bloque,
                AST_AGREGAR,
                "inexistente",
                0
            );
            break;
    }

    return a;
}

int main(void)
{
    for (int caso = 0; caso < 10; ++caso) {

        Ast a = construir(caso);
        SimTabla tabla = {0};

        assert(sim_inicializar(&tabla) == SIM_OK);

        int obtenido = analizar_semantica(
            &a,
            &tabla,
            "listas.bal",
            stderr
        );

        int valido =
            caso == 0 ||
            caso == 3 ||
            caso == 5 ||
            caso == 7;

        int esperado = valido ? 0 : 1;

        fprintf(
            stderr,
            "CASO %d: esperado=%d, obtenido=%d\n",
            caso,
            esperado,
            obtenido
        );

        assert(obtenido == esperado);

        sim_liberar(&tabla);
        ast_liberar(&a);
    }

    puts(
        "OK: listas, inicializadores, "
        "add, remove y size"
    );

    return 0;
}