#ifndef MENU_H_
#define MENU_H_

#include "sqlite3.h"

char menuPrincipal();
void iniciarSesion();
void registrarse(sqlite3 *db);
char menuUsuario();

int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena);
int autenticarUsuario(sqlite3 *db, char *dni, int *esAdmin);
void registrar_prestamo_nuevo(sqlite3 *db);
void devolver_libro(sqlite3 *db, const char *nombre_usuario);

void menu_alquiler();
#endif
