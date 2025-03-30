#ifndef MENU_H_
#define MENU_H_

#include "sqlite3.h"

#define MAX 80
#define MAX_STR 100

// Declaraciones de funciones
void leerContrasena(char *password);
char menuPrincipal();
char menuRegistro();
char menuUsuario();
void iniciarSesion();
void menuUsuarioNormal(sqlite3 *db, const char *usuario);

#endif /* MENU_H_ */
