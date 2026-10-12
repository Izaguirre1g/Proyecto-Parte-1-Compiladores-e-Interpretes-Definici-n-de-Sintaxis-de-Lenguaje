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
    AstNodo *elemento
)
{
    ast_agregar_hijo(padre, elemento);
}

/*
 * Crear funcion:
 *
 * create_funk #declare_infinite_void# nombre()
 */
static AstNodo *funcion(
    Ast *a,
    const char *nombre,
    AstNodo **parametros
)
{
    AstNodo *f = crear(a, AST_FUNCION, NULL);

    agregar(a->raiz, f);
    agregar(f, crear(a, AST_IDENTIFICADOR, nombre));
    agregar(f, crear(a, AST_TIPO, "declare_infinite_void"));

    *parametros = crear(a, AST_PARAMETROS, NULL);
    agregar(f, *parametros);

    AstNodo *cuerpo = crear(a, AST_BLOQUE, NULL);
    agregar(f, cuerpo);

    return cuerpo;
}

/*
 * Crear tipo de arreglo con N dimensiones.
 *
 * Parametros: longitudes no especificadas.
 * Variables: cada dimension mide 3.
 */
static AstNodo *tipo_arreglo(
    Ast *a,
    const char *base,
    unsigned dimensiones,
    int parametro
)
{
    AstNodo *tipo = crear(a, AST_TIPO_ARREGLO, NULL);

    agregar(tipo, crear(a, AST_TIPO, base));

    AstNodo *dims = crear(a, AST_DIMENSIONES, NULL);
    agregar(tipo, dims);

    for (unsigned i = 0; i < dimensiones; ++i) {
        AstNodo *d = crear(a, AST_DIMENSION, NULL);
        agregar(dims, d);

        if (!parametro) {
            agregar(d, crear(a, AST_ENTERO, "3"));
        }
    }

    return tipo;
}

/*
 * Crear tipo de lista.
 */
static AstNodo *tipo_lista(
    Ast *a,
    const char *base
)
{
    AstNodo *tipo = crear(a, AST_TIPO_LISTA, NULL);

    agregar(tipo, crear(a, AST_TIPO, base));

    return tipo;
}

/*
 * Declarar parametro de arreglo o lista.
 */
static void parametro(
    Ast *a,
    AstNodo *params,
    int lista,
    const char *base,
    unsigned dimensiones
)
{
    AstNodo *p = crear(a, AST_PARAMETRO, NULL);
    agregar(params, p);

    agregar(p, crear(a, AST_IDENTIFICADOR, "datos"));

    agregar(
        p,
        lista
            ? tipo_lista(a, base)
            : tipo_arreglo(a, base, dimensiones, 1)
    );
}

/*
 * Declarar argumento compuesto en main.
 */
static void declarar_argumento(
    Ast *a,
    AstNodo *bloque,
    int lista,
    const char *base,
    unsigned dimensiones
)
{
    AstNodo *decl = crear(
        a,
        lista ? AST_LISTA : AST_DECLARACION,
        NULL
    );

    agregar(bloque, decl);

    agregar(
        decl,
        crear(a, AST_IDENTIFICADOR, "argumento")
    );

    agregar(
        decl,
        lista
            ? crear(a, AST_TIPO, base)
            : tipo_arreglo(a, base, dimensiones, 0)
    );
}

/*
 * Crear llamada:
 *
 * procesar(argumento);
 *
 * O, para el caso invalido:
 *
 * procesar(5);
 */
static void llamar(
    Ast *a,
    AstNodo *bloque,
    int pasar_entero
)
{
    AstNodo *llamada = crear(a, AST_LLAMADA, NULL);
    agregar(bloque, llamada);

    agregar(
        llamada,
        crear(a, AST_IDENTIFICADOR, "procesar")
    );

    AstNodo *args = crear(a, AST_ARGUMENTOS, NULL);
    agregar(llamada, args);

    agregar(
        args,
        crear(
            a,
            pasar_entero ? AST_ENTERO : AST_IDENTIFICADOR,
            pasar_entero ? "5" : "argumento"
        )
    );
}

/*
 * Dentro de procesar:
 *
 * temporal * declare_int : datos[0];
 *
 * Comprueba que el parametro sea indexable.
 */
static void usar_indice(
    Ast *a,
    AstNodo *bloque
)
{
    AstNodo *decl = crear(a, AST_DECLARACION, NULL);
    agregar(bloque, decl);

    agregar(
        decl,
        crear(a, AST_IDENTIFICADOR, "temporal")
    );

    agregar(
        decl,
        crear(a, AST_TIPO, "declare_int")
    );

    AstNodo *indice = crear(a, AST_INDICE, NULL);

    agregar(
        indice,
        crear(a, AST_IDENTIFICADOR, "datos")
    );

    agregar(
        indice,
        crear(a, AST_ENTERO, "0")
    );

    agregar(decl, indice);
}

/*
 * Casos:
 *
 * 0: arreglo int[] -> int[]          valido
 * 1: arreglo boolean[] -> int[]      error
 * 2: matriz int[][] -> int[]         error
 * 3: lista int -> lista int          valido
 * 4: arreglo int[] -> lista int      error
 * 5: entero 5 -> arreglo int[]       error
 * 6: usar datos[0] en parametro array valido
 * 7: usar datos[0] en parametro lista valido
 */
static Ast construir(int caso)
{
    Ast a = {0};

    a.raiz = crear(&a, AST_PROGRAMA, NULL);

    AstNodo *params_procesar = NULL;

    AstNodo *cuerpo_procesar = funcion(
        &a,
        "procesar",
        &params_procesar
    );

    int parametro_lista = caso == 3 ||
                          caso == 4 ||
                          caso == 7;

    parametro(
        &a,
        params_procesar,
        parametro_lista,
        "declare_int",
        1
    );

    if (caso == 6 || caso == 7) {
        usar_indice(&a, cuerpo_procesar);
    }

    AstNodo *params_main = NULL;

    AstNodo *cuerpo_main = funcion(
        &a,
        "main",
        &params_main
    );

    (void)params_main;

    if (caso != 5) {

        int argumento_lista = caso == 3 ||
                              caso == 7;

        const char *tipo_base = caso == 1
            ? "declare_boolean"
            : "declare_int";

        unsigned dimensiones = caso == 2
            ? 2u
            : 1u;

        declarar_argumento(
            &a,
            cuerpo_main,
            argumento_lista,
            tipo_base,
            dimensiones
        );
    }

    llamar(
        &a,
        cuerpo_main,
        caso == 5
    );

    return a;
}

int main(void)
{
    for (int caso = 0; caso < 8; ++caso) {

        Ast arbol = construir(caso);

        SimTabla tabla = {0};

        assert(sim_inicializar(&tabla) == SIM_OK);

        int obtenido = analizar_semantica(
            &arbol,
            &tabla,
            "parametros_compuestos.bal",
            stderr
        );

        int esperado = (
            caso == 0 ||
            caso == 3 ||
            caso == 6 ||
            caso == 7
        ) ? 0 : 1;

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
        "OK: parametros de arreglos, "
        "matrices y listas"
    );

    return 0;
}