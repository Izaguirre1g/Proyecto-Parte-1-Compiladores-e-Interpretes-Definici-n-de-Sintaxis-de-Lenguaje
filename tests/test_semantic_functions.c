#include "semantic.h"

#include <assert.h>
#include <stdio.h>

/*
 * Crear un nodo.
 */
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

/*
 * Agregar un nodo hijo.
 */
static void agregar(
    AstNodo *padre,
    AstNodo *hijo
)
{
    ast_agregar_hijo(padre, hijo);
}

/*
 * Crear una función y devolver su cuerpo.
 */
static AstNodo *crear_funcion(
    Ast *a,
    const char *nombre,
    const char *retorno,
    AstNodo **parametros
)
{
    AstNodo *funcion = crear(
        a,
        AST_FUNCION,
        NULL
    );

    agregar(a->raiz, funcion);

    agregar(
        funcion,
        crear(a, AST_IDENTIFICADOR, nombre)
    );

    agregar(
        funcion,
        crear(a, AST_TIPO, retorno)
    );

    *parametros = crear(
        a,
        AST_PARAMETROS,
        NULL
    );

    agregar(funcion, *parametros);

    AstNodo *cuerpo = crear(
        a,
        AST_BLOQUE,
        NULL
    );

    agregar(funcion, cuerpo);

    return cuerpo;
}

/*
 * Crear un parámetro.
 */
static void agregar_parametro(
    Ast *a,
    AstNodo *parametros,
    const char *nombre,
    const char *tipo
)
{
    AstNodo *p = crear(
        a,
        AST_PARAMETRO,
        NULL
    );

    agregar(parametros, p);

    agregar(
        p,
        crear(a, AST_IDENTIFICADOR, nombre)
    );

    agregar(
        p,
        crear(a, AST_TIPO, tipo)
    );
}

/*
 * Agregar un retorno give.
 */
static void agregar_retorno(
    Ast *a,
    AstNodo *cuerpo,
    AstTipo tipo,
    const char *valor
)
{
    AstNodo *retorno = crear(
        a,
        AST_RETORNO,
        NULL
    );

    agregar(cuerpo, retorno);

    if (valor) {
        agregar(
            retorno,
            crear(a, tipo, valor)
        );
    }
}

/*
 * Crear una llamada a una función.
 */
static void agregar_llamada(
    Ast *a,
    AstNodo *cuerpo,
    const char *nombre,
    AstTipo tipo_argumento,
    const char *argumento
)
{
    AstNodo *llamada = crear(
        a,
        AST_LLAMADA,
        NULL
    );

    agregar(cuerpo, llamada);

    agregar(
        llamada,
        crear(a, AST_IDENTIFICADOR, nombre)
    );

    AstNodo *args = crear(
        a,
        AST_ARGUMENTOS,
        NULL
    );

    agregar(llamada, args);

    if (argumento) {
        agregar(
            args,
            crear(
                a,
                tipo_argumento,
                argumento
            )
        );
    }
}

/*
 * Construir distintos programas.
 *
 * Caso 0: función y llamada correctas.
 * Caso 1: argumento de tipo incorrecto.
 * Caso 2: cantidad incorrecta de argumentos.
 * Caso 3: retorno incompatible.
 * Caso 4: main devuelve un entero.
 * Caso 5: falta main.
 */
static Ast construir_programa(int caso)
{
    Ast a = {0};

    a.raiz = crear(
        &a,
        AST_PROGRAMA,
        NULL
    );

    AstNodo *params = NULL;

    /*
     * Función identidad.
     */
    AstNodo *cuerpo = crear_funcion(
        &a,
        "identidad",
        "declare_int",
        &params
    );

    agregar_parametro(
        &a,
        params,
        "numero",
        "declare_int"
    );

    /*
     * identity devuelve su parámetro,
     * salvo en el caso 3.
     */
    if (caso == 3) {
        agregar_retorno(
            &a,
            cuerpo,
            AST_BOOLEANO,
            "declare_true"
        );
    } else {
        agregar_retorno(
            &a,
            cuerpo,
            AST_IDENTIFICADOR,
            "numero"
        );
    }

    /*
     * Caso 5: programa sin main.
     */
    if (caso == 5) {
        return a;
    }

    /*
     * Función main.
     */
    cuerpo = crear_funcion(
        &a,
        "main",
        caso == 4
            ? "declare_int"
            : "declare_infinite_void",
        &params
    );

    /*
     * Llamada a identidad.
     */
    if (caso == 1) {
        agregar_llamada(
            &a,
            cuerpo,
            "identidad",
            AST_BOOLEANO,
            "declare_true"
        );
    } else if (caso == 2) {
        agregar_llamada(
            &a,
            cuerpo,
            "identidad",
            AST_ENTERO,
            NULL
        );
    } else {
        agregar_llamada(
            &a,
            cuerpo,
            "identidad",
            AST_ENTERO,
            "10"
        );
    }

    return a;
}

/*
 * Ejecutar pruebas.
 */
int main(void)
{
    for (int caso = 0; caso < 6; ++caso) {

        Ast arbol = construir_programa(caso);

        SimTabla tabla = {0};

        assert(
            sim_inicializar(&tabla) == SIM_OK
        );

        int resultado = analizar_semantica(
            &arbol,
            &tabla,
            "funciones.bal",
            stderr
        );

        /*
        * Mostrar el caso y el resultado antes
        * de comprobar la condición.
        */
        fprintf(
            stderr,
            "CASO %d: esperado=%d, obtenido=%d\n",
            caso,
            caso == 0 ? 0 : 1,
            resultado
        );

        /*
         * Caso 0 debe aceptarse.
         * Casos 1-5 deben rechazarse.
         */
        assert(
            resultado == (caso == 0 ? 0 : 1)
        );

        sim_liberar(&tabla);
        ast_liberar(&arbol);
    }

    puts(
        "OK: funciones, argumentos, "
        "retornos y main"
    );

    return 0;
}