#include "menu.h"
#include "usuario.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h> // Para Windows (ocultar contraseña)
#else
#include <termios.h> // Para Linux/Mac
#include <unistd.h>
#endif

#define MAX 30 // Tamaño máximo para username y password
#define MAX_STR 100 // Tamaño máximo para strings largos como nombre, apellidos, etc.

// Función para leer la contraseña y mostrar asteriscos
void leerContrasena(char* password) {
    int i = 0;
    char ch;

    #ifdef _WIN32
    while (1) {
        ch = _getch();  // Lee un carácter sin mostrarlo
        if (ch == '\r' || ch == '\n') {  // Si es Enter
            password[i] = '\0';  // Termina la cadena
            break;
        } else if (ch == 8 || ch == 127) {  // Manejo de retroceso
            if (i > 0) {
                i--;
                printf("\b \b");  // Borra el último carácter
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");  // Muestra un asterisco
        }
    }
    #else
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);  // Obtener la configuración actual
    newt = oldt;
    newt.c_lflag &= ~ECHO;  // Desactiva el eco
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);  // Aplicar nueva configuración

    while (1) {
        ch = getchar();  // Leer un carácter sin mostrarlo
        if (ch == '\n' || ch == '\r') {  // Si es Enter
            password[i] = '\0';  // Termina la cadena
            break;
        } else if (ch == 8 || ch == 127) {  // Manejo de retroceso
            if (i > 0) {
                i--;
                printf("\b \b");  // Borra el último carácter
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");  // Muestra un asterisco
        }
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);  // Restaurar la configuración original
    #endif

    printf("\n");
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
    char usuario[MAX], contrasena[MAX];
    FILE *archivo = fopen("usuarios.txt", "r");
    
    if (archivo == NULL) {
        printf("Error: No hay usuarios registrados.\n");
        return;
    }

    printf("\nINICIAR SESION\n");
    printf("---\n");
    printf("Usuario: ");
    scanf("%29s", usuario);  // Limita la lectura para evitar desbordamiento
    while (getchar() != '\n');

    printf("Contrasena: ");
    leerContrasena(contrasena);
    while (getchar() != '\n');

    // Verificar credenciales en el archivo
    char linea[MAX_STR * 7];  // Ajusta según el tamaño máximo de una línea
    int encontrado = 0;
    
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
        printf("\n¡Inicio de sesión exitoso! Bienvenido, %s.\n", usuario);
        // Aquí podrías llamar a un menú de usuario/administrador
        // Ej: menuUsuario();
    } else {
        printf("\nError: Usuario o contraseña incorrectos.\n");
    }
}

// Función para registrar un nuevo usuario
void registrarse() {
    Usuario nuevoUsuario;
    FILE *archivo = fopen("usuarios.txt", "a");
    
    if (archivo == NULL) {
        printf("Error al abrir el archivo de usuarios.\n");
        return;
    }
    
    printf("\nREGISTRO DE USUARIO\n");
    printf("---------------------\n");
    printf("Nombre del usuario: ");
    fgets(nuevoUsuario.nombre, MAX_STR, stdin);
    strtok(nuevoUsuario.nombre, "\n");
    
    printf("Apellidos: ");
    fgets(nuevoUsuario.apellidos, MAX_STR, stdin);
    strtok(nuevoUsuario.apellidos, "\n");
    
    printf("DNI: ");
    fgets(nuevoUsuario.dni, MAX_STR, stdin);
    strtok(nuevoUsuario.dni, "\n");
    
    printf("Dirección: ");
    fgets(nuevoUsuario.direccion, MAX_STR, stdin);
    strtok(nuevoUsuario.direccion, "\n");
    
    printf("Email: ");
    fgets(nuevoUsuario.email, MAX_STR, stdin);
    strtok(nuevoUsuario.email, "\n");
    
    printf("Teléfono: ");
    fgets(nuevoUsuario.telefono, MAX_STR, stdin);
    strtok(nuevoUsuario.telefono, "\n");
    
    // Solicitar la contraseña con asteriscos
    printf("Contraseña: ");
    leerContrasena(nuevoUsuario.contrasena);
    
    // Guardar datos en el archivo
    fprintf(archivo, "%s|%s|%s|%s|%s|%s|%s\n", 
            nuevoUsuario.nombre,
            nuevoUsuario.apellidos, 
            nuevoUsuario.dni, 
            nuevoUsuario.direccion,
            nuevoUsuario.email, 
            nuevoUsuario.telefono,
            nuevoUsuario.contrasena);
    
    fclose(archivo);
    
    printf("¡El usuario se ha registrado con éxito!\n");
}
