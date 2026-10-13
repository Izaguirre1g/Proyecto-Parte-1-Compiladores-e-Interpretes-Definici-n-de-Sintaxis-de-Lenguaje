%% Resultado esperado: suma = 6 (0, 2, 4), cuenta = 3 (3, 2, 1) y pares = 3.
create_funk #declare_infinite_void# main() {
    suma * declare_int : 0;
    cuenta * declare_int : 0;
    pares * declare_int : 0;
    cycle i let 0 until 5 step 2 {
        suma : suma gauss i;
    } endgame;
    cycle i let 3 until 0 step neumann 1 {
        cuenta : cuenta gauss 1;
    } endgame;
    cycle i let 0 until 3 {
        cycle j let 0 until 3 {
            whether (i < j) {
                pares : pares gauss 1;
            }
        } endgame;
    } endgame;
}
