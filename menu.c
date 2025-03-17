#include "menu.h"
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
    #include <conio.h>  // Para Windows (ocultar contraseña)
#else
    #include <termios.h> // Para Linux/Mac
    #include <unistd.h>
#endif

#define MAX 30  // Tamaño máximo para username y password

void leerContrasenia(char* password);
char menuPrincipal() {
    char opcion;
    printf("BIBLIOTECA\n");
    printf("------------------\n");
    printf("Bienvenido\n");
    printf("1. Iniciar Sesión\n");
    printf("2. Registrarse\n");
    printf("0. Salir\n");
    printf("Elige una opción: ");
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
    printf("0. Volver al menú principal\n");
    printf("Elige una opción: ");
    fflush(stdout);

    scanf(" %c", &opcionRegistro);
    while (getchar() != '\n'); // Limpiar buffer de entrada
    return opcionRegistro;
}



// Función para leer la contraseña sin mostrar caracteres en pantalla
void leerContrasena(char *password) {
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
            password[i] = '\0';  // Finaliza la cadena
            break;
        } else if (ch == 8 || ch == 127) { // Manejo de backspace
            if (i > 0) {
                i--;
                printf("\b \b"); // Borra el último carácter en pantalla
            }
        } else if (i < MAX - 1) {
            password[i++] = ch;
            printf("*");  // Muestra asterisco en pantalla
        }
    }
}

// Función para iniciar sesión
void iniciarSesion() {

}

