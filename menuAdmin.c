#include "menuAdmin.h"
#include "usuario.h"
#include "libro.h"
#include "prestamo.h"
#include <stdlib.h>
#include "bd.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

char menuAdministrador() {
	char opcion;
	printf("\n--- MENÚ ADMINISTRADOR ---\n");
	printf("1. Gestionar usuarios\n");
	printf("2. Gestionar libros\n");
	printf("3. Ver estadísticas\n");
	printf("4. Listado de libros disponibles\n");
	printf("5. Usuario con mas prestamos\n");
	printf("6. Libro mas prestado\n");
	printf("7. Gestionar devoluciones y prestamos\n");
	printf("0. Salir\n");
	printf("Elige una opción: ");
	fflush(stdout);

	scanf(" %c", &opcion);
	while (getchar() != '\n');

	return opcion;
}

void ejecutarMenuAdmin(sqlite3 *db) {
	char opcion;
	do {
		opcion = menuAdministrador();
		switch (opcion) {
		case '1':
			gestionarUsuarios(db);
			break;
		case '2':
			gestionarLibros(db);
			break;
		case '3':
			verEstadisticas(db);
			break;
		case '4':
			printf("\nListando libros disponibles...\n");
			listarLibrosDisponibles(db);
			break;
		case '5':
			mostrarUsuarioConMasPrestamos(db);
			break;
		case '6':
			mostrarLibroMasPrestado(db);
			break;
		case '7':
			gestionarPrestamosDevoluciones(db);
			break;
		case '0':
			printf("Volviendo al menú principal...\n");
			break;
		default:
			printf("Opción no válida.\n");
		}
	} while (opcion != '0');
}


// --------------------------
// GESTIÓN DE USUARIOS
// --------------------------

void listarUsuarios(sqlite3 *db) {
	sqlite3_stmt *stmt;
	const char *sql =
			"SELECT nombre, apellidos, dni, es_Admin FROM Usuario ORDER BY nombre;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al listar usuarios: %s\n", sqlite3_errmsg(db));
		return;
	}

	printf("\n--- LISTADO DE USUARIOS ---\n");
	printf("%-20s %-20s %-12s %-10s\n", "NOMBRE", "APELLIDOS", "DNI", "TIPO");
	printf("----------------------------------------------------\n");

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		printf("%-20s %-20s %-12s %-10s\n", sqlite3_column_text(stmt, 0),
				sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2),
				sqlite3_column_int(stmt, 3) ? "Admin" : "Usuario");
	}
	sqlite3_finalize(stmt);
}

void eliminarUsuario(sqlite3 *db) {
    char dni[MAX_STR];
    printf("\nDNI del usuario a eliminar: ");
    fflush(stdout);
    scanf("%19s", dni);
    while (getchar() != '\n');

    // Verificar que no es el admin principal
    if (strcmp(dni, "00000000A") == 0) {
        printf("No se puede eliminar al administrador principal.\n");
        return;
    }

    // Verificar si el usuario existe antes de eliminarlo
    sqlite3_stmt *checkStmt;
    const char *checkSql = "SELECT COUNT(*) FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar verificación: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(checkStmt, 1, dni, -1, SQLITE_STATIC);

    int userExists = 0;
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        userExists = sqlite3_column_int(checkStmt, 0);
    }
    sqlite3_finalize(checkStmt);

    if (userExists == 0) {
        printf("El DNI ingresado no está registrado.\n");
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar eliminación: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Usuario eliminado correctamente.\n");
    } else {
        printf("Error al eliminar usuario.\n");
    }
    sqlite3_finalize(stmt);
}


void cambiarPermisosUsuario(sqlite3 *db) {
    char dni[MAX_STR];
    int nuevoTipo;

    printf("\nDNI del usuario a modificar: ");
    fflush(stdout);
    scanf("%19s", dni);
    while (getchar() != '\n');

    // Verificar que no es el admin principal
    if (strcmp(dni, "00000000A") == 0) {
        printf("No se puede modificar al administrador principal.\n");
        fflush(stdout);
        return;
    }

    // Verificar si el usuario existe antes de modificar permisos
    sqlite3_stmt *checkStmt;
    const char *checkSql = "SELECT COUNT(*) FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar verificación: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(checkStmt, 1, dni, -1, SQLITE_STATIC);

    int userExists = 0;
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        userExists = sqlite3_column_int(checkStmt, 0);
    }
    sqlite3_finalize(checkStmt);

    if (userExists == 0) {
        printf("El DNI ingresado no está registrado.\n");
        return;
    }

    printf("Nuevo tipo (0=Usuario, 1=Admin): ");
    fflush(stdout);
    scanf("%d", &nuevoTipo);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql = "UPDATE Usuario SET es_Admin = ? WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar actualización: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, nuevoTipo);
    sqlite3_bind_text(stmt, 2, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Permisos actualizados correctamente.\n");
    } else {
        printf("Error al actualizar permisos.\n");
    }
    sqlite3_finalize(stmt);
}


