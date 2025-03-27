#include "menu.h"
#include "usuario.h"
#include "bd.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#ifdef _WIN32
#include <conio.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

#define MAX 80
#define MAX_STR 100

/* Función para validar DNI */
int validarDNI(const char *dni) {
    if (strlen(dni) != 9) {
        printf("El DNI debe tener 9 caracteres\n");
        return 0;
    }

    for (int i = 0; i < 8; i++) {
        if (!isdigit(dni[i])) {
            printf("Los primeros 8 caracteres deben ser números\n");
            return 0;
        }
    }

    if (!isalpha(dni[8])) {
        printf("El último carácter debe ser una letra\n");
        return 0;
    }

    return 1;
}

/* Función para leer contraseña sin mostrarla */
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
            }
        } else if (i < MAX - 1) {
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

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == EOF) {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
#endif
    printf("\n");
}

/* Menú principal */
char menuPrincipal() {
    char opcion;
    printf("\nBIBLIOTECA\n");
    printf("------------------\n");
    printf("1. Iniciar Sesion\n");
    printf("2. Registrarse\n");
    printf("0. Salir\n");
    printf("Elige una opcion: ");

    if (scanf(" %c", &opcion) != 1) {
        opcion = 'x'; // Carácter inválido
    }
    while (getchar() != '\n'); // Limpiar buffer

    return opcion;
}

/* Menú de usuario */
char menuUsuario() {
    char opcion;
    printf("\nMENU USUARIO\n");
    printf("1. Ver perfil\n");
    printf("2. Editar perfil\n");
    printf("3. Buscar libros\n");
    printf("4. Historial de prestamos\n");
    printf("5. Devolver libros\n");
    printf("0. Salir\n");
    printf("Elige una opcion: ");

    if (scanf(" %c", &opcion) != 1) {
        opcion = 'x';
    }
    while (getchar() != '\n');

    return opcion;
}

/* Función de registro */
int registrarse(sqlite3 *db, int esAdmin) {
    Usuario nuevoUsuario;
    char confirmacion[MAX];
    int intentos = 3;
    int c;

    // Limpiar buffer
    while ((c = getchar()) != '\n' && c != EOF);

    printf("\n--- FORMULARIO DE REGISTRO ---\n");

    // Nombre
    do {
        printf("\nNombre: ");
        if (fgets(nuevoUsuario.nombre, sizeof(nuevoUsuario.nombre), stdin) == NULL) {
            printf("Error al leer nombre\n");
            return -1;
        }
        nuevoUsuario.nombre[strcspn(nuevoUsuario.nombre, "\n")] = '\0';
    } while (strlen(nuevoUsuario.nombre) == 0);

    // Apellidos
    printf("Apellidos: ");
    if (fgets(nuevoUsuario.apellidos, sizeof(nuevoUsuario.apellidos), stdin) == NULL) {
        printf("Error al leer apellidos\n");
        return -1;
    }
    nuevoUsuario.apellidos[strcspn(nuevoUsuario.apellidos, "\n")] = '\0';

    // DNI
    do {
        printf("DNI (8 números + letra): ");
        if (fgets(nuevoUsuario.dni, sizeof(nuevoUsuario.dni), stdin) == NULL) {
            printf("Error al leer DNI\n");
            return -1;
        }
        nuevoUsuario.dni[strcspn(nuevoUsuario.dni, "\n")] = '\0';
    } while (!validarDNI(nuevoUsuario.dni));

    // Resto de campos
    printf("Dirección: ");
    fgets(nuevoUsuario.direccion, sizeof(nuevoUsuario.direccion), stdin);
    nuevoUsuario.direccion[strcspn(nuevoUsuario.direccion, "\n")] = '\0';

    printf("Email: ");
    fgets(nuevoUsuario.email, sizeof(nuevoUsuario.email), stdin);
    nuevoUsuario.email[strcspn(nuevoUsuario.email, "\n")] = '\0';

    printf("Teléfono: ");
    fgets(nuevoUsuario.telefono, sizeof(nuevoUsuario.telefono), stdin);
    nuevoUsuario.telefono[strcspn(nuevoUsuario.telefono, "\n")] = '\0';

    // Contraseña con confirmación
    do {
        printf("Contraseña: ");
        leerContrasena(nuevoUsuario.contrasena);

        printf("Confirmar contraseña: ");
        leerContrasena(confirmacion);

        if (strcmp(nuevoUsuario.contrasena, confirmacion) != 0) {
            printf("\nLas contraseñas no coinciden. Intentos restantes: %d\n", --intentos);
            if (intentos == 0) {
                printf("Demasiados intentos fallidos. Cancelando registro.\n");
                return -1;
            }
        } else {
            break;
        }
    } while (1);

    nuevoUsuario.es_Admin = esAdmin;

    // Insertar en la base de datos
    const char *sql = "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return -1;
    }

    sqlite3_bind_text(stmt, 1, nuevoUsuario.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nuevoUsuario.apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nuevoUsuario.dni, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, nuevoUsuario.direccion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, nuevoUsuario.email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, nuevoUsuario.telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, nuevoUsuario.contrasena, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 8, nuevoUsuario.es_Admin);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        fprintf(stderr, "Error al registrar usuario: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    printf("\n✅ Usuario registrado con éxito!\n");
    printf("Presiona Enter para continuar...");
    while ((c = getchar()) != '\n' && c != EOF);

    return 0;
}

