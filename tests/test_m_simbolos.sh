#!/bin/sh
set -eu
salida=$(mktemp)
error=$(mktemp)
binario=$(mktemp --suffix=.bin)
memoria="${binario%.bin}.mem"
trap 'rm -f "$salida" "$error" "$binario" "$memoria"' EXIT HUP INT TERM
# -m imprime la tabla y tambien compila: el binario va a un temporal.
./balc -m -o "$binario" tests/fixtures/3bd3b_escalar.bal >"$salida" 2>"$error"
grep -F '[SIMBOLOS]' "$salida" >/dev/null
grep -F 'identidad | funcion | declare_int' "$salida" >/dev/null
grep -F 'main | funcion | declare_infinite_void' "$salida" >/dev/null
grep -F 'parametro(sp)' "$salida" >/dev/null
grep -F 'local(sp)' "$salida" >/dev/null
grep -F ' | pendiente' "$salida" >/dev/null
# Tras compilar, cada funcion muestra su direccion de codigo.
grep -E '^identidad \| funcion \| declare_int \| global \| 0 \| codigo \| 0 \| pendiente \| 0x[0-9a-f]{4}$' "$salida" >/dev/null
test -s "$binario"
if ./balc -m -o "$binario" tests/modulos/principal_inexistente.bal >"$salida" 2>"$error"; then
    echo 'ERROR: -m acepto una importacion inexistente' >&2
    exit 1
fi
test ! -s "$salida"
# Las opciones se combinan: -m -t imprime el AST y la tabla.
./balc -m -t -o "$binario" tests/fixtures/3bd3b_escalar.bal >"$salida" 2>"$error"
grep -F '[AST]' "$salida" >/dev/null
grep -F '[SIMBOLOS]' "$salida" >/dev/null
printf '%s\n' 'OK: 3B-D3F -m simbolos, memoria y errores'