void gestionarUsuarios(sqlite3 *db) {
	char opcion;
	do {
		printf("\n--- GESTIÓN DE USUARIOS ---\n");
		printf("1. Listar usuarios\n");
		printf("2. Eliminar usuario\n");
		printf("3. Cambiar permisos\n");
		printf("0. Volver\n");
		printf("Opción: ");
		fflush(stdout);

		scanf(" %c", &opcion);
		while (getchar() != '\n');

		switch (opcion) {
		case '1':
			listarUsuarios(db);
			break;
		case '2':
			eliminarUsuario(db);
			break;
		case '3':
			cambiarPermisosUsuario(db);
			break;
		case '0':
			return;
		default:
			printf("Opción no válida\n");
		}
	} while (1);
}

// --------------------------
// GESTIÓN DE LIBROS
// --------------------------

void agregarLibro(sqlite3 *db) {
	Libro nuevoLibro;
	printf("\n--- AGREGAR LIBRO ---\n");

	printf("Título: ");
	fflush(stdout);
	fgets(nuevoLibro.nombre, sizeof(nuevoLibro.nombre), stdin);
	nuevoLibro.nombre[strcspn(nuevoLibro.nombre, "\n")] = 0;

	printf("Autor: ");
	fflush(stdout);
    fgets(nuevoLibro.autor, sizeof(nuevoLibro.autor), stdin);
    nuevoLibro.autor[strcspn(nuevoLibro.autor, "\n")] = 0;


	printf("Género: ");
	fflush(stdout);
    fgets(nuevoLibro.genero, sizeof(nuevoLibro.genero), stdin);
    nuevoLibro.genero[strcspn(nuevoLibro.genero, "\n")] = 0;


	nuevoLibro.estado = 1;

	 int idReciclado = -1;
	    sqlite3_stmt *stmtBuscarID;
	    const char *sqlBuscarID = "SELECT MIN(t1.id + 1) FROM Libro t1 LEFT JOIN Libro t2 ON t1.id + 1 = t2.id WHERE t2.id IS NULL;";

	    if (sqlite3_prepare_v2(db, sqlBuscarID, -1, &stmtBuscarID, NULL) == SQLITE_OK) {
	        if (sqlite3_step(stmtBuscarID) == SQLITE_ROW) {
	            idReciclado = sqlite3_column_int(stmtBuscarID, 0);
	        }
	    }
	    sqlite3_finalize(stmtBuscarID);

	    // Preparar la consulta de inserción
	    const char *sqlInsert = idReciclado > 0
	        ? "INSERT INTO Libro (id, nombre, autor, genero, estado) VALUES (?, ?, ?, ?, ?);"
	        : "INSERT INTO Libro (nombre, autor, genero, estado) VALUES (?, ?, ?, ?);";

	    sqlite3_stmt *stmt;
	    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK) {
	        fprintf(stderr, "Error al preparar inserción: %s\n", sqlite3_errmsg(db));
	        return;
	    }

	    // Vincular los valores a la consulta SQL
	    if (idReciclado > 0) sqlite3_bind_int(stmt, 1, idReciclado);
	    sqlite3_bind_text(stmt, idReciclado > 0 ? 2 : 1, nuevoLibro.nombre, -1, SQLITE_STATIC);
	    sqlite3_bind_text(stmt, idReciclado > 0 ? 3 : 2, nuevoLibro.autor, -1, SQLITE_STATIC);
	    sqlite3_bind_text(stmt, idReciclado > 0 ? 4 : 3, nuevoLibro.genero, -1, SQLITE_STATIC);
	    sqlite3_bind_int(stmt, idReciclado > 0 ? 5 : 4, nuevoLibro.estado);

	    // Ejecutar la consulta
	    if (sqlite3_step(stmt) == SQLITE_DONE) {
	        printf("Libro agregado correctamente. ID: %I64d\n",(long long) (idReciclado > 0 ? idReciclado : sqlite3_last_insert_rowid(db)));
	        fflush(stdout);
	    } else {
	        printf("Error al agregar libro.\n");
	    }

	    sqlite3_finalize(stmt);
	}


