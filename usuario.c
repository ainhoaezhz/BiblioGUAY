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
	printf("\n--- TU PERFIL ---\n");
    printf("Nombre: %s\n", u->nombre);
    printf("Apellidos: %s\n", u->apellidos);
    printf("DNI: %s\n", u->dni);
    printf("Dirección: %s\n", u->direccion);
    printf("Email: %s\n", u->email);
    printf("Teléfono: %s\n", u->telefono);
    printf("Tipo de usuario: %s\n", u->es_Admin ? "Administrador" : "Usuario normal");
    printf("-----------------\n");
}

void editarUsuario(Usuario *u) {
    char opcion;
    char temp[MAX_STR];

    do {
        printf("\n--- EDITAR PERFIL ---\n");
        printf("1. Nombre: %s\n", u->nombre);
        printf("2. Apellidos: %s\n", u->apellidos);
        printf("3. DNI: %s\n", u->dni);
        printf("4. Dirección: %s\n", u->direccion);
        printf("5. Email: %s\n", u->email);
        printf("6. Teléfono: %s\n", u->telefono);
        printf("7. Contraseña: ********\n");
        printf("0. Volver\n");
        printf("Seleccione el campo a modificar: ");

        scanf(" %c", &opcion);
        while (getchar() != '\n');

        switch(opcion) {
            case '1':
                printf("Nuevo nombre: ");
                fgets(temp, MAX_STR, stdin);
                strtok(temp, "\n");
                strncpy(u->nombre, temp, MAX_STR);
                printf("Nombre cambiado correctamente.");
                break;
            case '2':
                printf("Nuevos apellidos: ");
                fgets(temp, MAX_STR, stdin);
                strtok(temp, "\n");
                strncpy(u->apellidos, temp, MAX_STR);
                printf("Apellidos cambiados correctamente.");
                break;
            case '3':
                printf("Nuevo DNI: ");
                fgets(temp, MAX_STR, stdin);
                strtok(temp, "\n");
                strncpy(u->dni, temp, MAX_STR);
                printf("DNI cambiado correctamente.");
                break;
            case '4':
                printf("Nueva dirección: ");
                fgets(temp, MAX_STR, stdin);
                strtok(temp, "\n");
                strncpy(u->direccion, temp, MAX_STR);
                printf("Dirección cambiada correctamente.");
                break;
            case '5':
                printf("Nuevo email: ");
                fgets(temp, MAX_STR, stdin);
                strtok(temp, "\n");
                strncpy(u->email, temp, MAX_STR);
                printf("Email cambiado correctamente.");
                break;
            case '6':
                printf("Nuevo teléfono: ");
                fgets(temp, MAX_STR, stdin);
                strtok(temp, "\n");
                strncpy(u->telefono, temp, MAX_STR);
                printf("Teléfono cambiado correctamente.");
                break;
            case '7':
                printf("Nueva contraseña: ");
                leerContrasena(temp);
                strncpy(u->contrasena, temp, MAX_STR);
                printf("Contraseña cambiada correctamente.");
                break;
            case '0':
                printf("Volviendo al menú anterior...\n");
                break;
            default:
                printf("Opción no válida.\n");
        }
    } while(opcion != '0');
}
