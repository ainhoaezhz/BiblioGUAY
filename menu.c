//menu.c
#include "menu.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "bd.h"
#include "menuAdmin.h"
#include "libro.h"

#ifdef _WIN32
#include <conio.h> //Windows
#else
#include <termios.h> //Linux/Mac
#include <unistd.h>
#endif

#define MAX 80

void leerContrasena(char *password) {
    int i = 0;
    char ch;

#ifdef _WIN32
    while (1) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
            fflush(stdout);
        }
    }
#else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == EOF) {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {
            if (i > 0) {
                i--;
                printf("\b \b");
                fflush(stdout);
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
            fflush(stdout);
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif

    password[i] = '\0';
    printf("\nContraseña ingresada correctamente.\n");
    fflush(stdout);
}

int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena) {
    sqlite3_stmt *stmt;
    char sql[MAX];

    snprintf(sql, sizeof(sql),
             "SELECT 1 FROM Usuario WHERE nombre = ? AND contrasena = ?;");

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n",
                sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, contrasena, -1, SQLITE_STATIC);

    int autenticado = (sqlite3_step(stmt) == SQLITE_ROW);

    sqlite3_finalize(stmt);
    return autenticado;
}

int autenticarUsuario(sqlite3 *db, char *dni, int *esAdmin) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT es_Admin FROM Usuario WHERE nombre = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar consulta: %s\n", sqlite3_errmsg(db));
        return 0;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    int resultado = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        *esAdmin = sqlite3_column_int(stmt, 0);
        resultado = 1;
    } else {
        printf("Usuario no encontrado.\n");
    }

    sqlite3_finalize(stmt);
    return resultado;
}