void listarLibros(sqlite3 *db) {
	sqlite3_stmt *stmt;
	const char *sql =
			"SELECT id, nombre, autor, genero, estado FROM Libro ORDER BY id;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al listar libros: %s\n", sqlite3_errmsg(db));
		return;
	}

	printf("\n--- CATÁLOGO DE LIBROS ---\n");
	printf("%-5s %-30s %-20s %-15s %-10s\n", "ID", "TÍTULO", "AUTOR", "GÉNERO",
			"ESTADO");
	printf(
			"----------------------------------------------------------------\n");

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		printf("%-5d %-30s %-20s %-15s %-10s\n", sqlite3_column_int(stmt, 0),
				sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2),
				sqlite3_column_text(stmt, 3),
				sqlite3_column_int(stmt, 4) ? "Disponible" : "Prestado");
	}
	sqlite3_finalize(stmt);
}

void eliminarLibro(sqlite3 *db) {
	int id;
	printf("\nID del libro a eliminar: ");
	fflush(stdout);
	scanf("%d", &id);
	while (getchar() != '\n');

	sqlite3_stmt *stmt;
	const char *sql = "DELETE FROM Libro WHERE id = ?;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar eliminación: %s\n",
				sqlite3_errmsg(db));
		return;
	}

	sqlite3_bind_int(stmt, 1, id);

	if (sqlite3_step(stmt) == SQLITE_DONE) {
		printf("Libro eliminado correctamente.\n");
	} else {
		printf("Error al eliminar libro.\n");
	}
	sqlite3_finalize(stmt);
}

void editarLibro(sqlite3 *db) {
	int idLibro;
	char nuevoTitulo[MAX_NOMBRE];
	char nuevoAutor[MAX_NOMBRE];
	char nuevoGenero[MAX_NOMBRE];
	//
	listarLibros(db);
	printf("Elija un libro, introduciendo su ID: ");
	fflush(stdout);
	scanf("%d", &idLibro);
	while (getchar() != '\n');

	sqlite3_stmt *stmt;
	    const char *sql_verificar = "SELECT COUNT(*) FROM Libro WHERE id = ?;";

	    if (sqlite3_prepare_v2(db, sql_verificar, -1, &stmt, NULL) != SQLITE_OK) {
	        fprintf(stderr, "Error al verificar libro: %s\n", sqlite3_errmsg(db));
	        return;
	    }

	    sqlite3_bind_int(stmt, 1, idLibro);

	    if (sqlite3_step(stmt) != SQLITE_ROW || sqlite3_column_int(stmt, 0) == 0) {
	        printf("Error: No existe un libro con el ID %d.\n", idLibro);
	        sqlite3_finalize(stmt);
	        return;
	    }

	    sqlite3_finalize(stmt);

	    // Pedir nuevos datos
	    printf("Nuevo título (deje en blanco para no cambiar): ");
	    fflush(stdout);
	    fgets(nuevoTitulo, sizeof(nuevoTitulo), stdin);
	    nuevoTitulo[strcspn(nuevoTitulo, "\n")] = 0;

	    printf("Nuevo autor (deje en blanco para no cambiar): ");
	    fflush(stdout);
	    fgets(nuevoAutor, sizeof(nuevoAutor), stdin);
	    nuevoAutor[strcspn(nuevoAutor, "\n")] = 0;

	    printf("Nuevo género (deje en blanco para no cambiar): ");
	    fflush(stdout);
	    fgets(nuevoGenero, sizeof(nuevoGenero), stdin);
	    nuevoGenero[strcspn(nuevoGenero, "\n")] = 0;

	    //si el usuario deja algún campo vacío
	    const char *sql_obtener = "SELECT nombre, autor, genero FROM Libro WHERE id = ?;";

	    if (sqlite3_prepare_v2(db, sql_obtener, -1, &stmt, NULL) != SQLITE_OK) {
	        fprintf(stderr, "Error al obtener datos del libro: %s\n", sqlite3_errmsg(db));
	        return;
	    }

	    sqlite3_bind_int(stmt, 1, idLibro);

	    if (sqlite3_step(stmt) == SQLITE_ROW) {
	        if (strlen(nuevoTitulo) == 0) strcpy(nuevoTitulo, (const char *)sqlite3_column_text(stmt, 0));
	        if (strlen(nuevoAutor) == 0) strcpy(nuevoAutor, (const char *)sqlite3_column_text(stmt, 1));
	        if (strlen(nuevoGenero) == 0) strcpy(nuevoGenero, (const char *)sqlite3_column_text(stmt, 2));
	    }

	    sqlite3_finalize(stmt);

	    // Actualizar el libro en la base de datos
	    const char *sql_actualizar = "UPDATE Libro SET nombre = ?, autor = ?, genero = ? WHERE id = ?;";

	    if (sqlite3_prepare_v2(db, sql_actualizar, -1, &stmt, NULL) != SQLITE_OK) {
	        fprintf(stderr, "Error al preparar actualización: %s\n", sqlite3_errmsg(db));
	        return;
	    }

	    sqlite3_bind_text(stmt, 1, nuevoTitulo, -1, SQLITE_STATIC);
	    sqlite3_bind_text(stmt, 2, nuevoAutor, -1, SQLITE_STATIC);
	    sqlite3_bind_text(stmt, 3, nuevoGenero, -1, SQLITE_STATIC);
	    sqlite3_bind_int(stmt, 4, idLibro);

	    if (sqlite3_step(stmt) == SQLITE_DONE) {
	        printf("Libro editado correctamente :) .\n");
	        fflush(stdout);
	    } else {
	        printf("Error al editar el libro :(.\n");
	        fflush(stdout);
	    }

	    sqlite3_finalize(stmt);


}

