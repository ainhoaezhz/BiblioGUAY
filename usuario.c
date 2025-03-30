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
	const char *valorAnterior;

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

		scanf(" %c", &opcion);
		while (getchar() != '\n')
			;

		if (opcion == '0')
			break;

		switch (opcion) {
		case '1':
			snprintf(sql, sizeof(sql),
					"SELECT nombre FROM Usuario WHERE nombre = ?;");
			break;
		case '2':
			snprintf(sql, sizeof(sql),
					"SELECT direccion FROM Usuario WHERE nombre = ?;");
			break;
		case '3':
			snprintf(sql, sizeof(sql),
					"SELECT email FROM Usuario WHERE nombre = ?;");
			break;
		case '4':
			snprintf(sql, sizeof(sql),
					"SELECT telefono FROM Usuario WHERE nombre = ?;");
			break;
		case '5':
			snprintf(sql, sizeof(sql),
					"SELECT contrasena FROM Usuario WHERE nombre = ?;");
			break;
		default:
			printf("Opción no válida. Inténtelo nuevamente.\n");
			continue;
		}

		if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
			fprintf(stderr, "Error al preparar la consulta: %s\n",
					sqlite3_errmsg(db));
			continue;
		}

		sqlite3_bind_text(stmt, 1, nombreUsuario, -1, SQLITE_STATIC);

		if (sqlite3_step(stmt) == SQLITE_ROW) {
			valorAnterior = (const char*) sqlite3_column_text(stmt, 0);
			printf("Valor actual: %s\n", valorAnterior);
		} else {
			printf("Error: No se pudo recuperar el valor actual.\n");
			sqlite3_finalize(stmt);
			continue;
		}

		sqlite3_finalize(stmt);

		printf("Ingrese el nuevo valor: ");
		fflush(stdout);
		fgets(temp, MAX_STR, stdin);
		strtok(temp, "\n");

		printf("Valor ingresado: %s\n", temp);

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

		if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
			fprintf(stderr, "Error al preparar la actualización: %s\n",
					sqlite3_errmsg(db));
			continue;
		}

		sqlite3_bind_text(stmt, 1, temp, -1, SQLITE_STATIC);
		sqlite3_bind_text(stmt, 2, nombreUsuario, -1, SQLITE_STATIC);

		if (sqlite3_step(stmt) != SQLITE_DONE) {
			fprintf(stderr, "Error al actualizar el usuario: %s\n",
					sqlite3_errmsg(db));
		} else {
			printf("Actualización realizada con éxito.\n");
			if (opcion == '1') {
				strncpy(nombreUsuario, temp, MAX_STR);
			}
		}

		sqlite3_finalize(stmt);
	} while (opcion != '0');
}

// Función visualizarHistorial ahora está fuera de editarUsuario
void visualizarHistorial(sqlite3 *db, const char *dniUsuario) {
	sqlite3_stmt *stmt;
	const char *sql =
			"SELECT p.id, l.nombre, p.fecha_Prestamo, p.fecha_Devolucion "
					"FROM Prestamo p "
					"JOIN Libro l ON p.libro_id = l.id "
					"WHERE p.usuario_dni = ? "
					"ORDER BY p.fecha_Prestamo DESC;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar la consulta: %s\n",
				sqlite3_errmsg(db));
		return;
	}

	sqlite3_bind_text(stmt, 1, dniUsuario, -1, SQLITE_STATIC);

	printf("\n--- HISTORIAL DE PRÉSTAMOS ---\n");
	printf("%-5s %-30s %-12s %-12s\n", "ID", "LIBRO", "PRÉSTAMO", "DEVOLUCIÓN");
	printf("------------------------------------------------\n");

	int encontrados = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		encontrados = 1;
		int id = sqlite3_column_int(stmt, 0);
		const char *libro = (const char*) sqlite3_column_text(stmt, 1);
		const char *prestamo = (const char*) sqlite3_column_text(stmt, 2);
		const char *devolucion = (const char*) sqlite3_column_text(stmt, 3);

		printf("%-5d %-30s %-12s %-12s\n", id, libro, prestamo,
				devolucion ? devolucion : "Pendiente");
	}

	if (!encontrados) {
		printf("No se encontraron préstamos registrados.\n");
	}

	sqlite3_finalize(stmt);
}

