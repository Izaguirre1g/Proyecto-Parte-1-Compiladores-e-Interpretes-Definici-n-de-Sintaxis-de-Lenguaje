# AST en JSON

`--ast-json` construye el AST con el parser existente y escribe únicamente JSON en stdout. Los errores van a stderr; si el fuente tiene un error léxico o sintáctico, no se emite un árbol parcial. No realiza análisis semántico ni genera código.

```sh
make
./balc --ast-json examples/factorial.bal > factorial.ast.json
python3 -m json.tool factorial.ast.json > /dev/null
xdg-open tools/visualizar_ast.html
```

En el navegador, seleccionar `factorial.ast.json`. El árbol se dibuja con la raiz arriba, nodos circulares y lineas entre padres e hijos. Conserva el orden de los hijos. Permite zoom, desplazamiento, ajuste del arbol y expandir o contraer nodos. Al pasar el cursor sobre un nodo se muestran el texto completo y su ubicacion. El visualizador funciona sin conexión ni bibliotecas externas; no sube el archivo a un servidor.

Cada nodo contiene `tipo`, `texto` (cadena o null), `ubicacion` y `hijos` (arreglo). La ubicación conserva línea y columna inicial/final del AST; empiezan en 1 y el extremo final es exclusivo. Los literales conservan su lexema: `0005` aparece como `0005`, aunque su valor decimal sea 5. No se exportan punteros a padres ni la lista privada de reservas.

`-t` conserva la impresión de texto anterior. Se acepta `-t --ast-json` y en ese caso se emite JSON. `--ast-json` no se combina con `-v`, `-m` ni `-s` para evitar mezclar reportes en stdout. `-o` sigue reservado para la integración de la salida del compilador.

El exportador recorre los enlaces del AST sin recursión. El visualizador admite hasta 20 000 nodos y archivos de hasta 10 MiB. Estos límites pertenecen al visualizador, no al lenguaje.

```sh
make test-ast-json
make test-ast test-cli test-parser
```

Las pruebas leen la salida con el módulo JSON de Python y comprueban factorial, precedencia, ceros iniciales, escapes, Unicode, combinaciones de opciones y errores sin salida parcial.

La opcion `-s` incorporada en development se conserva: `./balc -s examples/factorial.bal` genera el ensamblador en bundles como antes. Las pruebas JSON tambien comprueban esa salida en una carpeta temporal.