void gestionarLibros(sqlite3 *db) {
	char opcion;
	do {
		printf("\n--- GESTIÓN DE LIBROS ---\n");
		printf("1. Listar libros\n");
		printf("2. Agregar libro\n");
		printf("3. Eliminar libro\n");
		printf("4. Editar Información de un libro\n");
		printf("0. Volver\n");
		printf("Opción: ");
		fflush(stdout);

		scanf(" %c", &opcion);
		while (getchar() != '\n');

		switch (opcion) {
		case '1':
			listarLibros(db);
			break;
		case '2':
			agregarLibro(db);
			break;
		case '3':
			eliminarLibro(db);
			break;
		case '4':
			editarLibro(db);
			break;
		case '0':
			return;
		default:
			printf("Opción no válida\n");
		}
	} while (1);
}

void mostrarLibroMasPrestado(sqlite3 *db) {
	sqlite3_stmt *stmt;
	const char *sql =
		"SELECT l.nombre, l.autor, COUNT(p.id) AS total_prestamos "
	    "FROM Libro l "
	    "JOIN Prestamo p ON l.id = p.libro_id "
	    "GROUP BY l.id "
	    "ORDER BY total_prestamos DESC "
	    "LIMIT 1;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
	    fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
	    return;
	}

	printf("\n--- LIBRO MÁS PRESTADO ---\n");

	if (sqlite3_step(stmt) == SQLITE_ROW) {

	    const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
	    const char *autor = (const char *)sqlite3_column_text(stmt, 1);
	    int total_prestamos = sqlite3_column_int(stmt, 2);

	    printf("Título:   %s\n", nombre);
	    printf("Autor:    %s\n", autor);
	    printf("Préstamos: %d\n", total_prestamos);
	} else {
		printf("No hay préstamos registrados.\n");
	}

	sqlite3_finalize(stmt);
}

// --------------------------
// ESTADÍSTICAS
// --------------------------

void verEstadisticas(sqlite3 *db) {
	printf("\n--- ESTADÍSTICAS DE LA BIBLIOTECA ---\n");

	// Total usuarios
	sqlite3_stmt *stmt;
	const char *sql = "SELECT COUNT(*) FROM Usuario;";
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			printf("Total usuarios: %d\n", sqlite3_column_int(stmt, 0));
		}
		sqlite3_finalize(stmt);
	}

	// Total libros
	sql = "SELECT COUNT(*) FROM Libro;";
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			printf("Total libros: %d\n", sqlite3_column_int(stmt, 0));
		}
		sqlite3_finalize(stmt);
	}

	// Libros prestados
	sql = "SELECT COUNT(*) FROM Libro WHERE estado = 0;";
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			printf("Libros prestados: %d\n", sqlite3_column_int(stmt, 0));
		}
		sqlite3_finalize(stmt);
	}

	// Préstamos activos
	sql = "SELECT COUNT(*) FROM Prestamo WHERE fecha_Devolucion IS NULL;";
	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
		if (sqlite3_step(stmt) == SQLITE_ROW) {
			printf("Préstamos activos: %d\n", sqlite3_column_int(stmt, 0));
		}
		sqlite3_finalize(stmt);
	}
}

