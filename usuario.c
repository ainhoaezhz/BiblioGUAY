/*
 * usuario.c
 *
 *  Created on: 11 mar 2025
 *      Author: ainhoa
 */

#include "usuario.h"
#include <stdio.h>
#include <string.h>

void inicializarUsuario(Usuario *u, const char *nombre, const char *apellidos,
                         const char *dni, const char *direccion, const char *email,
                         const char *telefono, const char *contrasena, int es_Admin) {
    strncpy(u->nombre, nombre, MAX_STR);
    strncpy(u->apellidos, apellidos, MAX_STR);
    strncpy(u->dni, dni, MAX_STR);
    strncpy(u->direccion, direccion, MAX_STR);
    strncpy(u->email, email, MAX_STR);
    strncpy(u->telefono, telefono, MAX_STR);
    strncpy(u->contrasena, contrasena, MAX_STR);
    u->es_Admin = es_Admin;
}


void mostrarUsuario(const Usuario *u) {
    printf("Nombre: %s\n", u->nombre);
    printf("Apellidos: %s\n", u->apellidos);
    printf("DNI: %s\n", u->dni);
    printf("Dirección: %s\n", u->direccion);
    printf("Email: %s\n", u->email);
    printf("Teléfono: %s\n", u->telefono);
    printf("Administrador: %s\n", u->es_Admin ? "Sí" : "No");
}
