#ifndef MENU_H_
#define MENU_H_

#include "sqlite3.h"
#include "usuario.h"

// Declaración de funciones
char menuPrincipal();
void menuRegistro(sqlite3 *db);
void iniciarSesion(sqlite3 *db);
int registrarse(sqlite3 *db, int esAdmin);
char menuUsuario();
void leerContrasena(char *password);
int validarDNI(const char *dni);

#endif /* MENU_H_ */
