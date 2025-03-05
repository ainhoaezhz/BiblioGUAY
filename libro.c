#include "libro.h"

void inicializarLibro(Libro *libro, const char *nombre, const char *autor,
		const char *genero, int id, int estado) {

	strncpy(libro->nombre, nombre, MAX_NOMBRE);
	libro->nombre[MAX_NOMBRE - 1] = '\0';

	strncpy(libro->autor, autor, MAX_AUTOR);
	libro->autor[MAX_AUTOR - 1] = '\0';

	strncpy(libro->genero, genero, MAX_GENERO);
	libro->genero[MAX_GENERO - 1] = '\0';

	libro->id = id;
	libro->estado = estado;

}

void mostrarLibro(const Libro *libro) {

	printf("ID: %d\n", libro->id);
	printf("Nombre: %s\n", libro->nombre);
	printf("Autor: %s\n", libro->autor);
	printf("Género: %s\n", libro->genero);
	printf("Estado: %s\n", libro->estado ? "Disponible" : "No disponible");

}

