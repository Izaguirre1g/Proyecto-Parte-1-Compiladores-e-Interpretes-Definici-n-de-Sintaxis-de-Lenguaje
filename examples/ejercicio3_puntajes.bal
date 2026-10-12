%% Ejercicio 3: sumar puntajes con funciones y control anidado.
create_funk #declare_int# sumar(a * declare_int, b * declare_int) {
    give a gauss b;
}

create_funk #declare_int# puntaje(n * declare_int) {
    whether (n < 3) {
        give n gauss 1;
    }
    also {
        whether (n < 5) {
            give n gauss 2;
        }
        also {
            give n gauss 3;
        }
    }
}

create_funk #declare_int# acumular(limite * declare_int) {
    i * declare_int : 1;
    total * declare_int : 0;
    whale (i < limite gauss 1) {
        actual * declare_int : puntaje(i);
        total : sumar(total, actual);
        i : i gauss 1;
    } stop;
    give total;
}

create_funk #declare_infinite_void# main() {
    resultado * declare_int : acumular(5);
    vacio * declare_int : acumular(0);
    combinado * declare_int : sumar(acumular(3), sumar(1, 2));
    total * declare_int : sumar(resultado, combinado);
}
