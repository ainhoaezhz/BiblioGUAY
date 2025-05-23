#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "usuario.h"
#include "bd.h"
#include "menu.h"
#include "libro.h"
#include "sqlite3.h"

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 6082

#define MAX 80


//admin
void listarLibrosSocket(sqlite3 *db, int socket);
void agregarLibroSocket(sqlite3 *db, int socket, char *datos);
void editarLibroSocket(sqlite3 *db, int socket, char *datos);
void eliminarLibroSocket(sqlite3 *db, int socket, char *datos);

void registrarNuevoUsuarioSocket(sqlite3 *db, int socket, char *datos);
void editarDatosUsuarioSocket(sqlite3 *db, int socket, char *datos);
void eliminarUsuarioSocket(sqlite3 *db, int socket, char *datos);

void registrarPrestamoSocket(sqlite3 *db, int socket, char *datos);
void registrarDevolucionSocket(sqlite3 *db, int socket, char *datos);
void mostrarPrestamosActivosSocket(sqlite3 *db, int socket);

void verEstadisticasSocket(sqlite3 *db, int socket);
void mostrarUsuarioConMasPrestamosSocket(sqlite3 *db, int socket);
void mostrarLibroMasPrestadoSocket(sqlite3 *db, int socket);
void mostrarPrestamosVencidosSocket(sqlite3 *db, int socket);

//usuario
void verPerfilSocket(sqlite3 *db, int socket, char *usuario);
void editarPerfilSocket(sqlite3 *db, int socket, char *datos);
void buscarLibroSocket(sqlite3 *db, int socket, char *titulo);
void mostrarHistorialPrestamosSocket(sqlite3 *db, int socket, char *datos);
void devolverLibroSocket(sqlite3 *db, int socket, char *idPrestamoStr);

int verificarSesion(sqlite3 *db, const char *dni, const char *contrasena) {
	sqlite3_stmt *stmt;
	const char *sql = "SELECT 1 FROM Usuario WHERE dni = ? AND contrasena = ?;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
		return 0;
	}

	sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);
	sqlite3_bind_text(stmt, 2, contrasena, -1, SQLITE_STATIC);

	int autenticado = (sqlite3_step(stmt) == SQLITE_ROW);

	sqlite3_finalize(stmt);
	return autenticado;
}


int autenticarUsuario(sqlite3 *db, char *dni, int *esAdmin) {
	sqlite3_stmt *stmt;
	const char *sql = "SELECT es_Admin FROM Usuario WHERE dni = ?;";

	if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
		fprintf(stderr, "Error al preparar consulta: %s\n", sqlite3_errmsg(db));
		return 0;
	}

	sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

	int resultado = 0;
	if (sqlite3_step(stmt) == SQLITE_ROW) {
		*esAdmin = sqlite3_column_int(stmt, 0);
		resultado = 1;
	} else {
		printf("Usuario no encontrado.\n");
	}

	sqlite3_finalize(stmt);
	return resultado;
}

