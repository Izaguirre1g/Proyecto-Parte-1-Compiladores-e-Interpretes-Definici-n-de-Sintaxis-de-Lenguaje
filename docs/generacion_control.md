# Generacion de condicionales — Meibel

Esta primera entrega traduce `AST_SI` (`whether / alif / also`) desde
`src/generacion_funciones.c`. Reutiliza `gb_generar()` para condiciones sin
llamadas: el resultado booleano queda en `x4`. Requiere analisis semantico previo.

## Recorrido

1. Evaluar la condicion.
2. Emitir `igualsi x4, x0, etiqueta_else` para la ruta falsa.
3. Generar el bloque verdadero.
4. Si hay alternativa y la rama verdadera continua, saltar a `etiqueta_fin`.
5. Emitir `etiqueta_else` y generar el bloque alternativo o el siguiente `alif`.
6. Emitir `etiqueta_fin` cuando existe un salto hacia ella.

Las etiquetas tienen el prefijo reservado `__gf_<funcion>_` y un contador local.
Esto separa condicionales anidados y funciones distintas. Cada funcion se genera
una sola vez por unidad de salida. Las etiquetas no contienen direcciones.

Los bloques informan si todos sus caminos retornan. Un `give` genera el valor
en `x8` y emite el epilogo en esa ruta: restaura `ra`, recupera `sp` y retorna.
Un condicional garantiza retorno solo si ambas ramas lo garantizan. Si una rama
continua, puede alcanzar instrucciones posteriores y un retorno posterior.
Las funciones void que llegan al final reciben un epilogo implicito.

Se conserva el ensamblador previo de funciones lineales. Los errores se detectan
en una salida temporal antes de publicar instrucciones al destino.

## Probar y ver el resultado

Desde la raiz del proyecto:

```sh
make test-funciones-backend test-referencias-integracion test-condicional-pipeline
./tests/test_condicional_pipeline --emitir
```

`examples/condicional_meibel.bal` usa un `whether`, un `alif` y un `also`.
Para `n = 5`, la rama correspondiente asigna 20 a `resultado`.
El test de pipeline utiliza parser, semantica, tabla de simbolos, memoria y
adaptador reales, y permite inspeccionar la salida lineal con `--emitir`.

`tests/test_generacion_funciones.c` comprueba las rutas verdaderas/falsas,
`alif`, anidamiento, ramas que continuan, ausencia de `also`, retornos y
restauracion de pila mediante un modelo secuencial limitado de instrucciones.
Tambien comprueba rechazo sin publicar salida ante estructuras invalidas y
caminos sin retorno. Este modelo no verifica latencias del procesador.

## Contrato con Dylan y Jose

La salida sigue siendo intermedia: instrucciones individuales y etiquetas.
Necesita empaquetado ALU/LSU/BRU/CRIPTO y separaciones por dependencias antes de
calcular PC y desplazamientos. La guia establece bundles de 16 bytes y consumo
de registros a partir de N+3. Los destinos de llamadas absolutas tambien deben
corresponder al layout final. Una etiqueta no consume bytes por si misma.

`rr_desplazamiento()` puede validar un salto una vez definidas sus direcciones;
esta entrega no calcula esas posiciones ni reemplaza el ensamblador binario.
Los textos recibidos de arquitectura son tests que importan `tools.assembler`
y `tools.bundle_linear`, no las implementaciones. La prueba compartida del
bundler rechaza BRU: hay que ampliar ese componente antes de usar esta salida.

## Limites de esta entrega

- No genera ciclos todavia.
- No admite llamadas dentro de condiciones.
- Se pueden usar variables de la funcion o globales admitidas por el adaptador.
  Se rechazan declaraciones dentro de ramas para evitar resolver incorrectamente
  nombres de ambitos anidados; esa integracion queda pendiente.
- No agrega opciones de CLI ni produce binarios o bundles ejecutables.
- Falta validar empaquetado, rangos de saltos y ejecucion con la arquitectura real.
