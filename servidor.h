// servidor.h
#ifndef SERVIDOR_H
#define SERVIDOR_H

#include <sqlite3.h>

void enviarRespuesta(int cliente_fd, const char *mensaje);

// Usuarios
void registrarUsuario(const char *param, int cliente_fd);
void iniciarSesion(const char *param, int cliente_fd);
void verPerfil(const char *dni, int cliente_fd);
void editarPerfil(const char *param, int cliente_fd);
void eliminarUsuario(const char *dni, int cliente_fd);

// Libros
void verCatalogo(int cliente_fd);
void anadirLibro(const char *param, int cliente_fd);
void editarLibro(const char *param, int cliente_fd);
void eliminarLibro(const char *param, int cliente_fd);
void buscarLibroPorTitulo(const char *titulo, int cliente_fd);
void listarLibrosDisponibles(int cliente_fd);

// Préstamos y devoluciones
void solicitarPrestamo(const char *param, int cliente_fd);
void devolverLibro(const char *param, int cliente_fd);
void verHistorialPrestamos(const char *dni, int cliente_fd);
void prestamosActivos(int cliente_fd);

// Informes (admin)
void verEstadisticas(int cliente_fd);
void libroMasPrestado(int cliente_fd);
void usuarioMasPrestamos(int cliente_fd);
void prestamosVencidos(int cliente_fd);

// Procesamiento general
void procesarComando(const char *comando, int cliente_fd);

#endif
