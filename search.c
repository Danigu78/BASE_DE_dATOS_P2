/*
 * search.c — versión estable y sin violación de segmento
 */

#include "search.h"
#include "odbc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void results_search(char *from, char *to,
                    int *n_choices, char ***choices,
                    int max_length,
                    int max_rows)
{
    SQLHENV env = SQL_NULL_HENV;
    SQLHDBC dbc = SQL_NULL_HDBC;
    SQLHSTMT stmt = SQL_NULL_HSTMT;
    SQLRETURN ret;

    char sql_query[512];
    char flight_no[32], sched_dep[64], sched_arr[64];
    char dep[32], arr[32], acode[16], status[32];
    int i = 0;

    /* Estado inicial seguro */
    *n_choices = 0;
    *choices = NULL;

    /*  Validar parámetros de entrada */
    if (!from || !to || strlen(from) == 0 || strlen(to) == 0) {
        *choices = malloc(sizeof(char *));
        if (!*choices) return;
        (*choices)[0] = malloc(max_length);
        if (!(*choices)[0]) return;
        strncpy((*choices)[0], "Error: faltan aeropuertos origen/destino.", max_length - 1);
        (*choices)[0][max_length - 1] = '\0';
        *n_choices = 1;
        return;
    }

    /* 🚀 Conectar con la BD */
    if (!SQL_SUCCEEDED(odbc_connect(&env, &dbc))) {
        *choices = malloc(sizeof(char *));
        (*choices)[0] = malloc(max_length);
        strncpy((*choices)[0], "Error al conectar con la base de datos.", max_length - 1);
        (*choices)[0][max_length - 1] = '\0';
        *n_choices = 1;
        return;
    }

    /* 🔹 Reservar espacio para resultados */
    *choices = malloc(max_rows * sizeof(char *));
    if (!*choices) {
        odbc_disconnect(env, dbc);
        return;
    }
    for (i = 0; i < max_rows; i++) {
        (*choices)[i] = calloc(max_length, sizeof(char));
        if (!(*choices)[i]) {
            for (int j = 0; j < i; j++) free((*choices)[j]);
            free(*choices);
            odbc_disconnect(env, dbc);
            return;
        }
    }

    /* 🧠 Crear consulta SQL */
    snprintf(sql_query, sizeof(sql_query),
             "SELECT flight_no, scheduled_departure, scheduled_arrival, "
             "departure_airport, arrival_airport, aircraft_code, status "
             "FROM flights "
             "WHERE departure_airport = '%s' AND arrival_airport = '%s';",
             from, to);

    printf("Consulta SQL: %s\n", sql_query);
    fflush(stdout);

    /* 🧱 Crear y ejecutar sentencia */
    ret = SQLAllocHandle(SQL_HANDLE_STMT, dbc, &stmt);
    if (!SQL_SUCCEEDED(ret)) {
        strncpy((*choices)[0], "Error al crear handle de sentencia.", max_length - 1);
        (*choices)[0][max_length - 1] = '\0';
        *n_choices = 1;
        odbc_disconnect(env, dbc);
        return;
    }

    ret = SQLExecDirect(stmt, (SQLCHAR *)sql_query, SQL_NTS);
    if (!SQL_SUCCEEDED(ret)) {
        strncpy((*choices)[0], "Error al ejecutar la consulta.", max_length - 1);
        (*choices)[0][max_length - 1] = '\0';
        *n_choices = 1;
        SQLFreeHandle(SQL_HANDLE_STMT, stmt);
        odbc_disconnect(env, dbc);
        return;
    }

    /* 🧩 Asociar columnas */
    SQLBindCol(stmt, 1, SQL_C_CHAR, flight_no, sizeof(flight_no), NULL);
    SQLBindCol(stmt, 2, SQL_C_CHAR, sched_dep, sizeof(sched_dep), NULL);
    SQLBindCol(stmt, 3, SQL_C_CHAR, sched_arr, sizeof(sched_arr), NULL);
    SQLBindCol(stmt, 4, SQL_C_CHAR, dep, sizeof(dep), NULL);
    SQLBindCol(stmt, 5, SQL_C_CHAR, arr, sizeof(arr), NULL);
    SQLBindCol(stmt, 6, SQL_C_CHAR, acode, sizeof(acode), NULL);
    SQLBindCol(stmt, 7, SQL_C_CHAR, status, sizeof(status), NULL);

    /* 🔄 Leer resultados */
    i = 0;
    while (SQL_SUCCEEDED(SQLFetch(stmt)) && i < max_rows) {
        snprintf((*choices)[i], max_length,
                 "%s | %s -> %s | %s -> %s | %s | %s",
                 flight_no, sched_dep, sched_arr, dep, arr, acode, status);
        i++;
    }

    /* Si no hay resultados */
    if (i == 0) {
        strncpy((*choices)[0], "No se encontraron vuelos entre esos aeropuertos.", max_length - 1);
        (*choices)[0][max_length - 1] = '\0';
        *n_choices = 1;
    } else {
        *n_choices = i;
    }

    /* 🔚 Limpieza segura */
    if (stmt != SQL_NULL_HSTMT)
    SQLFreeHandle(SQL_HANDLE_STMT, stmt);
    odbc_disconnect(env, dbc);
}
