// servidor.c (continuación con préstamos e informes)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sqlite3.h>
#include "bd.h"
#include "servidor.h"

#define PUERTO 6000
#define MAX_CLIENTES 5
#define BUFFER_SIZE 2048

sqlite3 *db = NULL;

void enviarRespuesta(int cliente_fd, const char *mensaje) {
    send(cliente_fd, mensaje, strlen(mensaje), 0);
}

// ------------------- PRÉSTAMOS -------------------
void solicitarPrestamo(const char *param, int cliente_fd) {
    int libro_id;
    char dni[20], fecha[20];
    sscanf(param, "%d|%19[^|]|%19s", &libro_id, dni, fecha);

    char sql1[128];
    snprintf(sql1, sizeof(sql1), "UPDATE Libro SET estado=0 WHERE id=%d;", libro_id);
    char sql2[256];
    snprintf(sql2, sizeof(sql2), "INSERT INTO Prestamo (dni_usuario, id_libro, fecha_inicio, devuelto) VALUES ('%s', %d, '%s', 0);", dni, libro_id, fecha);

    if (sqlite3_exec(db, sql1, NULL, NULL, NULL) == SQLITE_OK &&
        sqlite3_exec(db, sql2, NULL, NULL, NULL) == SQLITE_OK)
        enviarRespuesta(cliente_fd, "PRESTAMO_REALIZADO");
    else
        enviarRespuesta(cliente_fd, "ERROR_PRESTAMO");
}

void devolverLibro(const char *param, int cliente_fd) {
    int id_prestamo;
    char fecha[20];
    sscanf(param, "%d|%19s", &id_prestamo, fecha);

    char sql1[256], sql2[128];
    snprintf(sql1, sizeof(sql1), "UPDATE Prestamo SET fecha_fin='%s', devuelto=1 WHERE id=%d;", fecha, id_prestamo);
    snprintf(sql2, sizeof(sql2), "UPDATE Libro SET estado=1 WHERE id=(SELECT id_libro FROM Prestamo WHERE id=%d);", id_prestamo);

    if (sqlite3_exec(db, sql1, NULL, NULL, NULL) == SQLITE_OK &&
        sqlite3_exec(db, sql2, NULL, NULL, NULL) == SQLITE_OK)
        enviarRespuesta(cliente_fd, "LIBRO_DEVUELTO");
    else
        enviarRespuesta(cliente_fd, "ERROR_DEVOLUCION");
}

void verHistorialPrestamos(const char *dni, int cliente_fd) {
    char sql[256];
    snprintf(sql, sizeof(sql), "SELECT id_libro, fecha_inicio, fecha_fin FROM Prestamo WHERE dni_usuario='%s';", dni);
    sqlite3_stmt *stmt;
    char respuesta[BUFFER_SIZE] = "HISTORIAL\n";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            char linea[256];
            snprintf(linea, sizeof(linea), "Libro ID: %d | Inicio: %s | Fin: %s\n",
                     sqlite3_column_int(stmt, 0),
                     sqlite3_column_text(stmt, 1),
                     sqlite3_column_text(stmt, 2));
            strcat(respuesta, linea);
        }
    }
    sqlite3_finalize(stmt);
    enviarRespuesta(cliente_fd, respuesta);
}

void prestamosActivos(int cliente_fd) {
    const char *sql = "SELECT dni_usuario, id_libro, fecha_inicio FROM Prestamo WHERE devuelto=0;";
    sqlite3_stmt *stmt;
    char respuesta[BUFFER_SIZE] = "ACTIVOS\n";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            char linea[256];
            snprintf(linea, sizeof(linea), "Usuario: %s | Libro ID: %d | Inicio: %s\n",
                     sqlite3_column_text(stmt, 0),
                     sqlite3_column_int(stmt, 1),
                     sqlite3_column_text(stmt, 2));
            strcat(respuesta, linea);
        }
    }
    sqlite3_finalize(stmt);
    enviarRespuesta(cliente_fd, respuesta);
}

// ------------------- INFORMES -------------------
void verEstadisticas(int cliente_fd) {
    const char *sql = "SELECT COUNT(*) FROM Prestamo;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        char respuesta[64];
        snprintf(respuesta, sizeof(respuesta), "TOTAL_PRESTAMOS: %d", sqlite3_column_int(stmt, 0));
        enviarRespuesta(cliente_fd, respuesta);
    } else enviarRespuesta(cliente_fd, "ERROR_ESTADISTICAS");
    sqlite3_finalize(stmt);
}

