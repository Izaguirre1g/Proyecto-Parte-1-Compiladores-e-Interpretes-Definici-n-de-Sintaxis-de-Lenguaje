# Analizador léxico

## Alcance

El lexer implementa en C la primera fase del compilador. Lee el archivo fuente
y produce tokens con su posición. También se incluye una interfaz de línea de
comandos con la opción `-v` para inspeccionar el resultado del análisis.

El lexer reconoce tokens; no decide si su orden forma una instrucción válida.
Por ejemplo, reconoce `alif` y `also`, pero aún falta definir la gramática de
esos bloques. La presencia de un token no implica que la construcción completa
esté implementada.

## Tokens reconocidos

| Categoría | Lexemas o regla |
|---|---|
| Entrada, declaración, funciones | `main`, `declare_const`, `create_funk`, `give`, `show` |
| Condicionales y ciclos | `whether`, `alif`, `also`, `whale`, `stop`, `cycle`, `let`, `until`, `step`, `endgame` |
| Módulos, errores, listas | `bring`, `aka`, `seek`, `seize`, `declare_list`, `add`, `remove`, `size` |
| Tipos | `declare_int`, `declare_boolean`, `declare_text`, `declare_char`, `declare_infinite_void` |
| Booleanos | `declare_true`, `declare_false` |
| Aritmética | `gauss`, `neumann`, `pitagoras`, `euclides`, `euler`, `descartes` |
| Relaciones | `==`, `=/=`, `<`, `>`, `<=`, `>=` |
| Lógica | `&&`, `\|\|`, `~`, `^` |
| Asignación y acceso | `:`, `.` |
| Delimitadores | `(`, `)`, `[`, `]`, `{`, `}`, `,`, `;`, `*`, `#` |
| Identificador | `[A-Za-z_][A-Za-z0-9_]*`; distingue mayúsculas |
| Entero | Uno o más dígitos ASCII; el signo negativo se expresa con `neumann` |
| Cadena | Comillas rectas `"..."`, sin salto de línea ni interpretación de escapes |
| Carácter | `$` + un carácter Unicode UTF-8 + `$`; no se interpretan escapes |
| Comentarios | `%%` hasta el fin de línea; `%%//` hasta el primer `//%%`, incluso entre líneas |
| Control y errores | `TOKEN_EOF` al acabar bytes; `TOKEN_ERROR` con causa y posición para entrada inválida |

Esta versión no admite `declare_float`, literales reales ni identificadores
con acentos. `declare_float` se reconoce como un `IDENTIFIER` ordinario:
su uso en posición de tipo es un error que debe señalar el parser. Una forma
decimal `1.85` se informa como `TOKEN_INVALID_NUMBER`. Se permiten acentos
en cadenas, caracteres y comentarios, como `"café"` y `$ñ$`.

`#` permite tokenizar `create_funk #declare_int#`; `.` permite tokenizar
`math.suma`. `show()` se tokeniza como `SHOW LPAREN RPAREN`, no como una sola
palabra. Los marcadores documentales `@`, `!!!`, `!?`, `%?` no son lexemas
válidos del fuente; la API expresa EOF y los errores mediante enumeraciones.

Quedan por formalizar la precedencia de operadores, el significado
de `alif` frente a `also`, el uso preciso de `stop` y `endgame`, y la sintaxis
definitiva de construcciones que aparecen distintas en los ejemplos. Estas
decisiones no impiden reconocer sus tokens individuales.

## Funcionamiento

`lexer_init` recibe bytes de entrada y longitud explícita, incluso para poder
señalar un byte NUL en un archivo. `lexer_next` avanza de izquierda a derecha,
ignorando espacios y comentarios. Cuando encuentra `%%//`, prueba ese prefijo
antes de `%%` y busca el primer `//%%`; EOF previo produce un error. Las
palabras se leen completas antes de comparar con la tabla de reservadas, de
modo que `main2` es identificador. Los operadores compuestos (`=/=`, `<=`,
`>=`, `==`, `&&`, `||`) se comprueban antes que símbolos individuales. Un
símbolo desconocido produce `TOKEN_ERROR`; se avanza y se continúa si es
posible. El lexema devuelto es una copia con longitud en bytes.

Línea y columna empiezan en 1. `CRLF` cuenta como un solo salto. Las columnas
cuentan caracteres Unicode, para que un error después de `ñ` coincida con lo
que se ve; los offsets y longitudes de lexema siguen contando **bytes**. Un
tabulador ocupa una columna lógica, no un número variable de espacios visuales.
Para identificadores, un carácter acentuado da un error recuperable en ese
punto (`tamaño` resulta en `tama`, error `ñ`, `o`).

## Errores y recuperación

