/*
 * biblioteca.h
 *
 *  Created on: 5 mar 2025
 *      Author: Genesis
 */

#ifndef BIBLIOTECA_H_
#define BIBLIOTECA_H_

typedef struct{
	int cod;
	char nombre[20];
	long telefono;
	char direccion[50];
	int cond_ciu;
}Biblioteca;

//Funciones biblioteca
void consultaDireccionBiblioteca(Biblioteca b);
void inicializarBibloiteca();
void cargarDatosDesdeArchivo();
void cerrarBibloiteca();


#endif /* BIBLIOTECA_H_ */
