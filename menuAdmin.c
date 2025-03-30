#include "menuAdmin.h"
#include "usuario.h"
#include "libro.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

char menuAdministrador() {
    char opcion;
    printf("\n--- MENÚ ADMINISTRADOR ---\n");
    printf("1. Gestionar usuarios\n");
    printf("2. Gestionar libros\n");
    printf("3. Ver estadísticas\n");
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
    const char *sql = "SELECT nombre, apellidos, dni, es_Admin FROM Usuario ORDER BY nombre;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al listar usuarios: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- LISTADO DE USUARIOS ---\n");
    printf("%-20s %-20s %-12s %-10s\n", "NOMBRE", "APELLIDOS", "DNI", "TIPO");
    printf("----------------------------------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-20s %-20s %-12s %-10s\n",
               sqlite3_column_text(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2),
               sqlite3_column_int(stmt, 3) ? "Admin" : "Usuario");
    }
    sqlite3_finalize(stmt);
}

void eliminarUsuario(sqlite3 *db) {
    char dni[MAX_STR];
    printf("\nDNI del usuario a eliminar: ");
    scanf("%19s", dni);
    while (getchar() != '\n');

    // Verificar que no es el admin principal
    if (strcmp(dni, "00000000A") == 0) {
        printf("No se puede eliminar al administrador principal.\n");
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
    scanf("%19s", dni);
    while (getchar() != '\n');

    // Verificar que no es el admin principal
    if (strcmp(dni, "00000000A") == 0) {
        printf("No se puede modificar al administrador principal.\n");
        return;
    }

    printf("Nuevo tipo (0=Usuario, 1=Admin): ");
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

        switch(opcion) {
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
    fgets(nuevoLibro.nombre, MAX_NOMBRE, stdin);  // Cambiado de titulo a nombre
    strtok(nuevoLibro.nombre, "\n");

    printf("Autor: ");
    fgets(nuevoLibro.autor, MAX_AUTOR, stdin);
    strtok(nuevoLibro.autor, "\n");

    printf("Género: ");
    fgets(nuevoLibro.genero, MAX_GENERO, stdin);
    strtok(nuevoLibro.genero, "\n");

    nuevoLibro.estado = 1;

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Libro (nombre, autor, genero, estado) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar inserción: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoLibro.nombre, -1, SQLITE_STATIC);  // Cambiado aquí también
    sqlite3_bind_text(stmt, 2, nuevoLibro.autor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nuevoLibro.genero, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, nuevoLibro.estado);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Libro agregado correctamente. ID: %lld\n", sqlite3_last_insert_rowid(db));
    } else {
        printf("Error al agregar libro.\n");
    }
    sqlite3_finalize(stmt);
}

void listarLibros(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, nombre, autor, genero, estado FROM Libro ORDER BY nombre;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al listar libros: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- CATÁLOGO DE LIBROS ---\n");
    printf("%-5s %-30s %-20s %-15s %-10s\n", "ID", "TÍTULO", "AUTOR", "GÉNERO", "ESTADO");
    printf("----------------------------------------------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-5d %-30s %-20s %-15s %-10s\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_text(stmt, 2),
               sqlite3_column_text(stmt, 3),
               sqlite3_column_int(stmt, 4) ? "Disponible" : "Prestado");
    }
    sqlite3_finalize(stmt);
}

void eliminarLibro(sqlite3 *db) {
    int id;
    printf("\nID del libro a eliminar: ");
    scanf("%d", &id);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Libro WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar eliminación: %s\n", sqlite3_errmsg(db));
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

void gestionarLibros(sqlite3 *db) {
    char opcion;
    do {
        printf("\n--- GESTIÓN DE LIBROS ---\n");
        printf("1. Listar libros\n");
        printf("2. Agregar libro\n");
        printf("3. Eliminar libro\n");
        printf("0. Volver\n");
        printf("Opción: ");
        fflush(stdout);

        scanf(" %c", &opcion);
        while (getchar() != '\n');

        switch(opcion) {
            case '1':
                listarLibros(db);
                break;
            case '2':
                agregarLibro(db);
                break;
            case '3':
                eliminarLibro(db);
                break;
            case '0':
                return;
            default:
                printf("Opción no válida\n");
        }
    } while (1);
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
