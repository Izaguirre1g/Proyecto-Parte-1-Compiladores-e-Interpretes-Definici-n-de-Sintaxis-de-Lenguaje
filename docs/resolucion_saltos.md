# Resolución de saltos (Fase 5)

`src/resolucion_saltos.c` recibe el ensamblador lineal de los generadores
(instrucciones y etiquetas `__gf_<funcion>_..._N:`) y produce bundles VLIW con
cada salto convertido en un desplazamiento numérico. Usa la tabla de
`resolucion_referencias.c` para registrar etiquetas y calcular desplazamientos.

```c
int rs_resolver(const char *lineal, uint32_t inicio, FILE *salida,
                FILE *diagnosticos, RsResumen *resumen);
```

## Layout

Cada instrucción ocupa un bundle en su slot (`ALU | LSU | BRU | CRIPTO`) y
le siguen dos bundles `nop`. Es la misma regla de `bundle_linear.py`: un valor
escrito en el bundle N se lee desde N+3 por cualquier ruta, incluidos los
destinos de salto, que es lo que exige el chequeo del ensamblador. Es
conservador: no empaqueta varias instrucciones por bundle.

Una etiqueta toma el PC del bundle siguiente. Si queda al final, se agrega un
`nop` para que el destino exista. `inicio` es el PC del primer bundle; el
programa no puede llegar a 0x3FF0, reservado para el manejador de excepciones.

## Cálculo de saltos

`desplazamiento = PC de la etiqueta − PC del bundle del salto`, en bytes,
múltiplo de 16 (contrato de la ISA). Condicionales (`igualsi`, `igualno`,
`menora`, `mayoroigual`): 11 bits, −1024 a 1008. `sye`: 16 bits.
Los destinos ya numéricos y `syer` se copian sin cambios.

Como las etiquetas llevan el nombre de la función y un contador único, los
saltos de condicionales y ciclos anidados no se confunden entre sí.

## Saltos largos

Con dos `nop` por instrucción, un condicional alcanza unas 21 instrucciones.
Las salidas de ciclos con cuerpos medianos ya lo superan (por ejemplo el
`whale` de `acumular` en el ejercicio 3: 1824 bytes). Un condicional fuera de
rango se reescribe con la condición invertida y un `sye`:

```text
igualsi x4,x0,L_fin          igualno x4,x0,__rs_cerca_N   (96 bytes)
                       =>    sye x0,L_fin                 (16 bits)
                             __rs_cerca_N:
```

Después se repite el layout. Cada vuelta solo agrega código, así que cada
condicional se reescribe a lo sumo una vez y el proceso termina.

## Salida

```text
# __gf_f_while_cond_0:
nop | cargai x4,x2,8 | nop | nop
nop
nop
nop | nop | igualsi x4,x0,240 | nop  # -> __gf_f_while_fin_0
```

Las etiquetas quedan como comentarios y cada salto indica su destino.
Errores (etiqueta inexistente o duplicada, instrucción desconocida, operandos
inválidos, programa que no cabe) devuelven 1 sin escribir salida.

## Pruebas

```sh
make test-resolucion-saltos
./tests/test_resolucion_saltos --emitir   # bundles del ejercicio 3
```

- Caso manual con desplazamientos exactos, hacia atrás y hacia adelante.
- Errores sin salida parcial.
- `cycle` dentro de `cycle` con `whether/alif/also` y `whale` con `whether`.
- Cuerpos largos que obligan a reescribir el salto de un `whale` y de un `whether`.
- Ejercicio 3 completo: se reescribe la salida del `whale` de `acumular`.

Cada salida se verifica de dos formas: cada desplazamiento llega exactamente a
su etiqueta y respeta su rango, y un modelo que avanza por bundles ejecuta el
programa y obtiene el mismo resultado que el modelo lineal con etiquetas.

## Pendiente

- Direcciones de funciones: las llamadas cargan la dirección absoluta en x13
  antes del layout. Hay que iterar generación y layout hasta que coincidan.
- Arranque (gp, sp) y ubicación de `main`.
- Integración en `balc` (`-s`) y prueba con `tools/assembler.py` y el RTL.