int main(int argc, char *argv[]) {

	int conn_socket;
	int comm_socket;
	sqlite3 *db = NULL;

	if (inicializarBD(&db) != SQLITE_OK) {
	    printf("Error al abrir la base de datos\n");
	    return 1;
	}

	crearTablas(db);

	struct sockaddr_in server;
	struct sockaddr_in client;
	//char sendBuff[512], recvBuff[512]; // lo que yo envio, lo que yo recibo

	printf("\nInitialising...\n");
	printf("Initialised.\n");

	//SOCKET creation
	if ((conn_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Could not create socket");
		return -1;
	}

	printf("Socket created.\n");

	server.sin_addr.s_addr = inet_addr(SERVER_IP); //INADDR_ANY;
	server.sin_family = AF_INET;
	server.sin_port = htons(SERVER_PORT);

	//BIND
	if (bind(conn_socket, (struct sockaddr*) &server, sizeof(server)) < 0) {
		perror("Bind failed");
		close(conn_socket);
		return -1;
	}

	printf("Bind done.\n");

	//LISTEN
	if (listen(conn_socket, 1) < 0) {
		perror("Listen failed");
		close(conn_socket);
		return -1;
	}

	printf("Waiting for incoming connections...\n");
	socklen_t stsize = sizeof(struct sockaddr);


	comm_socket = accept(conn_socket, (struct sockaddr*) &client, &stsize);
	if (comm_socket < 0) {
		perror("accept failed");
		close(conn_socket);
		return -1;
	}
	printf("Incomming connection from: %s (%d)\n", inet_ntoa(client.sin_addr),
			ntohs(client.sin_port));

	close(conn_socket);

	int fin = 0;
	do {
		//lógica del servidor

		while (1) {
		    char buffer[1024];
		    int bytesRecibidos = recv(comm_socket, buffer, sizeof(buffer) - 1, 0);
		    if (bytesRecibidos <= 0) {
		        printf("Cliente desconectado.\n");
		        break;
		    }

		    buffer[bytesRecibidos] = '\0';
		    printf("Comando recibido: %s\n", buffer);

		    // Separar el comando
		    char *comando = strtok(buffer, "|");
		    char *datos = buffer + strlen(comando) + 1;

		    if (strcmp(comando, "LOGIN") == 0) {
		   	        char *usuario = datos;
		   	        char *contrasena = strchr(datos, '|');
		   	        if (contrasena) {
		   	            *contrasena = '\0';
		   	            contrasena++;
		   	        } else {
		   	            send(comm_socket, "ERROR|Formato LOGIN inválido", 27, 0);
		   	            continue;
		   	        }

		   	        int autenticado = verificarSesion(db, usuario, contrasena);
		   	        if (autenticado) {
		   	            int esAdmin;
		   	            autenticarUsuario(db, usuario, &esAdmin);
		   	            char respuesta[64];
		   	            sprintf(respuesta, "OK|%d", esAdmin); // 1 = admin, 0 = usuario
		   	            send(comm_socket, respuesta, strlen(respuesta), 0);
		   	        } else {
		   	            send(comm_socket, "ERROR|Credenciales inválidas", 30, 0);
		   	        }


		   	        //LIBROS
		    		} else if (strcmp(comando, "LISTAR_LIBROS") == 0) {
		    	        listarLibrosSocket(db, comm_socket);

		    	    } else if (strcmp(comando, "SALIR") == 0) {
		    	        printf("Cliente solicitó salir.\n");
		    	        break;

		    	    } else if (strcmp(comando, "AGREGAR_LIBRO") == 0) {
		    	        agregarLibroSocket(db, comm_socket, datos);

		    	    } else if (strcmp(comando, "EDITAR_LIBRO") == 0) {
		    	        editarLibroSocket(db, comm_socket, datos);

		    	    } else if (strcmp(comando, "ELIMINAR_LIBRO") == 0) {
		    	        eliminarLibroSocket(db, comm_socket, datos);

		    	    }

		    		//USUARIOS
		    	    else if (strcmp(comando, "REGISTRAR_USUARIO") == 0) {
		    	    	registrarNuevoUsuarioSocket(db, comm_socket, datos);

		    	    } else if (strcmp(comando, "EDITAR_USUARIO") == 0) {
		    	    	 editarDatosUsuarioSocket(db, comm_socket, datos);

		    	    } else if (strcmp(comando, "ELIMINAR_USUARIO") == 0) {
		    	    	  eliminarUsuarioSocket(db, comm_socket, datos);
		    	    }

		    		// Préstamos y devoluciones
		    	   else if (strcmp(comando, "REGISTRAR_PRESTAMO") == 0) {
		    	   	   registrarPrestamoSocket(db, comm_socket, datos);

		    	   } else if (strcmp(comando, "REGISTRAR_DEVOLUCION") == 0) {
		    	   	   registrarDevolucionSocket(db, comm_socket, datos);

		    	   } else if (strcmp(comando, "MOSTRAR_PRESTAMOS_ACTIVOS") == 0) {
		    	   	   mostrarPrestamosActivosSocket(db, comm_socket);
		    	   }

		    		// Informes
					else if (strcmp(comando, "ESTADISTICAS") == 0) {
						verEstadisticasSocket(db, comm_socket);
					}
					else if (strcmp(comando, "USUARIO_TOP") == 0) {
						mostrarUsuarioConMasPrestamosSocket(db, comm_socket);
					}
					else if (strcmp(comando, "LIBRO_TOP") == 0) {
						mostrarLibroMasPrestadoSocket(db, comm_socket);
					}
					else if (strcmp(comando, "PRESTAMOS_VENCIDOS") == 0) {
						mostrarPrestamosVencidosSocket(db, comm_socket);
					}

		    		//de usuario
		    		else if (strcmp(comando, "VER_PERFIL") == 0) {
		            verPerfilSocket(db, comm_socket, datos);
					}
					else if (strcmp(comando, "EDITAR_PERFIL") == 0) {
						editarPerfilSocket(db, comm_socket, datos);
					}
					else if (strcmp(comando, "BUSCAR_LIBRO") == 0) {
						buscarLibroSocket(db, comm_socket, datos);
					}
					else if (strcmp(comando, "HISTORIAL") == 0) {
						mostrarHistorialPrestamosSocket(db, comm_socket, datos);
					}

					else if (strcmp(comando, "DEVOLVER") == 0) {
						devolverLibroSocket(db, comm_socket, datos);
					}

		    else {
		        send(comm_socket, "ERROR|Comando no reconocido", 28, 0);
		    }
		}
		//hasta aquí

	} while (fin == 0);

	// CLOSING
	//close(comm_socket);

	return 0;
}



void agregarLibroSocket(sqlite3 *db, int socket, char *datos) {
    char *titulo = strtok(datos, "|");
    char *autor = strtok(NULL, "|");
    char *genero = strtok(NULL, "|");

    if (!titulo || !autor || !genero) {
        send(socket, "ERROR|Datos incompletos", 22, 0);
        return;
    }

    Libro nuevoLibro;
    strncpy(nuevoLibro.nombre, titulo, sizeof(nuevoLibro.nombre) - 1);
    strncpy(nuevoLibro.autor, autor, sizeof(nuevoLibro.autor) - 1);
    strncpy(nuevoLibro.genero, genero, sizeof(nuevoLibro.genero) - 1);
    nuevoLibro.estado = 1;

    int idReciclado = -1;
    sqlite3_stmt *stmtBuscarID;
    const char *sqlBuscarID = "SELECT MIN(t1.id + 1) FROM Libro t1 LEFT JOIN Libro t2 ON t1.id + 1 = t2.id WHERE t2.id IS NULL;";

    if (sqlite3_prepare_v2(db, sqlBuscarID, -1, &stmtBuscarID, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmtBuscarID) == SQLITE_ROW) {
            idReciclado = sqlite3_column_int(stmtBuscarID, 0);
        }
    }
    sqlite3_finalize(stmtBuscarID);

    const char *sqlInsert = idReciclado > 0
        ? "INSERT INTO Libro (id, nombre, autor, genero, estado) VALUES (?, ?, ?, ?, ?);"
        : "INSERT INTO Libro (nombre, autor, genero, estado) VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error en BD preparar inserción", 34, 0);
        return;
    }

    if (idReciclado > 0) sqlite3_bind_int(stmt, 1, idReciclado);
    sqlite3_bind_text(stmt, idReciclado > 0 ? 2 : 1, nuevoLibro.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, idReciclado > 0 ? 3 : 2, nuevoLibro.autor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, idReciclado > 0 ? 4 : 3, nuevoLibro.genero, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, idReciclado > 0 ? 5 : 4, nuevoLibro.estado);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        char msg[128];
        sprintf(msg, "OK|Libro agregado ID: %lld", (long long)(idReciclado > 0 ? idReciclado : sqlite3_last_insert_rowid(db)));
        send(socket, msg, strlen(msg), 0);
    } else {
        send(socket, "ERROR|No se pudo agregar libro", 29, 0);
    }

    sqlite3_finalize(stmt);
}

