#include "biblioteca.h"
#include "menu.h"
#include "libro.h"
#include "sqlite3.h"
#include "bd.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>

sqlite3 *db = NULL;

int main() {
    char opcion;

    if (inicializarBD(&db) != SQLITE_OK) {
        printf("Error al abrir la base de datos\n");
        return 1;
    }

    crearTablas(db);

    // Debug: Ver contenido de la tabla Usuario
    printf("\n=== DEBUG: Usuarios en la base de datos ===\n");
    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, contrasena, es_Admin FROM Usuario;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Usuario: %s | Contraseña: %s | %s\n",
                  sqlite3_column_text(stmt, 0),
                  sqlite3_column_text(stmt, 1),
                  sqlite3_column_int(stmt, 2) ? "Admin" : "Usuario");
        }
        sqlite3_finalize(stmt);
    } else {
        printf("Error al leer usuarios: %s\n", sqlite3_errmsg(db));
    }
    printf("=======================================\n\n");

    do {
        opcion = menuPrincipal();
        switch (opcion) {
            case '1': {
                iniciarSesion();
                break;
            }
            case '2': {
                char opcionRegistro;
                do {
                    opcionRegistro = menuRegistro();
                    switch (opcionRegistro) {
                        case '1':
                            registrarUsuario(db, 1); // Admin
                            break;
                        case '2':
                            registrarUsuario(db, 0); // Usuario normal
                            break;
                        case '0':
                            printf("Volviendo al menú principal...\n");
                            break;
                        default:
                            printf("ERROR! Opción incorrecta\n");
                    }
                    fflush(stdout);
                } while (opcionRegistro != '0');
                break;
            }
            case '0':
                printf("Hasta la próxima\n");
                break;
            default:
                printf("ERROR! La opción seleccionada no es correcta\n");
        }
        fflush(stdout);
    } while (opcion != '0');

    sqlite3_close(db);
    return 0;
}