void mostrarUsuarioConMasPrestamos(sqlite3 *db) {
	sqlite3_stmt *stmt;
	const char *sql =
		"SELECT u.nombre, u.apellidos, u.dni, COUNT(p.id) AS total_prestamos "
	    "FROM Usuario u "
	    "JOIN Prestamo p ON u.dni = p.usuario_dni "
	    "GROUP BY u.dni "
	    "ORDER BY total_prestamos DESC "
	    "LIMIT 1;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
	    return;
	}

	printf("\n--- USUARIO CON MÁS PRÉSTAMOS ---\n");

	if(sqlite3_step(stmt) == SQLITE_ROW) {
		const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
		const char *apellidos = (const char *)sqlite3_column_text(stmt, 1);
		const char *dni = (const char *)sqlite3_column_text(stmt, 2);
		int total_prestamos = sqlite3_column_int(stmt, 3);

		printf("Nombre:    %s %s\n", nombre, apellidos);
		printf("DNI:       %s\n", dni);
		printf("Préstamos: %d\n", total_prestamos);
	} else {
		printf("No hay prestamos registrados.\n");
	}

	sqlite3_finalize(stmt);
}

void gestionarPrestamosDevoluciones(sqlite3 *db) {
	char opcion;
	do {
		printf("\n--- GESTIÓN DE DEVOLUCIONES Y PRESTAMOS ---\n");
		printf("1. Registrar prestamo\n");
		printf("2. Registrar devolucion\n");
		printf("3. Mostrar prestamos activos\n");
		printf("0. Volver\n");
		printf("Opción: ");
		fflush(stdout);

		scanf(" %c", &opcion);
		while (getchar() != '\n');

		switch (opcion) {
		case '1':
			registrarPrestamo(db);
			break;
		case '2':
			break;
		case '3':
			break;
		case '0':
			return;
		default:
			printf("Opción no válida\n");
		}
	} while (1);
}

void registrarPrestamo(sqlite3 *db) {
    Prestamo nuevoPrestamo;
    printf("\n--- AGREGAR PRÉSTAMO ---\n");

    // Solicitar datos del préstamo
    printf("DNI del usuario: ");
    fflush(stdout);
    fgets(nuevoPrestamo.usuario_dni, sizeof(nuevoPrestamo.usuario_dni), stdin);
    nuevoPrestamo.usuario_dni[strcspn(nuevoPrestamo.usuario_dni, "\n")] = 0;

    printf("ID del libro: ");
    fflush(stdout);
    char input[20];
    fgets(input, sizeof(input), stdin);
    nuevoPrestamo.libro_id = atoi(input);

    printf("Fecha de préstamo (YYYY-MM-DD): ");
    fflush(stdout);
    fgets(nuevoPrestamo.fecha_prestamo, sizeof(nuevoPrestamo.fecha_prestamo), stdin);
    nuevoPrestamo.fecha_prestamo[strcspn(nuevoPrestamo.fecha_prestamo, "\n")] = 0;

    // Buscar ID reciclado (opcional)
    int idReciclado = -1;
    sqlite3_stmt *stmtBuscarID;
    const char *sqlBuscarID = "SELECT MIN(t1.id + 1) FROM Prestamo t1 LEFT JOIN Prestamo t2 ON t1.id + 1 = t2.id WHERE t2.id IS NULL;";

    if (sqlite3_prepare_v2(db, sqlBuscarID, -1, &stmtBuscarID, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmtBuscarID) == SQLITE_ROW) {
            idReciclado = sqlite3_column_int(stmtBuscarID, 0);
        }
    }
    sqlite3_finalize(stmtBuscarID);

    // Preparar la consulta de inserción
    const char *sqlInsert = idReciclado > 0
        ? "INSERT INTO Prestamo (id, usuario_dni, libro_id, fecha_Prestamo, fecha_Devolucion) VALUES (?, ?, ?, ?, ?);"
        : "INSERT INTO Prestamo (usuario_dni, libro_id, fecha_Prestamo, fecha_Devolucion) VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar inserción: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Vincular los valores
    int bindIndex = 1;
    if (idReciclado > 0) {
        sqlite3_bind_int(stmt, bindIndex++, idReciclado);
    }
    sqlite3_bind_text(stmt, bindIndex++, nuevoPrestamo.usuario_dni, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, bindIndex++, nuevoPrestamo.libro_id);
    sqlite3_bind_text(stmt, bindIndex++, nuevoPrestamo.fecha_prestamo, -1, SQLITE_STATIC);
    
    // Manejar fecha de devolución (puede ser NULL)
    if (strlen(nuevoPrestamo.fecha_devolucion) > 0) {
        sqlite3_bind_text(stmt, bindIndex, nuevoPrestamo.fecha_devolucion, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, bindIndex);
    }

    // Ejecutar la consulta
    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Préstamo agregado correctamente. ID: %I64d\n", 
              (long long)(idReciclado > 0 ? idReciclado : sqlite3_last_insert_rowid(db)));
    } else {
        fprintf(stderr, "Error al agregar préstamo: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}