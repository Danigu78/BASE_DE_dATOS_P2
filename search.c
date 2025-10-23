/*
 * search.c — versión estable y sin violación de segmento
 */

#include "search.h"
#include "odbc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void results_search(char *from, char *to,   /*ahi que meter ahi el date**/
                    int *n_choices, char ***choices,  /*nchoices es el numero de filas que vamos a imprimir*/ /* ***Choices es el array en elq ue vamos a imprimir las cosas*/
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

    /* Estado inicial  */
    *n_choices = 0;
    *choices = NULL;

    /*  Validar parámetros de entrada */
    if (!from || !to || strlen(from) == 0 || strlen(to) == 0) {/*Si no tenemos nada metido en fronm ni en to nos da un error ya que faltan los argumentos*/
        *choices = malloc(sizeof(char *));/*Reserva memoria para una cadena, la de error*/
        if (!*choices) return;/*COMPROBAMOS QUE LA MEMORIA SE HA RESERVADO CORRECTAMEANTE*/
        (*choices)[0] = malloc(max_length); /*Reservamos la cadena y reservamos memoria para la cadena de texto de la longitud que nos pasan por argumento max_lenght */
        if (!(*choices)[0]) /*Comprobamos que la memoria se haya reservado correctamente*/
        return;/*Si no returneamos*/
        sprintf((*choices)[0], "Error: faltan aeropuertos origen/destino.", max_length - 1);/*Escribimos en la fila que hemos reservado erorr */
        (*choices)[0][max_length - 1] = '\0';/*Asegura que la cadena esté terminada con un carácter nulo ('\0').*/
        *n_choices = 1;/*Ponemos el numero de choices a 1*/
        return;
    }

    
    if (!SQL_SUCCEEDED(odbc_connect(&env, &dbc))) {/*Si no se conecta bien a la base de datos*/
        *choices = malloc(sizeof(char *)); /*Reserva memoria para una cadena, la de error*/
        (*choices)[0] = malloc(max_length);/*Reservamos la cadena y reservamos memoria para la cadena de texto de la longitud que nos pasan por argumento max_lenght */
        strncpy((*choices)[0], "Error al conectar con la base de datos.", max_length - 1);
        (*choices)[0][max_length - 1] = '\0';/*Me aseguro que el ultimo caracter sea el nulo*/
        *n_choices = 1; /*Ponemos el numero de choices a 1*/
        return;
    }


    /*Reservar espacio para resultados */
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
            *choices = NULL;
            *n_choices = 0;
            return;
        }
    }




    /* Construimos la consulta utilizando los aeropuertos que tenemos en from y to */
    snprintf(sql_query, sizeof(sql_query),
             "SELECT flight_no, scheduled_departure, scheduled_arrival, "
             "departure_airport, arrival_airport, aircraft_code, status "
             "FROM flights "
             "WHERE departure_airport = '%s' AND arrival_airport = '%s';",
             from, to);

    printf("Consulta SQL: %s\n", sql_query);
    fflush(stdout);


   /*Creamos el handle de la sentencia*/
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
