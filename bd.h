#ifndef BD_H_
#define BD_H_
#include "sqlite3.h"
#define NOMBRE_BD "Biblioteca.db"

extern sqlite3 *db;

int inicializarBD(sqlite3 **db);
void crearTablas(sqlite3 *db);
int verificarSesion(sqlite3 *db, const char *usuario, const char *contrasena);

//void volcarFicheroV2ALaBBDD(char *nomfich, sqlite3 *db);
#endif
