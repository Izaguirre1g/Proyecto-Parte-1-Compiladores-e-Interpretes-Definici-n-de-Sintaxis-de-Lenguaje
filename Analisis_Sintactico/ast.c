/* Implementa los nodos del AST: creación, enlaces, liberación e impresión.
 * Las acciones de gramatica.y usan estas funciones durante el análisis.
 * Este archivo es código propio del proyecto; Bison no lo genera.
 */
#include "ast.h" /* Estructuras Ast/AstNodo, tipos y declaraciones de funciones. */

#include <stdlib.h> /* calloc, malloc y free: reservar y liberar memoria. */
#include <string.h> /* strlen y memcpy: medir y copiar cadenas de texto. */

/* Crea un nodo y lo registra en arbol; todavía no lo conecta con un padre.
 * texto puede ser NULL. Devuelve NULL si no logra reservar memoria.
 */
AstNodo *ast_crear(Ast *arbol, AstTipo tipo, const char *texto, AstUbicacion ubicacion) {
    /* Reserva espacio para un nodo y pone sus bytes a cero. */
    AstNodo *nodo = calloc(1, sizeof(*nodo));
    /* Si falló la reserva, sale sin intentar acceder al nodo. */
    if (!nodo) return NULL;
    /* Solo reserva una copia del texto si se recibió una cadena. */
    if (texto) {
        /* Cuenta los bytes del texto, sin incluir el terminador '\0'. */
        size_t longitud = strlen(texto);
        /* El byte adicional guarda el terminador de la cadena. */
        nodo->texto = malloc(longitud + 1);
        /* Si falla esta reserva, libera el nodo ya creado y comunica el fallo. */
        if (!nodo->texto) { free(nodo); return NULL; }
        /* Copia también '\0'; el nodo conserva su texto aunque cambie el original. */
        memcpy(nodo->texto, texto, longitud + 1);
    }
    /* Guarda la categoría del nodo, por ejemplo AST_ENTERO o AST_BINARIO. */
    nodo->tipo = tipo;
    /* Guarda las líneas y columnas de inicio y fin en el código fuente. */
    nodo->ubicacion = ubicacion;
    /* Enlaza el nodo con la lista de reservas, independiente de padres e hijos. */
    nodo->reservado_siguiente = arbol->reservados;
    /* Lo coloca al inicio de esa lista para poder liberarlo después. */
    arbol->reservados = nodo;
    /* Entrega el nodo al código que lo solicitó. */
    return nodo;
}

/* Añade hijo al final de los hijos de padre, conservando su orden.
 * padre debe existir; un hijo no nulo debe estar sin padre ni hermanos.
 */
void ast_agregar_hijo(AstNodo *padre, AstNodo *hijo) {
    /* Un hijo ausente no agrega nada al árbol. */
    if (!hijo) return;
    /* Guarda el enlace hacia arriba para poder regresar al padre. */
    hijo->padre = padre;
    /* Si había hijos, el último pasa a tener a hijo como siguiente hermano. */
    if (padre->ultimo_hijo) padre->ultimo_hijo->siguiente = hijo;
    /* Si no había ninguno, hijo se convierte en el primero. */
    else padre->primer_hijo = hijo;
    /* En ambos casos, el recién agregado es ahora el último hijo. */
    padre->ultimo_hijo = hijo;
}

/* Libera todos los nodos registrados, incluso los de un análisis incompleto. */
void ast_liberar(Ast *arbol) {
    /* Empieza por la cabeza de la lista privada de reservas. */
    AstNodo *nodo = arbol->reservados;
    /* Repite hasta llegar al final de la lista. */
    while (nodo) {
        /* Guarda el siguiente antes de liberar nodo: después no podrá leerlo. */
        AstNodo *siguiente = nodo->reservado_siguiente;
        /* Libera la copia del texto; free(NULL) también es válido. */
        free(nodo->texto);
        /* Libera la estructura que contenía el nodo. */
        free(nodo);
        /* Continúa con la reserva que guardó antes. */
        nodo = siguiente;
    }
    /* Deja raíz y lista de reservas vacías para poder reutilizar el árbol. */
    *arbol = (Ast){0};
}