void editarLibroSocket(sqlite3 *db, int socket, char *datos) {
    char *idStr = strtok(datos, "|");
    char *nuevoTitulo = strtok(NULL, "|");
    char *nuevoAutor = strtok(NULL, "|");
    char *nuevoGenero = strtok(NULL, "|");

    if (!idStr) {
        send(socket, "ERROR|Falta ID libro", 19, 0);
        return;
    }

    int idLibro = atoi(idStr);

    sqlite3_stmt *stmt;
    const char *sqlVerificar = "SELECT nombre, autor, genero FROM Libro WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sqlVerificar, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar consulta", 31, 0);
        return;
    }

    sqlite3_bind_int(stmt, 1, idLibro);

    char tituloActual[128] = "";
    char autorActual[128] = "";
    char generoActual[128] = "";

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        strncpy(tituloActual, (const char *)sqlite3_column_text(stmt, 0), sizeof(tituloActual) - 1);
        strncpy(autorActual, (const char *)sqlite3_column_text(stmt, 1), sizeof(autorActual) - 1);
        strncpy(generoActual, (const char *)sqlite3_column_text(stmt, 2), sizeof(generoActual) - 1);
    } else {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|Libro no encontrado", 24, 0);
        return;
    }
    sqlite3_finalize(stmt);

    if (!nuevoTitulo || strlen(nuevoTitulo) == 0) nuevoTitulo = tituloActual;
    if (!nuevoAutor || strlen(nuevoAutor) == 0) nuevoAutor = autorActual;
    if (!nuevoGenero || strlen(nuevoGenero) == 0) nuevoGenero = generoActual;

    const char *sqlActualizar = "UPDATE Libro SET nombre = ?, autor = ?, genero = ? WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sqlActualizar, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar actualización", 36, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoTitulo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nuevoAutor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nuevoGenero, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, idLibro);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        send(socket, "OK|Libro editado correctamente", 29, 0);
    } else {
        send(socket, "ERROR|No se pudo editar libro", 28, 0);
    }

    sqlite3_finalize(stmt);
}

