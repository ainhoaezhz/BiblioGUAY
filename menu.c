#include "menu.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "bd.h"
#include "menuAdmin.h"
#include "libro.h"

#ifdef _WIN32
#include <conio.h> //Windows
#else
#include <termios.h> //Linux/Mac
#include <unistd.h>
#include "bd.h"
#endif

#define MAX 80
#define MAX_STR 100

void leerContrasena(char *password) {
    int i = 0;
    char ch;

#ifdef _WIN32
    while (1) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
            fflush(stdout);
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == EOF) {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
            fflush(stdout);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    password[i] = '\0';
    printf("\nContraseña ingresada correctamente.\n");
    fflush(stdout);
}



char menuPrincipal() {
	char opcion;

	printf("BIBLIOTECA\n");
	printf("------------------\n");
	printf("Bienvenido\n");
	printf("1. Iniciar Sesion\n");
	printf("2. Registrarse\n");
	printf("0. Salir\n");
	printf("Elige una opcion: ");
	fflush(stdout);

	scanf(" %c", &opcion);
	while (getchar() != '\n');

	return opcion;
}

void iniciarSesion() {
	char usuario[MAX], contrasena[MAX];
	int esAdmin = 0;

	do {
		printf("\nINICIO DE SESIÓN\n");
		printf("--------------\n");
		printf("Usuario: ");
		fflush(stdout);
		scanf("%29s", usuario);
		while (getchar() != '\n');

		printf("Contraseña: ");
		fflush(stdout);
		scanf("%79s", contrasena);

		while (getchar() != '\n');

		if (verificarSesion(db, usuario, contrasena)) {
			if (autenticarUsuario(db, usuario, &esAdmin)) {
				printf("\n¡Inicio de sesión exitoso! Bienvenido, %s.\n",
						usuario);
				printf("Rol: %s\n",
						esAdmin ? "Administrador" : "Usuario Normal");

				if (esAdmin) {
					ejecutarMenuAdmin(db);
					return;
				} else {
					break;
				}

			} else {
				printf("\nError: No se pudo obtener el rol del usuario.\n");
			}
		} else {
			printf(
					"\nError: Usuario o contraseña incorrectos. Inténtelo de nuevo.\n");
		}
	} while (1);

	char opcionMenu;
	do {
		opcionMenu = menuUsuario();
		switch (opcionMenu) {
		case '1':
			printf("Viendo perfil...\n");
			mostrarUsuario(db, usuario);
			break;
		case '2':
			printf("Editando perfil...\n");
			editarUsuario(db, usuario);
			break;
		case '3':
		    printf("Buscando libros...\n");
		    printf("Introduzca el título del libro:");
		    fflush(stdout);
		    char titulo[MAX_NOMBRE];
		    scanf("%29s", titulo);
		    buscar_libros_por_titulo(db, titulo);
		    break;
		case '4':
			printf("Historial de préstamos...\n");
			{
				sqlite3_stmt *stmt;
				const char *sql = "SELECT dni FROM Usuario WHERE nombre = ?;";
				if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
					sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
					if (sqlite3_step(stmt) == SQLITE_ROW) {
						const char *dni = (const char*) sqlite3_column_text(
								stmt, 0);
						visualizarHistorial(db, dni);
					}
					sqlite3_finalize(stmt);
				}
			}
			break;
		case '5':
			printf("Devolviendo libros...\n");
			break;
		case '6':
			printf("Listando libros disponibles...\n");
			listarLibrosDisponibles(db);
			break;
		case '0':
			printf("Saliendo...\n");
			break;
		default:
			printf("ERROR! Opción incorrecta\n");
		}
		printf("\n");
	} while (opcionMenu != '0');
}

char menuUsuario() {
	char opcionMenu;
	printf("----------\n");
	printf("USUARIO: MENU\n");
	printf("1. Ver perfil\n");
	printf("2. Editar perfil\n");
	printf("3. Buscar libros\n");
	printf("4. Historial de prestamos\n");
	printf("5. Devolver libros\n");
	printf("6. Listado de libros disponibles\n");
	printf("0. Salir\n");
	printf("Elige una opcion: ");
	fflush(stdout);

	scanf(" %c", &opcionMenu);
	while (getchar() != '\n');

	return opcionMenu;
}

void registrarse(sqlite3 *db) {
    Usuario nuevoUsuario;
    printf("\nREGISTRO DE USUARIO\n");
    printf("----------------------\n");
    fflush(stdout);

    printf("Nombre: ");
    fflush(stdout);
    fgets(nuevoUsuario.nombre, sizeof(nuevoUsuario.nombre), stdin);
    nuevoUsuario.nombre[strcspn(nuevoUsuario.nombre, "\n")] = 0;

    printf("Apellidos: ");
    fflush(stdout);
    fgets(nuevoUsuario.apellidos, sizeof(nuevoUsuario.apellidos), stdin);
    nuevoUsuario.apellidos[strcspn(nuevoUsuario.apellidos, "\n")] = 0;

    printf("DNI: ");
    fflush(stdout);
    scanf("%19s", nuevoUsuario.dni);
    while (getchar() != '\n');

    printf("Direccion: ");
    fflush(stdout);
    fgets(nuevoUsuario.direccion, sizeof(nuevoUsuario.direccion), stdin);
    nuevoUsuario.direccion[strcspn(nuevoUsuario.direccion, "\n")] = 0;

    printf("Email: ");
    fflush(stdout);
    scanf("%99s", nuevoUsuario.email);
    while (getchar() != '\n');

    printf("Telefono: ");
    fflush(stdout);
    scanf("%14s", nuevoUsuario.telefono);
    while (getchar() != '\n');

    printf("Contraseña: ");
    fflush(stdout);
    scanf("%14s", nuevoUsuario.contrasena);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, 0);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoUsuario.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nuevoUsuario.apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nuevoUsuario.dni, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, nuevoUsuario.direccion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, nuevoUsuario.email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, nuevoUsuario.telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, nuevoUsuario.contrasena, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Usuario registrado exitosamente.\n");
        fflush(stdout);
    } else {
        printf("Error al registrar usuario: %s\n", sqlite3_errmsg(db));
        fflush(stdout);
    }

    sqlite3_finalize(stmt);
}

int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena) {
	sqlite3_stmt *stmt;
	char sql[MAX];

	snprintf(sql, sizeof(sql),
			"SELECT 1 FROM Usuario WHERE nombre = ? AND contrasena = ?;");

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar la consulta: %s\n",
				sqlite3_errmsg(db));
		return 0;
	}

	sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, contrasena, -1, SQLITE_STATIC);

	int autenticado = (sqlite3_step(stmt) == SQLITE_ROW);

	sqlite3_finalize(stmt);
	return autenticado;

}

int autenticarUsuario(sqlite3 *db, char *dni, int *esAdmin) {
	sqlite3_stmt *stmt;
	const char *sql = "SELECT es_Admin FROM Usuario WHERE nombre = ?;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar consulta: %s\n", sqlite3_errmsg(db));
		return 0;
	}

	sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

	int resultado = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		*esAdmin = sqlite3_column_int(stmt, 0);
		resultado = 1;
	} else {
		printf("Usuario no encontrado.\n");
	}

	sqlite3_finalize(stmt);
	return resultado;
}
