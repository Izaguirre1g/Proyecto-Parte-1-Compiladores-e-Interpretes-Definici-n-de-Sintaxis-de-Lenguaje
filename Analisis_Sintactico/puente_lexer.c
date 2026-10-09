/* Conecta el lexer propio del proyecto con el parser generado por Bison.
 * Traduce tokens, entrega su ubicación y crea las hojas del AST.
 * Este archivo se escribe a mano; no lo genera Bison.
 */
#include "parser.h" /* Tokens de Bison, YYSTYPE, YYLTYPE y contexto del parser. */

/* Traducción explícita: las enumeraciones no tienen los mismos números.
 * Cada case recibe un TOKEN_* del lexer y devuelve el equivalente de Bison.
 * static hace que esta función solo sea accesible desde este archivo.
 */
static int token_bison(TokenType tipo) {
    /* Selecciona la traducción según la categoría reconocida por el lexer. */
    switch (tipo) {
        case TOKEN_EOF: return YYEOF; /* Valor 0: indica fin de la entrada. */
        /* Nombres y literales que pueden aportar texto al AST. */
        case TOKEN_IDENTIFIER: return IDENTIFIER;
        case TOKEN_INTEGER: return INTEGER;
        case TOKEN_STRING: return STRING;
        case TOKEN_CHARACTER: return CHARACTER;
        /* Palabras reservadas para tipos básicos y valores booleanos. */
        case TOKEN_DECLARE_INT: return DECLARE_INT;
        case TOKEN_DECLARE_BOOLEAN: return DECLARE_BOOLEAN;
        case TOKEN_DECLARE_TEXT: return DECLARE_TEXT;
        case TOKEN_DECLARE_CHAR: return DECLARE_CHAR;
        case TOKEN_DECLARE_TRUE: return DECLARE_TRUE;
        case TOKEN_DECLARE_FALSE: return DECLARE_FALSE;
        /* Signos de declaración, asignación y separación. */
        case TOKEN_STAR: return STAR;
        case TOKEN_ASSIGN: return ASSIGN;
        case TOKEN_SEMICOLON: return SEMICOLON;
        case TOKEN_COMMA: return COMMA;
        /* Operadores aritméticos: suma, resta, multiplicación y división. */
        case TOKEN_GAUSS: return GAUSS;
        case TOKEN_NEUMANN: return NEUMANN;
        case TOKEN_PITAGORAS: return PITAGORAS;
        case TOKEN_EUCLIDES: return EUCLIDES;
        /* Paréntesis para expresiones/llamadas y llaves para bloques. */
        case TOKEN_LPAREN: return LPAREN;
        case TOKEN_RPAREN: return RPAREN;
        case TOKEN_LBRACE: return LBRACE;
        case TOKEN_RBRACE: return RBRACE;
        /* Condicionales, ciclo whale, su cierre stop y retorno give. */
        case TOKEN_WHETHER: return WHETHER;
        case TOKEN_ALIF: return ALIF;
        case TOKEN_ALSO: return ALSO;
        case TOKEN_WHALE: return WHALE;
        case TOKEN_STOP: return STOP;
        case TOKEN_GIVE: return GIVE;
        /* Elementos usados en las declaraciones de funciones. */
        case TOKEN_CREATE_FUNK: return CREATE_FUNK;
        case TOKEN_HASH: return HASH;
        case TOKEN_MAIN: return MAIN;
        case TOKEN_DECLARE_INFINITE_VOID: return DECLARE_INFINITE_VOID;
        /* Comparaciones: igualdad, desigualdad y relaciones de orden. */
        case TOKEN_EQUAL: return EQUAL;
        case TOKEN_NOT_EQUAL: return NOT_EQUAL;
        case TOKEN_LESS: return LESS;
        case TOKEN_GREATER: return GREATER;
        case TOKEN_LESS_EQUAL: return LESS_EQUAL;
        case TOKEN_GREATER_EQUAL: return GREATER_EQUAL;
        /* Residuo, potencia y operadores lógicos. */
        case TOKEN_EULER: return EULER;
        case TOKEN_DESCARTES: return DESCARTES;
        case TOKEN_AND: return AND;
        case TOKEN_OR: return OR;
        case TOKEN_NOT: return NOT;
        case TOKEN_XOR: return XOR;
        /* Palabras que forman el ciclo cycle, con límites y paso opcional. */
        case TOKEN_CYCLE: return CYCLE;
        case TOKEN_LET: return LET;
        case TOKEN_UNTIL: return UNTIL;
        case TOKEN_STEP: return STEP;
        case TOKEN_ENDGAME: return ENDGAME;
        /* Corchetes para colecciones/índices y punto para acceso a miembros. */
        case TOKEN_LBRACKET: return LBRACKET;
        case TOKEN_RBRACKET: return RBRACKET;
        case TOKEN_DOT: return DOT;
        /* Declaración de listas y operaciones sobre ellas. */
        case TOKEN_DECLARE_LIST: return DECLARE_LIST;
        case TOKEN_ADD: return ADD;
        case TOKEN_REMOVE: return REMOVE;
        case TOKEN_SIZE: return SIZE;
        /* Importaciones, alias, constantes y manejo de excepciones. */
        case TOKEN_BRING: return BRING;
        case TOKEN_AKA: return AKA;
        case TOKEN_DECLARE_CONST: return DECLARE_CONST;
        case TOKEN_SEEK: return SEEK;
        case TOKEN_SEIZE: return SEIZE;
        default: return YYUNDEF; /* No existe una traducción para este token. */
    }
}

