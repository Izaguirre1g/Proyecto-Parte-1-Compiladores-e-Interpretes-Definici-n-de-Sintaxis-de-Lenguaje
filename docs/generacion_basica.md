# Generación básica

El módulo recibe un nodo del AST y genera instrucciones de la ISA del equipo.
Admite expresiones escalares, asignaciones y declaraciones con inicializador.

## Operaciones

Se traducen suma, resta, multiplicación, negación, comparaciones y lógica booleana. Los valores
se cargan con `cargai` y las asignaciones se guardan con `guardap`. Los enteros
son de 32 bits con signo; los booleanos se representan con 0 y 1.

Se evalúa primero el operando izquierdo, después el derecho y finalmente la
operación. Se usan x4–x7; la multiplicación también modifica los temporales x12–x15.
El resultado queda en x4. Los registros x8–x11 de argumentos se conservan. Si hacen falta más registros
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

Las pruebas comprueban resultados con un modelo secuencial. La tabla de
referencias usada en ellas sirve únicamente para probar el módulo.

## Pendientes

El adaptador `generacion_adaptador.c` conecta referencias de la tabla real de
símbolos para su subconjunto escalar. Falta completar control de flujo, organizar
las instrucciones en paquetes VLIW y producir el binario. El fragmento anterior
presupone datos y x3 inicializados; todavía no se puede ejecutar directamente.

División, residuo y potencia aún no tienen traducción.
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