/* Menú de registro */
void menuRegistro(sqlite3 *db) {
    int opcion;
    char input[10];
    int c;

    do {
        printf("\nREGISTRO DE USUARIO\n");
        printf("Tipo de usuario:\n");
        printf("1. Administrador\n");
        printf("2. Usuario Normal\n");
        printf("0. Volver al menu principal\n");
        printf("Elige una opcion: ");

        if (fgets(input, sizeof(input), stdin) == NULL) {
            printf("Error al leer la opción\n");
            continue;
        }

        if (sscanf(input, "%d", &opcion) != 1) {
            printf("Entrada no válida. Introduce un número.\n");
            continue;
        }

        // Limpiar buffer si la entrada fue más larga de lo esperado
        if (strchr(input, '\n') == NULL) {
            while ((c = getchar()) != '\n' && c != EOF);
        }

        switch (opcion) {
            case 1:
                printf("\nRegistrando Administrador...\n");
                if (registrarse(db, 1) == 0) {
                    return;  // Salir completamente si el registro fue exitoso
                }
                break;  // Solo continuar si hubo error en el registro
            case 2:
                printf("\nRegistrando Usuario Normal...\n");
                if (registrarse(db, 0) == 0) {
                    return;  // Salir completamente si el registro fue exitoso
                }
                break;  // Solo continuar si hubo error en el registro
            case 0:
                printf("\nVolviendo al menú principal...\n");
                return;  // Salir para volver al menú principal
            default:
                printf("Opción inválida. Inténtalo de nuevo.\n");
        }
    } while (1);  // Bucle infinito (se sale con returns)
}
/* Iniciar sesión */
void iniciarSesion(sqlite3 *db) {
    char usuario[MAX], contrasena[MAX];
    int c;

    do {
        printf("\nINICIAR SESIÓN\n");
        printf("Usuario: ");
        if (fgets(usuario, sizeof(usuario), stdin) == NULL) {
            printf("Error al leer usuario\n");
            continue;
        }
        usuario[strcspn(usuario, "\n")] = '\0';

        printf("Contraseña: ");
        leerContrasena(contrasena);

        if (verificarSesion(db, usuario, contrasena)) {
            printf("\n¡Bienvenido, %s!\n", usuario);
            break;
        } else {
            printf("\nUsuario o contraseña incorrectos\n");
        }
    } while (1);

    // Menú después de iniciar sesión
    char opcion;
    do {
        opcion = menuUsuario();
        switch (opcion) {
            case '1':
                printf("Viendo perfil...\n");
                break;
            case '2':
                printf("Editando perfil...\n");
                break;
            case '3':
                printf("Buscando libros...\n");
                break;
            case '4':
                printf("Mostrando historial...\n");
                break;
            case '5':
                printf("Devolviendo libros...\n");
                break;
            case '0':
                printf("Cerrando sesión...\n");
                break;
            default:
                printf("Opción no válida\n");
        }
    } while (opcion != '0');
}
