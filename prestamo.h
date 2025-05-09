#ifndef PRESTAMO_H
#define PRESTAMO_H
typedef struct {
    char usuario_dni[20];
    int libro_id;
    char fecha_prestamo[11];
    char fecha_devolucion[11];
} Prestamo;
#endif
