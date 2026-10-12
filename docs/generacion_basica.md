# Generación básica

El módulo recibe un nodo del AST y genera instrucciones de la ISA del equipo.
Admite expresiones escalares, asignaciones y declaraciones con inicializador.

## Operaciones

Se traducen suma, resta, multiplicación, división, residuo, potencia, negación, comparaciones y lógica booleana. Los valores
se cargan con `cargai` y las asignaciones se guardan con `guardap`. Los enteros
son de 32 bits con signo; los booleanos se representan con 0 y 1.

Se evalúa primero el operando izquierdo, después el derecho y finalmente la
operación. Se usan x4–x7; la multiplicación también modifica los temporales x12–x15.
El resultado queda en x4. División, residuo y potencia usan también x8–x15;
el llamador debe guardar cualquier valor vivo en esos temporales. Si hacen falta más registros
o la expresión supera 256 niveles, se informa un error. Las constantes grandes
se construyen con `sumai`, `cizqi` y `ori`, usando inmediatos de 11 bits.

La lógica evalúa ambos operandos. Las llamadas y otras expresiones con efectos
no están admitidas en este módulo.

## Multiplicación

`pitagoras` calcula los 32 bits bajos del producto mediante sumas y
desplazamientos, usando instrucciones existentes del procesador. Los negativos
usan complemento a dos; el desbordamiento conserva el resultado módulo 2^32,
igual que suma y resta. No se agrega un opcode MUL.

El generador expande 32 pasos sin saltos ni etiquetas. El producto agrega 194
instrucciones, además de evaluar sus operandos. Con el adaptador conservador de
Arquitectura ocupa 580 bundles por producto; un programa con varios productos
puede exceder los 1024 bundles de la memoria actual. El ensamblador debe rechazar
ese caso. Esta expansión prioriza compatibilidad con el generador lineal; una
rutina compartida y su calendarización requieren coordinación con el backend.

Las pruebas incluyen negativos, límites de 32 bits, productos anidados y
128 pares reproducibles. También se comprobó `-7 * 6 = -42` desde fuente BAL
hasta la memoria del procesador, sin precargar las variables.

## División, residuo y potencia

`euclides` divide enteros con signo y trunca hacia cero. `euler` devuelve el
residuo con el signo del dividendo: `-17 / 5 = -3` y `-17 % 5 = -2`.
La división procesa 32 bits de las magnitudes. El caso `INT_MIN / -1` conserva
los 32 bits bajos: `INT_MIN`, con residuo cero.

`descartes` usa exponenciación por cuadrados, con exponentes enteros no
negativos. El resultado conserva los 32 bits bajos; se define `0^0 = 1`.
El número de iteraciones depende de los bits del exponente, no de su valor.

No se agregan opcodes. Estos operadores generan bucles con instrucciones ALU
y saltos del ISA. Las etiquetas usan los prefijos reservados `__gb_div_` y
`__gb_pow_`, con nombres diferentes en cada invocación. La generación actual
se realiza en un solo hilo. El backend debe resolver sus direcciones después
de calendarizar, e incluir dos NOP en cada destino para proteger las vueltas.
El adaptador `bundle_control.py` de Arquitectura permite probar esos fragmentos.

Un divisor cero literal o un exponente negativo literal se rechazan sin emitir
salida parcial. Si esos valores llegan por variables, el código entra en un
salto estable de detención antes de guardar el resultado. No hay mensaje de
error en el hardware ni retorno normal del programa en esos casos.

Ejemplo reproducible: `examples/division_residuo_potencia.bal`, con q=-3,
r=-2 y p=1024. Para la integración con Arquitectura hay que asignar ciclos de
simulación suficientes para los bucles; el ejemplo usa `--cycles 20000`.

## Interfaz

`gb_generar` recibe el nodo, una función para resolver identificadores, los datos
de esa función, la salida y los diagnósticos. Devuelve 0 en éxito y 1 en error.

La función de resolución entrega el tipo, registro base, desplazamiento en bytes
y permiso de escritura. Las bases admitidas son x0, x2 y x3; el desplazamiento
debe estar alineado a 4 bytes y caber entre -1024 y 1023. El llamador garantiza
que las bases y variables estén inicializadas y que las direcciones sean válidas.

Los temporales se reutilizan en cada invocación. El resultado en x4 debe
consumirse antes de generar otra expresión. Si falla la generación, no se
agregan instrucciones al destino. Si ocurre un fallo de escritura al copiar
el resultado, se debe descartar la salida.

## Pruebas

Desde la raíz del repositorio:

```bash
make test-generacion
./tests/test_generacion --ejemplo
```

El ejemplo traduce `r : (a gauss b) neumann 1;` con referencias de prueba en
x3+0, x3+4 y x3+8:

```text
cargai x4, x3, 0
cargai x5, x3, 4
suma x4, x4, x5
sumai x5, x0, 1
resta x4, x4, x5
guardap x3, x4, 8
```

Las pruebas comprueban resultados y saltos con un modelo secuencial; incluyen
128 productos, 256 divisiones/residuos y 64 potencias reproducibles. La tabla de
referencias usada en ellas sirve únicamente para probar el módulo.

## Pendientes

El adaptador `generacion_adaptador.c` conecta referencias de la tabla real de
símbolos para su subconjunto escalar. Falta completar control de flujo, organizar
las instrucciones en paquetes VLIW y producir el binario. El fragmento anterior
presupone datos y x3 inicializados; todavía no se puede ejecutar directamente.

Tampoco se admiten llamadas, índices, colecciones, cadenas ni caracteres.
El factorial dispone de multiplicación, pero todavía necesita generación de
condicionales/ciclos y la integración final para producir un programa ejecutable.

El contrato actual reserva x4–x15 como temporales y x16–x31 como preservados.
La convención escalar de funciones usa x8–x11 para argumentos y x8 para retorno.
El empaquetador debe respetar que una escritura del bundle N se lea desde N+3.

## Referencias

- Jason Leitón Jiménez. *Introducción*, TEC CE1108, 2026. Diapositivas 16 y 20:
  AST, generación de código, registros y memoria.
- Jason Leitón Jiménez. *Proyecto: Compilador para Código Binario*, II semestre
  2026. Secciones 2.4, 3 y 8.
- [ISA vigente](https://github.com/Izaguirre1g/Proyecto1_Arquitectura1/blob/818e1de/docs/isa.md).
- [Contrato de integración](https://github.com/Izaguirre1g/Proyecto1_Arquitectura1/blob/818e1de/docs/compiler-integration.md).
