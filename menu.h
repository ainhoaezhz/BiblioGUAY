/*
 * menu.h
 *
 *  Created on: 5 mar 2025
 *      Author: Genesis
 */

#ifndef MENU_H_
#define MENU_H_

#include "sqlite3.h"  // Incluye el encabezado de SQLite3

char menuPrincipal();
char menuRegistro();
void iniciarSesion();
void registrarse(sqlite3 *db);
char menuUsuario();
void leerContrasena(char *password);
int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena);
int autenticarUsuario(sqlite3 *db, char *dni, int *esAdmin);

#endif /* MENU_H_ */
