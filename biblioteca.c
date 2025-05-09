#include "biblioteca.h"
#include <stdio.h>

void consultaDireccionBiblioteca(Biblioteca b){
	printf("%s", b.nombre);
}

void inicializarBibloiteca(){
	printf("Inicializar biblioteca");


	printf("Biblioteca inicializada correctamente");
}

void cargarDatosDesdeArchivo(){
    FILE* pf = fopen("libros.txt", "r");
    if (!pf) {
        perror("No se pudo abrir el fichero libros.txt");
        return;
    }

    char linea[256];
    while (fgets(linea, sizeof(linea), pf)) {
        Libro libro;
        char estadoStr[10];

        linea[strcspn(linea, "\n")] = 0;

        if (sscanf(linea, "%99[^|]|%99[^|]|%49[^|]|%9s", libro.nombre, libro.autor, libro.genero, estadoStr) == 4) {
            libro.estado = atoi(estadoStr);
            libro.id = -1;
            printf("Libro cargado desde archivo: %s - %s\n", libro.nombre, libro.autor);
        }
    }

    fclose(pf);
}

void cerrarBibloiteca(){
	printf("cerrando biblioteca");
}
