# Árbol de sintaxis abstracta (AST)

El AST representa la estructura del programa. No ejecuta sus instrucciones ni
verifica los tipos semánticos. La gramática actual construye nodos para todas
las construcciones que reconoce: funciones, parámetros, bloques, declaraciones,
asignaciones, expresiones, condicionales, ciclos, retornos y llamadas.

## Archivos y responsabilidades

- `include/ast.h`: tipos `Ast`, `AstNodo`, `AstTipo`, `AstUbicacion` y API.
- `Analisis_Sintactico/ast.c`: creación de nodos, hijos, impresión y memoria.
- `Analisis_Sintactico/gramatica.y`: acciones que conectan los nodos al reconocer reglas.
- `Analisis_Sintactico/puente_lexer.c`: hojas de identificadores, tipos y literales.
- `tests/test_ast.c`: pruebas de la estructura real del árbol.

Bison sigue generando `parser.c` y `parser.h`; no se editan directamente.

## Cómo Bison construye el árbol

`%define api.value.type {AstNodo *}` hace que cada valor semántico sea un puntero
a un nodo. `%locations` y `AstUbicacion` conservan las posiciones. En una acción,
`$$` es el nodo resultante y `$1`, `$2`, etc., los valores de los elementos de
la regla; `@$` es la ubicación de la construcción completa.

Por ejemplo:

```bison
expresion:
    expresion GAUSS expresion
    {
        $$ = ast_crear(ctx->arbol, AST_BINARIO, "gauss", @$);
        if (!$$) YYNOMEM;
        ast_agregar_hijo($$, $1);
        ast_agregar_hijo($$, $3);
    }
;
```

La precedencia de Bison determina qué nodos se unen primero. `2 gauss 3 pitagoras 4`
produce una suma cuyo hijo derecho es una multiplicación. Los paréntesis alteran
la agrupación y devuelven el nodo interior; no crean un nodo de puntuación.
La ubicación de ese nodo interior conserva la de su construcción original.



## Contrato de los nodos

Cada nodo contiene `tipo`, `texto` opcional, `ubicacion`, `primer_hijo`,
`ultimo_hijo`, `siguiente` (hermano) y `padre`. El orden de hijos es significativo:

| Nodo | Hijos en orden |
|---|---|
| Programa | Funciones en orden del fuente |
| Funcion | Nombre, tipo de retorno, Parametros, Bloque |
| Parametros / Argumentos | Elementos en orden; pueden estar vacíos |
| Parametro | Nombre, tipo |
| Bloque | Sentencias en orden; puede estar vacío |
| Declaracion | Nombre, tipo, expresión inicial |
| Asignacion | Nombre de destino, expresión |
| Binario | Operando izquierdo, operando derecho; texto = operador |
| Unario | Operando; texto = operador |
| Si | Condición, bloque verdadero, alternativa opcional |
| Mientras | Condición, bloque del cuerpo |
| Retorno | Expresión opcional |
| Llamada | Nombre, Argumentos |
| Identificador / Tipo / literales | Sin hijos; texto = lexema original |

`alif` se representa como otro nodo `Si` en la alternativa del anterior; su texto
es `alif`. `also` es el bloque final. Sin alternativa, el nodo solo tiene dos
hijos. Un `Retorno` sin expresión representa `give;`.

Los lexemas mantienen su escritura original, incluidas comillas de cadenas,
`$` de caracteres y enteros arbitrariamente largos. No se convierten a valores
numéricos ni se resuelven símbolos todavía. Las posiciones cuentan líneas y
columnas Unicode desde 1; el final es exclusivo. Las listas vacías toman la
posición final del símbolo anterior, porque no consumen texto.

## Uso y propiedad de memoria

```c
Ast arbol = {0};
int estado = construir_ast(fuente, longitud, nombre, stderr, &arbol);
if (estado == 0) {
    ast_imprimir(&arbol, stdout);
    /* La siguiente fase puede recorrer arbol.raiz. */
}
ast_liberar(&arbol);
```

El argumento de salida debe ser un `Ast` vacío. En éxito el llamador conserva
el árbol hasta llamar a `ast_liberar`. Sus textos son copias propias: el fuente
puede liberarse después del análisis. En error, `construir_ast` libera todos
los nodos y devuelve un árbol vacío. Los códigos siguen siendo 0 éxito, 1 error
de entrada y 2 falta de memoria.

Todos los nodos se registran en una lista de reservas propiedad del `Ast`.
Por eso no se liberan individualmente en destructores de Bison: la lista limpia
también símbolos descartados y nodos que aún no llegaron a la raíz. Las acciones
usan `YYNOMEM` si no pueden crear un nodo. La impresión y liberación son iterativas.
`ast_agregar_hijo` requiere un hijo sin padre ni hermanos; no permite compartir
un nodo entre dos padres. El campo `reservado_siguiente` es de uso interno.

`analizar_sintaxis` sigue disponible: construye y libera el AST internamente
para devolver solo el resultado de validación.

## Ver el árbol y ejecutar pruebas

Desde la raíz del proyecto:

```bash
make
./micomp -t examples/factorial.bal
./micomp -t examples/llamadas.bal
./micomp -t examples/condicionales.bal
make test-ast
make test
```

`-t` muestra sangría por nivel y la posición inicial de cada nodo. Los mensajes
no presentan un árbol parcial ante errores. `-v -t` añade el listado léxico previo.
Las pruebas comprueban precedencia, asociatividad, llamadas y control anidados,
literales, posiciones UTF-8/CRLF, independencia del fuente y limpieza tras errores.
El AST aún no asigna ámbitos, tipos semánticos, direcciones ni instrucciones.
