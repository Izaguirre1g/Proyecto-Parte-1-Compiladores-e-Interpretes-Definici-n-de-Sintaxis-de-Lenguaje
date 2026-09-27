# Conexión del lexer con Bison

El flujo es `cli_run -> analizar_sintaxis -> yyparse -> yylex -> lexer_next`.
La CLI lee el archivo y entrega sus bytes al parser. Bison llama a `yylex`
cada vez que necesita un token; `puente_lexer.c` llama al lexer de Javier y
traduce explícitamente su enumeración (`TOKEN_MAIN`, etc.) a la de Bison
(`MAIN`, etc.). EOF se traduce a `YYEOF`, cuyo valor es cero.

## Archivos que se editan

- `Analisis_Sintactico/gramatica.y`: reglas y configuración de Bison.
- `Analisis_Sintactico/puente_lexer.c`: adaptador, errores y función pública.
- `include/sintactico.h`: contexto y contrato de `analizar_sintaxis`.

`parser.c` y `parser.h` son generados: no se editan manualmente. `make` los
regenera cuando cambia la gramática. Al agregar un token hay que declararlo
en la gramática y agregar su traducción al switch del adaptador.

## Estado y memoria

`api.pure full` y los parámetros de contexto evitan un lexer global.
Cada llamada a `analizar_sintaxis` tiene su propio estado. El adaptador guarda
línea y columna iniciales del token y libera su lexema con `token_dispose`.
Por ahora el parser solo valida estructura: no conserva valores ni crea AST.
Cuando se agregue el AST habrá que definir valores semánticos y su propiedad.

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

`-t` es provisional: valida sintaxis pero todavía no imprime AST. `-v -t`
muestra primero todos los tokens y, si no hubo errores léxicos, inicia un
nuevo recorrido del fuente para validar sintaxis. Sin `-t`, se mantiene el
modo léxico de Javier. Resultado: 0 éxito, 1 error léxico/sintáctico, 2 fallo
de lectura, invocación o memoria.

La gramática admite funciones con parámetros opcionales, declaraciones inicializadas,
asignaciones, aritmética básica, comparaciones, bloques, whether/also y whale
con cierre `stop;`. Permite bloques vacíos y estructuras anidadas. Todavía no
admite llamadas, give, alif, operadores lógicos, listas ni módulos.
No verifica tipos, declaración de variables, existencia/unicidad de main ni
retornos obligatorios. No ejecuta el factorial ni calcula su resultado.

Las pruebas en `tests/test_parser.c` pasan archivos reales por la CLI:
factorial, funciones y estructuras anidadas, operadores, errores de cierre,
comparaciones encadenadas, tokens pendientes y errores léxicos (incluido UTF-8).
Los casos válidos verifican aceptación, no construcción de AST ni evaluación.

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
o al final. Las llamadas y `give` quedan para el siguiente paso.
La verificación de nombres duplicados, ámbitos y firma de main corresponde
al análisis semántico; aquí solo se valida la forma de la declaración.

Las pruebas están escritas en C y se ejecutan con `make test-parser`.
Usan `fork`/`execv` y archivos temporales POSIX para ejecutar el binario real
y capturar salida, errores y código de retorno. No requieren Python.

`make test-parser` muestra cada caso con su código fuente, el resultado esperado,
el obtenido y los diagnósticos. Los rechazos esperados se marcan como pruebas
correctas. Al final se muestra el resumen de la batería.
