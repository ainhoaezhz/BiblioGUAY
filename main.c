#include "menu.h"
#include "bd.h"
#include <stdio.h>
#include <stdlib.h>

int main() {
    char opcion;
    sqlite3 *db;

    // Inicializar base de datos
    if (inicializarBD(&db) != SQLITE_OK) {
        fprintf(stderr, "Error crítico: No se pudo abrir la base de datos\n");
        fprintf(stderr, "Código de error: %d\n", sqlite3_errcode(db));
        fprintf(stderr, "Mensaje: %s\n", sqlite3_errmsg(db));

        if (db) sqlite3_close(db);
        return EXIT_FAILURE;
    }

    // Crear tablas si no existen
    crearTablas(db);

    // Bucle principal del programa
    do {
        opcion = menuPrincipal();
        switch (opcion) {
            case '1':
                iniciarSesion(db);
                break;
            case '2':
                menuRegistro(db);
                break;
            case '0':
                printf("Saliendo del programa...\n");
                break;
            default:
                printf("ERROR! Opción no válida\n");
        }
    } while (opcion != '0');

    // Cerrar la base de datos antes de salir
    cerrarBD(db);
    return EXIT_SUCCESS;
}
