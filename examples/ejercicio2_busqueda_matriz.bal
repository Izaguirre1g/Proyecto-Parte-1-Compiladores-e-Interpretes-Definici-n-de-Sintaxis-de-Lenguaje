%% Ejercicio 2: buscar la primera aparicion de objetivo en una matriz 2x3.
%% Al terminar: fila_encontrada, columna_encontrada = (-1, -1) si no existe.
create_funk #declare_infinite_void# main() {
    matriz * declare_int[2][3] : [[4, 7, 1], [9, 5, 7]];
    objetivo * declare_int : 7;
    fila * declare_int : 0;
    columna * declare_int : 0;
    fila_encontrada * declare_int : neumann 1;
    columna_encontrada * declare_int : neumann 1;

    whale (fila < 2) {
        columna : 0;
        whale (columna < 3) {
            whether (matriz[fila][columna] == objetivo && fila_encontrada == neumann 1) {
                fila_encontrada : fila;
                columna_encontrada : columna;
            }
            columna : columna gauss 1;
        } stop;
        fila : fila gauss 1;
    } stop;
}
