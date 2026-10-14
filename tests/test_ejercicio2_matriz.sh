#!/bin/sh
# Ejercicio 2: pruebas del frontend y de asignacion de memoria (no ejecucion BAL).
set -eu
salida=$(mktemp)
error=$(mktemp)
caso=$(mktemp --suffix=.bal)
binario=$(mktemp --suffix=.bin)
trap 'rm -f "$salida" "$error" "$caso" "$binario" "${binario%.bin}.mem"' EXIT HUP INT TERM
fuente=examples/ejercicio2_busqueda_matriz.bal
if ! ./balc --semantico "$fuente" >"$salida" 2>"$error"; then
    echo 'FALLO: ejercicio 2 no supera analisis semantico' >&2
    cat "$error" >&2
    exit 1
fi
# -m tambien compila hasta el binario.
if ! ./balc -m -o "$binario" "$fuente" >"$salida" 2>"$error"; then
    echo 'FALLO: ejercicio 2 no compila' >&2
    cat "$error" >&2
    exit 1
fi
if [ ! -s "$binario" ]; then
    echo 'FALLO: ejercicio 2 no genero el binario' >&2
    exit 1
fi
for patron in '[SIMBOLOS]' 'matriz | variable' 'objetivo | variable' 'fila_encontrada | variable' 'columna_encontrada | variable' '[MEMORIA]'; do
    if ! grep -F "$patron" "$salida" >/dev/null; then
        echo "FALLO: falta en -m: $patron" >&2
        cat "$salida" >&2
        exit 1
    fi
done
# Mutacion negativa: indice estatico fuera de limite en primera dimension.
# Se agrega dentro de main, reemplazando una instruccion existente sin alterar el resto.
sed 's/columna : 0;/columna : matriz[2][0];/' "$fuente" > "$caso"
if ./balc --semantico "$caso" >"$salida" 2>"$error"; then
    echo 'FALLO: se acepto acceso matriz[2][0] fuera de rango' >&2
    exit 1
fi
if ! grep -F 'fuera de rango' "$error" "$salida" >/dev/null 2>&1; then
    echo 'FALLO: no se informo indice fuera de rango' >&2
    cat "$error" >&2
    exit 1
fi
printf '%s\n' 'OK: Ejercicio 2 - sintaxis, semantica, -m, binario y rechazo de indices'
