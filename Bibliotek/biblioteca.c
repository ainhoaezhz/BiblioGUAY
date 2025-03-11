/*
 * biblioteca.c
 *
 *  Created on: 5 mar 2025
 *      Author: Genesis
 */
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
	FILE*pf;
	pf=fopen("libros.txt", "r");
	if(pf!=(FILE*)NULL){
		 //SEGUIR LUEGO
	}
}

void cerrarBibloiteca(){
	printf("cerrando biblioteca");
}