void libroMasPrestado(int cliente_fd) {
    const char *sql = "SELECT id_libro, COUNT(*) as total FROM Prestamo GROUP BY id_libro ORDER BY total DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        char respuesta[64];
        snprintf(respuesta, sizeof(respuesta), "LIBRO_MAS_PRESTADO: %d", sqlite3_column_int(stmt, 0));
        enviarRespuesta(cliente_fd, respuesta);
    } else enviarRespuesta(cliente_fd, "NO_DATOS");
    sqlite3_finalize(stmt);
}

void usuarioMasPrestamos(int cliente_fd) {
    const char *sql = "SELECT dni_usuario, COUNT(*) as total FROM Prestamo GROUP BY dni_usuario ORDER BY total DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK && sqlite3_step(stmt) == SQLITE_ROW) {
        char respuesta[64];
        snprintf(respuesta, sizeof(respuesta), "USUARIO_TOP: %s", sqlite3_column_text(stmt, 0));
        enviarRespuesta(cliente_fd, respuesta);
    } else enviarRespuesta(cliente_fd, "NO_DATOS");
    sqlite3_finalize(stmt);
}

void prestamosVencidos(int cliente_fd) {
    const char *sql = "SELECT id, id_libro, fecha_inicio FROM Prestamo WHERE devuelto=0 AND DATE(fecha_inicio,'+15 days') < DATE('now');";
    sqlite3_stmt *stmt;
    char respuesta[BUFFER_SIZE] = "VENCIDOS\n";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            char linea[128];
            snprintf(linea, sizeof(linea), "ID: %d | Libro ID: %d | Fecha: %s\n",
                     sqlite3_column_int(stmt, 0),
                     sqlite3_column_int(stmt, 1),
                     sqlite3_column_text(stmt, 2));
            strcat(respuesta, linea);
        }
    }
    sqlite3_finalize(stmt);
    enviarRespuesta(cliente_fd, respuesta);
}

void procesarComando(const char *comando, int cliente_fd) {
    if (strncmp(comando, "SOLICITAR_PRESTAMO", 18) == 0) {
        solicitarPrestamo(comando + 19, cliente_fd);
    } else if (strncmp(comando, "DEVOLVER_LIBRO", 14) == 0) {
        devolverLibro(comando + 15, cliente_fd);
    } else if (strncmp(comando, "HISTORIAL_PRESTAMOS", 20) == 0) {
        verHistorialPrestamos(comando + 21, cliente_fd);
    } else if (strncmp(comando, "PRESTAMOS_ACTIVOS", 18) == 0) {
        prestamosActivos(cliente_fd);
    } else if (strncmp(comando, "ESTADISTICAS", 12) == 0) {
        verEstadisticas(cliente_fd);
    } else if (strncmp(comando, "LIBRO_MAS_PRESTADO", 19) == 0) {
        libroMasPrestado(cliente_fd);
    } else if (strncmp(comando, "USUARIO_MAS_PRESTAMOS", 22) == 0) {
        usuarioMasPrestamos(cliente_fd);
    } else if (strncmp(comando, "PRESTAMOS_VENCIDOS", 19) == 0) {
        prestamosVencidos(cliente_fd);
    } else {
        enviarRespuesta(cliente_fd, "COMANDO_NO_RECONOCIDO");
    }
}


int main() {
    if (inicializarBD(&db) != SQLITE_OK) {
        printf("Error al abrir la base de datos\n");
        return 1;
    }

    crearTablas(db);

    int servidor_fd, cliente_fd;
    struct sockaddr_in servidor_addr, cliente_addr;
    socklen_t cliente_len = sizeof(cliente_addr);
    char buffer[BUFFER_SIZE];

    servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    servidor_addr.sin_family = AF_INET;
    servidor_addr.sin_addr.s_addr = INADDR_ANY;
    servidor_addr.sin_port = htons(PUERTO);

    bind(servidor_fd, (struct sockaddr*)&servidor_addr, sizeof(servidor_addr));
    listen(servidor_fd, MAX_CLIENTES);

    printf("Servidor escuchando en el puerto %d...\n", PUERTO);

    while (1) {
        cliente_fd = accept(servidor_fd, (struct sockaddr*)&cliente_addr, &cliente_len);
        int bytes = recv(cliente_fd, buffer, BUFFER_SIZE - 1, 0);
        if (bytes > 0) {
            buffer[bytes] = '\0';
            procesarComando(buffer, cliente_fd);
        }
        close(cliente_fd);
    }

    sqlite3_close(db);
    return 0;
}

