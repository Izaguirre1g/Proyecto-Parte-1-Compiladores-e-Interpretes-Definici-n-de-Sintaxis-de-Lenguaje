"""Pruebas del flujo real: archivo -> lexer de Javier -> Bison -> CLI."""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[1]


def ejecutar(archivo, *opciones):
    return subprocess.run(
        [str(ROOT / 'micomp'), *opciones, str(archivo)],
        capture_output=True, text=True, check=False,
    )


def cuerpo(texto):
    return 'create_funk #declare_infinite_void# main() {\n' + texto + '\n}\n'


casos = [
    ('bloque vacio', cuerpo(''), 0, ''),
    ('tipos y literales', cuerpo('n * declare_int : 1; t * declare_text : "café"; '
                               'c * declare_char : $ñ$; b * declare_boolean : declare_true;'), 0, ''),
    ('aritmetica y comparacion', cuerpo('n * declare_int : neumann (2 gauss 3) pitagoras 4 euclides 2; '
                                      'b * declare_boolean : n =/= 0;'), 0, ''),
    ('operadores de comparacion', cuerpo('b * declare_boolean : 1 == 1; b : 1 < 2; b : 2 > 1; '
                                        'b : 1 <= 2; b : 2 >= 1;'), 0, ''),
    ('anidamiento', cuerpo('whether (n <= 1) { whale (i < n) { '
                          'whether (i == 2) {} also { i : i gauss 1; } } stop; } also {}'), 0, ''),
    ('varias funciones', 'create_funk #declare_infinite_void# auxiliar() {}\n' + cuerpo(''), 0, ''),
    ('comentarios', cuerpo('%% comentario\n%%// bloque //%%\nn * declare_int : 5;'), 0, ''),
    ('falta punto y coma', cuerpo('n * declare_int : 5'), 1, 'error del parser'),
    ('falta stop', cuerpo('whale (n > 0) {}'), 1, 'error del parser'),
    ('also suelto', cuerpo('also {}'), 1, 'error del parser'),
    ('comparacion encadenada', cuerpo('b * declare_boolean : 1 < 2 < 3;'), 1, 'error del parser'),
    ('falta llave', 'create_funk #declare_infinite_void# main() {', 1, 'error del parser'),
    ('vacio', '', 1, 'error del parser'),
    ('sentencia fuera de funcion', 'n * declare_int : 5;', 1, 'error del parser'),
    ('token aun no soportado', cuerpo('give 1;'), 1, 'token GIVE aún no admitido'),
    ('error lexico', cuerpo('@'), 1, ':2:1: error léxico'),
    ('posicion tras Unicode', cuerpo('t * declare_text : "ñ"; @'), 1, ':2:25: error léxico'),
    ('decimal invalido', cuerpo('n * declare_int : 1.5;'), 1, 'error léxico'),
    ('NUL', cuerpo('\0'), 1, 'error léxico'),
    ('texto sobrante', cuerpo('') + 'n', 1, 'error del parser'),
]

with tempfile.TemporaryDirectory() as carpeta:
    entrada = Path(carpeta) / 'prueba.bal'
    for nombre, fuente, codigo, mensaje in casos:
        entrada.write_text(fuente, encoding='utf-8')
        resultado = ejecutar(entrada, '-t')
        assert resultado.returncode == codigo, (nombre, resultado)
        assert mensaje in resultado.stderr, (nombre, resultado.stderr)
        if codigo:
            assert 'Análisis sintáctico correcto' not in resultado.stdout, nombre
        else:
            assert 'Análisis sintáctico correcto' in resultado.stdout, nombre
    # Los tokens validos no garantizan sintaxis valida: conservar modo lexico.
    entrada.write_text('n * declare_int : ;', encoding='utf-8')
    assert ejecutar(entrada).returncode == 0
    assert ejecutar(entrada, '-t').returncode == 1
    assert ejecutar(Path(carpeta) / 'no_existe.bal', '-t').returncode == 2

for opciones in [('-t',), ('-v', '-t')]:
    resultado = ejecutar(ROOT / 'examples/factorial.bal', *opciones)
    assert resultado.returncode == 0, resultado.stderr
    assert 'Análisis sintáctico correcto' in resultado.stdout
    if '-v' in opciones:
        assert '64 tokens, 0 errores' in resultado.stdout

print(f'OK: {len(casos)} casos del parser, modos de CLI y factorial con/sin -v')
