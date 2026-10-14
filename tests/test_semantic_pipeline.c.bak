/*
 * =====================================================
 * ETAPA 2K
 * Prueba de integracion Parser + Semantica
 * =====================================================
 *
 * Uso:
 *
 * ./tests/test_semantic_pipeline archivo.bal 0
 * ./tests/test_semantic_pipeline archivo.bal 1
 *
 * 0 = programa semanticamente valido
 * 1 = programa con errores semanticos
 *
 * El ejecutable:
 *
 * 1. Lee el archivo .bal.
 * 2. Construye el AST usando el parser real.
 * 3. Inicializa la tabla de simbolos.
 * 4. Ejecuta el analisis semantico.
 * 5. Comprueba el resultado esperado.
 *
 * No modifica el comportamiento de balc.
 */

#include "sintactico.h"
#include "semantic.h"
#include "symbol_table.h"

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static char *leer_archivo(
    const char *ruta,
    size_t *longitud
)
{
    FILE *archivo = fopen(
        ruta,
        "rb"
    );

    if (!archivo) {
        perror(ruta);
        return NULL;
    }

    size_t capacidad = 4096;
    size_t usados = 0;

    char *datos = malloc(capacidad);

    if (!datos) {
        fclose(archivo);
        return NULL;
    }

    for (;;) {

        if (usados == capacidad) {

            if (capacidad > SIZE_MAX / 2) {

                fprintf(
                    stderr,
                    "archivo demasiado grande\n"
                );

                free(datos);
                fclose(archivo);

                return NULL;
            }

            size_t nueva_capacidad =
                capacidad * 2;

            char *nuevos = realloc(
                datos,
                nueva_capacidad
            );

            if (!nuevos) {

                free(datos);
                fclose(archivo);

                return NULL;
            }

            datos = nuevos;
            capacidad = nueva_capacidad;
        }

        size_t leidos = fread(
            datos + usados,
            1,
            capacidad - usados,
            archivo
        );

        usados += leidos;

        if (leidos == 0) {

            if (ferror(archivo)) {

                perror(ruta);

                free(datos);
                fclose(archivo);

                return NULL;
            }

            break;
        }
    }

    fclose(archivo);

    *longitud = usados;

    return datos;
}


static int ejecutar_pipeline(
    const char *ruta
)
{
    size_t longitud = 0;

    char *fuente = leer_archivo(
        ruta,
        &longitud
    );

    if (!fuente) {
        return 2;
    }

    /*
     * Primera fase:
     * analisis lexico y sintactico.
     */
    Ast arbol = {0};

    int estado = construir_ast(
        fuente,
        longitud,
        ruta,
        stderr,
        &arbol
    );

    free(fuente);

    if (estado != 0) {

        ast_liberar(&arbol);

        return estado;
    }

    /*
     * Segunda fase:
     * inicializar tabla de simbolos.
     */
    SimTabla tabla = {0};

    SimResultado resultado = sim_inicializar(
        &tabla
    );

    if (resultado != SIM_OK) {

        fprintf(
            stderr,
            "no se pudo inicializar "
            "la tabla de simbolos\n"
        );

        ast_liberar(&arbol);

        return 2;
    }

    /*
     * Tercera fase:
     * analisis semantico.
     */
        estado = analizar_semantica(
            &arbol,
            &tabla,
            ruta,
            stderr
        );

        /*
        * Verificar que la semantica no haya
        * asignado desplazamientos ni memoria.
        *
        * La comprobacion se realiza incluso
        * cuando existen errores semanticos.
        */
        if (!sim_verificar_pre_memoria(
                &tabla,
                stderr
            )) {
            fprintf(
                stderr,
                "ERROR: la tabla de simbolos "
                "no esta preparada para "
                "la etapa de memoria\n"
            );

            estado = 2;
        }

        /*
        * Liberar recursos.
        */
        sim_liberar(&tabla);
        ast_liberar(&arbol);

        return estado;
}

int main(
    int argc,
    char **argv
)
{
    if (argc != 3) {

        fprintf(
            stderr,
            "Uso: %s <archivo.bal> "
            "<resultado_esperado>\n",
            argv[0]
        );

        return 2;
    }

    /*
     * Leer el resultado esperado:
     *
     * 0: valido
     * 1: error semantico
     */
    char *fin = NULL;

    errno = 0;

    long esperado = strtol(
        argv[2],
        &fin,
        10
    );

    if (errno != 0 ||
        fin == argv[2] ||
        *fin != '\0' ||
        (esperado != 0 && esperado != 1)) {

        fprintf(
            stderr,
            "el resultado esperado "
            "debe ser 0 o 1\n"
        );

        return 2;
    }

    /*
     * Ejecutar parser y semantica.
     */
    int obtenido = ejecutar_pipeline(
        argv[1]
    );

    fprintf(
        stderr,
        "PIPELINE: esperado=%ld, "
        "obtenido=%d\n",
        esperado,
        obtenido
    );

    if (obtenido != (int)esperado) {

        fprintf(
            stderr,
            "ERROR: resultado inesperado "
            "del pipeline\n"
        );

        return 1;
    }

    puts(
        "OK: integracion "
        "parser + semantica"
    );

    return 0;
}