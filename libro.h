#ifndef LIBRO_H
#define LIBRO_H

#include <stdio.h>
#include <string.h>
#define MAX_NOMBRE 200
#define MAX_AUTOR 100
#define MAX_GENERO 50

// Definición de la estructura Libro

typedef struct {

	char nombre[MAX_NOMBRE];
	char autor[MAX_AUTOR];
	char genero[MAX_GENERO];
	int id;
	int estado; // 1: No disponible, 0: Disponible

} Libro;

// Funciones asociadas a Libro

void inicializarLibro(Libro *libro, const char *nombre, const char *autor,
		const char *genero, int id, int estado);

void mostrarLibro(const Libro *libro);

#endif // LIBRO_H
