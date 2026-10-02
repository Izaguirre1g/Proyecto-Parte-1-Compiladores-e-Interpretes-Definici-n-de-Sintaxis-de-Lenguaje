#include "ast.h"

#include <stdlib.h>
#include <string.h>

AstNodo *ast_crear(Ast *arbol, AstTipo tipo, const char *texto, AstUbicacion ubicacion) {
    AstNodo *nodo = calloc(1, sizeof(*nodo));
    if (!nodo) return NULL;
    if (texto) {
        size_t longitud = strlen(texto);
        nodo->texto = malloc(longitud + 1);
        if (!nodo->texto) { free(nodo); return NULL; }
        memcpy(nodo->texto, texto, longitud + 1);
    }
    nodo->tipo = tipo;
    nodo->ubicacion = ubicacion;
    nodo->reservado_siguiente = arbol->reservados;
    arbol->reservados = nodo;
    return nodo;
}

void ast_agregar_hijo(AstNodo *padre, AstNodo *hijo) {
    if (!hijo) return;
    hijo->padre = padre;
    if (padre->ultimo_hijo) padre->ultimo_hijo->siguiente = hijo;
    else padre->primer_hijo = hijo;
    padre->ultimo_hijo = hijo;
}

void ast_liberar(Ast *arbol) {
    AstNodo *nodo = arbol->reservados;
    while (nodo) {
        AstNodo *siguiente = nodo->reservado_siguiente;
        free(nodo->texto);
        free(nodo);
        nodo = siguiente;
    }
    *arbol = (Ast){0};
}

const char *ast_nombre_tipo(AstTipo tipo) {
    static const char *const nombres[] = {
        "Programa", "Funcion", "Parametros", "Parametro", "Tipo",
        "Bloque", "Declaracion", "Asignacion", "Binario", "Unario",
        "Identificador", "Entero", "Cadena", "Caracter", "Booleano",
        "Si", "Mientras", "Retorno", "Llamada", "Argumentos",
        "Importacion", "Constante", "Declaraciones", "TipoArreglo", "TipoLista", "Dimensiones", "Dimension", "Indice", "AccesoMiembro", "LiteralColeccion", "Lista", "Agregar", "Eliminar", "Tamano", "Recorrido", "Intentar", "Capturas", "Captura"
    };
    return (unsigned)tipo < sizeof(nombres) / sizeof(nombres[0])
        ? nombres[tipo] : "Desconocido";
}

static void imprimir_texto(FILE *salida, const char *texto) {
    fputc('"', salida);
    for (const unsigned char *p = (const unsigned char *)texto; *p; ++p) {
        if (*p == '"' || *p == '\\') { fputc('\\', salida); fputc(*p, salida); }
        else if (*p == '\n') fputs("\\n", salida);
        else if (*p == '\r') fputs("\\r", salida);
        else if (*p == '\t') fputs("\\t", salida);
        else if (*p < 0x20 || *p == 0x7f) fprintf(salida, "\\x%02X", *p);
        else fputc(*p, salida);
    }
    fputc('"', salida);
}

void ast_imprimir(const Ast *arbol, FILE *salida) {
    const AstNodo *nodo = arbol->raiz;
    size_t profundidad = 0;
    while (nodo) {
        for (size_t i = 0; i < profundidad; ++i) fputs("  ", salida);
        fputs(ast_nombre_tipo(nodo->tipo), salida);
        if (nodo->texto) { fputc(' ', salida); imprimir_texto(salida, nodo->texto); }
        fprintf(salida, " @%zu:%zu\n", nodo->ubicacion.first_line, nodo->ubicacion.first_column);
        if (nodo->primer_hijo) {
            nodo = nodo->primer_hijo;
            ++profundidad;
        } else {
            while (nodo && !nodo->siguiente) {
                nodo = nodo->padre;
                if (profundidad) --profundidad;
            }
            if (nodo) nodo = nodo->siguiente;
        }
    }
}
