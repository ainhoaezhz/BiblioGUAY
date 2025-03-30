#include "menu.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "sqlite3.h"
#include "bd.h"
#include "menuAdmin.h"

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

/* ================== FUNCIÓN LEER CONTRASEÑA ================== */
void leerContrasena(char *password) {
    int i = 0;
    char ch;

    #ifdef _WIN32
    while (i < MAX - 1) {
        ch = _getch();
        if (ch == '\r' || ch == '\n') {
            break;
        } else if (ch == '\b') {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (isprint(ch)) {
            password[i++] = ch;
            printf("*");
        }
    }
    #else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    while (i < MAX - 1 && read(STDIN_FILENO, &ch, 1) == 1 && ch != '\n') {
        if (ch == '\b' || ch == 127) {
            if (i > 0) {
                i--;
                write(STDOUT_FILENO, "\b \b", 3);
            }
        } else if (isprint(ch)) {
            password[i++] = ch;
            write(STDOUT_FILENO, "*", 1);
        }
    }
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    #endif

    password[i] = '\0';
    printf("\n");
}

/* ================== FUNCIONES DE MENÚ ================== */
char menuPrincipal() {
    char opcion;
    printf("\n=== BIBLIOTECA ===\n");
    printf("1. Iniciar Sesion\n");
    printf("2. Registrarse\n");
    printf("0. Salir\n");
    printf("Seleccione: ");
    fflush(stdout);

    scanf(" %c", &opcion);
    while (getchar() != '\n');

    return opcion;
}

char menuRegistro() {
    char opcion;
    printf("\n=== TIPO DE USUARIO ===\n");
    printf("1. Administrador\n");
    printf("2. Usuario Normal\n");
    printf("0. Volver\n");
    printf("Seleccione: ");
    fflush(stdout);

    scanf(" %c", &opcion);
    // Limpiar el buffer de entrada completamente
    int c;
    while ((c = getchar()) != '\n' && c != EOF);

    return opcion;
}

char menuUsuario() {
    char opcion;
    printf("\n=== MENU USUARIO ===\n");
    printf("1. Ver perfil\n");
    printf("2. Editar perfil\n");
    printf("3. Buscar libros\n");
    printf("4. Historial de prestamos\n");
    printf("5. Devolver libros\n");
    printf("0. Cerrar sesion\n");
    printf("Seleccione: ");
    fflush(stdout);

    scanf(" %c", &opcion);
    while (getchar() != '\n');

    return opcion;
}

/* ================== FUNCIÓN INICIAR SESIÓN ================== */
void iniciarSesion() {
    char usuario[MAX], contrasena[MAX];
    int intentos = 0;
    const int MAX_INTENTOS = 3;

    do {
        printf("\n=== INICIAR SESION ===\n");

        memset(usuario, 0, sizeof(usuario));
        memset(contrasena, 0, sizeof(contrasena));

        printf("Usuario: ");
        fflush(stdout);
        if (!fgets(usuario, sizeof(usuario), stdin)) continue;
        usuario[strcspn(usuario, "\n")] = '\0';

        printf("Contraseña: ");
        fflush(stdout);
        leerContrasena(contrasena);

        sqlite3_stmt *stmt;
        const char *sql = "SELECT es_Admin FROM Usuario WHERE nombre = ? AND contrasena = ?";

        if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
            fprintf(stderr, "Error SQL: %s\n", sqlite3_errmsg(db));
            continue;
        }

        sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, contrasena, -1, SQLITE_STATIC);

        if (sqlite3_step(stmt) == SQLITE_ROW) {
            int esAdmin = sqlite3_column_int(stmt, 0);
            sqlite3_finalize(stmt);

            printf("\nBienvenido %s!\n", usuario);
            if (esAdmin) {
                ejecutarMenuAdmin(db);
            } else {
                menuUsuarioNormal(db, usuario);
            }
            return;
        }

        sqlite3_finalize(stmt);
        intentos++;
        printf("\nCredenciales incorrectas. Intentos: %d/%d\n", intentos, MAX_INTENTOS);

    } while (intentos < MAX_INTENTOS);

    printf("\nDemasiados intentos fallidos.\n");
}