/* Convierte un valor de AstTipo en el nombre que se muestra al imprimir. */
const char *ast_nombre_tipo(AstTipo tipo) {
    /* Tabla persistente de cadenas y punteros constantes.
     * Su orden debe coincidir con el enum AstTipo definido en ast.h.
     */
    static const char *const nombres[] = {
        /* Tipos para el programa, las funciones y sus parámetros. */
        "Programa", "Funcion", "Parametros", "Parametro", "Tipo",
        /* Tipos para bloques, instrucciones y operadores. */
        "Bloque", "Declaracion", "Asignacion", "Binario", "Unario",
        /* Tipos para nombres y valores literales. */
        "Identificador", "Entero", "Cadena", "Caracter", "Booleano",
        /* Tipos para control de flujo, retornos y llamadas. */
        "Si", "Mientras", "Retorno", "Llamada", "Argumentos",
        /* Tipos adicionales para módulos, colecciones, recorridos y capturas. */
        "Importacion", "Constante", "Declaraciones", "TipoArreglo", "TipoLista", "Dimensiones", "Dimension", "Indice", "AccesoMiembro", "LiteralColeccion", "Lista", "Agregar", "Eliminar", "Tamano", "Recorrido", "Intentar", "Capturas", "Captura"
    };
    /* La división calcula cuántos elementos tiene la tabla.
     * Convertir a unsigned también deja fuera del rango los valores negativos.
     * El operador ?: elige el nombre si el índice es válido, o "Desconocido".
     */
    return (unsigned)tipo < sizeof(nombres) / sizeof(nombres[0])
        ? nombres[tipo] : "Desconocido";
}

/* Función privada de este archivo (static): imprime una cadena entre comillas.
 * Escapa caracteres especiales para que no alteren las líneas del árbol.
 */
static void imprimir_texto(FILE *salida, const char *texto) {
    /* Escribe la comilla inicial en el archivo o terminal indicado por salida. */
    fputc('"', salida);
    /* Recorre los bytes hasta '\0'; unsigned char evita valores negativos. */
    for (const unsigned char *p = (const unsigned char *)texto; *p; ++p) {
        /* Ante una comilla o barra inversa, escribe una barra antes del carácter. */
        if (*p == '"' || *p == '\\') { fputc('\\', salida); fputc(*p, salida); }
        /* Representa un salto de línea con los caracteres visibles \n. */
        else if (*p == '\n') fputs("\\n", salida);
        /* Representa un retorno de carro con los caracteres visibles \r. */
        else if (*p == '\r') fputs("\\r", salida);
        /* Representa una tabulación con los caracteres visibles \t. */
        else if (*p == '\t') fputs("\\t", salida);
        /* Muestra otros controles ASCII en hexadecimal, por ejemplo \x01. */
        else if (*p < 0x20 || *p == 0x7f) fprintf(salida, "\\x%02X", *p);
        /* Copia los demás bytes tal cual, incluidos los que forman texto UTF-8. */
        else fputc(*p, salida);
    }
    /* Cierra las comillas del texto. */
    fputc('"', salida);
}

/* Imprime primero cada nodo y después sus hijos (recorrido en preorden).
 * Usa los enlaces del árbol en lugar de llamadas recursivas.
 */
void ast_imprimir(const Ast *arbol, FILE *salida) {
    /* El recorrido comienza en la raíz; si es NULL, no se imprime nada. */
    const AstNodo *nodo = arbol->raiz;
    /* La raíz está en el nivel cero, sin sangría. */
    size_t profundidad = 0;
    /* Continúa mientras exista un nodo pendiente de imprimir. */
    while (nodo) {
        /* Escribe dos espacios por cada nivel de profundidad. */
        for (size_t i = 0; i < profundidad; ++i) fputs("  ", salida);
        /* Escribe el nombre legible del tipo, por ejemplo "Identificador". */
        fputs(ast_nombre_tipo(nodo->tipo), salida);
        /* Si hay texto asociado, agrega un espacio y ese texto entre comillas. */
        if (nodo->texto) { fputc(' ', salida); imprimir_texto(salida, nodo->texto); }
        /* Escribe @línea:columna y termina la línea; %zu imprime valores size_t. */
        fprintf(salida, " @%zu:%zu\n", nodo->ubicacion.first_line, nodo->ubicacion.first_column);
        /* Si el nodo tiene hijos, el recorrido baja al primero. */
        if (nodo->primer_hijo) {
            /* Selecciona el primer hijo como próximo nodo que se imprimirá. */
            nodo = nodo->primer_hijo;
            /* Al bajar un nivel, aumenta la sangría de la próxima impresión. */
            ++profundidad;
        } else {
            /* Sin hijos, busca un hermano; si no lo hay, sube hasta encontrar
             * un antecesor con hermano pendiente o hasta salir de la raíz.
             */
            while (nodo && !nodo->siguiente) {
                /* Sube al padre; al subir desde la raíz, nodo queda en NULL. */
                nodo = nodo->padre;
                /* Reduce el nivel sin restar por debajo de cero. */
                if (profundidad) --profundidad;
            }
            /* Si encontró un hermano pendiente, lo visitará en la siguiente vuelta. */
            if (nodo) nodo = nodo->siguiente;
        }
    }
}
