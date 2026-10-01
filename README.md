# Analizadores léxico y sintáctico

El contenido de este directorio se coloca directamente en la raíz del repositorio:
`src/`, `include/`, `tests/`, `examples/`, `docs/`, `Makefile` y este README.
Todos los comandos de abajo se ejecutan desde esa raíz. Se necesita GCC para C11
y Bison (probado con 3.8.2). El Makefile usa GNU Make 4.3 o posterior. Las pruebas del parser están en C
y usan funciones POSIX disponibles en Ubuntu/WSL. También se puede compilar sin make.

## Opción 1: WSL

Abre Ubuntu en WSL y entra en el repositorio. Las carpetas de la unidad `C:` de
Windows aparecen bajo `/mnt/c/`. Sustituye la ruta del ejemplo por la tuya:

```bash
cd "/mnt/c/ruta/al/repositorio"
bison -Wall -Werror -d -o Analisis_Sintactico/parser.c Analisis_Sintactico/gramatica.y
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude \
  src/main.c src/cli.c src/lexer.c src/token.c \
  Analisis_Sintactico/parser.c Analisis_Sintactico/puente_lexer.c \
  Analisis_Sintactico/ast.c -o micomp
./micomp -v examples/factorial.bal
```

Para ejecutar las pruebas sin `make`:

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude \
  tests/test_lexer.c src/lexer.c src/token.c -o tests/test_lexer
./tests/test_lexer
```

## Opción 2: terminal de Ubuntu

Abre una terminal, entra en la raíz del repositorio y compila:

```bash
cd "/ruta/al/repositorio"
make
make test
```

`make test` ejecuta las pruebas de ambos analizadores y del AST y muestra los tokens del
factorial. Sin make, los comandos de la opción WSL funcionan igual en Ubuntu;
las pruebas del parser se compilan y ejecutan así (con `micomp` ya compilado):

```bash
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 tests/test_parser.c -o tests/test_parser
./tests/test_parser
```

## Resultado esperado

Las pruebas terminan con `OK: 18 casos del lexer, incluido factorial.bal`.
El análisis del factorial termina con `64 tokens, 0 errores`. Para analizar otro
archivo, usa `./micomp -v ruta/al/archivo.bal`.

Para validar la sintaxis y mostrar el AST:

```bash
./micomp -t examples/factorial.bal
./micomp -v -t examples/factorial.bal
make test-parser
make test-ast
```

`-t` valida la sintaxis y muestra el AST con sangría y posiciones del fuente.
Solo imprime el árbol si el archivo completo es válido. Sin `-t` se conserva
el análisis exclusivamente léxico. La estructura del árbol y las acciones de
Bison se explican en `docs/ast.md`.
`micomp` devuelve 0 si el análisis solicitado tiene éxito, 1 por errores de
entrada y 2 por fallos de lectura, invocación o memoria. No genera ensamblador
ni binario. La integración y sus límites se explican en `docs/parser.md`;
el diseño léxico está en `docs/lexer.md`.
