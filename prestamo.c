#include <stdio.h>
#include <string.h>
#include "prestamo.h"

void mostrarPrestamo(const Prestamo *p) {
    printf("Usuario DNI: %s\n", p->usuario_dni);
    printf("ID Libro: %d\n", p->libro_id);
    printf("Fecha Préstamo: %s\n", p->fecha_prestamo);
    printf("Fecha Devolución: %s\n", p->fecha_devolucion);
}

void guardarPrestamo(FILE *f, const Prestamo *p) {
    fwrite(p, sizeof(Prestamo), 1, f);
}

int cargarPrestamo(FILE *f, Prestamo *p) {
    return fread(p, sizeof(Prestamo), 1, f);
}

int compararPrestamo(const Prestamo *a, const Prestamo *b) {
    return strcmp(a->usuario_dni, b->usuario_dni) == 0 && a->libro_id == b->libro_id;
}
