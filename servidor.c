
#include "Servidor.h"
#include "bd.h"
#include "libro.h"
#include "prestamo.h"
#include "usuario.h"
#include "sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PUERTO 6000
#define MAX_CLIENTES 5
#define BUFFER_SIZE 2048

sqlite3 *db = NULL;

void devolverLibro(const char *parametros, int cliente_fd) {
    char dni[20];
    int id_prestamo;
    char respuesta[BUFFER_SIZE];

    sscanf(parametros, "%19[^|]|%d", dni, &id_prestamo);

    const char *sql_verificar = "SELECT libro_id FROM Prestamo WHERE id = ? AND usuario_dni = ? AND fecha_Devolucion IS NULL;";
    sqlite3_stmt *stmt;
    int libro_id = -1;

    if (sqlite3_prepare_v2(db, sql_verificar, -1, &stmt, NULL) == SQLITE_OK) {
        sqlite3_bind_int(stmt, 1, id_prestamo);
        sqlite3_bind_text(stmt, 2, dni, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            libro_id = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
    }

    if (libro_id == -1) {
        snprintf(respuesta, BUFFER_SIZE, "Préstamo no encontrado o ya devuelto.");
        send(cliente_fd, respuesta, strlen(respuesta), 0);
        return;
    }

    const char *sql_actualizar = "UPDATE Prestamo SET fecha_Devolucion = DATE('now') WHERE id = ?;";
    sqlite3_prepare_v2(db, sql_actualizar, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, id_prestamo);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    const char *sql_libro = "UPDATE Libro SET estado = 1 WHERE id = ?;";
    sqlite3_prepare_v2(db, sql_libro, -1, &stmt, NULL);
    sqlite3_bind_int(stmt, 1, libro_id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    snprintf(respuesta, BUFFER_SIZE, "Devolución registrada correctamente.");
    send(cliente_fd, respuesta, strlen(respuesta), 0);
}

void procesarComando(const char *comando, int cliente_fd) {
    const char *respuesta = NULL;

    if (strncmp(comando, "REGISTRAR_USUARIO|", 18) == 0) {
        registrarUsuarioDesdeComando(comando + 18, cliente_fd);
        return;
    } else if (strncmp(comando, "INICIAR_SESION|", 15) == 0) {
        iniciarSesionDesdeComando(comando + 15, cliente_fd);
        return;
    } else if (strcmp(comando, "VER_CATALOGO") == 0) {
        enviarCatalogo(cliente_fd);
        return;
    } else if (strncmp(comando, "SOLICITAR_PRESTAMO|", 19) == 0) {
        procesarPrestamo(comando + 19, cliente_fd);
        return;
    } else if (strncmp(comando, "DEVOLVER_LIBRO|", 15) == 0) {
        devolverLibro(comando + 15, cliente_fd);
        return;
    }

    respuesta = "Comando no reconocido";
    send(cliente_fd, respuesta, strlen(respuesta), 0);
}
