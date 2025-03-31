#include "menu.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "bd.h"
#include "menuAdmin.h"

#ifdef _WIN32
#include <conio.h> // Para Windows (ocultar contraseña)
#else
#include <termios.h> // Para Linux/Mac
#include <unistd.h>
#include "bd.h"
#endif

#define MAX 80 // Tamaño máximo para username y password
#define MAX_STR 100 // Tamaño máximo para strings largos como nombre, apellidos, etc.

// Función para leer la contraseña y mostrar asteriscos
void leerContrasena(char *password) {
	int i = 0;
	char ch;

#ifdef _WIN32
	while (1) {
		ch = _getch();  // Lee un carácter sin mostrarlo
		if (ch == '\r' || ch == '\n') {  // Si es Enter
			password[i] = '\0';
			break;
		} else if (ch == 8 || ch == 127) {  // Manejo de retroceso
			if (i > 0) {
				i--;
				printf("\b \b");
			}
		} else if (i < MAX - 1) {
			password[i++] = ch;
			printf("*");
		}
	}
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt); // Obtener atributos actuales del terminal
    newt = oldt;
    newt.c_lflag &= ~(ECHO); // Desactivar eco de la entrada
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Aplicar cambios

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == EOF) {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {  // Manejo de retroceso
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restaurar configuración original
#endif
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

	scanf(" %c", &opcion); // Espacio antes de %c para evitar problemas con '\n'
	while (getchar() != '\n'); // Limpiar el buffer de entrada

	return opcion;
}

char menuRegistro() {
	char opcionRegistro;

	printf("REGISTRO DE USUARIO\n");
	printf("Tipo de usuario: \n");
	printf("1. Administrador\n");
	printf("2. Usuario Normal\n");
	printf("0. Volver al menu principal\n");
	printf("Elige una opcion: ");
	fflush(stdout);

	scanf(" %c", &opcionRegistro);
	while (getchar() != '\n'); // Limpiar buffer de entrada

	return opcionRegistro;
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
		scanf("%79s", contrasena); // Usar MAX-1 para dejar espacio para el '\0'        //leerContrasena(contrasena);  // Si estás utilizando leerContrasena, déjalo aquí.

		while (getchar() != '\n');

		// Verificar sesión del usuario CONTRASEÑA + NOMBRE
		if (verificarSesion(db, usuario, contrasena)) {
			//El usuario es admin?
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
	char *titulo;
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
			scanf("%30s", titulo);
			buscar_libros_por_titulo(db, titulo);
			break;
		case '4':
			printf("Historial de préstamos...\n");
			{
				// Necesitamos obtener el DNI del usuario actual
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
// Función para registrar un nuevo usuario

void registrarse(sqlite3 *db) {
	Usuario nuevoUsuario;
	printf("\nREGISTRO DE USUARIO\n");
	printf("----------------------\n");

	printf("Nombre: ");
	scanf("%99s", nuevoUsuario.nombre);
	while (getchar() != '\n');

	printf("Apellidos: ");
	scanf("%99s", nuevoUsuario.apellidos);
	while (getchar() != '\n');

	printf("DNI: ");
	scanf("%19s", nuevoUsuario.dni);
	while (getchar() != '\n');

	printf("Direccion: ");
	scanf("%99s", nuevoUsuario.direccion);
	while (getchar() != '\n');

	printf("Email: ");
	scanf("%99s", nuevoUsuario.email);
	while (getchar() != '\n');

	printf("Telefono: ");
	scanf("%14s", nuevoUsuario.telefono);
	while (getchar() != '\n');

	printf("Contraseña: ");
	leerContrasena(nuevoUsuario.contrasena);

	printf("\u00bfEs administrador? (1: S\u00ed, 0: No): ");
	scanf("%d", &nuevoUsuario.es_Admin);
	while (getchar() != '\n');
	printf("Usuario registrado exitosamente\n");
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

	// Bind de los parámetros
	sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, contrasena, -1, SQLITE_STATIC);

	// Ejecutar consulta
	int autenticado = (sqlite3_step(stmt) == SQLITE_ROW); // Devuelve 1 si encuentra una coincidencia

	sqlite3_finalize(stmt); // Liberar recursos
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
