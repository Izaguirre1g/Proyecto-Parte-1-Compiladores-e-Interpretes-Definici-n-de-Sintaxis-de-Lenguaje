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
make
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

## Uso del compilador

```bash
balc [-o <archivo>] [-v] [-s] [-t] [-m] [-p] <archivo_fuente.bal>
balc -x [-o <archivo>] [-v] [-p] <archivo.asm>
```

`balc` siempre compila hasta el binario; las opciones agregan salidas y se
pueden combinar.

| Opción | Descripción |
|---|---|
| `-o <archivo>` | Nombre del binario de salida. Si se omite, el del fuente con `.bin`. |
| `-v` | Muestra cada fase del compilador y las decisiones importantes. |
| `-s` | Genera el ensamblador (`.asm`) además del binario. |
| `-t` | Imprime el AST. |
| `-m` | Imprime la tabla de símbolos, la memoria asignada y la dirección de cada función. |
| `-x` | Toma un archivo de ensamblador y genera el binario. |
| `-p` | Muestra los paquetes generados en ensamblador y en binario. |

El `.bin` usa el [formato BALB v1](docs/formato_binario.md): encabezado,
codigo y datos globales. Junto a el se genera un `.mem` de instrucciones
compatible con la IMEM de Arquitectura. El encabezado no se carga en la IMEM.

```bash
./balc -v examples/factorial.bal          # factorial.bin y factorial.mem
./balc -s -p examples/ejercicio3_puntajes.bal
```

## Modo de pruebas del frontend

Para revisar solo el lexer o el parser, sin generar archivos:

```bash
./balc --frontend -t examples/factorial.bal
./balc --frontend -v -t examples/factorial.bal
make test-parser
make test-ast
```

Con `--frontend`, `-t` valida la sintaxis y muestra el AST con sangría y
posiciones del fuente; sin `-t` solo analiza tokens y `-v` los lista. La
estructura del árbol y las acciones de Bison se explican en `docs/ast.md`. La integración y sus
límites se explican en `docs/parser.md`; el diseño léxico está en `docs/lexer.md`.

## Lenguaje ampliado

El ejecutable se llama `balc`. Las reglas actuales,
las diferencias con el PDF del avance y los límites de esta fase están en
[docs/lenguaje_actual.md](docs/lenguaje_actual.md).

```bash
./balc --frontend -t examples/lenguaje_ampliado.bal
./balc --frontend -t examples/matematicas.bal
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