void eliminarLibroSocket(sqlite3 *db, int socket, char *datos) {
    if (!datos) {
        send(socket, "ERROR|No se recibió ID", 22, 0);
        return;
    }

    int id = atoi(datos);

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Libro WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar eliminación", 34, 0);
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        send(socket, "OK|Libro eliminado correctamente", 31, 0);
    } else {
        send(socket, "ERROR|No se pudo eliminar libro", 31, 0);
    }

    sqlite3_finalize(stmt);
}

void listarLibrosSocket(sqlite3 *db, int socket) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, nombre, autor, genero, estado FROM Libro ORDER BY id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al listar libros", 27, 0);
        return;
    }

    char buffer[4096];
    buffer[0] = '\0';

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char linea[512];
        snprintf(linea, sizeof(linea), "%d|%s|%s|%s|%d\n",
            sqlite3_column_int(stmt, 0),
            (const char*)sqlite3_column_text(stmt, 1),
            (const char*)sqlite3_column_text(stmt, 2),
            (const char*)sqlite3_column_text(stmt, 3),
            sqlite3_column_int(stmt, 4));

        strncat(buffer, linea, sizeof(buffer) - strlen(buffer) - 1);
    }

    sqlite3_finalize(stmt);

    send(socket, buffer, strlen(buffer), 0);
}


void registrarNuevoUsuarioSocket(sqlite3 *db, int socket, char *datos) {
    char *nombre = strtok(datos, "|");
    char *apellidos = strtok(NULL, "|");
    char *dni = strtok(NULL, "|");
    char *direccion = strtok(NULL, "|");
    char *email = strtok(NULL, "|");
    char *telefono = strtok(NULL, "|");
    char *contrasena = strtok(NULL, "|");

    if (!nombre || !apellidos || !dni || !direccion || !email || !telefono || !contrasena) {
        send(socket, "ERROR|Datos incompletos", 22, 0);
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
                      "VALUES (?, ?, ?, ?, ?, ?, ?, 0);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al preparar consulta BD", 33, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, dni, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, direccion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, contrasena, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        send(socket, "OK|Usuario registrado", 21, 0);
    } else {
        send(socket, "ERROR|No se pudo registrar usuario", 34, 0);
    }

    sqlite3_finalize(stmt);
}

