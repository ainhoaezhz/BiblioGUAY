#include "bd.h"
#include <stdio.h>

// Definición de la variable global (solo aquí)
sqlite3 *db = NULL;

int inicializarBD(sqlite3 **db) {
    return sqlite3_open(NOMBRE_BD, db);
}

void crearTablas(sqlite3 *db) {
    char *errMsg = 0;
    char sql[512];

    // Crear tabla Libro
    snprintf(sql, sizeof(sql),
            "CREATE TABLE IF NOT EXISTS Libro ("
            "nombre VARCHAR(100), autor VARCHAR(100), genero VARCHAR(50), "
            "id INTEGER PRIMARY KEY AUTOINCREMENT, estado INTEGER);");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al crear tabla Libro: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    // Crear tabla Usuario
    snprintf(sql, sizeof(sql),
            "CREATE TABLE IF NOT EXISTS Usuario ("
            "nombre VARCHAR(100), apellidos VARCHAR(100), dni VARCHAR(100) UNIQUE, "
            "direccion VARCHAR(100), email VARCHAR(100) UNIQUE, telefono VARCHAR(100), "
            "contrasena VARCHAR(100), es_Admin INTEGER);");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al crear tabla Usuario: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    // Crear tabla Prestamo
    snprintf(sql, sizeof(sql),
            "CREATE TABLE IF NOT EXISTS Prestamo("
            "id INTEGER PRIMARY KEY AUTOINCREMENT, usuario_dni INTEGER, libro_id INTEGER,"
            "fecha_Prestamo DATE, fecha_Devolucion DATE, "
            "FOREIGN KEY (usuario_dni) REFERENCES Usuario(dni),"
            "FOREIGN KEY (libro_id) REFERENCES Libro(id));");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al crear tabla Prestamo: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    // Insertar usuario admin si no existe
    snprintf(sql, sizeof(sql),
            "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
            "SELECT 'admin', 'admin', '00000000A', 'admin', 'admin@admin.com', '000000000', 'admin', 1 "
            "WHERE NOT EXISTS (SELECT 1 FROM Usuario WHERE dni = '00000000A');");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al insertar admin: %s\n", errMsg);
        sqlite3_free(errMsg);
    }
}

void cerrarBD(sqlite3 *db) {
    if (db) {
        sqlite3_close(db);
    }
}

int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT 1 FROM Usuario WHERE nombre = ? AND contrasena = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contrasena, -1, SQLITE_STATIC);

    int resultado = (sqlite3_step(stmt) == SQLITE_ROW);
    sqlite3_finalize(stmt);

    return resultado;
}
