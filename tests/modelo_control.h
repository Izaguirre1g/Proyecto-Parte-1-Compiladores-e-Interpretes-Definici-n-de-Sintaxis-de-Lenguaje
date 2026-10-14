#ifndef TEST_MODELO_CONTROL_H
#define TEST_MODELO_CONTROL_H
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* Modelo secuencial de las instrucciones usadas por estos casos.
 * Comprueba que se ejecuta la rama correcta y que se restaura sp/ra.
 * No modela bundles, latencias ni reemplaza la simulacion del procesador. */
static int ejecutar_control(char *texto)
{
    char *lineas[256];
    size_t cantidad = 0;
    for (char *p = strtok(texto, "\n"); p; p = strtok(NULL, "\n")) {
        assert(cantidad < 256);
        lineas[cantidad++] = p;
    }
    int registros[32] = {0}, memoria[256] = {0};
    registros[2] = 512;
    registros[1] = 1232;
    for (size_t pc = 0, pasos = 0; pc < cantidad && pasos < 1000; ++pc, ++pasos) {
        char *linea = lineas[pc];
        unsigned a, b, d;
        int inmediato;
        char etiqueta[128];
        if (linea[strlen(linea) - 1] == ':') continue;
        if (sscanf(linea, "sumai x%u, x%u, %d", &a, &b, &inmediato) == 3)
            registros[a] = registros[b] + inmediato;
        else if (sscanf(linea, "restai x%u, x%u, %d", &a, &b, &inmediato) == 3)
            registros[a] = registros[b] - inmediato;
        else if (sscanf(linea, "suma x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] + registros[d];
        else if (sscanf(linea, "resta x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] - registros[d];
        else if (sscanf(linea, "mrq x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] < registros[d];
        else if (sscanf(linea, "myq x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] > registros[d];
        else if (sscanf(linea, "and x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] & registros[d];
        else if (sscanf(linea, "xor x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] ^ registros[d];
        else if (sscanf(linea, "or x%u, x%u, x%u", &a, &b, &d) == 3)
            registros[a] = registros[b] | registros[d];
        else if (sscanf(linea, "xori x%u, x%u, %d", &a, &b, &inmediato) == 3)
            registros[a] = registros[b] ^ inmediato;
        else if (sscanf(linea, "guardap x%u, x%u, %d", &a, &b, &inmediato) == 3) {
            int direccion = registros[a] + inmediato;
            assert(direccion >= 0 && direccion < 1024 && direccion % 4 == 0);
            memoria[direccion / 4] = registros[b];
        } else if (sscanf(linea, "cargai x%u, x%u, %d", &a, &b, &inmediato) == 3) {
            int direccion = registros[b] + inmediato;
            assert(direccion >= 0 && direccion < 1024 && direccion % 4 == 0);
            registros[a] = memoria[direccion / 4];
        } else if (!strcmp(linea, "syer x0, x1, 0")) {
            assert(registros[2] == 512 && registros[1] == 1232);
            return registros[8];
        } else {
            int saltar = 0;
            if (sscanf(linea, "igualsi x%u, x%u, %127s", &a, &b, etiqueta) == 3)
                saltar = registros[a] == registros[b];
            else {
                assert(sscanf(linea, "sye x0, %127s", etiqueta) == 1);
                saltar = 1;
            }
            if (saltar) {
                size_t destino;
                for (destino = 0; destino < cantidad; ++destino)
                    if (strlen(lineas[destino]) == strlen(etiqueta) + 1 &&
                        !strncmp(lineas[destino], etiqueta, strlen(etiqueta)) &&
                        lineas[destino][strlen(etiqueta)] == ':') break;
                assert(destino < cantidad);
                pc = destino;
            }
        }
        registros[0] = 0;
    }
    assert(!"la funcion no retorno");
    return -1;
}

#endif