void editarDatosUsuarioSocket(sqlite3 *db, int socket, char *datos) {
	char *usuario = strtok(datos, "|");
	char *campo = strtok(NULL, "|");
	char *nuevoValor = strtok(NULL, "|");

	if (!usuario || !campo || !nuevoValor) {
	    send(socket, "ERROR|Datos incompletos", 24, 0);
	    return;
	}


    const char *sql_template = "UPDATE Usuario SET %s = ? WHERE dni = ?;";
    char sql[256];

    if (strcmp(campo, "nombre") == 0 ||
        strcmp(campo, "apellidos") == 0 ||
        strcmp(campo, "direccion") == 0 ||
        strcmp(campo, "email") == 0 ||
        strcmp(campo, "telefono") == 0 ||
        strcmp(campo, "contrasena") == 0) {
        snprintf(sql, sizeof(sql), sql_template, campo);
    } else {
        send(socket, "ERROR|Campo no válido", 20, 0);
        return;
    }

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al preparar consulta BD", 33, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoValor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, usuario, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        send(socket, "OK|Usuario actualizado", 22, 0);
    } else {
        send(socket, "ERROR|No se pudo actualizar usuario", 35, 0);
    }

    sqlite3_finalize(stmt);
}

void eliminarUsuarioSocket(sqlite3 *db, int socket, char *datos) {
    if (!datos) {
        send(socket, "ERROR|No se recibió DNI", 22, 0);
        return;
    }

    if (strcmp(datos, "00000000A") == 0) {
        send(socket, "ERROR|No se puede eliminar administrador principal", 49, 0);
        return;
    }

    sqlite3_stmt *stmtCheck;
    const char *sqlCheck = "SELECT COUNT(*) FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sqlCheck, -1, &stmtCheck, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD al verificar usuario", 34, 0);
        return;
    }

    sqlite3_bind_text(stmtCheck, 1, datos, -1, SQLITE_STATIC);

    int exists = 0;
    if (sqlite3_step(stmtCheck) == SQLITE_ROW) {
        exists = sqlite3_column_int(stmtCheck, 0);
    }
    sqlite3_finalize(stmtCheck);

    if (!exists) {
        send(socket, "ERROR|DNI no registrado", 23, 0);
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar eliminación", 34, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, datos, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        send(socket, "OK|Usuario eliminado", 20, 0);
    } else {
        send(socket, "ERROR|No se pudo eliminar usuario", 33, 0);
    }

    sqlite3_finalize(stmt);
}


void registrarPrestamoSocket(sqlite3 *db, int socket, char *datos) {
    char *dni = strtok(datos, "|");
    char *idLibroStr = strtok(NULL, "|");
    if (!dni || !idLibroStr) {
        send(socket, "ERROR|Datos incompletos", 22, 0);
        return;
    }

    int idLibro = atoi(idLibroStr);

    sqlite3_stmt *stmt;
    const char *sqlCheck = "SELECT estado FROM Libro WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar consulta", 33, 0);
        return;
    }
    sqlite3_bind_int(stmt, 1, idLibro);

    int estadoLibro = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        estadoLibro = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (estadoLibro != 1) {
        send(socket, "ERROR|Libro no disponible", 24, 0);
        return;
    }

    const char *sqlInsert = "INSERT INTO Prestamo (usuario_dni, libro_id, fecha_Prestamo) VALUES (?, ?, datetime('now'));";
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar inserción", 33, 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, idLibro);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|No se pudo registrar préstamo", 35, 0);
        return;
    }
    sqlite3_finalize(stmt);

    const char *sqlUpdateLibro = "UPDATE Libro SET estado = 0 WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sqlUpdateLibro, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar actualización", 36, 0);
        return;
    }
    sqlite3_bind_int(stmt, 1, idLibro);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|No se pudo actualizar estado libro", 39, 0);
        return;
    }
    sqlite3_finalize(stmt);

    send(socket, "OK|Préstamo registrado", 21, 0);
}

