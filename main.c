#include "biblioteca.c"
#include "menu.c"
#include "libro.c"
#include <stdio.c>

int main(){
	char opcion;
	Biblioteca b;
//	Libro l;

	do {
			opcion = menuPrincipal();
			switch (opcion) {
			case '1':
				consultaDireccionBiblioteca();
				printf("Se ha añadido correctamente\n");
				fflush(stdout);
				break;
			case '2':

				break;
			case '3':

				break;
			case '4':

				break;
			case '0':
				printf("Hasta la próxima\n");
				fflush(stdout);
				break;
			default:
				printf("ERROR! La opción seleccionada no es correcta\n");
				fflush(stdout);
			}
		} while (opcion != '0');
	//	liberaMemoria(&v);
		return 0;
}


