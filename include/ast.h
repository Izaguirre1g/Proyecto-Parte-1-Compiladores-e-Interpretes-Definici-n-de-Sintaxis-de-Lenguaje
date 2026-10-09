/* Define las estructuras y declara las funciones del árbol de sintaxis
 * abstracta (AST). Las funciones se implementan en ast.c.
 * Este encabezado es propio del proyecto; Bison no lo genera.
 */
#ifndef MEIBEL_AST_H /* Procesa el contenido solo si no se incluyó antes. */
#define MEIBEL_AST_H /* Marca el encabezado para evitar definiciones repetidas. */

#include <stddef.h> /* Proporciona size_t, usado para líneas y columnas. */
#include <stdio.h>  /* Proporciona FILE, usado para indicar dónde imprimir. */

/* Bison usa estos cuatro campos con %locations. Columnas Unicode, desde 1.
 * La posicion final es exclusiva: apunta justo despues del texto reconocido.
 */
typedef struct {
    size_t first_line, first_column; /* Línea y columna donde comienza el nodo. */
    size_t last_line, last_column;   /* Línea y columna inmediatamente después. */
} AstUbicacion; /* typedef permite usar este nombre como un tipo. */

/* Categorías de nodos. El enum asigna valores consecutivos desde cero.
 * Se debe conservar el mismo orden en la tabla de ast_nombre_tipo (ast.c).
 */
typedef enum {
    AST_PROGRAMA,          /* Raíz que agrupa los elementos del programa. */
    AST_FUNCION,           /* Definición de una función. */
    AST_PARAMETROS,        /* Grupo de parámetros de una función. */
    AST_PARAMETRO,         /* Un parámetro individual. */
    AST_TIPO,              /* Tipo de dato, por ejemplo declare_int. */
    AST_BLOQUE,            /* Grupo de instrucciones entre llaves. */
    AST_DECLARACION,       /* Declaración de una variable. */
    AST_ASIGNACION,        /* Asignación de un valor a un destino. */
    AST_BINARIO,           /* Operador con dos operandos, como la suma. */
    AST_UNARIO,            /* Operador con un operando, como la negación. */
    AST_IDENTIFICADOR,     /* Nombre de variable, función u otro elemento. */
    AST_ENTERO,            /* Literal entero, conservado como texto. */
    AST_CADENA,            /* Literal de texto. */
    AST_CARACTER,          /* Literal de un carácter. */
    AST_BOOLEANO,          /* Literal booleano. */
    AST_SI,               /* Condición whether o alif y sus ramas. */
    AST_MIENTRAS,         /* Ciclo whale. */
    AST_RETORNO,          /* Instrucción give, con o sin valor. */
    AST_LLAMADA,          /* Invocación de una función. */
    AST_ARGUMENTOS,       /* Valores o expresiones enviados en una llamada. */
    AST_IMPORTACION,      /* Importación de un módulo mediante bring. */
    AST_CONSTANTE,        /* Declaración de una constante. */
    AST_DECLARACIONES,    /* Agrupación de varias declaraciones. */
    AST_TIPO_ARREGLO,     /* Tipo de un arreglo, con sus dimensiones. */
    AST_TIPO_LISTA,       /* Tipo de una lista. */
    AST_DIMENSIONES,      /* Grupo de dimensiones de un arreglo. */
    AST_DIMENSION,        /* Una dimensión, con tamaño si se especifica. */
    AST_INDICE,           /* Acceso por índice, como m[0]. */
    AST_ACCESO_MIEMBRO,   /* Acceso mediante punto, como math.suma. */
    AST_LITERAL_COLECCION, /* Elementos escritos entre corchetes. */
    AST_LISTA,            /* Declaración de una lista. */
    AST_AGREGAR,          /* Operación add sobre una lista. */
    AST_ELIMINAR,         /* Operación remove sobre una lista. */
    AST_TAMANO,           /* Operación size para consultar el tamaño. */
    AST_RECORRIDO,        /* Ciclo cycle con límites y paso. */
    AST_INTENTAR,         /* Bloque seek con sus capturas. */
    AST_CAPTURAS,         /* Grupo ordenado de cláusulas seize. */
    AST_CAPTURA           /* Una cláusula seize. */
} AstTipo;

/* Cada nodo guarda datos y enlaces a otros nodos.
 * Los punteros permiten representar un árbol sin copiar las estructuras.
 * Dentro de la definición se usa struct AstNodo para referirse al mismo tipo.
 */
typedef struct AstNodo {
    AstTipo tipo;                /* Categoría del nodo, tomada del enum anterior. */
    char *texto;                 /* Copia propia del texto; NULL si no hay texto. */
    AstUbicacion ubicacion;      /* Posición del nodo en el código fuente. */
    struct AstNodo *primer_hijo; /* Primer hijo; NULL si el nodo no tiene hijos. */
    struct AstNodo *ultimo_hijo; /* Último hijo; permite añadir otro al final. */
    struct AstNodo *siguiente;   /* Siguiente hermano, en orden del fuente. */
    struct AstNodo *padre;       /* Nodo que lo contiene; NULL para la raíz. */
    struct AstNodo *reservado_siguiente; /* Lista privada para liberar memoria. */
} AstNodo;

/* Contenedor del árbol completo y de la memoria que le pertenece. */
typedef struct {
    AstNodo *raiz;       /* Punto de entrada para recorrer el árbol. */
    AstNodo *reservados; /* Inicio de la lista de todos los nodos reservados,
                         * incluidos los que aún no se conectaron al árbol. */
} Ast;

/* El propietario es Ast: no liberar nodos ni textos individualmente.
 * Usar Ast arbol = {0}; y ast_liberar(&arbol) al terminar.
 * Todos los nodos, incluso los de un parseo incompleto, se liberan juntos.
 */
/* Reserva un nodo, copia texto si existe y lo registra en arbol.
 * Devuelve su dirección o NULL si falla la reserva de memoria.
 * No lo conecta con un padre ni lo asigna automáticamente como raíz.
 */
AstNodo *ast_crear(Ast *arbol, AstTipo tipo, const char *texto, AstUbicacion ubicacion);
/* Añade hijo al final de los hijos de un padre válido.
 * hijo puede ser NULL; de otro modo debe estar sin padre ni hermanos.
 */
void ast_agregar_hijo(AstNodo *padre, AstNodo *hijo);
/* Libera los textos y nodos registrados y deja arbol vacío. */
void ast_liberar(Ast *arbol);
/* Devuelve el nombre legible del tipo o "Desconocido" si no es válido.
 * El texto devuelto es constante: no debe modificarse ni liberarse.
 */
const char *ast_nombre_tipo(AstTipo tipo);
/* Imprime los nodos con sangría, texto y @línea:columna en salida.
 * salida puede ser stdout (terminal) o un archivo abierto para escritura.
 * const indica que la función recibe el árbol para lectura.
 * Recorrido iterativo: no consume pila C segun la profundidad del AST.
 */
void ast_imprimir(const Ast *arbol, FILE *salida);

#endif /* Fin de la protección MEIBEL_AST_H contra inclusiones repetidas. */
