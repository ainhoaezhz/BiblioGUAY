#include "usuario.h"
#include "menu.h"
#include "bd.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

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

void mostrarUsuario(sqlite3 *db, const char *nombreUsuario) {
	sqlite3_stmt *stmt;
	char sql[MAX_STR];

	snprintf(sql, sizeof(sql),
			"SELECT nombre, apellidos, dni, direccion, email, telefono, es_Admin FROM Usuario WHERE nombre = ?;");

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar la consulta: %s\n",
				sqlite3_errmsg(db));
		return;
	}

	// Bind de los parámetros
	sqlite3_bind_text(stmt, 1, nombreUsuario, -1, SQLITE_STATIC);

	if (sqlite3_step(stmt) != SQLITE_ROW) {
		printf("\nError: Usuario no encontrado.\n");
		sqlite3_finalize(stmt);
		return;
	}

	printf("\n--- TU PERFIL ---\n");
	printf("Nombre: %s\n", sqlite3_column_text(stmt, 0));
	printf("Apellidos: %s\n", sqlite3_column_text(stmt, 1));
	printf("DNI: %s\n", sqlite3_column_text(stmt, 2));
	printf("Dirección: %s\n", sqlite3_column_text(stmt, 3));
	printf("Email: %s\n", sqlite3_column_text(stmt, 4));
	printf("Teléfono: %s\n", sqlite3_column_text(stmt, 5));
	printf("Tipo de usuario: %s\n",
			sqlite3_column_int(stmt, 6) ? "Administrador" : "Usuario Normal");

	sqlite3_finalize(stmt);
}

void editarUsuario(sqlite3 *db, char *nombreUsuario) {
	char opcion;
	char temp[MAX_STR];
	char sql[MAX_STR];
	sqlite3_stmt *stmt;
	const char *valorAnterior;  // Variable para almacenar el valor anterior

	do {
		printf("\n--- EDITAR PERFIL ---\n");
		printf("1. Nombre\n");
		printf("2. Dirección\n");
		printf("3. Email\n");
		printf("4. Teléfono\n");
		printf("5. Contraseña\n");
		printf("0. Volver\n");
		printf("Seleccione el campo a modificar: ");
		fflush(stdout);

		scanf(" %c", &opcion);  // Leer opción del menú
		while (getchar() != '\n')
			;  // Limpiar el buffer de entrada

		if (opcion == '0')
			break;  // Si se elige '0', salir del bucle

		// Dependiendo de la opción seleccionada, recuperar el valor actual de la base de datos
		switch (opcion) {
		case '1': // Nombre
			snprintf(sql, sizeof(sql),
					"SELECT nombre FROM Usuario WHERE nombre = ?;");
			break;
		case '2': // Dirección
			snprintf(sql, sizeof(sql),
					"SELECT direccion FROM Usuario WHERE nombre = ?;");
			break;
		case '3': // Email
			snprintf(sql, sizeof(sql),
					"SELECT email FROM Usuario WHERE nombre = ?;");
			break;
		case '4': // Teléfono
			snprintf(sql, sizeof(sql),
					"SELECT telefono FROM Usuario WHERE nombre = ?;");
			break;
		case '5': // Contraseña
			snprintf(sql, sizeof(sql),
					"SELECT contrasena FROM Usuario WHERE nombre = ?;");
			break;
		default:
			printf("Opción no válida. Inténtelo nuevamente.\n");
			continue;  // Si la opción no es válida, volvemos a pedirla
		}

		// Preparar la sentencia SQL
		if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
			fprintf(stderr, "Error al preparar la consulta: %s\n",
					sqlite3_errmsg(db));
			continue; // Si no se puede preparar la sentencia, continuamos con el bucle
		}

		// Vincular el nombre del usuario para obtener el valor actual
		sqlite3_bind_text(stmt, 1, nombreUsuario, -1, SQLITE_STATIC);

		// Ejecutar la consulta SQL y recuperar el valor anterior
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			valorAnterior = (const char*) sqlite3_column_text(stmt, 0); // Obtener el valor del campo correspondiente
			printf("Valor actual: %s\n", valorAnterior); // Mostrar el valor actual
		} else {
			printf("Error: No se pudo recuperar el valor actual.\n");
			sqlite3_finalize(stmt);
			continue; // Si no se puede obtener el valor actual, continuamos con el bucle
		}

		sqlite3_finalize(stmt);  // Finalizar la sentencia SQL

		// Solicitar el nuevo valor para el campo a modificar
		printf("Ingrese el nuevo valor: ");
		fflush(stdout);
		fgets(temp, MAX_STR, stdin);  // Leer entrada
		strtok(temp, "\n"); // Eliminar el salto de línea al final de la entrada

		// Depuración: Imprimir el valor ingresado
		printf("Valor ingresado: %s\n", temp); // Mostrar lo que se ha ingresado

		// Preparar la consulta SQL para la actualización
		switch (opcion) {
		case '1':
			snprintf(sql, sizeof(sql),
					"UPDATE Usuario SET nombre = ? WHERE nombre = ?;");
			break;
		case '2':
			snprintf(sql, sizeof(sql),
					"UPDATE Usuario SET direccion = ? WHERE nombre = ?;");
			break;
		case '3':
			snprintf(sql, sizeof(sql),
					"UPDATE Usuario SET email = ? WHERE nombre = ?;");
			break;
		case '4':
			snprintf(sql, sizeof(sql),
					"UPDATE Usuario SET telefono = ? WHERE nombre = ?;");
			break;
		case '5':
			snprintf(sql, sizeof(sql),
					"UPDATE Usuario SET contrasena = ? WHERE nombre = ?;");
			break;
		}

		// Preparar la sentencia SQL para la actualización
		if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
			fprintf(stderr, "Error al preparar la actualización: %s\n",
					sqlite3_errmsg(db));
			continue; // Si no se puede preparar la sentencia, continuamos con el bucle
		}

		// Vincular los parámetros de la consulta SQL
		sqlite3_bind_text(stmt, 1, temp, -1, SQLITE_STATIC);  // Valor ingresado
		sqlite3_bind_text(stmt, 2, nombreUsuario, -1, SQLITE_STATIC); // Nombre de usuario

		// Ejecutar la consulta SQL para actualizar
		if (sqlite3_step(stmt) != SQLITE_DONE) {
			fprintf(stderr, "Error al actualizar el usuario: %s\n",
					sqlite3_errmsg(db));
		} else {
			printf("Actualización realizada con éxito.\n");
			if (opcion == '1') {
				strncpy(nombreUsuario, temp, MAX_STR);
			}
		}

		sqlite3_finalize(stmt);  // Finalizar la sentencia SQL

	} while (opcion != '0'); // El bucle continúa hasta que el usuario elija salir
}