| Código | Condición | Recuperación |
|---|---|---|
| `TOKEN_INVALID_CHARACTER` | Símbolo externo al lenguaje, `@`, acento en identificador o NUL | Consumir el carácter y continuar |
| `TOKEN_INVALID_NUMBER` | Decimal o combinación como `12abc` | Consumir el literal mal formado y continuar |
| `TOKEN_UNTERMINATED_STRING` | Falta `"` antes del salto de línea o EOF | Dejar el salto para el siguiente token |
| `TOKEN_UNTERMINATED_CHARACTER` | Falta `$` de cierre antes del salto de línea o EOF | Dejar el salto para el siguiente token |
| `TOKEN_INVALID_CHARACTER_LITERAL` | `$$`, `$ab$` o secuencia UTF-8 inválida | Consumir hasta el `$` de cierre |
| `TOKEN_UNTERMINATED_COMMENT` | `%%//` sin `//%%` | Consumir hasta EOF |
| `TOKEN_OUT_OF_MEMORY` | Falló una asignación | Abortar: el token carece de lexema |

El archivo puede producir varios errores léxicos antes del EOF. El lexer no
infiere errores sintácticos ni semánticos a partir de tokens válidos.

## Interfaz con el parser

La interfaz define `TokenType`, `TokenError`, `Token`, `Lexer`, `lexer_init` y
`lexer_next`. `lexer_init(&l, fuente, longitud,
nombre)` toma prestados ambos punteros: el llamador conserva sus datos vivos
durante todo el recorrido. Cada llamada a `lexer_next(&l)` devuelve un `Token`
por valor. Su `lexeme` pertenece al llamador, incluso en EOF (cadena vacía),
y se libera con `token_dispose(&t)`. `length` expresa bytes; `line` y `column`
son la posición inicial, ambas desde 1. `TOKEN_ERROR` lleva una causa en
`error`; `token_error_message(error)` devuelve un texto estático para mostrarla.

Los comentarios no se entregan al parser. Una vez alcanzado EOF, llamadas
posteriores repiten EOF. Si `error == TOKEN_OUT_OF_MEMORY`, `lexeme == NULL` y
el llamador detiene la compilación. El parser decide si trata otros errores
como fatales o acumula diagnósticos. Un identificador todavía no contiene
dirección, ámbito ni tipo semántico.

## Línea de comandos y pruebas

El nombre del ejecutable es `micomp`, según el enunciado oficial. Con `-v`
imprime nombre de fase, tokens, posición, lexema y recuento final. Sin `-v`
solo muestra errores o un aviso de análisis léxico correcto. Códigos de salida:
0 éxito léxico; 1 hubo errores léxicos; 2 error de invocación, lectura o
memoria. `-o`, `-s`, `-t`, `-m`, `-x` y `-p` no simulan un backend: informan que
la fase necesaria aún no está integrada. No se crea archivo `.bin`.

La batería automatizada ejecuta 18 casos con resultados esperados concretos:
reservadas de todos los grupos, identificadores parecidos a reservadas,
operadores, delimitadores, literales enteros, booleanos, cadenas y caracteres,
comentarios, CRLF, EOF repetido y archivo vacío; además comprueba cadenas,
caracteres y comentarios sin cierre, símbolos inválidos, decimales excluidos,
un byte NUL y posiciones después de UTF-8. Incluye el ejemplo de acceso a
módulo y tokeniza el ejercicio de factorial. La prueba falla con código distinto
de cero al encontrar cualquier discrepancia. La compilación se efectúa con C11,
`-Wall -Wextra -Wpedantic -Werror`; `make test` también muestra el factorial
mediante `-v`. También se pueden compilar las pruebas directamente con GCC.

## Ejercicio de factorial

El programa contiene `main`, tres variables locales enteras,
`whether`/`also`, un `whale`, `pitagoras` y `gauss`. Para `n = 5`, el algoritmo
pretende calcular `resultado = 120` (si `n <= 1`, da 1; de otro modo multiplica
desde `i = 2` hasta `n`). **Este valor aún no se ha ejecutado ni verificado
sobre el procesador.** Se verificaron 64 tokens y cero errores léxicos.

La representación sintáctica del programa requiere una función `main` con
declaraciones y un condicional. La rama `also` contiene el ciclo, la
multiplicación y el avance de `i`. Aún faltan la asignación de ámbitos y
direcciones, así como la generación de ensamblador y binario. La ISA actual
no define una instrucción de multiplicación para `pitagoras`; por eso el
factorial no puede traducirse por completo. Los bundles VLIW tienen slots
ALU, LSU, BRU y CRIPTO, y necesitan una planificación que respete dependencias.

## Trabajo pendiente

1. Acordar el contrato `Token` e integrar el lexer con el parser.
2. Definir las construcciones que todavía tienen ejemplos contradictorios,
   especialmente `alif`, `also` y el cierre de los ciclos.
3. Generar y probar ensamblador de expresiones, asignaciones y accesos a
   memoria cuando estén disponibles el AST, los símbolos y una ISA estable.
4. Comprobar el factorial de principio a fin y documentar el ensamblador,
   binario y resultado obtenidos.
