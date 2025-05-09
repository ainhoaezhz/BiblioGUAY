#include "biblioteca.h"
#include <stdio.h>
#include "libro.h"

void consultaDireccionBiblioteca(Biblioteca b){
	printf("%s", b.nombre);
}

void inicializarBibloiteca(){
	printf("Inicializar biblioteca\n");
	printf("Biblioteca inicializada correctamente\n");
}

void cargarDatosDesdeArchivo(){
	FILE *pf = fopen("libros.txt", "r");
	if (pf != NULL) {
		char linea[512];
		numLibros = 0;

		while (fgets(linea, sizeof(linea), pf) && numLibros < MAX_LIBROS) {
			Libro l;
			if (sscanf(linea, "%199[^;];%99[^;];%49[^;];%d;%d\n",
			           l.nombre, l.autor, l.genero, &l.id, &l.estado) == 5) {
				listaLibros[numLibros++] = l;
			}
		}

		fclose(pf);
		printf("Se cargaron %d libros desde el archivo.\n", numLibros);
	} else {
		perror("Error al abrir el archivo libros.txt");
	}
}

void cerrarBibloiteca(){
	printf("cerrando biblioteca\n");
}