/* Solo los tokens con datos crean hojas. Puntuacion y operadores se usan
 * en las reglas de Bison; no se guardan como nodos independientes aquí.
 * Devuelve 1 y escribe en *tipo si corresponde crear una hoja; si no, devuelve 0.
 */
static int tipo_hoja(TokenType token, AstTipo *tipo) {
    switch (token) {
        /* Tanto un nombre común como main se representan como identificadores. */
        case TOKEN_IDENTIFIER: case TOKEN_MAIN: *tipo = AST_IDENTIFICADOR; return 1;
        /* Cada clase de literal recibe su categoría de nodo correspondiente. */
        case TOKEN_INTEGER: *tipo = AST_ENTERO; return 1;
        case TOKEN_STRING: *tipo = AST_CADENA; return 1;
        case TOKEN_CHARACTER: *tipo = AST_CARACTER; return 1;
        case TOKEN_DECLARE_TRUE: case TOKEN_DECLARE_FALSE: *tipo = AST_BOOLEANO; return 1;
        /* Estos case comparten la misma acción: todos representan tipos de datos. */
        case TOKEN_DECLARE_INT: case TOKEN_DECLARE_BOOLEAN:
        case TOKEN_DECLARE_TEXT: case TOKEN_DECLARE_CHAR:
        case TOKEN_DECLARE_INFINITE_VOID: *tipo = AST_TIPO; return 1;
        default: return 0; /* No crea hoja ni asigna un valor a *tipo. */
    }
}

/* Bison llama a esta función cada vez que necesita otro token.
 * valor recibe el nodo asociado (YYSTYPE es un puntero a AstNodo).
 * ubicacion recibe la posición (YYLTYPE corresponde a AstUbicacion).
 * ctx comparte el lexer, el árbol, los diagnósticos y el estado del análisis.
 */
int yylex(YYSTYPE *valor, YYLTYPE *ubicacion, ContextoSintactico *ctx) {
    /* Por defecto, el token no tiene un nodo asociado. */
    *valor = NULL;
    /* Lee el siguiente token y avanza la posición interna del lexer. */
    Token token = lexer_next(&ctx->lexer);
    /* Guarda en el contexto la línea y columna donde empieza ese token. */
    ctx->linea = token.line;
    ctx->columna = token.column;
    /* El inicio viene del token; el final es la posición del lexer tras leerlo. */
    *ubicacion = (AstUbicacion){token.line, token.column,
                               ctx->lexer.line, ctx->lexer.column};
    int tipo = token_bison(token.type); /* Convierte la categoría para Bison. */
    /* Primero atiende los errores detectados por el análisis léxico. */
    if (token.type == TOKEN_ERROR) {
        /* Estado 2 para falta de memoria; estado 1 para los demás errores léxicos. */
        ctx->estado = token.error == TOKEN_OUT_OF_MEMORY ? 2 : 1;
        /* Informa archivo, línea, columna y descripción del error. */
        fprintf(ctx->diagnosticos, "%s:%zu:%zu: error léxico: %s\n",
                ctx->lexer.filename, ctx->linea, ctx->columna,
                token_error_message(token.error));
        tipo = YYerror; /* Avisa a Bison sin pedirle otro diagnóstico del mismo error. */
    } else if (tipo == YYUNDEF) {
        /* El lexer reconoció el token, pero el puente no tiene traducción para él. */
        ctx->estado = 1;
        /* Muestra el nombre de ese token junto a su ubicación. */
        fprintf(ctx->diagnosticos,
                "%s:%zu:%zu: token %s aún no admitido por la gramática\n",
                ctx->lexer.filename, ctx->linea, ctx->columna,
                token_type_name(token.type));
        tipo = YYerror; /* Comunica al parser el error ya informado. */
    } else {
        /* Guarda la categoría AST que tipo_hoja asignará si corresponde. */
        AstTipo hoja;
        /* Solo identificadores, literales y tipos crean una hoja en este punto. */
        if (tipo_hoja(token.type, &hoja)) {
            /* Copia el lexema en un nodo y lo entrega como valor semántico a Bison. */
            *valor = ast_crear(ctx->arbol, hoja, token.lexeme, *ubicacion);
            /* Un puntero NULL significa que no se pudo reservar memoria. */
            if (!*valor) {
                /* Conserva el código de fallo de memoria en el contexto. */
                ctx->estado = 2;
                /* Escribe el diagnóstico en el flujo indicado por el llamador. */
                fprintf(ctx->diagnosticos, "%s:%zu:%zu: memoria insuficiente para el AST\n",
                        ctx->lexer.filename, ctx->linea, ctx->columna);
                tipo = YYerror; /* Hace que Bison reciba una señal de error. */
            }
        }
    }
    /* Libera los recursos del token. Si creó una hoja, ast_crear ya copió el lexema. */
    token_dispose(&token);
    /* Devuelve el código de token o la señal de error/fin que Bison espera. */
    return tipo;
}

