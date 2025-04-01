#ifndef MENU_ADMIN_H
#define MENU_ADMIN_H

#include "sqlite3.h"

char menuAdministrador();
void ejecutarMenuAdmin(sqlite3 *db);

// Funciones específicas de administrador
void gestionarUsuarios(sqlite3 *db);
void gestionarLibros(sqlite3 *db);
void verEstadisticas(sqlite3 *db);
void mostrarUsuarioConMasPrestamos(sqlite3 *db);
void mostrarLibroMasPrestado(sqlite3 *db);
void gestionarPrestamosDevoluciones(sqlite3 *db);

#endif
