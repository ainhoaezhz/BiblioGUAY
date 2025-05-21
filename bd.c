#include "bd.h"
#include <stdio.h>
#include <string.h>
#include "libro.h"
#include "prestamo.h"

int inicializarBD(sqlite3 **db) {
	int result;

	result = sqlite3_open(NOMBRE_BD, db);
	return result;
}

void crearTablas(sqlite3 *db) {
    char *errMsg = 0;
    char sql[512];

    snprintf(sql, sizeof(sql),
            "CREATE TABLE IF NOT EXISTS Libro ("
            "nombre VARCHAR(100), "
            "autor VARCHAR(100), "
            "genero VARCHAR(50), "
            "id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "estado INTEGER);");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al crear tabla Libro: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    snprintf(sql, sizeof(sql),
            "CREATE TABLE IF NOT EXISTS Usuario ("
            "nombre VARCHAR(100), "
            "apellidos VARCHAR(100), "
            "dni VARCHAR(100) UNIQUE, "
            "direccion VARCHAR(100), "
            "email VARCHAR(100) UNIQUE, "
            "telefono VARCHAR(100), "
            "contrasena VARCHAR(100), "
            "es_Admin INTEGER);");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al crear tabla Usuario: %s\n", errMsg);
        sqlite3_free(errMsg);
    }


    snprintf(sql, sizeof(sql),
    		"CREATE TABLE IF NOT EXISTS Prestamo("
    		"id INTEGER PRIMARY KEY AUTOINCREMENT, "
    		"usuario_dni INTEGER, "
    		"libro_id INTEGER,"
    		"fecha_Prestamo DATE, "
    		"fecha_Devolucion DATE, "
    		"FOREIGN KEY (usuario_dni) REFERENCES Usuario(dni),"
    		"FOREIGN KEY (libro_id) REFERENCES Libro(id));"

    		);
    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
            fprintf(stderr, "Error al crear tabla Prestamo: %s\n", errMsg);
            sqlite3_free(errMsg);
        }

    snprintf(sql, sizeof(sql),
            "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
            "SELECT 'admin', 'admin', '00000000A', 'admin', 'admin@admin.com', '000000000', 'admin', 1 "
            "WHERE NOT EXISTS (SELECT 1 FROM Usuario WHERE dni = '00000000A');");

    if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
        fprintf(stderr, "Error al insertar admin: %s\n", errMsg);
        sqlite3_free(errMsg);

    }


    snprintf(sql, sizeof(sql),
                "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
                "SELECT 'juan', 'perez', '56289643N', 'c/lasMercedes 5', 'juanperez@gmail.com', '634895496', 'contrasena', 0 "
                "WHERE NOT EXISTS (SELECT 1 FROM Usuario WHERE dni = '56289643N');");

        if (sqlite3_exec(db, sql, NULL, NULL, &errMsg) != SQLITE_OK) {
            fprintf(stderr, "Error al insertar admin: %s\n", errMsg);
            sqlite3_free(errMsg);
        }

    insertarLibrosBase(db);
    insertarPrestamosBase(db);


}

void insertarPrestamosBase(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Prestamo (usuario_dni, libro_id, fecha_Prestamo, fecha_Devolucion) "
                     "VALUES (?, ?, ?, ?);";

    Prestamo prestamos[] = {
        {"12345678A", 1, "2023-10-01", "2023-10-15"},
        {"87654321B", 2, "2023-10-05", "2023-10-20"},
        {"11223344C", 3, "2023-10-10", ""},  // NULL
        {"12345678A", 4, "2023-11-01", "2023-11-10"}
    };

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    for (int i = 0; i < sizeof(prestamos) / sizeof(prestamos[0]); i++) {
        sqlite3_bind_text(stmt, 1, prestamos[i].usuario_dni, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, prestamos[i].libro_id);
        sqlite3_bind_text(stmt, 3, prestamos[i].fecha_prestamo, -1, SQLITE_STATIC);
        
        if (strlen(prestamos[i].fecha_devolucion) > 0) {
            sqlite3_bind_text(stmt, 4, prestamos[i].fecha_devolucion, -1, SQLITE_STATIC);
        } else {
            sqlite3_bind_null(stmt, 4);
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Error al insertar préstamo: %s\n", sqlite3_errmsg(db));
        }
        sqlite3_reset(stmt);
    }
    sqlite3_finalize(stmt);
}

