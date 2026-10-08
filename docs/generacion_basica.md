# Generación básica

El módulo recibe un nodo del AST y genera instrucciones de la ISA del equipo.
Admite expresiones escalares, asignaciones y declaraciones con inicializador.

## Operaciones

Se traducen suma, resta, negación, comparaciones y lógica booleana. Los valores
se cargan con `cargai` y las asignaciones se guardan con `guardap`. Los enteros
son de 32 bits con signo; los booleanos se representan con 0 y 1.

Se evalúa primero el operando izquierdo, después el derecho y finalmente la
operación. Se usan x4–x7; el resultado queda en x4. Si hacen falta más registros
o la expresión supera 256 niveles, se informa un error. Las constantes grandes
se construyen con `sumai`, `cizqi` y `ori`, usando inmediatos de 11 bits.

La lógica evalúa ambos operandos. Las llamadas y otras expresiones con efectos
no están admitidas en este módulo.

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

Falta conectar la tabla real de símbolos, generar control de flujo, organizar
las instrucciones en paquetes VLIW y producir el binario. El fragmento anterior
presupone datos y x3 inicializados; todavía no se puede ejecutar directamente.

Multiplicación, división, residuo y potencia aún no tienen traducción.
Tampoco se admiten llamadas, índices, colecciones, cadenas ni caracteres.
El factorial requiere completar la multiplicación y su integración con el resto
del compilador.

La propuesta de ISA y el mapa nuevo difieren en el uso de x8–x15; por eso se
usan x4–x7, temporales en ambos documentos. El mapa de memoria sigue pendiente
de confirmación. El empaquetador deberá respetar las dependencias del pipeline.

## Referencias

- Jason Leitón Jiménez. *Introducción*, TEC CE1108, 2026. Diapositivas 16 y 20:
  AST, generación de código, registros y memoria.
- Jason Leitón Jiménez. *Proyecto: Compilador para Código Binario*, II semestre
  2026. Secciones 2.4, 3 y 8.
- [ISA del equipo](https://github.com/Izaguirre1g/Proyecto1_Arquitectura1/blob/eb9e5eee2184ab6afb51b739d42ac1247cef4e7d/ISA_Proyecto.md).
- [Mapa de memoria y registros](https://github.com/Izaguirre1g/Proyecto1_Arquitectura1/blob/ba1b49edc4f0b621552998cdc8bdf8b2c2c40cbf/docs/mapa_memoria.md).
