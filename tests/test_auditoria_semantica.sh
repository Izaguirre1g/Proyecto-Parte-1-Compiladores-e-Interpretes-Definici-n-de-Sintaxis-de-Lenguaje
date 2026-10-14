#!/bin/sh
# Pruebas de cobertura semantica sobre el parser y el analizador reales.
set -eu
dir=$(mktemp -d)
trap 'rm -rf "$dir"' EXIT HUP INT TERM
validar() {
    nombre=$1
    contenido=$2
    printf '%s\n' "$contenido" >"$dir/$nombre.bal"
    if ! ./balc --semantico "$dir/$nombre.bal" >"$dir/salida" 2>"$dir/error"; then
        echo "FALLO: $nombre deberia ser valido" >&2
        cat "$dir/error" >&2
        exit 1
    fi
    echo "[OK auditoria] $nombre"
}
rechazar() {
    nombre=$1
    contenido=$2
    patron=$3
    printf '%s\n' "$contenido" >"$dir/$nombre.bal"
    if ./balc --semantico "$dir/$nombre.bal" >"$dir/salida" 2>"$dir/error"; then
        echo "FALLO: $nombre deberia rechazarse" >&2
        exit 1
    fi
    if ! grep -F "$patron" "$dir/error" "$dir/salida" >/dev/null 2>&1; then
        echo "FALLO: $nombre carece de diagnostico esperado: $patron" >&2
        cat "$dir/error" "$dir/salida" >&2
        exit 1
    fi
    echo "[OK auditoria] $nombre rechazado"
}
validar captura_valida 'create_funk #declare_infinite_void# main() { seek {} seize (ErrorTipo e) {} }'
validar capturas_distintas 'create_funk #declare_infinite_void# main() { seek {} seize (ErrorUno e) {} seize (ErrorDos e) {} }'
rechazar captura_duplicada 'create_funk #declare_infinite_void# main() { seek {} seize (ErrorUno e) {} seize (ErrorUno otro) {} }' 'capturado mas de una vez'
rechazar fuga_variable_captura 'create_funk #declare_infinite_void# main() { seek {} seize (ErrorUno e) {} e : 2; }' "identificador 'e' no declarado"
rechazar identificador_no_declarado 'create_funk #declare_infinite_void# main() { x : 1; }' "identificador 'x' no declarado"
rechazar tipo_retorno 'create_funk #declare_int# identidad() { give declare_true; } create_funk #declare_infinite_void# main() { }' 'retorno incompatible'
printf '%s\n' 'OK: auditoria semantica adicional (6 escenarios)'