void insertarLibrosBase(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Libro (nombre, autor, genero, estado) "
                     "SELECT ?, ?, ?, ? "
                     "WHERE NOT EXISTS (SELECT 1 FROM Libro WHERE nombre = ? AND autor = ?);";

    Libro libros[] = {
        {"Cien años de soledad", "Gabriel García Márquez", "Realismo mágico", 0, 1},
        {"1984", "George Orwell", "Ciencia ficción", 0, 1},
        {"El señor de los anillos", "J.R.R. Tolkien", "Fantasía", 0, 1},
        {"Orgullo y prejuicio", "Jane Austen", "Romance", 0, 1},
        {"Crimen y castigo", "Fiódor Dostoyevski", "Novela psicológica", 0, 1}
    };

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    int numLibros = sizeof(libros) / sizeof(libros[0]);

    for (int i = 0; i < numLibros; i++) {
        sqlite3_bind_text(stmt, 1, libros[i].nombre, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, libros[i].autor, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 3, libros[i].genero, -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 4, libros[i].estado);
        sqlite3_bind_text(stmt, 5, libros[i].nombre, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 6, libros[i].autor, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            fprintf(stderr, "Error al insertar libro %s: %s\n",
                    libros[i].nombre, sqlite3_errmsg(db));
        }

        sqlite3_reset(stmt);
    }

    sqlite3_finalize(stmt);
}

void buscar_libros_por_titulo(sqlite3 *db,const char *titulo_buscar) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, nombre, autor, genero, estado FROM Libro WHERE nombre LIKE ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    char patron[100];
    snprintf(patron, sizeof(patron), "%%%s%%", titulo_buscar);
    sqlite3_bind_text(stmt, 1, patron, -1, SQLITE_TRANSIENT);

    printf("\nResultados para '%s':\n", titulo_buscar);
    printf("ID\tTítulo\t\t\tAutor\t\tGénero\t\tEstado\n");
    printf("----------------------------------------------------------------\n");

    int encontrados = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Libro libro;

        libro.id = sqlite3_column_int(stmt, 0);
        const char *nombre = (const char *)sqlite3_column_text(stmt, 1);
        const char *autor = (const char *)sqlite3_column_text(stmt, 2);
        const char *genero = (const char *)sqlite3_column_text(stmt, 3);
        libro.estado = sqlite3_column_int(stmt, 4);

        strncpy(libro.nombre, nombre, MAX_NOMBRE - 1);
        libro.nombre[MAX_NOMBRE - 1] = '\0';

        strncpy(libro.autor, autor, MAX_AUTOR - 1);
        libro.autor[MAX_AUTOR - 1] = '\0';

        strncpy(libro.genero, genero, MAX_GENERO - 1);
        libro.genero[MAX_GENERO - 1] = '\0';

        printf("%d\t%.20s\t%.15s\t%.10s\t%s\n",
               libro.id,
               libro.nombre,
               libro.autor,
               libro.genero,
               libro.estado == 1 ? "Disponible" : "Prestado");

        encontrados++;
    }

    if (encontrados == 0) {
        printf("No se encontraron libros con ese título\n");
    }

    sqlite3_finalize(stmt);
}

void listarLibrosDisponibles(sqlite3 *db) {
	sqlite3_stmt *stmt;
	const char *sql = "SELECT id, nombre, autor, genero FROM Libro WHERE estado = 1 ORDER BY nombre;";

	if(sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al listar libros disponibles: %s\n", sqlite3_errmsg(db));
		return;
	}

	 printf("\n--- LIBROS DISPONIBLES ---\n");
	 printf("%-5s %-30s %-25s %-15s\n", "ID", "TÍTULO", "AUTOR", "GÉNERO");
     printf("----------------------------------------------------------------------\n");

     int encontrados = 0;
     while(sqlite3_step(stmt) == SQLITE_ROW) {
    	 encontrados = 1;
    	 printf("%-5d %-30s %-25s %-15s\n", sqlite3_column_int(stmt, 0), sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2), sqlite3_column_text(stmt, 3));
     }

     if(!encontrados) {
    	 printf("No hay libros libros disponibles.\n");
     }

     sqlite3_finalize(stmt);
}

