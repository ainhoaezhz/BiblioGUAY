#include "biblioteca.h"
#include "menu.h"
#include "libro.h"
#include "sqlite3.h"
#include "bd.h"

#include <stdio.h>
#define MAX 30

int main() {
	char opcion;
	sqlite3 *db;

	if (inicializarBD(&db) != SQLITE_OK) {
		printf("Error al abrir la base de datos\n");
		return 1;
	}

	crearTablas(db);

	do {
		opcion = menuPrincipal();
		switch (opcion) {
		case '1': {
			iniciarSesion();
			menuUsuario();
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
	sqlite3_close(db);

	return 0;
}