/* ================== FUNCIÓN REGISTRAR USUARIO ================== */
void registrarUsuario(sqlite3 *db, int esAdmin) {
    Usuario u;
    memset(&u, 0, sizeof(Usuario));

    printf("\n=== REGISTRO DE %s ===\n", esAdmin ? "ADMINISTRADOR" : "USUARIO NORMAL");

    // Nombre
    printf("Nombre: ");
    fflush(stdout);
    if (!fgets(u.nombre, MAX_STR, stdin)) {
        printf("Error al leer el nombre\n");
        return;
    }
    u.nombre[strcspn(u.nombre, "\n")] = '\0';

    // Apellidos
    printf("Apellidos: ");
    fflush(stdout);
    if (!fgets(u.apellidos, MAX_STR, stdin)) {
        printf("Error al leer los apellidos\n");
        return;
    }
    u.apellidos[strcspn(u.apellidos, "\n")] = '\0';

    // DNI
    printf("DNI: ");
    fflush(stdout);
    if (!fgets(u.dni, MAX_STR, stdin)) {
        printf("Error al leer el DNI\n");
        return;
    }
    u.dni[strcspn(u.dni, "\n")] = '\0';

    // Dirección
    printf("Direccion: ");
    fflush(stdout);
    if (!fgets(u.direccion, MAX_STR, stdin)) {
        printf("Error al leer la dirección\n");
        return;
    }
    u.direccion[strcspn(u.direccion, "\n")] = '\0';

    // Email
    printf("Email: ");
    fflush(stdout);
    if (!fgets(u.email, MAX_STR, stdin)) {
        printf("Error al leer el email\n");
        return;
    }
    u.email[strcspn(u.email, "\n")] = '\0';

    // Teléfono
    printf("Telefono: ");
    fflush(stdout);
    if (!fgets(u.telefono, MAX_STR, stdin)) {
        printf("Error al leer el teléfono\n");
        return;
    }
    u.telefono[strcspn(u.telefono, "\n")] = '\0';

    // Contraseña
    printf("Contraseña: ");
    fflush(stdout);
    leerContrasena(u.contrasena);

    u.es_Admin = esAdmin;

    // Validación básica de campos obligatorios
    if (strlen(u.nombre) == 0 || strlen(u.contrasena) == 0 || strlen(u.dni) == 0) {
        printf("\nERROR: Nombre, DNI y contraseña son campos obligatorios\n");
        return;
    }

    // Insertar en la base de datos
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("\nError al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    // Bind parameters
    sqlite3_bind_text(stmt, 1, u.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, u.apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, u.dni, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, u.direccion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, u.email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, u.telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, u.contrasena, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, u.es_Admin);

    // Execute statement
    int result = sqlite3_step(stmt);
    if (result == SQLITE_DONE) {
        printf("\nRegistro exitoso! Bienvenido %s\n", u.nombre);
    } else {
        printf("\nError en el registro (Código %d): %s\n", result, sqlite3_errmsg(db));

        // Verificar si es error de duplicado (DNI o nombre de usuario)
        if (result == SQLITE_CONSTRAINT) {
            printf("Posible error: DNI o nombre de usuario ya existente\n");
        }
    }

    sqlite3_finalize(stmt);
}

/* ================== MENU USUARIO NORMAL ================== */
void menuUsuarioNormal(sqlite3 *db, const char *usuario) {
    char opcion;
    char input[MAX_STR];

    do {
        opcion = menuUsuario();

        switch(opcion) {
            case '1':
                mostrarUsuario(db, usuario);
                break;

            case '2':
                editarUsuario(db, (char *)usuario);
                break;

            case '3': {
                printf("Titulo a buscar: ");
                fflush(stdout);
                if (fgets(input, sizeof(input), stdin)) {
                    input[strcspn(input, "\n")] = '\0';
                    buscar_libros_por_titulo(db, input);
                }
                break;
            }

            case '4': {
                sqlite3_stmt *stmt;
                const char *sql = "SELECT dni FROM Usuario WHERE nombre = ?";
                if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
                    sqlite3_bind_text(stmt, 1, usuario, -1, SQLITE_STATIC);
                    if (sqlite3_step(stmt) == SQLITE_ROW) {
                        visualizarHistorial(db, (const char *)sqlite3_column_text(stmt, 0));
                    }
                    sqlite3_finalize(stmt);
                }
                break;
            }

            case '5':
                printf("Devolución de libros (no implementado)\n");
                break;

            case '0':
                printf("Cerrando sesion...\n");
                break;

            default:
                printf("Opcion no valida\n");
        }

    } while (opcion != '0');
}
