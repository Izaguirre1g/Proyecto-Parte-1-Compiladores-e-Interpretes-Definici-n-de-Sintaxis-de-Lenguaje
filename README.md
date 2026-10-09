# Analizadores léxico y sintáctico

Todos los comandos de abajo se ejecutan desde la raíz del repositorio. Se necesita GCC para C11,
Flex y Bison (el parser actual usa Bison y el lexer se genera con Flex). El Makefile usa GNU Make
4.3 o posterior. Las pruebas del parser están en C y usan funciones POSIX disponibles en Ubuntu/WSL.

## Opción 1: WSL

Abre Ubuntu en WSL y entra en el repositorio. Las carpetas de la unidad `C:` de
Windows aparecen bajo `/mnt/c/`. Sustituye la ruta del ejemplo por la tuya:

```bash
sudo apt update
sudo apt install build-essential flex bison
cd "/mnt/c/ruta/al/repositorio"
flex -o src/lexer_flex.c src/lexer.l
bison -Wall -Werror -d -o Analisis_Sintactico/parser.c Analisis_Sintactico/gramatica.y
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude -Isrc \
  src/main.c src/cli.c src/lexer.c src/lexer_flex.c src/token.c \
  Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c \
  Analisis_Sintactico/ast.c -o balc
./balc -v examples/factorial.bal
```

Para ejecutar las pruebas del lexer sin `make`:

```bash
flex -o src/lexer_flex.c src/lexer.l
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude -Isrc \
  tests/test_lexer.c src/lexer.c src/lexer_flex.c src/token.c -o tests/test_lexer
./tests/test_lexer
```

## Opción 2: terminal de Ubuntu

Abre una terminal, entra en la raíz del repositorio y compila:

```bash
sudo apt update
sudo apt install build-essential flex bison
cd "/ruta/al/repositorio"
make
make test
```

`make` genera `src/lexer_flex.c` desde `src/lexer.l` y conserva la interfaz pública
`lexer_init`/`lexer_next`. Por eso el parser y el AST no necesitan cambios para usar
el nuevo lexer.

`make test` ejecuta las pruebas de ambos analizadores y del AST y muestra los tokens del
factorial. Las pruebas del parser se compilan y ejecutan así (con `balc` ya compilado):

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 tests/test_parser.c -o tests/test_parser
./tests/test_parser
```

## Resultado esperado

Las pruebas terminan con `OK: 18 casos del lexer, incluido factorial.bal`.
El análisis del factorial termina con `64 tokens, 0 errores`. Para analizar otro
archivo, usa `./balc -v ruta/al/archivo.bal`.

Para validar la sintaxis y mostrar el AST:

```bash
./balc -t examples/factorial.bal
./balc -v -t examples/factorial.bal
make test-parser
make test-ast
```

`-t` valida la sintaxis y muestra el AST con sangría y posiciones del fuente.
Solo imprime el árbol si el archivo completo es válido. Sin `-t` se conserva
el análisis exclusivamente léxico. La estructura del árbol y las acciones de
Bison se explican en `docs/ast.md`.
`balc` devuelve 0 si el análisis solicitado tiene éxito, 1 por errores de
entrada y 2 por fallos de lectura, invocación o memoria. La integración y sus
límites se explican en `docs/parser.md`; el diseño léxico está en `docs/lexer.md`.

## Lenguaje ampliado

El ejecutable se llama `balc` y conserva las opciones `-v` y `-t`. Las reglas actuales,
las diferencias con el PDF del avance y los límites de esta fase están en
[docs/lenguaje_actual.md](docs/lenguaje_actual.md).

```bash
./balc -t examples/lenguaje_ampliado.bal
./balc -t examples/matematicas.bal
```

Incluye declaraciones sin inicialización, lógica, residuo/potencia, cycle,
colecciones, importaciones, constantes y seek/seize en el parser y AST.
No incluye ejecución ni carga/enlace de módulos.

## Generación básica para integración

Se añadió un módulo de traducción de expresiones puras, inicializaciones y
asignaciones escalares al ensamblador de la ISA del equipo. Recibe referencias
de memoria externas y no está conectado todavía a la CLI como compilador completo.

```bash
make test-generacion
make test-cli
./tests/test_generacion --ejemplo
```

El alcance, la conexión con símbolos y las limitaciones están en
[docs/generacion_basica.md](docs/generacion_basica.md).
`make test` incluye las pruebas nuevas. `-s` sigue reservado hasta integrar el backend.
