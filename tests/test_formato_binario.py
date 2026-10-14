import pathlib
import shutil
import struct
import subprocess
import tempfile
import unittest

ROOT = pathlib.Path(__file__).resolve().parents[1]


class FormatoBinarioTests(unittest.TestCase):
    def setUp(self):
        self.tmp = tempfile.TemporaryDirectory()
        self.addCleanup(self.tmp.cleanup)
        self.folder = pathlib.Path(self.tmp.name)

    def ejecutar(self, *args):
        return subprocess.run([str(ROOT / "balc"), *map(str, args)],
                              capture_output=True, text=True)

    def leer(self, ruta):
        raw = ruta.read_bytes()
        magic, version, header, total, entry, co, cs, ca, do, ds, da, flags = \
            struct.unpack_from("<4s11I", raw)
        self.assertEqual((magic, version, header), (b"BALB", 1, 48))
        self.assertEqual(total, len(raw))
        self.assertEqual((entry, co, ca, do, da), (0, 48, 0, 48 + cs, 0x4000))
        self.assertEqual(cs % 16, 0)
        self.assertGreater(cs, 0)
        self.assertEqual(do + ds, total)
        codigo = raw[co:co + cs]
        mem = ruta.with_suffix(".mem").read_text().splitlines()
        self.assertEqual(codigo, b"".join(int(x, 16).to_bytes(16, "little") for x in mem))
        return codigo, raw[do:], flags

    def test_tres_ejercicios_y_opciones(self):
        for nombre, bundles in [("factorial", 189), ("ejercicio2_busqueda_matriz", 291),
                                ("ejercicio3_puntajes", 801)]:
            with self.subTest(nombre=nombre):
                fuente = self.folder / (nombre + ".bal")
                shutil.copyfile(ROOT / "examples" / fuente.name, fuente)
                p = self.ejecutar("-s", "-t", "-m", "-v", "-p", fuente)
                self.assertEqual(p.returncode, 0, p.stderr)
                codigo, datos, flags = self.leer(fuente.with_suffix(".bin"))
                self.assertEqual(len(codigo), bundles * 16)
                self.assertEqual((datos, flags), (b"", 1))
                # La ruta -x codifica exactamente las mismas instrucciones.
                destino = self.folder / "desde_asm.bin"
                p = self.ejecutar("-x", fuente.with_suffix(".asm"), "-o", destino)
                self.assertEqual(p.returncode, 0, p.stderr)
                otro, datos, flags = self.leer(destino)
                self.assertEqual(otro, codigo)
                self.assertEqual((datos, flags), (b"", 0))

    def test_codigo_contra_fixture_de_arquitectura(self):
        destino = self.folder / "isa.bin"
        p = self.ejecutar("-x", ROOT / "tests/fixtures/isa_todas.asm", "-o", destino)
        self.assertEqual(p.returncode, 0, p.stderr)
        codigo, datos, flags = self.leer(destino)
        self.assertEqual(codigo, (ROOT / "tests/fixtures/isa_todas.bin").read_bytes())
        self.assertEqual((datos, flags), (b"", 0))

    def test_datos_globales_con_signo_y_referencias(self):
        fuente = self.folder / "globales.bal"
        fuente.write_text("""declare_const A * declare_int : 0005;
declare_const B * declare_int : neumann 7;
declare_const C * declare_int : A gauss B;
declare_const D * declare_int : B euclides 3;
declare_const E * declare_int : B euler 3;
declare_const F * declare_int : 2 descartes 10;
create_funk #declare_infinite_void# main() {
    valor * declare_int : C;
}
""")
        p = self.ejecutar(fuente)
        self.assertEqual(p.returncode, 0, p.stderr)
        _, datos, flags = self.leer(fuente.with_suffix(".bin"))
        self.assertEqual(struct.unpack("<6i", datos), (5, -7, -2, -2, -1, 1024))
        self.assertEqual(flags, 1)

    def test_limites_aritmeticos_en_datos(self):
        fuente = self.folder / "limites.bal"
        fuente.write_text("""declare_const A * declare_int : neumann 2147483648;
declare_const B * declare_int : A euclides (neumann 1);
declare_const C * declare_int : A euler (neumann 1);
declare_const D * declare_int : 2147483647 gauss 1;
declare_const E * declare_int : 65536 pitagoras 65536;
declare_const F * declare_int : 0 descartes 0;
create_funk #declare_infinite_void# main() { valor * declare_int : B; }
""")
        p = self.ejecutar(fuente)
        self.assertEqual(p.returncode, 0, p.stderr)
        _, datos, _ = self.leer(fuente.with_suffix(".bin"))
        self.assertEqual(struct.unpack("<6i", datos),
                         (-2147483648, -2147483648, 0, -2147483648, 0, 1))

    def test_inicializador_invalido_no_crea_archivos(self):
        fuente = self.folder / "invalido.bal"
        fuente.write_text("""declare_const CERO * declare_int : 0;
declare_const MAL * declare_int : 7 euclides CERO;
create_funk #declare_infinite_void# main() { valor * declare_int : 1; }
""")
        p = self.ejecutar("-s", fuente)
        self.assertNotEqual(p.returncode, 0)
        self.assertIn("datos del binario", p.stderr)
        for extension in (".bin", ".mem", ".asm"):
            self.assertFalse(fuente.with_suffix(extension).exists())


if __name__ == "__main__":
    unittest.main()
