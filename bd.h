#ifndef BD_H_
#define BD_H_

#include "sqlite3.h"  // Asegurar que está presente

#define NOMBRE_BD "Biblioteca.db"

// Declaración externa de la variable compartida
extern sqlite3 *db;

// Prototipos de funciones
int inicializarBD(sqlite3 **db);
void crearTablas(sqlite3 *db);
void cerrarBD(sqlite3 *db);
int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena);
#endif /* BD_H_ */
