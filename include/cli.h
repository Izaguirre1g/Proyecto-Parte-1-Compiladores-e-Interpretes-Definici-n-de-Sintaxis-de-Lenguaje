#ifndef JAVIER_CLI_H
#define JAVIER_CLI_H

/* Compilador segun la especificacion: siempre genera el binario y las
 * opciones -o, -v, -s, -t, -m, -x y -p agregan salidas. */
int cli_run(int argc, char **argv);

/* Modo interno de pruebas del frontend (balc --frontend [-v] [-t] fuente):
 * solo lexico, -v lista los tokens y -t valida sintaxis e imprime el AST.
 * No genera archivos. Lo usan test_parser y test_cli. */
int cli_frontend(int argc, char **argv);

#endif
