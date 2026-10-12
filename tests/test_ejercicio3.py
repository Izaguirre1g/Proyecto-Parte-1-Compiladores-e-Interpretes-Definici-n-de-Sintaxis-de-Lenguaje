"""Ejecuta la salida C real en un modelo secuencial, sin assembler ni bundler."""
from pathlib import Path
import subprocess
import unittest

ROOT = Path(__file__).resolve().parents[1]


class Ejercicio3(unittest.TestCase):
    def test_programa_completo(self):
        source = subprocess.check_output([str(ROOT / 'tests/test_ejercicio3')],
                                         cwd=ROOT, text=True)
        functions, destinations, locals_main = {}, {}, {}
        current = None
        for line in source.splitlines():
            if line.startswith('# funcion '):
                _, _, name, address, frame = line.split()
                current = functions[name] = {'lines': [], 'labels': {}, 'frame': int(frame)}
                destinations[int(address)] = name
            elif line.startswith('# local '):
                _, _, name, offset = line.split()
                locals_main[name] = int(offset)
            elif line.startswith('#') or not line:
                continue
            elif line.endswith(':'):
                current['labels'][line[:-1]] = len(current['lines'])
            else:
                current['lines'].append(line)
        self.assertEqual(set(functions), {'sumar', 'puntaje', 'acumular', 'main'})
        registers, memory = [0] * 32, {}
        registers[2], registers[1] = 32768, -1
        function, pc, depth, maximum, calls = 'main', 0, 0, 0, 0
        continuations = {}
        visits = set()
        for _ in range(10000):
            code = functions[function]
            self.assertLess(pc, len(code['lines']))
            instruction = code['lines'][pc]
            visits.add((function, pc))
            pc += 1
            op, operands = instruction.split(None, 1)
            args = [a.strip() for a in operands.split(',')]
            def reg(index):
                return int(args[index][1:])
            if op in ('sumai', 'restai'):
                a, b, value = reg(0), reg(1), int(args[2])
                registers[a] = registers[b] + (value if op == 'sumai' else -value)
            elif op in ('suma', 'mrq'):
                a, b, c = map(lambda a: int(a[1:]), args)
                registers[a] = (registers[b] + registers[c] if op == 'suma'
                                else int(registers[b] < registers[c]))
            elif op in ('guardap', 'cargai'):
                a, b, offset = reg(0), reg(1), int(args[2])
                address = registers[a if op == 'guardap' else b] + offset
                self.assertTrue(0 <= address < 65536 and address % 4 == 0)
                if op == 'guardap':
                    memory[address] = registers[b]
                else:
                    self.assertIn(address, memory, 'lectura sin inicializar')
                    registers[a] = memory[address]
            elif op == 'igualsi':
                if registers[reg(0)] == registers[reg(1)]:
                    pc = code['labels'][args[2]]
            elif op == 'sye':
                self.assertEqual(args[0], 'x0')
                pc = code['labels'][args[1]]
            elif op == 'syer':
                self.assertEqual(int(args[2]), 0)
                target = registers[reg(1)]
                if reg(0) == 1:
                    self.assertIn(target, destinations)
                    calls += 1
                    # Enlaces del modelo: tokens, NO PC de bundles.
                    link = 100000 + calls
                    continuations[link] = (function, pc, registers[2])
                    registers[1] = link
                    function, pc = destinations[target], 0
                    depth += 1
                    maximum = max(maximum, depth)
                else:
                    self.assertEqual(reg(0), 0)
                    if target == -1:
                        self.assertEqual(function, 'main')
                        break
                    self.assertIn(target, continuations)
                    function, pc, expected_sp = continuations.pop(target)
                    self.assertEqual(registers[2], expected_sp)
                    depth -= 1
            else:
                self.fail(f'instruccion no modelada: {instruction}')
            registers[0] = 0
        else:
            self.fail('no termino el programa')
        self.assertEqual((registers[2], registers[1], depth), (32768, -1, 0))
        self.assertFalse(continuations)
        self.assertGreaterEqual(maximum, 2)
        # 3 llamadas a acumular, 8 a puntaje y 11 a sumar.
        self.assertEqual(calls, 22)
        base = 32768 - functions['main']['frame']
        values = {name: memory[base + offset] for name, offset in locals_main.items()}
        self.assertEqual(values, {'resultado': 24, 'vacio': 0, 'combinado': 13, 'total': 37})
        # Todas las ramas de puntaje se ejecutan con las entradas 1..5.
        self.assertEqual({i for name, i in visits if name == 'puntaje'},
                         set(range(len(functions['puntaje']['lines']))))


if __name__ == '__main__':
    unittest.main()
