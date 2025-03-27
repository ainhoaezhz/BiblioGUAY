#include "menu.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"

#ifdef _WIN32
#include <conio.h> // Para Windows (ocultar contraseña)
#else
#include <termios.h> // Para Linux/Mac
#include <unistd.h>
#endif

#define MAX 30 // Tamaño máximo para username y password
#define MAX_STR 100 // Tamaño máximo para strings largos como nombre, apellidos, etc.

// Función para leer la contraseña y mostrar asteriscos
// Función para leer la contraseña y mostrar asteriscos
void leerContrasena(char* password) {
    int i = 0;
    char ch;

#ifdef _WIN32
    while (1) {
        ch = _getch();  // Lee un carácter sin mostrarlo
        if (ch == '\r' || ch == '\n') {  // Si es Enter
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {  // Manejo de retroceso
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
    tcgetattr(STDIN_FILENO, &oldt); // Obtener atributos actuales del terminal
    newt = oldt;
    newt.c_lflag &= ~(ECHO); // Desactivar eco de la entrada
    tcsetattr(STDIN_FILENO, TCSANOW, &newt); // Aplicar cambios

    while (1) {
        ch = getchar();
        if (ch == '\n' || ch == EOF) {
            password[i] = '\0';
            break;
        } else if (ch == 8 || ch == 127) {  // Manejo de retroceso
            if (i > 0) {
                i--;
                printf("\b \b");
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restaurar configuración original
#endif
}


char menuPrincipal() {
    char opcion;
    
    printf("BIBLIOTECA\n");
    printf("------------------\n");
    printf("Bienvenido\n");
    printf("1. Iniciar Sesion\n");
    printf("2. Registrarse\n");
    printf("0. Salir\n");
    printf("Elige una opcion: ");
    fflush(stdout);
    
    scanf(" %c", &opcion); // Espacio antes de %c para evitar problemas con '\n'
    while (getchar() != '\n'); // Limpiar el buffer de entrada
    
    return opcion;
}

char menuRegistro() {
    char opcionRegistro;
    
    printf("REGISTRO DE USUARIO\n");
    printf("Tipo de usuario: \n");
    printf("1. Administrador\n");
    printf("2. Usuario Normal\n");
    printf("0. Volver al menu principal\n");
    printf("Elige una opcion: ");
    fflush(stdout);
    
    scanf(" %c", &opcionRegistro);
    while (getchar() != '\n'); // Limpiar buffer de entrada
    
    return opcionRegistro;
}

// Función para iniciar sesión
void iniciarSesion() {
    char usuario[MAX], contrasena[MAX], opcionMenu;
    FILE *archivo = fopen("usuarios.txt", "r");
    
    if (archivo == NULL) {
        printf("Error: No hay usuarios registrados.\n");
        return;
    }

    printf("\nINICIAR SESION\n");
    printf("---\n");
    printf("Usuario: ");
    fflush(stdout);
    scanf("%29s", usuario);  // Limita la lectura para evitar desbordamiento
    while (getchar() != '\n');

    printf("Contrasena: ");
    fflush(stdout);
    leerContrasena(contrasena);
    while (getchar() != '\n');

    menuUsuario();


    // Verificar credenciales en el archivo
    char linea[MAX_STR * 7];  // Ajusta según el tamaño máximo de una línea
    int encontrado = 1;
    
    while (fgets(linea, sizeof(linea), archivo)) {
        char *token = strtok(linea, "|");
        char *campos[7];  // Para almacenar los campos del usuario (nombre, apellidos, dni, etc.)
        int i = 0;
        
        while (token != NULL && i < 7) {
            campos[i++] = token;
            token = strtok(NULL, "|");
        }
        
        // Comparar usuario y contraseña (asumiendo que el email es el "usuario")
        if (strcmp(campos[4], usuario) == 0 && strcmp(campos[6], contrasena) == 0) {
            encontrado = 1;
            break;
        }
    }

    fclose(archivo);

    if (encontrado) {
    	do {
    		printf("\n¡Inicio de sesión exitoso! Bienvenido, %s.\n", usuario);
    		Usuario usuarioActual;
    		//CARGAR USUARIO DESDE LA BD
    		opcionMenu = menuUsuario();
    		switch (opcionMenu) {
    		case '1':
    			printf("Viendo perfil...\n");
    			mostrarUsuario(&usuarioActual);
    			fflush(stdout);
    			break;
    		case '2':
    			printf("Editando perfil...\n");
    			editarUsuario(&usuarioActual);
    			//Guardar cambios en la BD
    			fflush(stdout);
    			break;
    		case '3':
    			printf("Buscando libros...\n");
    			fflush(stdout);
    			break;
    		case '4':
    		    printf("Historial de prestamos...\n");
    		    fflush(stdout);
    		    break;
    		case '5':
    		    printf("Devolviendo libros...\n");
    		    fflush(stdout);
    		    break;
    		default:
    			printf("ERROR! Opción incorrecta\n");
    			fflush(stdout);
    		}
    	} while(opcionMenu != 0);
    }
}
char menuUsuario() {
	char opcionMenu;
	printf("----------\n");
	printf("USUARIO: MENU\n");
	printf("1. Ver perfil\n");
	printf("2. Editar perfil\n");
	printf("3. Buscar libros\n");
	printf("4. Historial de prestamos\n");
	printf("5. Devolver libros\n");
	printf("0. Salir\n");
	printf("Elige una opcion: ");
	fflush(stdout);

	scanf(" %c", &opcionMenu);

	while (getchar() != '\n');

	return opcionMenu;
}
// Función para registrar un nuevo usuario

void registrarse(sqlite3 *db) {
    Usuario nuevoUsuario;
    printf("\nREGISTRO DE USUARIO\n");
    printf("----------------------\n");

    printf("Nombre: ");
    scanf("%99s", nuevoUsuario.nombre);
    while (getchar() != '\n');

    printf("Apellidos: ");
    scanf("%99s", nuevoUsuario.apellidos);
    while (getchar() != '\n');

    printf("DNI: ");
    scanf("%19s", nuevoUsuario.dni);
    while (getchar() != '\n');

    printf("Direccion: ");
    scanf("%99s", nuevoUsuario.direccion);
    while (getchar() != '\n');

    printf("Email: ");
    scanf("%99s", nuevoUsuario.email);
    while (getchar() != '\n');

    printf("Telefono: ");
    scanf("%14s", nuevoUsuario.telefono);
    while (getchar() != '\n');

    printf("Contraseña: ");
    leerContrasena(nuevoUsuario.contrasena);

    printf("\u00bfEs administrador? (1: S\u00ed, 0: No): ");
    scanf("%d", &nuevoUsuario.es_Admin);
    while (getchar() != '\n');
    printf("Usuario registrado exitosamente\n");
}

