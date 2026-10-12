#!/bin/sh
set -eu
salida=$(mktemp)
error=$(mktemp)
trap 'rm -f "$salida" "$error"' EXIT HUP INT TERM
./balc -m tests/fixtures/3bd3b_escalar.bal >"$salida" 2>"$error"
grep -F '[SIMBOLOS]' "$salida" >/dev/null
grep -F 'identidad | funcion | declare_int' "$salida" >/dev/null
grep -F 'main | funcion | declare_infinite_void' "$salida" >/dev/null
grep -F 'parametro(sp)' "$salida" >/dev/null
grep -F 'local(sp)' "$salida" >/dev/null
grep -F ' | pendiente' "$salida" >/dev/null
if ./balc -m tests/modulos/principal_inexistente.bal >"$salida" 2>"$error"; then
    echo 'ERROR: -m acepto una importacion inexistente' >&2
    exit 1
fi
test ! -s "$salida"
if ./balc -m -t tests/fixtures/3bd3b_escalar.bal >"$salida" 2>"$error"; then
    echo 'ERROR: -m -t debio rechazarse' >&2
    exit 1
fi
printf '%s\n' 'OK: 3B-D3F -m simbolos, memoria y errores'
