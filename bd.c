#include "bd.h"
#include <stdio.h>

int inicializarBD(sqlite3 **db) {
	int result;

	result = sqlite3_open(NOMBRE_BD, db);
	return result;
}

void creartablas(sqlite3 **db) {
	sqlite3_stmt *stmt;
	    char sql[256];  // Tamaño suficiente para almacenar cada consulta

	    // Crear tabla Libro
	    snprintf(sql, sizeof(sql),
	             "CREATE TABLE IF NOT EXISTS Libro ("
	             "nombre VARCHAR(100), "
	             "autor VARCHAR(100), "
	             "genero VARCHAR(50), "
	             "id INTEGER PRIMARY KEY AUTOINCREMENT, "
	             "estado INTEGER);");

	    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
	        sqlite3_step(stmt);
	    }
	    sqlite3_finalize(stmt);

	    // Crear tabla Usuario
	    snprintf(sql, sizeof(sql),
	             "CREATE TABLE IF NOT EXISTS Usuario ("
	             "nombre VARCHAR(100), "
	             "apellidos VARCHAR(100), "
	             "dni VARCHAR(100) UNIQUE, "
	             "direccion VARCHAR(100), "
	             "email VARCHAR(100) UNIQUE, "
	             "telefono VARCHAR(100), "
	             "contrasena VARCHAR(100), "
	             "es_Admin INTEGER);");

	    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
	        sqlite3_step(stmt);
	    }
	    sqlite3_finalize(stmt);

}
