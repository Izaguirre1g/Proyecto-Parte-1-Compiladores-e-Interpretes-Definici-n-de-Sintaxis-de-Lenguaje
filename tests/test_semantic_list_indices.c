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
 * Declarar:
 *
 * declare_list numeros * declare_int : [1, 2];
 */
static void declarar_lista(
    Ast *a,
    AstNodo *bloque
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
        crear(a, AST_ENTERO, "2")
    );
}

/*
 * Declarar:
 *
 * valor * declare_int;
 */
static void declarar_entero(
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
        crear(a, AST_IDENTIFICADOR, "valor")
    );

    agregar(
        decl,
        crear(a, AST_TIPO, "declare_int")
    );
}

/*
 * Crear acceso:
 *
 * numeros[indice]
 *
 * cantidad = 2 crea:
 * numeros[indice][0]
 */
static AstNodo *acceso_lista(
    Ast *a,
    AstTipo tipo_indice,
    const char *texto_indice,
    unsigned cantidad
)
{
    AstNodo *actual = crear(
        a,
        AST_IDENTIFICADOR,
        "numeros"
    );

    for (unsigned i = 0; i < cantidad; ++i) {

        AstNodo *indice = crear(
            a,
            AST_INDICE,
            NULL
        );

        agregar(indice, actual);

        agregar(
            indice,
            crear(
                a,
                i == 0
                    ? tipo_indice
                    : AST_ENTERO,
                i == 0
                    ? texto_indice
                    : "0"
            )
        );

        actual = indice;
    }

    return actual;
}

/*
 * Crear:
 *
 * numeros[0] : 5;
 *
 * o un caso incorrecto.
 */
static void asignar_elemento(
    Ast *a,
    AstNodo *bloque,
    AstTipo tipo_indice,
    const char *indice,
    unsigned cantidad,
    int valor_booleano
)
{
    AstNodo *asignacion = crear(
        a,
        AST_ASIGNACION,
        NULL
    );

    agregar(bloque, asignacion);

    agregar(
        asignacion,
        acceso_lista(
            a,
            tipo_indice,
            indice,
            cantidad
        )
    );

    agregar(
        asignacion,
        crear(
            a,
            valor_booleano
                ? AST_BOOLEANO
                : AST_ENTERO,
            valor_booleano
                ? "declare_true"
                : "5"
        )
    );
}

/*
 * Crear:
 *
 * valor : numeros[0];
 */
static void leer_elemento(
    Ast *a,
    AstNodo *bloque
)
{
    AstNodo *asignacion = crear(
        a,
        AST_ASIGNACION,
        NULL
    );

    agregar(bloque, asignacion);

    agregar(
        asignacion,
        crear(a, AST_IDENTIFICADOR, "valor")
    );

    agregar(
        asignacion,
        acceso_lista(
            a,
            AST_ENTERO,
            "0",
            1
        )
    );
}

/*
 * Casos:
 *
 * 0: numeros[0] : 5;           valido
 * 1: valor : numeros[0];       valido
 * 2: numeros[true] : 5;        invalido
 * 3: numeros[0][0] : 5;        invalido
 * 4: numeros[0] : true;        invalido
 * 5: numeros[desconocida] : 5; invalido
 */
static Ast construir(int caso)
{
    Ast a = {0};

    AstNodo *bloque = crear_main(&a);

    declarar_lista(&a, bloque);
    declarar_entero(&a, bloque);

    switch (caso) {

        case 0:
            asignar_elemento(
                &a,
                bloque,
                AST_ENTERO,
                "0",
                1,
                0
            );
            break;

        case 1:
            leer_elemento(
                &a,
                bloque
            );
            break;

        case 2:
            asignar_elemento(
                &a,
                bloque,
                AST_BOOLEANO,
                "declare_true",
                1,
                0
            );
            break;

        case 3:
            asignar_elemento(
                &a,
                bloque,
                AST_ENTERO,
                "0",
                2,
                0
            );
            break;

        case 4:
            asignar_elemento(
                &a,
                bloque,
                AST_ENTERO,
                "0",
                1,
                1
            );
            break;

        case 5:
            asignar_elemento(
                &a,
                bloque,
                AST_IDENTIFICADOR,
                "desconocida",
                1,
                0
            );
            break;
    }

    return a;
}

int main(void)
{
    for (int caso = 0; caso < 6; ++caso) {

        Ast arbol = construir(caso);
        SimTabla tabla = {0};

        assert(
            sim_inicializar(&tabla) == SIM_OK
        );

        int obtenido = analizar_semantica(
            &arbol,
            &tabla,
            "indices_listas.bal",
            stderr
        );

        int esperado = caso <= 1 ? 0 : 1;

        fprintf(
            stderr,
            "CASO %d: esperado=%d, obtenido=%d\n",
            caso,
            esperado,
            obtenido
        );

        assert(obtenido == esperado);

        sim_liberar(&tabla);
        ast_liberar(&arbol);
    }

    puts(
        "OK: indices de listas y "
        "tipos de elementos"
    );

    return 0;
}
