#include "biblioteca.h"
#include "menu.h"
#include "libro.h"

#include <stdio.h>
#define MAX 30

int main() {
	char opcion;

	do {
		opcion = menuPrincipal();
		switch (opcion) {
		case '1': {
		    char usuario[MAX], contrasena[MAX];

		    printf("\nINICIAR SESION\n");
		    printf("---\n");
		    printf("Usuario: ");
		    fflush(stdout);
		    scanf("%s", usuario);
		    while (getchar() != '\n'); // Limpiar el buffer de entrada

		    printf("Contrasenya: ");
		    fflush(stdout);
		    scanf("%s", contrasena);
		    while (getchar() != '\n'); // Limpiar el buffer de entrada

		    printf("\n");

		    // Aquí iría la verificación de credenciales
		    break;
		}
		case '2': {
			char opcionRegistro;
			do {
				opcionRegistro = menuRegistro();
				switch (opcionRegistro) {
				case '1':
					printf("Registrando Administrador...\n");
					fflush(stdout);
					break;
				case '2':
					printf("Registrando Usuario Normal...\n");
					fflush(stdout);
					break;
				case '0':
					printf("Volviendo al menú principal...\n");
					fflush(stdout);
					break;
				default:
					printf("ERROR! Opción incorrecta\n");
					fflush(stdout);
				}
			} while (opcionRegistro != '0');
			break;
		}

		case '0':
			printf("Hasta la próxima\n");
			fflush(stdout);
			break;

		default:
			printf("ERROR! La opción seleccionada no es correcta\n");
			fflush(stdout);
		}
	} while (opcion != '0');

	return 0;
}

