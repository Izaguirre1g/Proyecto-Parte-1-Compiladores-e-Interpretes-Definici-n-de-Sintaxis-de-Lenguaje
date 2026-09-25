# Analizador léxico

El contenido de este directorio se coloca directamente en la raíz del repositorio:
`src/`, `include/`, `tests/`, `examples/`, `docs/`, `Makefile` y este README.
Todos los comandos de abajo se ejecutan desde esa raíz. Se necesita GCC para C11;
`make` es opcional.

## Opción 1: WSL

Abre Ubuntu en WSL y entra en el repositorio. Las carpetas de la unidad `C:` de
Windows aparecen bajo `/mnt/c/`. Sustituye la ruta del ejemplo por la tuya:

```bash
cd "/mnt/c/ruta/al/repositorio"
gcc -std=c11 -Wall -Wextra -Wpedantic -Werror -O2 -Iinclude \
  src/main.c src/cli.c src/lexer.c src/token.c -o micomp
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

`make test` ejecuta las pruebas y muestra los tokens del factorial. Si no tienes
`make`, los dos comandos `gcc` de la opción WSL funcionan igual en Ubuntu.

## Resultado esperado

Las pruebas terminan con `OK: 18 casos del lexer, incluido factorial.bal`.
El análisis del factorial termina con `64 tokens, 0 errores`. Para analizar otro
archivo, usa `./micomp -v ruta/al/archivo.bal`.

`micomp` devuelve 0 si no encuentra errores léxicos, 1 si los encuentra y 2 si
falla la lectura o la invocación. Esta entrega llega hasta el análisis léxico:
no produce ensamblador ni binario. El diseño de tokens, los errores y la interfaz
con el parser se describen en `docs/lexer.md`.
