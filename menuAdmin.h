#ifndef MENU_ADMIN_H
#define MENU_ADMIN_H

#include "sqlite3.h"

char menuAdministrador();
void ejecutarMenuAdmin(sqlite3 *db);

// Submenús
void gestionarInventarioLibros(sqlite3 *db);
void gestionarUsuarios(sqlite3 *db);
void gestionarPrestamosDevoluciones(sqlite3 *db);
void generarInformes(sqlite3 *db);

// Funciones específicas
void agregarLibro(sqlite3 *db);
void editarLibro(sqlite3 *db);
void eliminarLibro(sqlite3 *db);
void registrarNuevoUsuario(sqlite3 *db);
void editarDatosUsuario(sqlite3 *db);
void eliminarUsuario(sqlite3 *db);
void registrarPrestamo(sqlite3 *db);
void registrarDevolucion(sqlite3 *db);
void mostrar_prestamos_activos(sqlite3 *db);
void verEstadisticas(sqlite3 *db);
//void listarLibrosDisponibles(sqlite3 *db);
void mostrarUsuarioConMasPrestamos(sqlite3 *db);
void mostrarLibroMasPrestado(sqlite3 *db);
void mostrar_prestamos_vencidos(sqlite3 *db);
void listarLibros(sqlite3 *db);

#endif
