#include "menu.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <conio.h> // Para Windows (ocultar contraseña)
#else
#include <termios.h> // Para Linux/Mac
#include <unistd.h>
#endif
#include "usuario.h"

#define MAX 30 // Tamaño máximo para username y password

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

// Función para leer la contraseña sin mostrar caracteres en pantalla
void leerContrasena(char* password) {
    int i = 0;
    char ch;
    
    while (1) {
        #ifdef _WIN32
        ch = getch(); // Windows
        #else
        struct termios oldt, newt;
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~ECHO; // Desactiva la impresión de caracteres
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
        ch = getchar();
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt); // Restaura la configuración
        #endif
        
        if (ch == '\n' || ch == '\r') {
            password[i] = '\0'; // Finaliza la cadena
            break;
        } else if (ch == 8 || ch == 127) { // Manejo de backspace
            if (i > 0) {
                i--;
                printf("\b \b"); // Borra el último carácter en pantalla
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*"); // Muestra asterisco en pantalla
        }
    }
}

// Función para iniciar sesión
void iniciarSesion() {
    // Implementación pendiente
}

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
    
    // Guardar datos en el archivo
    fprintf(archivo, "%s|%s|%s|%s|%s|%s\n", 
            nuevoUsuario.nombre,
            nuevoUsuario.apellidos, 
            nuevoUsuario.dni, 
            nuevoUsuario.direccion,
            nuevoUsuario.email, 
            nuevoUsuario.telefono);
    
    fclose(archivo);
    
    printf("¡El usuario se ha registrado con éxito!\n");
}