#include "semantic.h"

#include <assert.h>
#include <stdio.h>

/*
 * Crear un nodo del AST.
 */
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

/*
 * Agregar un nodo hijo.
 */
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
 * Crear una funcion y devolver
 * su bloque principal.
 */
static AstNodo *crear_funcion(
    Ast *a,
    const char *nombre,
    const char *tipo_retorno
)
{
    AstNodo *funcion = crear(
        a,
        AST_FUNCION,
        NULL
    );

    agregar(
        a->raiz,
        funcion
    );

    agregar(
        funcion,
        crear(
            a,
            AST_IDENTIFICADOR,
            nombre
        )
    );

    agregar(
        funcion,
        crear(
            a,
            AST_TIPO,
            tipo_retorno
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

    AstNodo *cuerpo = crear(
        a,
        AST_BLOQUE,
        NULL
    );

    agregar(funcion, cuerpo);

    return cuerpo;
}

/*
 * Agregar una instruccion give.
 *
 * Si valor == NULL, se crea give
 * sin expresion.
 */
static void agregar_give(
    Ast *a,
    AstNodo *bloque,
    const char *valor
)
{
    AstNodo *retorno = crear(
        a,
        AST_RETORNO,
        NULL
    );

    agregar(
        bloque,
        retorno
    );

    if (valor) {
        agregar(
            retorno,
            crear(
                a,
                AST_ENTERO,
                valor
            )
        );
    }
}

/*
 * Agregar una instruccion
 * que no garantiza retorno.
 *
 * Usamos un ciclo whale con
 * condicion booleana valida.
 */
static void agregar_whale(
    Ast *a,
    AstNodo *bloque,
    int retorna
)
{
    AstNodo *ciclo = crear(
        a,
        AST_MIENTRAS,
        NULL
    );

    agregar(
        bloque,
        ciclo
    );

    agregar(
        ciclo,
        crear(
            a,
            AST_BOOLEANO,
            "declare_true"
        )
    );

    AstNodo *cuerpo = crear(
        a,
        AST_BLOQUE,
        NULL
    );

    agregar(
        ciclo,
        cuerpo
    );

    if (retorna) {
        agregar_give(
            a,
            cuerpo,
            "1"
        );
    }
}

/*
 * Crear una estructura whether.
 *
 * Devuelve el bloque de la
 * rama verdadera.
 *
 * El nodo whether queda accesible
 * mediante el parametro salida.
 */
static AstNodo *crear_si(
    Ast *a,
    AstNodo *bloque,
    const char *clase,
    AstNodo **salida
)
{
    AstNodo *si = crear(
        a,
        AST_SI,
        clase
    );

    agregar(
        bloque,
        si
    );

    agregar(
        si,
        crear(
            a,
            AST_BOOLEANO,
            "declare_true"
        )
    );

    AstNodo *verdadero = crear(
        a,
        AST_BLOQUE,
        NULL
    );

    agregar(
        si,
        verdadero
    );

    *salida = si;

    return verdadero;
}

/*
 * Agregar el bloque also
 * como tercer hijo de AST_SI.
 */
static AstNodo *agregar_also(
    Ast *a,
    AstNodo *si
)
{
    AstNodo *alternativa = crear(
        a,
        AST_BLOQUE,
        NULL
    );

    agregar(
        si,
        alternativa
    );

    return alternativa;
}

/*
 * Construir programas para comprobar
 * las rutas de retorno.
 */
static Ast construir_programa(int caso)
{
    Ast a = {0};

    a.raiz = crear(
        &a,
        AST_PROGRAMA,
        NULL
    );

    /*
     * Siempre agregamos main para
     * cumplir la comprobacion de
     * funcion principal.
     */
    (void)crear_funcion(
        &a,
        "main",
        "declare_infinite_void"
    );

    /*
     * Caso 7:
     * Funcion void sin retorno.
     */
    if (caso == 7) {

        (void)crear_funcion(
            &a,
            "calcular",
            "declare_infinite_void"
        );

        return a;
    }

    /*
     * El resto de casos utilizan una
     * funcion que debe retornar int.
     */
    AstNodo *cuerpo = crear_funcion(
        &a,
        "calcular",
        "declare_int"
    );

    switch (caso) {

        /*
         * Caso 0:
         * give 5;
         */
        case 0:
            agregar_give(
                &a,
                cuerpo,
                "5"
            );
            break;

        /*
         * Caso 1:
         * Funcion sin give.
         */
        case 1:
            break;

        /*
         * Caso 2:
         * whether sin also.
         */
        case 2: {
            AstNodo *si = NULL;

            AstNodo *verdadero = crear_si(
                &a,
                cuerpo,
                "whether",
                &si
            );

            agregar_give(
                &a,
                verdadero,
                "1"
            );

            break;
        }

        /*
         * Caso 3:
         * whether y also retornan.
         */
        case 3: {
            AstNodo *si = NULL;

            AstNodo *verdadero = crear_si(
                &a,
                cuerpo,
                "whether",
                &si
            );

            agregar_give(
                &a,
                verdadero,
                "1"
            );

            AstNodo *alternativa = agregar_also(
                &a,
                si
            );

            agregar_give(
                &a,
                alternativa,
                "0"
            );

            break;
        }

        /*
         * Caso 4:
         * Solo retorna whether.
         */
        case 4: {
            AstNodo *si = NULL;

            AstNodo *verdadero = crear_si(
                &a,
                cuerpo,
                "whether",
                &si
            );

            agregar_give(
                &a,
                verdadero,
                "1"
            );

            (void)agregar_also(
                &a,
                si
            );

            break;
        }

        /*
         * Caso 5:
         *
         * whether -> give 1
         * alif    -> give 2
         * also    -> give 3
         */
        case 5: {
            AstNodo *si = NULL;

            AstNodo *verdadero = crear_si(
                &a,
                cuerpo,
                "whether",
                &si
            );

            agregar_give(
                &a,
                verdadero,
                "1"
            );

            /*
             * Crear alif como alternativa
             * del whether.
             */
            AstNodo *alif = crear(
                &a,
                AST_SI,
                "alif"
            );

            agregar(
                si,
                alif
            );

            agregar(
                alif,
                crear(
                    &a,
                    AST_BOOLEANO,
                    "declare_true"
                )
            );

            AstNodo *bloque_alif = crear(
                &a,
                AST_BLOQUE,
                NULL
            );

            agregar(
                alif,
                bloque_alif
            );

            agregar_give(
                &a,
                bloque_alif,
                "2"
            );

            AstNodo *alternativa = agregar_also(
                &a,
                alif
            );

            agregar_give(
                &a,
                alternativa,
                "3"
            );

            break;
        }

        /*
         * Caso 6:
         * El give solo esta dentro
         * de un whale.
         */
        case 6:
            agregar_whale(
                &a,
                cuerpo,
                1
            );
            break;
    }

    return a;
}

/*
 * Ejecutar las pruebas.
 */
int main(void)
{
    for (int caso = 0; caso < 8; ++caso) {

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
            "retornos.bal",
            stderr
        );

        /*
         * Casos validos:
         * 0, 3, 5 y 7.
         *
         * Casos invalidos:
         * 1, 2, 4 y 6.
         */
        int esperado = (
            caso == 0 ||
            caso == 3 ||
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

        assert(
            resultado == esperado
        );

        sim_liberar(
            &tabla
        );

        ast_liberar(
            &arbol
        );
    }

    puts(
        "OK: retornos obligatorios "
        "y rutas de ejecucion"
    );

    return 0;
}