/* Bison llama a esta funcion cuando detecta un error de sintaxis o memoria. */
void yyerror(YYLTYPE *ubicacion, ContextoSintactico *ctx, const char *mensaje) {
    /* Imprime el mensaje de Bison con el archivo y la posición del error. */
    fprintf(ctx->diagnosticos, "%s:%zu:%zu: error del parser: %s\n",
            ctx->lexer.filename, ubicacion->first_line, ubicacion->first_column, mensaje);
    /* Marca un error general sin sobrescribir un fallo de memoria ya registrado. */
    if (ctx->estado != 2) ctx->estado = 1;
}

/* Analiza longitud bytes de fuente y construye el AST en arbol.
 * nombre identifica el archivo en los mensajes; diagnosticos recibe los errores.
 * El llamador debe entregar un Ast vacío, inicializado con {0}.
 * Devuelve 0 si todo salió bien, 1 si hay errores o 2 si falta memoria.
 */
int construir_ast(const char *fuente, size_t longitud, const char *nombre,
                  FILE *diagnosticos, Ast *arbol) {
    /* Inicializa el contexto con ceros y punteros nulos. */
    ContextoSintactico ctx = {0};
    /* Prepara el lexer para leer el texto recibido. */
    lexer_init(&ctx.lexer, fuente, longitud, nombre);
    /* Conecta el contexto con el árbol donde se guardarán los nodos. */
    ctx.arbol = arbol;
    /* Guarda el flujo de mensajes, normalmente stderr. */
    ctx.diagnosticos = diagnosticos;
    /* Las posiciones del código se cuentan desde 1. */
    ctx.linea = ctx.columna = 1;
    /* Ejecuta el parser generado por Bison, que irá llamando a yylex. */
    int resultado = yyparse(&ctx);
    /* Combina el resultado del parser y el estado del puente.
     * Prioriza falta de memoria (2), luego cualquier error (1), o éxito (0).
     */
    int estado = resultado == 2 || ctx.estado == 2 ? 2 :
                 resultado != 0 || ctx.estado != 0 ? 1 : 0;
    /* Incluye nodos parciales y hojas que Bison descarto al abortar. */
    if (estado != 0) ast_liberar(arbol);
    /* En éxito conserva el árbol para que el llamador lo use y luego lo libere. */
    return estado;
}

/* Conserva la interfaz anterior para quien solo necesite validar. */
int analizar_sintaxis(const char *fuente, size_t longitud,
                     const char *nombre, FILE *diagnosticos) {
    /* Crea un contenedor temporal vacío para el AST. */
    Ast arbol = {0};
    /* Realiza el mismo análisis que construir_ast y conserva su resultado. */
    int estado = construir_ast(fuente, longitud, nombre, diagnosticos, &arbol);
    /* Esta interfaz solo valida: descarta el árbol incluso si el análisis fue válido. */
    ast_liberar(&arbol);
    /* Entrega el código de éxito o error al llamador. */
    return estado;
}
