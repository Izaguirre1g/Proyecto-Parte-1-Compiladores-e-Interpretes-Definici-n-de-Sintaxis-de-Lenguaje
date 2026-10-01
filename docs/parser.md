# Conexión del lexer con Bison

El flujo es `cli_run -> analizar_sintaxis -> yyparse -> yylex -> lexer_next`.
La CLI lee el archivo y entrega sus bytes al parser. Bison llama a `yylex`
cada vez que necesita un token; `puente_lexer.c` llama al lexer de Javier y
traduce explícitamente su enumeración (`TOKEN_MAIN`, etc.) a la de Bison
(`MAIN`, etc.). EOF se traduce a `YYEOF`, cuyo valor es cero.

## Archivos que se editan

- `Analisis_Sintactico/gramatica.y`: reglas y configuración de Bison.
- `Analisis_Sintactico/puente_lexer.c`: adaptador, errores y función pública.
- `include/sintactico.h`: contexto y contratos de `analizar_sintaxis` y `construir_ast`.
- `include/ast.h` y `Analisis_Sintactico/ast.c`: nodos, creación, impresión y liberación del AST.

`parser.c` y `parser.h` son generados: no se editan manualmente. `make` los
regenera cuando cambia la gramática. Al agregar un token hay que declararlo
en la gramática y agregar su traducción al switch del adaptador.

## Estado y memoria

`api.pure full` y los parámetros de contexto evitan un lexer global.
Cada llamada a `analizar_sintaxis` tiene su propio estado. El adaptador guarda
la ubicación del token y copia los lexemas necesarios en hojas del AST antes
de liberar el token con `token_dispose`. Bison transporta punteros a nodos
mediante `api.value.type` y construye el árbol con acciones en la gramática.
`Ast` es propietario de todos los nodos, incluidos los parciales: se liberan
juntos al terminar o al abortar por error. Véase `docs/ast.md`.

Los errores léxicos se muestran con la causa del lexer; los sintácticos con
el mensaje de Bison y la posición del último token solicitado. El análisis
se detiene en el primer error. Los tokens léxicos válidos que todavía no están
en la gramática se rechazan con un mensaje explícito; nunca se omiten.

## Uso y alcance

```bash
make
./micomp -t examples/factorial.bal
./micomp -v -t examples/factorial.bal
make test
```

`-t` valida sintaxis e imprime el AST completo si no hubo errores. `-v -t`
muestra primero todos los tokens y, si no hubo errores léxicos, inicia un
nuevo recorrido del fuente para validar sintaxis. Sin `-t`, se mantiene el
modo léxico de Javier. Resultado: 0 éxito, 1 error léxico/sintáctico, 2 fallo
de lectura, invocación o memoria.

La gramática admite funciones con parámetros opcionales, declaraciones inicializadas,
asignaciones, llamadas, retornos con `give`, aritmética básica, comparaciones, bloques, whether/alif/also y whale
con cierre `stop;`. Permite bloques vacíos y estructuras anidadas. Todavía no
admite operadores lógicos, listas ni módulos.
No verifica tipos, declaración de variables, existencia/unicidad de main ni
retornos obligatorios. No ejecuta el factorial ni calcula su resultado.

Las pruebas en `tests/test_parser.c` pasan archivos reales por la CLI:
factorial, funciones y estructuras anidadas, operadores, errores de cierre,
comparaciones encadenadas, tokens pendientes y errores léxicos (incluido UTF-8).
Los casos válidos comprueban aceptación y presencia del AST en la CLI; los
inválidos comprueban que no se muestre un árbol parcial. `tests/test_ast.c`
verifica nodos, hijos, precedencia, posiciones y limpieza de árboles incompletos
con `make test-ast`. Ninguna de estas pruebas ejecuta el programa fuente.

## Parámetros de funciones

La sintaxis adoptada para este paso es `nombre * tipo`, separada por comas:

```text
create_funk #declare_infinite_void# procesar(n * declare_int, activo * declare_boolean) {
    whether (activo) { n : n gauss 1; }
}

create_funk #declare_infinite_void# main() {}
```

Se permiten cero, uno o varios parámetros. Cada uno requiere identificador,
`*` y un tipo de variable (`declare_int`, `declare_boolean`, `declare_text`
o `declare_char`). No admite valores iniciales, tipo void ni comas al inicio
o al final. Los argumentos de las llamadas son expresiones, sin declarar tipos.
La verificación de nombres duplicados, ámbitos y firma de main corresponde
al análisis semántico; aquí solo se valida la forma de la declaración.

Las pruebas están escritas en C y se ejecutan con `make test-parser`.
Usan `fork`/`execv` y archivos temporales POSIX para ejecutar el binario real
y capturar salida, errores y código de retorno. No requieren Python.

`make test-parser` muestra cada caso con su código fuente, el resultado esperado,
el obtenido y los diagnósticos. Los rechazos esperados se marcan como pruebas
correctas. Al final se muestra el resumen de la batería.

## Retorno con give

Se reconocen `give expresion;` y `give;` como sentencias dentro de funciones,
incluso en bloques, condicionales y ciclos anidados. El punto y coma es
obligatorio y `give` no puede usarse como expresión ni fuera de una función.

```text
create_funk #declare_int# sumar(a * declare_int, b * declare_int) {
    give a gauss b;
}

create_funk #declare_infinite_void# main() {
    give;
}
```

El parser solo valida esta estructura. El análisis semántico deberá verificar
que el valor corresponda al tipo de retorno, que las funciones void no devuelvan
un valor y que los caminos requeridos devuelvan un resultado. Aún no se ejecutan
los retornos; sí se representan como nodos del AST.

## Llamadas a funciones

Una llamada tiene la forma `nombre(argumentos)`. Los argumentos son expresiones
separadas por comas; también se admite una lista vacía. Se puede usar la llamada
en una expresión (`resultado : sumar(2, 3);`, `give sumar(a, b);`) o como
sentencia independiente (`saludar();`). Se admiten llamadas anidadas, como
`sumar(2, sumar(3, 4))`. En una expresión, el punto y coma corresponde a la
sentencia completa, no a cada llamada interior.

```bash
make
./micomp -t examples/llamadas.bal
make test-parser
```

El nombre sigue la misma regla que las declaraciones de funciones (identificador
o `main`). La etapa semántica deberá comprobar la existencia de la función,
la cantidad y los tipos de argumentos, el uso de resultados void y cualquier
restricción sobre llamadas a main. Aceptar la sintaxis no ejecuta la función.
Las palabras reservadas como `show` no son identificadores de función y siguen
pendientes de reglas propias.

## Condicionales con alif

Se adopta `alif` como rama intermedia con condición (else if), y `also` como
rama final sin condición (else). Después del bloque de `whether` se permiten
cero o más `alif (expresion) { ... }` y un único `also { ... }` opcional al final.
Cada rama requiere llaves, aunque esté vacía. No se escribe punto y coma entre
ramas ni después del condicional. Se permiten condicionales anidados.

```text
whether (n < 0) { give neumann 1; }
alif (n == 0) { give 0; }
alif (n < 10) { give 1; }
also { give 2; }
```

Un `alif` suelto, posterior a `also` o separado de su cadena por otra sentencia
es un error sintáctico. La validación del tipo de las condiciones corresponde
al análisis semántico. El ejemplo completo se analiza con:

```bash
./micomp -t examples/condicionales.bal
```
