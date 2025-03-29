/*
 * usuario.h
 *
 *  Created on: 11 mar 2025
 *      Author: ainhoa
 */

 #ifndef USUARIO_H_
 #define USUARIO_H_
 #include "sqlite3.h"
 
 #define MAX_STR 100
 
 typedef struct {
     char nombre[MAX_STR];
     char apellidos[MAX_STR];
     char dni[MAX_STR];
     char direccion[MAX_STR];
     char email[MAX_STR];
     char telefono[MAX_STR];
     char contrasena[MAX_STR];  // Agregado para almacenar la contraseña
     int es_Admin; //0=NO 1=SI
 } Usuario;
 
 void inicializarUsuario(Usuario *u, const char *nombre, const char *apellidos,
                          const char *dni, const char *direccion, const char *email,
                          const char *telefono, const char *contrasena, int es_Admin);

 void mostrarUsuario(sqlite3 *db, const char *nombreUsuario);
 void editarUsuario(sqlite3 *db, char *nombreUsuario);
 
 
 
 #endif /* USUARIO_H_ */