void registrarDevolucionSocket(sqlite3 *db, int socket, char *datos) {
    char *dni = strtok(datos, "|");
    char *idLibroStr = strtok(NULL, "|");
    if (!dni || !idLibroStr) {
        send(socket, "ERROR|Datos incompletos", 22, 0);
        return;
    }

    int idLibro = atoi(idLibroStr);

    sqlite3_stmt *stmt;
    const char *sqlCheck = "SELECT id FROM Prestamo WHERE usuario_dni = ? AND libro_id = ? AND fecha_Devolucion IS NULL;";
    if (sqlite3_prepare_v2(db, sqlCheck, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar consulta", 33, 0);
        return;
    }
    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, idLibro);

    int prestamoId = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        prestamoId = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (prestamoId == -1) {
        send(socket, "ERROR|Préstamo activo no encontrado", 33, 0);
        return;
    }

    const char *sqlUpdatePrestamo = "UPDATE Prestamo SET fecha_Devolucion = datetime('now') WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sqlUpdatePrestamo, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar actualización", 36, 0);
        return;
    }
    sqlite3_bind_int(stmt, 1, prestamoId);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|No se pudo registrar devolución", 37, 0);
        return;
    }
    sqlite3_finalize(stmt);

    const char *sqlUpdateLibro = "UPDATE Libro SET estado = 1 WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sqlUpdateLibro, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar actualización", 36, 0);
        return;
    }
    sqlite3_bind_int(stmt, 1, idLibro);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|No se pudo actualizar estado libro", 39, 0);
        return;
    }
    sqlite3_finalize(stmt);

    send(socket, "OK|Devolución registrada", 23, 0);
}

void mostrarPrestamosActivosSocket(sqlite3 *db, int socket) {
    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, usuario_dni, libro_id, fecha_Prestamo FROM Prestamo WHERE fecha_Devolucion IS NULL ORDER BY fecha_Prestamo;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al listar préstamos activos", 37, 0);
        return;
    }

    char buffer[4096];
    buffer[0] = '\0';

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char linea[256];
        snprintf(linea, sizeof(linea), "%d|%s|%d|%s\n",
                 sqlite3_column_int(stmt, 0),
                 (const char*)sqlite3_column_text(stmt, 1),
                 sqlite3_column_int(stmt, 2),
                 (const char*)sqlite3_column_text(stmt, 3));
        strncat(buffer, linea, sizeof(buffer) - strlen(buffer) - 1);
    }
    sqlite3_finalize(stmt);

    send(socket, buffer, strlen(buffer), 0);
}

void mostrarUsuarioConMasPrestamosSocket(sqlite3 *db, int socket) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT Usuario.dni, Usuario.nombre, COUNT(*) AS total_prestamos "
        "FROM Usuario "
        "JOIN Prestamo ON Usuario.dni = Prestamo.usuario_dni "
        "GROUP BY Usuario.dni "
        "ORDER BY total_prestamos DESC "
        "LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al obtener usuario top", 33, 0);
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char respuesta[256];
        snprintf(respuesta, sizeof(respuesta), "%s|%s|%d",
                 (const char*)sqlite3_column_text(stmt, 0),
                 (const char*)sqlite3_column_text(stmt, 1),
                 sqlite3_column_int(stmt, 2));
        send(socket, respuesta, strlen(respuesta), 0);
    } else {
        send(socket, "ERROR|No se encontró usuario", 28, 0);
    }

    sqlite3_finalize(stmt);
}


void mostrarLibroMasPrestadoSocket(sqlite3 *db, int socket) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT Libro.id, Libro.nombre, COUNT(*) AS total_prestamos "
        "FROM Libro "
        "JOIN Prestamo ON Libro.id = Prestamo.libro_id "
        "GROUP BY Libro.id "
        "ORDER BY total_prestamos DESC "
        "LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al obtener libro top", 31, 0);
        return;
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char respuesta[256];
        snprintf(respuesta, sizeof(respuesta), "%d|%s|%d",
                 sqlite3_column_int(stmt, 0),
                 (const char*)sqlite3_column_text(stmt, 1),
                 sqlite3_column_int(stmt, 2));
        send(socket, respuesta, strlen(respuesta), 0);
    } else {
        send(socket, "ERROR|No se encontró libro", 26, 0);
    }

    sqlite3_finalize(stmt);
}


