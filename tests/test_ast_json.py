"""Comprueba el JSON real de la CLI y su relacion con el fuente."""
import json
from pathlib import Path
import subprocess
import tempfile
import unittest


def nodos(raiz):
    pendientes = [raiz]
    while pendientes:
        nodo = pendientes.pop()
        yield nodo
        pendientes.extend(reversed(nodo["hijos"]))


class AstJsonTests(unittest.TestCase):
    def ejecutar(self, fuente, *opciones):
        with tempfile.TemporaryDirectory() as carpeta:
            archivo = Path(carpeta) / "prueba.bal"
            archivo.write_text(fuente, encoding="utf-8")
            resultado = subprocess.run(["./balc", "--ast-json", *opciones, str(archivo)],
                                       capture_output=True, text=True, check=False)
            for extension in (".bin", ".mem", ".asm"):
                self.assertFalse(archivo.with_suffix(extension).exists())
            return resultado

    def test_factorial(self):
        salida = subprocess.run(["./balc", "--ast-json", "examples/factorial.bal"],
                                capture_output=True, text=True, check=True)
        arbol = json.loads(salida.stdout)
        self.assertEqual(arbol["tipo"], "Programa")
        tipos = {n["tipo"] for n in nodos(arbol)}
        self.assertTrue({"Funcion", "Declaracion", "Si", "Mientras", "Binario"} <= tipos)
        self.assertEqual(salida.stderr, "")
        for nodo in nodos(arbol):
            self.assertEqual(set(nodo), {"tipo", "texto", "ubicacion", "hijos"})
            self.assertGreaterEqual(nodo["ubicacion"]["linea_inicio"], 1)

    def test_ceros_y_precedencia(self):
        salida = self.ejecutar("create_funk #declare_infinite_void# main() {\n"
                               "n * declare_int : 0005 gauss 0008 pitagoras 0002;\n}")
        self.assertEqual(salida.returncode, 0, salida.stderr)
        arbol = json.loads(salida.stdout)
        suma = next(n for n in nodos(arbol) if n["texto"] == "gauss")
        self.assertEqual(suma["hijos"][0]["texto"], "0005")
        self.assertEqual(suma["hijos"][1]["texto"], "pitagoras")
        self.assertEqual([n["texto"] for n in suma["hijos"][1]["hijos"]], ["0008", "0002"])

    def test_escapes_y_unicode(self):
        literal = '"barra \\ y tab\t y control\x01 y ñ"'
        salida = self.ejecutar("create_funk #declare_infinite_void# main() {\n"
                               "s * declare_text : " + literal + ";\n}")
        self.assertEqual(salida.returncode, 0, salida.stderr)
        cadena = next(n for n in nodos(json.loads(salida.stdout)) if n["tipo"] == "Cadena")
        self.assertEqual(cadena["texto"], literal)

    def test_error_no_emite_ast_parcial(self):
        for fuente in ["create_funk #declare_infinite_void# main() {",
                       "create_funk #declare_infinite_void# main() { @ }"]:
            with self.subTest(fuente=fuente):
                salida = self.ejecutar(fuente)
                self.assertNotEqual(salida.returncode, 0)
                self.assertEqual(salida.stdout, "")
                self.assertTrue(salida.stderr)

    def test_opciones(self):
        fuente = Path("examples/factorial.bal").read_text()
        salida = self.ejecutar(fuente, "-t")
        self.assertEqual(salida.returncode, 0)
        self.assertEqual(json.loads(salida.stdout)["tipo"], "Programa")
        salida = self.ejecutar(fuente, "-o", "salida.bin")
        self.assertEqual(salida.returncode, 2)
        self.assertEqual(salida.stdout, "")
        for opcion in ["-v", "-m", "-s", "-x", "-p"]:
            salida = self.ejecutar(fuente, opcion)
            self.assertEqual(salida.returncode, 2)
            self.assertEqual(salida.stdout, "")

    def test_ensamblador_sigue_funcionando(self):
        with tempfile.TemporaryDirectory() as carpeta:
            fuente = Path(carpeta) / "factorial.bal"
            fuente.write_text(Path("examples/factorial.bal").read_text())
            salida = subprocess.run(["./balc", "-s", str(fuente)],
                                    capture_output=True, text=True, check=False)
            self.assertEqual(salida.returncode, 0, salida.stderr)
            asm = fuente.with_suffix(".asm").read_text()
            self.assertIn("guardap", asm)
            self.assertTrue(any(len(linea.split("|")) == 4 for linea in asm.splitlines()))
            self.assertIn("[ensamblador]", salida.stdout)
            self.assertEqual(fuente.with_suffix(".bin").read_bytes()[:4], b"BALB")
            self.assertTrue(fuente.with_suffix(".mem").is_file())


if __name__ == "__main__":
    unittest.main()
