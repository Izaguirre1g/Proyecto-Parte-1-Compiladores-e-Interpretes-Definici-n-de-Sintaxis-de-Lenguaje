# Sintaxis acordada e implementada en el parser y AST

Este documento registra las decisiones confirmadas para el proyecto y las
convenciones necesarias para las extensiones. Complementa el PDF del avance:
los ejemplos contradictorios del PDF no cambian estas reglas.

El ejecutable se llama `balc`, con `-v` para tokens y `-t` para AST.
`tree`, `trace` y demás opciones antiguas del avance no se implementan.
No se admiten `declare_float`, literales decimales ni `show` por ahora.
`alif` siempre requiere una condición entre paréntesis.

## Alcance

Todo lo siguiente está implementado como **reconocimiento sintáctico y nodos
AST**. El compilador todavía no ejecuta los programas ni genera ensamblador.
No carga archivos importados, asigna memoria a colecciones, implementa captura
de excepciones en ejecución ni verifica tipos o símbolos. Estas tareas requieren
las fases semánticas y de generación de código del equipo.

## Variables y constantes

```text
n * declare_int;
n : 5;
a * declare_int : 1, b * declare_int;
declare_const LIMITE * declare_int : 5;
```

El valor inicial es opcional para variables. Su ausencia no inserta un cero en
el AST ni garantiza un valor de ejecución. Cada declaración termina con `;`.
Las constantes usan el prefijo `declare_const` y requieren un inicializador.
Esta es la convención adoptada para concretar el token del avance. Se admiten
constantes globales y locales; las variables y listas se declaran en bloques.
Prohibir reasignaciones a constantes corresponde al análisis semántico.

## Operadores

Precedencia adoptada, de menor a mayor:

| Grupo | Asociatividad |
|---|---|
| `\|\|` (OR) | Izquierda |
| `^` (XOR lógico) | Izquierda |
| `&&` (AND) | Izquierda |
| `==`, `=/=`, `<`, `>`, `<=`, `>=` | Sin encadenar |
| `gauss`, `neumann` binario | Izquierda |
| `pitagoras`, `euclides`, `euler` (residuo) | Izquierda |
| `neumann` unario, `~` (NOT) | Prefijos |
| `descartes` (potencia) | Derecha |

Paréntesis, llamadas, acceso a miembros e índices permiten agrupar operandos.
`2 descartes 3 descartes 2` significa `2 descartes (3 descartes 2)`.
`neumann 2 descartes 2` se agrupa como `neumann (2 descartes 2)`.
Para negar una comparación completa, escribir `~(n == 0)`.
Las prioridades se eligieron explícitamente porque el PDF no las formaliza.
Todavía no hay evaluación ni implementación de cortocircuito; los nodos
preservan los operadores para la siguiente fase.

## Cycle

```text
cycle i let 0 until 5 step 2 {
    ultimo : i;
} endgame;
```

No se ponen paréntesis alrededor de la cabecera. El cierre es `endgame;`.
Inicio, límite y paso admiten expresiones. Si `step` se omite, el AST incluye
un entero `1` como paso. El límite final es **exclusivo**: el ejemplo anterior
describe los valores 0, 2 y 4. Para un recorrido descendente puede escribirse
`step neumann 1`; la condición prevista es mayor que el límite final.
La validación de paso cero, tipos y ámbito del índice queda para semántica.

Para visitar todos los elementos de una colección se escribe:

```text
cycle i let 0 until size(numeros) {
    numeros[i] : numeros[i] gauss 1;
} endgame;
```

No se resta 1 al límite exclusivo. `whale (...) { ... } stop;` sigue disponible.

## Arreglos, matrices y parámetros

```text
a * declare_int[5];
m * declare_int[2][3] : [[1, 2, 3], [4, 5, 6]];
dinamica * declare_int[filas][columnas gauss 1];
m[1][2] : a[0] gauss 1;
```

Las dimensiones de variables requieren expresiones; no se permite `a * declare_int[];`.
En parámetros sí se admiten dimensiones sin tamaño:

```text
create_funk #declare_int# leer(a * declare_int[], m * declare_int[filas][cols]) {
    give a[0] gauss m[0][1];
}
```

También se admiten parámetros como `m * declare_int[][]`. Las dimensiones y
los índices pueden repetirse: el parser no limita el número de dimensiones a dos.
Los inicializadores usan corchetes anidados y comas; cada elemento puede ser una
expresión. Varias declaraciones se separan por comas antes del `;` final.
Comprobar tamaños positivos, forma de inicializadores, límites de índices y tipos
corresponde al análisis semántico y, cuando proceda, a la ejecución.

## Listas

```text
declare_list numeros * declare_int : [10, 20], vacia * declare_int : [];
add(numeros, 30);
remove(numeros, 0);
cantidad * declare_int : size(numeros);
```

El inicializador de una lista es opcional; su ausencia se conserva en el AST.
`add` y `remove` requieren exactamente dos argumentos; `size`, exactamente uno.
`remove` recibe la colección y el índice, siguiendo el ejemplo del avance.
El parser permite usar estas operaciones como sentencias o expresiones; la fase
semántica deberá restringir el uso del resultado de operaciones sin retorno.

Para pasar una lista tipada se adopta `nombre * declare_list tipo`, por ejemplo:

```text
create_funk #declare_int# contar(xs * declare_list declare_int) {
    give size(xs);
}
```

Esta extensión diferencia listas de tamaño variable y parámetros de arreglo.

## Importaciones y acceso a módulos

```text
bring matematicas aka math;
bring utilidades;

create_funk #declare_int# sumar() {
    give math.suma(2, 3);
}
```

Las importaciones se escriben al nivel del programa, con alias opcional; los
nombres son identificadores, sin comillas ni extensión `.bal`. Las funciones,
importaciones y constantes globales se conservan en orden en `Programa`.
No se permite `bring` dentro de una función. Los módulos sin `main` pueden
analizarse: la verificación del punto de entrada corresponde a la integración.

`math.suma` genera un acceso a miembro; la llamada contiene ese acceso como
destino. También se representan accesos indexados como `datos.matriz[i][j]`.
Registrar una importación en el AST **no abre el archivo ni enlaza sus funciones**.

## Seek y seize

Se adopta la forma de captura tipada del avance:

```text
seek {
    resultado : math.suma(a, b);
} seize (ErrorAritmetico e) {
    valido : declare_false;
}
```

`seek` requiere un bloque y una o más capturas `seize (TipoError nombre) { ... }`.
Cada captura conserva el nombre del tipo, la variable y el bloque. El tipo del
error es un identificador, no un tipo de variable como `declare_int`.
Se admiten capturas sucesivas y bloques anidados. No llevan `;` al final.
La existencia de los tipos de error, la selección de una captura, el ámbito de
su variable y la traducción a la arquitectura quedan pendientes de integración.

## Ejemplos y pruebas

```bash
make
./balc -t examples/lenguaje_ampliado.bal
./balc -t examples/matematicas.bal
make test-parser
make test-ast
make test
```

`lenguaje_ampliado.bal` combina las extensiones y `matematicas.bal` contiene la
función importada de ejemplo. Cada archivo se analiza por separado en esta fase.
EOF y errores siguen siendo estados de la API: `@`, `!!!`, `!?` y `%?` no se
adoptan como símbolos del código fuente. Los identificadores siguen las reglas
ASCII del lexer de Javier; los acentos se admiten en textos y caracteres.
