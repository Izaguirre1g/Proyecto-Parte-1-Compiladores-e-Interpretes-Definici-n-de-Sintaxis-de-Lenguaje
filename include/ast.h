#ifndef MEIBEL_AST_H
#define MEIBEL_AST_H

#include <stddef.h>
#include <stdio.h>

/* Bison usa estos cuatro campos con %locations. Columnas Unicode, desde 1.
 * La posicion final es exclusiva: apunta justo despues del texto reconocido.
 */
typedef struct {
    size_t first_line, first_column;
    size_t last_line, last_column;
} AstUbicacion;

typedef enum {
    AST_PROGRAMA, AST_FUNCION, AST_PARAMETROS, AST_PARAMETRO, AST_TIPO,
    AST_BLOQUE, AST_DECLARACION, AST_ASIGNACION, AST_BINARIO, AST_UNARIO,
    AST_IDENTIFICADOR, AST_ENTERO, AST_CADENA, AST_CARACTER, AST_BOOLEANO,
    AST_SI, AST_MIENTRAS, AST_RETORNO, AST_LLAMADA, AST_ARGUMENTOS,
    AST_IMPORTACION, AST_CONSTANTE, AST_DECLARACIONES, AST_TIPO_ARREGLO, AST_TIPO_LISTA, AST_DIMENSIONES, AST_DIMENSION, AST_INDICE, AST_ACCESO_MIEMBRO, AST_LITERAL_COLECCION, AST_LISTA, AST_AGREGAR, AST_ELIMINAR, AST_TAMANO, AST_RECORRIDO, AST_INTENTAR, AST_CAPTURAS, AST_CAPTURA
} AstTipo;

typedef struct AstNodo {
    AstTipo tipo;
    char *texto;                 /* Copia propia del nombre, literal u operador. */
    AstUbicacion ubicacion;
    struct AstNodo *primer_hijo;
    struct AstNodo *ultimo_hijo;
    struct AstNodo *siguiente;   /* Siguiente hermano, en orden del fuente. */
    struct AstNodo *padre;
    struct AstNodo *reservado_siguiente; /* Lista privada para liberar memoria. */
} AstNodo;

typedef struct {
    AstNodo *raiz;
    AstNodo *reservados;
} Ast;

/* El propietario es Ast: no liberar nodos ni textos individualmente.
 * Usar Ast arbol = {0}; y ast_liberar(&arbol) al terminar.
 * Todos los nodos, incluso los de un parseo incompleto, se liberan juntos.
 */
AstNodo *ast_crear(Ast *arbol, AstTipo tipo, const char *texto, AstUbicacion ubicacion);
/* hijo puede ser NULL; de otro modo debe estar sin padre ni hermanos. */
void ast_agregar_hijo(AstNodo *padre, AstNodo *hijo);
void ast_liberar(Ast *arbol);
const char *ast_nombre_tipo(AstTipo tipo);
/* Recorrido iterativo: no consume pila C segun la profundidad del AST. */
void ast_imprimir(const Ast *arbol, FILE *salida);

#endif
