#include "Servidor.h"
#include "sqlite3.h"
#include "bd.h"
#include <stdio.h>

sqlite3 *db = NULL;

int main() {
    if (inicializarBD(&db) != SQLITE_OK) {
        printf("Error al abrir la base de datos\n");
        return 1;
    }

    crearTablas(db);

    iniciarServidor();  // ahora espera conexiones

    sqlite3_close(db);
    return 0;
}
