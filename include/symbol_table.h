#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include <stddef.h>
#include <stdio.h>
#include <uthash.h>

//simbolos permitidos por el compilador

typedef enum{
    SIM_VARIABLE,
    SIM_CONSTANTE,
    SIM_FUNCION,
    SIM_PARAMETRO,
    SIM_LISTA,
    SIM_MODULO,
    SIM_ETIQUETA
} SimClase;

//tipos de variables definidas
typedef enum {
    SIM_TIPO_INT,
    SIM_TIPO_BOOLEAN,
    SIM_TIPO_TEXT,
    SIM_TIPO_CHAR,
    SIM_TIPO_VOID,
    SIM_TIPO_ERROR
} SimTipo;

//Clasificación de almacenamiento

typedef enum {
    SIM_ALMACEN_SIN_ASIGNAR,
    SIM_ALMACEN_GLOBAL,
    SIM_ALMACEN_LOCAL,
    SIM_ALMACEN_PARAMETRO,
    SIM_ALMACEN_CODIGO
} SimAlmacen;

//estructuras a utilizar
struct AstNodo;
typedef struct SimAmbito SimAmbito;
typedef struct SimSimbolo SimSimbolo;
typedef struct SimTabla SimTabla;

//Resultados posibles de las operaciones
typedef enum {
    SIM_OK = 0,
    SIM_DUPLICADO,
    SIM_SIN_MEMORIA,
    SIM_ARGUMENTO_INVALIDO
} SimResultado;

//Representa un simbolo del programa.
struct SimSimbolo {

    char *nombre;

    SimClase clase;
    SimTipo tipo;

    /*
    * Propiedades logicas del simbolo.
    * No son direcciones de memoria.
    */
    int es_parametro;

    //Arreglos y matrices
    unsigned dimensiones;
    size_t *longitudes;

    //Funciones 
    size_t cantidad_parametros;

    // Información de memoria  
    SimAlmacen almacen;
    long desplazamiento;
    size_t bytes;

    // Posición en el código fuente  
    size_t linea;
    size_t columna;

    /* Enlace prestado al identificador de declaracion. El AST debe vivir
     * durante la generacion; la tabla nunca libera estos nodos. */
    const struct AstNodo *declaracion;

    // Ámbito al que pertenece  
    SimAmbito *ambito;

    // Mantiene el orden global de inserción  
    SimSimbolo *siguiente_orden;

    //Estructura utilizada internamente por la biblioteca uthash.
      
    UT_hash_handle hh;
};


//Representa un ambito
// Cada ambito contiene su propia tabla hash.
// Por ambito entiendase el scope o alcance de una variable o constante

struct SimAmbito {
    /* Bloque AST que origino el ambito, prestado; NULL en tablas manuales. */
    const struct AstNodo *bloque;

    char *nombre;

    unsigned nivel;

    SimAmbito *padre;

    SimAmbito *siguiente_creacion;

    //Tabla hash del ámbito
    SimSimbolo *simbolos;
};

//Tabla de simbolos completa

struct SimTabla {

    SimAmbito *global;
    SimAmbito *actual;

    SimAmbito *ambitos_ultimo;

    SimSimbolo *primero;
    SimSimbolo *ultimo;

    size_t cantidad;
};

//Inicialización y liberacion 

SimResultado sim_inicializar(SimTabla *tabla);

void sim_liberar(SimTabla *tabla);

//Manejo de ambitos 

SimResultado sim_entrar_ambito(
    SimTabla *tabla,
    const char *nombre
);

int sim_salir_ambito(SimTabla *tabla);

SimAmbito *sim_ambito_actual(
    const SimTabla *tabla
);

//Búsqueda de simbolos 

SimSimbolo *sim_buscar_actual(
    const SimTabla *tabla,
    const char *nombre
);

SimSimbolo *sim_buscar(
    const SimTabla *tabla,
    const char *nombre
);

//Inserción de simbolos 

SimResultado sim_insertar(
    SimTabla *tabla,
    const char *nombre,
    SimClase clase,
    SimTipo tipo,
    size_t linea,
    size_t columna,
    SimSimbolo **creado
);

//Configuración de arreglos 

SimResultado sim_definir_dimensiones(
    SimSimbolo *simbolo,
    const size_t *longitudes,
    unsigned cantidad
);

//Información de memoria 

void sim_definir_ubicacion(
    SimSimbolo *simbolo,
    SimAlmacen almacen,
    long desplazamiento,
    size_t bytes
);

//Impresión de tabla 

void sim_imprimir(
    const SimTabla *tabla,
    FILE *salida
);

//Funciones auxiliares 

const char *sim_nombre_tipo(SimTipo tipo);

const char *sim_nombre_clase(SimClase clase);

/*
 * Comprueba la integridad de la tabla
 * antes de asignar memoria.
 *
 * Retorna:
 * 1: tabla consistente y sin memoria asignada.
 * 0: tabla invalida.
 */
int sim_verificar_pre_memoria(
    const SimTabla *tabla,
    FILE *diagnosticos
);

#endif
