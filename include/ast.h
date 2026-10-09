/* Comprueba que esta cabecera no se haya incluido antes en la misma unidad de traducción. */
#ifndef MEIBEL_AST_H
/* Define la macro de protección para evitar declaraciones duplicadas al incluir la cabecera varias veces. */
#define MEIBEL_AST_H

/* Incluye la definición de size_t, usada para representar líneas y columnas. */
#include <stddef.h>
/* Incluye la definición de FILE, usada para indicar el flujo de salida al imprimir el árbol. */
#include <stdio.h>

/* Bison usa estos cuatro campos con %locations. Columnas Unicode, desde 1.
 * La posicion final es exclusiva: apunta justo despues del texto reconocido.
 */
/* Define una estructura para almacenar el intervalo del código fuente asociado a un nodo. */
typedef struct {
    /* Guarda la línea y la columna iniciales del intervalo. */
    size_t first_line, first_column;
    /* Guarda la línea y la columna finales; esta posición queda justo después del texto reconocido. */
    size_t last_line, last_column;
/* Asigna el nombre AstUbicacion al tipo de estructura de ubicación. */
} AstUbicacion;

/* Define los tipos de nodo; sus valores consecutivos comienzan en cero y siguen el orden de la tabla de nombres de ast.c. */
typedef enum {
    /* Representa programas, funciones, listas de parámetros, parámetros individuales y tipos. */
    AST_PROGRAMA, AST_FUNCION, AST_PARAMETROS, AST_PARAMETRO, AST_TIPO,
    /* Representa bloques, declaraciones, asignaciones y operaciones binarias y unarias. */
    AST_BLOQUE, AST_DECLARACION, AST_ASIGNACION, AST_BINARIO, AST_UNARIO,
    /* Representa identificadores y literales de tipo entero, cadena, carácter y booleano. */
    AST_IDENTIFICADOR, AST_ENTERO, AST_CADENA, AST_CARACTER, AST_BOOLEANO,
    /* Representa condicionales, ciclos mientras, retornos, llamadas y listas de argumentos. */
    AST_SI, AST_MIENTRAS, AST_RETORNO, AST_LLAMADA, AST_ARGUMENTOS,
    /* Representa importaciones, constantes, grupos de declaraciones, tipos de arreglo y lista, dimensiones, índices, accesos a miembros, colecciones, operaciones de listas, recorridos y manejo de excepciones. */
    AST_IMPORTACION, AST_CONSTANTE, AST_DECLARACIONES, AST_TIPO_ARREGLO, AST_TIPO_LISTA, AST_DIMENSIONES, AST_DIMENSION, AST_INDICE, AST_ACCESO_MIEMBRO, AST_LITERAL_COLECCION, AST_LISTA, AST_AGREGAR, AST_ELIMINAR, AST_TAMANO, AST_RECORRIDO, AST_INTENTAR, AST_CAPTURAS, AST_CAPTURA
/* Asigna el nombre AstTipo al tipo enumerado que clasifica los nodos. */
} AstTipo;

/* Define la estructura etiquetada AstNodo; la etiqueta permite declarar punteros a este mismo tipo dentro de ella. */
typedef struct AstNodo {
    /* Indica la clase de construcción sintáctica representada por el nodo. */
    AstTipo tipo;
    /* Apunta a una copia del texto asociada al nodo, o a NULL si el nodo no tiene texto. */
    char *texto;                 /* Copia propia del nombre, literal u operador. */
    /* Almacena el intervalo del código fuente correspondiente al nodo. */
    AstUbicacion ubicacion;
    /* Apunta al primer hijo del nodo, o a NULL si no tiene hijos. */
    struct AstNodo *primer_hijo;
    /* Apunta al último hijo para permitir añadir nuevos hijos al final sin recorrer toda la lista. */
    struct AstNodo *ultimo_hijo;
    /* Apunta al siguiente hermano del nodo, o a NULL si es el último hermano. */
    struct AstNodo *siguiente;   /* Siguiente hermano, en orden del fuente. */
    /* Apunta al padre del nodo; la raíz no tiene padre. */
    struct AstNodo *padre;
    /* Enlaza con el siguiente nodo de la lista de reservas, independiente de los enlaces entre padres e hijos. */
    struct AstNodo *reservado_siguiente; /* Lista privada para liberar memoria. */
/* Permite usar el nombre AstNodo sin escribir struct AstNodo. */
} AstNodo;

/* Define la estructura que mantiene la raíz y las reservas de memoria del árbol sintáctico abstracto. */
typedef struct {
    /* Apunta a la raíz del árbol, o a NULL cuando no hay raíz. */
    AstNodo *raiz;
    /* Apunta al inicio de la lista de todos los nodos reservados, usada para liberar su memoria. */
    AstNodo *reservados;
/* Asigna el nombre Ast a la estructura que administra el árbol. */
} Ast;

/* El propietario es Ast: no liberar nodos ni textos individualmente.
 * Usar Ast arbol = {0}; y ast_liberar(&arbol) al terminar.
 * Todos los nodos, incluso los de un parseo incompleto, se liberan juntos.
 */
/* Declara la creación de un nodo registrado en el árbol, con copia opcional del texto; devuelve NULL si falla la reserva de memoria. */
AstNodo *ast_crear(Ast *arbol, AstTipo tipo, const char *texto, AstUbicacion ubicacion);
/* hijo puede ser NULL; de otro modo debe estar sin padre ni hermanos. */
/* Declara la operación que añade un hijo al final de los hijos del padre y establece su enlace al padre. */
void ast_agregar_hijo(AstNodo *padre, AstNodo *hijo);
/* Declara la liberación de todos los nodos y sus textos, y el restablecimiento del árbol a su estado vacío. */
void ast_liberar(Ast *arbol);
/* Declara la consulta del nombre legible de un tipo de nodo; devuelve Desconocido si el valor no es válido. */
const char *ast_nombre_tipo(AstTipo tipo);
/* Recorrido iterativo: no consume pila C segun la profundidad del AST. */
/* Declara la impresión del árbol en el flujo indicado, sin modificarlo, con sangría, textos y ubicaciones. */
void ast_imprimir(const Ast *arbol, FILE *salida);

/* Cierra la protección contra inclusiones repetidas iniciada con la directiva ifndef. */
#endif
