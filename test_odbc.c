#include <stdio.h>
#include <stdlib.h>
#include <sql.h>
#include <sqlext.h>
#include "odbc.h"  // Debes tener tu odbc.h con las funciones que definimos

int main() {
    SQLHENV env;
    SQLHDBC dbc;

    /* Intentar conectar */
    if (odbc_connect(&env, &dbc) != SQL_SUCCESS) {
        fprintf(stderr, "Fallo la conexión a la base de datos\n");
        return 1;
    }

    /* Si llegamos aquí, conexión OK */
    printf("Conexión OK\n");

    /* Desconectar */
    odbc_disconnect(env, dbc);

    return 0;
}