void mostrarPrestamosVencidosSocket(sqlite3 *db, int socket) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT p.id, p.usuario_dni, p.libro_id, p.fecha_Prestamo "
        "FROM Prestamo p "
        "WHERE p.fecha_Devolucion IS NULL AND p.fecha_Prestamo <= datetime('now', '-30 days') "
        "ORDER BY p.fecha_Prestamo;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al listar préstamos vencidos", 40, 0);
        return;
    }

    char buffer[4096];
    buffer[0] = '\0';

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char linea[256];
        snprintf(linea, sizeof(linea), "%d|%s|%d|%s\n",
                 sqlite3_column_int(stmt, 0),
                 (const char*)sqlite3_column_text(stmt, 1),
                 sqlite3_column_int(stmt, 2),
                 (const char*)sqlite3_column_text(stmt, 3));
        strncat(buffer, linea, sizeof(buffer) - strlen(buffer) - 1);
    }

    sqlite3_finalize(stmt);

    if (strlen(buffer) == 0) {
        send(socket, "ERROR|No hay préstamos vencidos", 31, 0);
    } else {
        send(socket, buffer, strlen(buffer), 0);
    }
}


void verEstadisticasSocket(sqlite3 *db, int socket) {
    char buffer[512];
    int offset = 0;
    sqlite3_stmt *stmt;

    // Total usuarios
    const char *sql = "SELECT COUNT(*) FROM Usuario;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                "Total usuarios: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sql = "SELECT COUNT(*) FROM Libro;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                "Total libros: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sql = "SELECT COUNT(*) FROM Libro WHERE estado = 0;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                "Libros prestados: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sql = "SELECT COUNT(*) FROM Prestamo WHERE fecha_Devolucion IS NULL;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                "Préstamos activos: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    send(socket, buffer, strlen(buffer), 0);
}

void verPerfilSocket(sqlite3 *db, int socket, char *dni) {
    if (!dni) {
        send(socket, "ERROR|No se recibió DNI", 22, 0);
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, apellidos, dni, direccion, email, telefono FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al preparar consulta BD", 33, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        char respuesta[512];
        snprintf(respuesta, sizeof(respuesta), "OK|%s|%s|%s|%s|%s|%s",
                 (const char*)sqlite3_column_text(stmt, 0),
                 (const char*)sqlite3_column_text(stmt, 1),
                 (const char*)sqlite3_column_text(stmt, 2),
                 (const char*)sqlite3_column_text(stmt, 3),
                 (const char*)sqlite3_column_text(stmt, 4),
                 (const char*)sqlite3_column_text(stmt, 5));
        send(socket, respuesta, strlen(respuesta), 0);
    } else {
        send(socket, "ERROR|Usuario no encontrado", 27, 0);
    }

    sqlite3_finalize(stmt);
}

void editarPerfilSocket(sqlite3 *db, int socket, char *datos) {
    char *dni = strtok(datos, "|");
    char *campo = strtok(NULL, "|");
    char *nuevoValor = strtok(NULL, "|");

    if (!dni || !campo || !nuevoValor) {
        send(socket, "ERROR|Datos incompletos", 22, 0);
        return;
    }

    if (strcmp(campo, "nombre") != 0 &&
        strcmp(campo, "apellidos") != 0 &&
        strcmp(campo, "direccion") != 0 &&
        strcmp(campo, "email") != 0 &&
        strcmp(campo, "telefono") != 0 &&
        strcmp(campo, "contrasena") != 0) {
        send(socket, "ERROR|Campo no válido", 20, 0);
        return;
    }

    char sql[256];
    snprintf(sql, sizeof(sql), "UPDATE Usuario SET %s = ? WHERE dni = ?;", campo);

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al preparar consulta BD", 33, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoValor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        send(socket, "OK|Perfil actualizado", 20, 0);
    } else {
        send(socket, "ERROR|No se pudo actualizar perfil", 33, 0);
    }

    sqlite3_finalize(stmt);
}

