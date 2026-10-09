#include "cli.h"  // Permite utilizar la función cli_run.

// Punto de entrada del compilador.
int main(int argc, char **argv) {

    // argc indica cuántos argumentos recibimos.
    // argv contiene los argumentos de la terminal.

    // Pasamos los argumentos a cli_run, que procesa los comandos.
    // Devolvemos el resultado de esa función.
    return cli_run(argc, argv);
}
