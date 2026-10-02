%% Demostracion del parser y AST: no se ejecuta ni se cargan modulos todavia.
bring matematicas aka math;
declare_const LIMITE * declare_int : 5;

create_funk #declare_int# leer(m * declare_int[][], fila * declare_int, columna * declare_int) {
    give m[fila][columna];
}

create_funk #declare_int# contar(xs * declare_list declare_int) {
    give size(xs);
}

create_funk #declare_infinite_void# main() {
    resultado * declare_int;
    valido * declare_boolean : declare_false;
    matriz * declare_int[2][3] : [[1, 2, 3], [4, 5, 6]];
    copia * declare_int[2][3];
    declare_list numeros * declare_int : [10, 20], vacia * declare_int : [];

    cycle i let 0 until 2 {
        cycle j let 0 until 3 {
            copia[i][j] : matriz[i][j];
        } endgame;
    } endgame;

    cycle i let 0 until LIMITE step 2 {
        add(numeros, i descartes 2 euler 3);
    } endgame;
    remove(numeros, 0);
    resultado : math.suma(leer(copia, 1, 2), contar(numeros));

    whether (resultado >= 0 && ~(size(numeros) == 0)) {
        valido : declare_true;
    } alif (resultado == 0 || declare_false ^ declare_true) {
        valido : declare_false;
    } also {
        resultado : neumann 1;
    }

    seek {
        resultado : math.suma(resultado, 1);
    } seize (ErrorAritmetico e) {
        valido : declare_false;
    }
}
