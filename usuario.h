/*
 * usuario.h
 *
 *  Created on: 11 mar 2025
 *      Author: ainhoa
 */

 #ifndef USUARIO_H_
 #define USUARIO_H_
 
 #define MAX_STR 100
 
 typedef struct {
     char nombre[MAX_STR];
     char apellidos[MAX_STR];
     char dni[MAX_STR];
     char direccion[MAX_STR];
     char email[MAX_STR];
     char telefono[MAX_STR];
 } Usuario;
 
 void inicializarUsuario(Usuario *u, const char *nombre, const char
 *apellidos, const char *dni, const char *direccion, const char *email,
 const char *telefono);
 void mostrarUsuario(const Usuario *u);
 
 
 
 
 #endif /* USUARIO_H_ */