void buscarLibroSocket(sqlite3 *db, int socket, char *datos) {
    if (!datos || strlen(datos) == 0) {
        send(socket, "ERROR|Título no proporcionado", 27, 0);
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, nombre, autor, genero, estado FROM Libro WHERE nombre LIKE ? ORDER BY id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al preparar consulta BD", 33, 0);
        return;
    }

    char patron[256];
    snprintf(patron, sizeof(patron), "%%%s%%", datos);
    sqlite3_bind_text(stmt, 1, patron, -1, SQLITE_STATIC);

    char buffer[4096];
    buffer[0] = '\0';

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        char linea[512];
        snprintf(linea, sizeof(linea), "%d|%s|%s|%s|%d\n",
                 sqlite3_column_int(stmt, 0),
                 (const char*)sqlite3_column_text(stmt, 1),
                 (const char*)sqlite3_column_text(stmt, 2),
                 (const char*)sqlite3_column_text(stmt, 3),
                 sqlite3_column_int(stmt, 4));
        strncat(buffer, linea, sizeof(buffer) - strlen(buffer) - 1);
    }

    sqlite3_finalize(stmt);

    if (strlen(buffer) == 0) {
        send(socket, "ERROR|No se encontraron libros", 29, 0);
    } else {
        send(socket, buffer, strlen(buffer), 0);
    }
}

void mostrarHistorialPrestamosSocket(sqlite3 *db, int socket, char *datos) {
    if (!datos || strlen(datos) == 0) {
        send(socket, "ERROR|DNI no proporcionado", 25, 0);
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT id, libro_id, fecha_Prestamo, "
        "IFNULL(fecha_Devolucion, 'No devuelto') "
        "FROM Prestamo WHERE usuario_dni = ? ORDER BY fecha_Prestamo DESC;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error al preparar consulta BD", 33, 0);
        return;
    }

    sqlite3_bind_text(stmt, 1, datos, -1, SQLITE_STATIC);

    char buffer[4096];
    buffer[0] = '\0';

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *fechaDevolucion = (const char*)sqlite3_column_text(stmt, 3);
        char linea[256];
        snprintf(linea, sizeof(linea), "%d|%d|%s|%s\n",
                 sqlite3_column_int(stmt, 0),
                 sqlite3_column_int(stmt, 1),
                 (const char*)sqlite3_column_text(stmt, 2),
                 fechaDevolucion);
        strncat(buffer, linea, sizeof(buffer) - strlen(buffer) - 1);
    }

    sqlite3_finalize(stmt);

    if (strlen(buffer) == 0) {
        send(socket, "ERROR|No se encontraron préstamos", 33, 0);
    } else {
        send(socket, buffer, strlen(buffer), 0);
    }
}

void devolverLibroSocket(sqlite3 *db, int socket, char *datos) {
    if (!datos || strlen(datos) == 0) {
        send(socket, "ERROR|ID de préstamo no proporcionado", 35, 0);
        return;
    }

    int idPrestamo = atoi(datos);
    if (idPrestamo <= 0) {
        send(socket, "ERROR|ID de préstamo inválido", 28, 0);
        return;
    }

    sqlite3_stmt *stmt;
    const char *sqlBuscarPrestamo = "SELECT libro_id FROM Prestamo WHERE id = ? AND fecha_Devolucion IS NULL;";

    if (sqlite3_prepare_v2(db, sqlBuscarPrestamo, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar consulta", 33, 0);
        return;
    }

    sqlite3_bind_int(stmt, 1, idPrestamo);

    int idLibro = -1;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        idLibro = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    if (idLibro == -1) {
        send(socket, "ERROR|Préstamo no encontrado o ya devuelto", 40, 0);
        return;
    }

    const char *sqlActualizarPrestamo = "UPDATE Prestamo SET fecha_Devolucion = datetime('now') WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sqlActualizarPrestamo, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar actualización préstamo", 44, 0);
        return;
    }
    sqlite3_bind_int(stmt, 1, idPrestamo);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|No se pudo actualizar fecha de devolución", 47, 0);
        return;
    }
    sqlite3_finalize(stmt);

    const char *sqlActualizarLibro = "UPDATE Libro SET estado = 1 WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sqlActualizarLibro, -1, &stmt, NULL) != SQLITE_OK) {
        send(socket, "ERROR|Error BD preparar actualización libro", 40, 0);
        return;
    }
    sqlite3_bind_int(stmt, 1, idLibro);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        sqlite3_finalize(stmt);
        send(socket, "ERROR|No se pudo actualizar estado del libro", 43, 0);
        return;
    }
    sqlite3_finalize(stmt);

    send(socket, "OK|Devolución registrada", 23, 0);
}

