# Archivo binario BALB, version 1

La salida `.bin` contiene encabezado, codigo y datos, como pide la seccion
6.1 del enunciado. La imagen `.mem` sigue siendo solo codigo para la IMEM.

## Encabezado

Ocupa 48 bytes. Salvo la firma, todos los campos son enteros sin signo de
32 bits en little-endian. Se escriben byte a byte, sin depender del padding
de estructuras de C.

| Offset | Campo | Valor en esta version |
|---|---|---|
| 0 | Firma | Cuatro bytes ASCII `BALB` |
| 4 | Version | 1 |
| 8 | Bytes de encabezado | 48 |
| 12 | Bytes del archivo completo | Encabezado + codigo + datos |
| 16 | Punto de entrada | Direccion 0, donde comienza el arranque |
| 20 | Offset de codigo en el archivo | 48 |
| 24 | Bytes de codigo | Cantidad de bundles multiplicada por 16 |
| 28 | Direccion de carga del codigo | 0 en IMEM |
| 32 | Offset de datos en el archivo | 48 + bytes de codigo |
| 36 | Bytes de datos | Espacio global asignado por semantica |
| 40 | Direccion de carga de datos | 0x4000 en memoria de datos |
| 44 | Flags | Bit 0: el arranque tambien inicializa los datos |

Los offsets de archivo no son direcciones del procesador. El PC sigue
empezando en 0: agregar el encabezado no desplaza los saltos.

## Codigo y datos

El codigo conserva los 16 bytes por bundle en little-endian, con el slot
ALU primero. Es identico a la salida cruda del ensamblador de Arquitectura.
La funcion `cb_escribir_bin` conserva esa salida cruda para sus usuarios;
la CLI utiliza `fb_escribir` para producir el contenedor completo.

La seccion de datos usa los desplazamientos de la tabla de simbolos. Guarda
las constantes globales enteras admitidas por el generador, con cuatro bytes
little-endian por valor. Se evaluan referencias a constantes anteriores y
operaciones aritmeticas con la misma aritmetica de 32 bits del generador:
desbordamiento modulo 2^32, division hacia cero y residuo con signo del
dividendo. Una constante imposible de evaluar, por ejemplo division entre
cero, detiene la compilacion antes de escribir los archivos.

Las variables locales pertenecen a la pila y no se incluyen en esta seccion.
Si no hay globales, el tamano de datos es cero; el encabezado registra la
seccion vacia. No se agrega soporte para tipos globales que el generador
todavia no admite.

El arranque conserva sus escrituras de inicializacion, indicadas por el flag
1, para mantener la ejecucion mediante `.mem` sin precarga de datos. Un
cargador del contenedor puede copiar los datos antes de ejecutar: el arranque
escribira nuevamente los mismos valores.

Con `-x`, el ensamblador de entrada no tiene directivas de datos ni metadatos
semanticos. Por eso se genera un contenedor con datos vacios y flags=0. Las
instrucciones explicitas de inicializacion presentes en el `.asm` se conservan.

## Compatibilidad con Arquitectura

Para las pruebas actuales del procesador se sigue utilizando `.mem`, sin
cambiar el RTL. No cargar el `.bin` completo directamente en la IMEM: los
primeros 48 bytes son metadatos. Un lector de BALB debe validar firma,
version, tamanos y limites; copiar el codigo a IMEM y los datos a su memoria
respectiva; y comenzar en el punto de entrada del encabezado.

La comparacion con `tools/assembler.py --binary` se hace contra la seccion
de codigo, no contra todo el contenedor. Los fixtures crudos del ensamblador
se mantienen sin modificar.

## Pruebas

```bash
make test-formato-binario test-cli test-codificacion-binaria
make test
./balc -s examples/factorial.bal
```

`test-formato-binario` comprueba offsets, tamanos, firma, valores globales,
errores y los tres ejercicios. Compara el codigo contra el `.mem`, contra
la ruta `-x` y contra el fixture independiente del ensamblador de Arquitectura.
Estas pruebas comprueban el formato; no sustituyen la ejecucion en RTL.
