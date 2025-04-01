#include "menuAdmin.h"
#include "usuario.h"
#include "libro.h"
#include "prestamo.h"
#include <stdlib.h>
#include "bd.h"
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include <stdbool.h>

char menuAdministrador() {
    char opcion;
    printf("\n--- MENÚ ADMINISTRADOR ---\n");
    printf("1. Gestionar inventario de libros\n");
    printf("2. Gestionar usuarios\n");
    printf("3. Registros de préstamos y devoluciones\n");
    printf("4. Generar informes\n");
    printf("0. Salir\n");
    printf("Elige una opción: ");
    fflush(stdout);

    scanf(" %c", &opcion);
    while (getchar() != '\n');

    return opcion;
}

void ejecutarMenuAdmin(sqlite3 *db) {
    char opcion;
    do {
        opcion = menuAdministrador();
        switch (opcion) {
        case '1':
            gestionarInventarioLibros(db);
            break;
        case '2':
            gestionarUsuarios(db);
            break;
        case '3':
            gestionarPrestamosDevoluciones(db);
            break;
        case '4':
            generarInformes(db);
            break;
        case '0':
            printf("Volviendo al menú principal...\n");
            break;
        default:
            printf("Opción no válida.\n");
        }
    } while (opcion != '0');
}

// --------------------------
// GESTIÓN DE INVENTARIO DE LIBROS
// --------------------------

void gestionarInventarioLibros(sqlite3 *db) {
    char opcion;
    do {
        printf("\n--- GESTIÓN DE INVENTARIO DE LIBROS ---\n");
        printf("a. Añadir nuevo libro\n");
        printf("b. Editar información de un libro\n");
        printf("c. Eliminar un libro\n");
        printf("0. Volver\n");
        printf("Opción: ");
        fflush(stdout);

        scanf(" %c", &opcion);
        while (getchar() != '\n');

        switch (opcion) {
        case 'a':
            agregarLibro(db);
            break;
        case 'b':
            editarLibro(db);
            break;
        case 'c':
            eliminarLibro(db);
            break;
        case '0':
            return;
        default:
            printf("Opción no válida\n");
        }
    } while (1);
}

void agregarLibro(sqlite3 *db) {
    Libro nuevoLibro;
    printf("\n--- AGREGAR LIBRO ---\n");

    printf("Título: ");
    fflush(stdout);
    fgets(nuevoLibro.nombre, sizeof(nuevoLibro.nombre), stdin);
    nuevoLibro.nombre[strcspn(nuevoLibro.nombre, "\n")] = 0;

    printf("Autor: ");
    fflush(stdout);
    fgets(nuevoLibro.autor, sizeof(nuevoLibro.autor), stdin);
    nuevoLibro.autor[strcspn(nuevoLibro.autor, "\n")] = 0;

    printf("Género: ");
    fflush(stdout);
    fgets(nuevoLibro.genero, sizeof(nuevoLibro.genero), stdin);
    nuevoLibro.genero[strcspn(nuevoLibro.genero, "\n")] = 0;

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
        fprintf(stderr, "Error al preparar inserción: %s\n", sqlite3_errmsg(db));
        return;
    }

    if (idReciclado > 0) sqlite3_bind_int(stmt, 1, idReciclado);
    sqlite3_bind_text(stmt, idReciclado > 0 ? 2 : 1, nuevoLibro.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, idReciclado > 0 ? 3 : 2, nuevoLibro.autor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, idReciclado > 0 ? 4 : 3, nuevoLibro.genero, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, idReciclado > 0 ? 5 : 4, nuevoLibro.estado);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Libro agregado correctamente. ID: %lld\n", (long long)(idReciclado > 0 ? idReciclado : sqlite3_last_insert_rowid(db)));
        fflush(stdout);
    } else {
        printf("Error al agregar libro.\n");
    }

    sqlite3_finalize(stmt);
}

void editarLibro(sqlite3 *db) {
    int idLibro;
    char nuevoTitulo[MAX_NOMBRE];
    char nuevoAutor[MAX_NOMBRE];
    char nuevoGenero[MAX_NOMBRE];

    listarLibros(db);
    printf("Elija un libro, introduciendo su ID: ");
    fflush(stdout);
    scanf("%d", &idLibro);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql_verificar = "SELECT COUNT(*) FROM Libro WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql_verificar, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al verificar libro: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, idLibro);

    if (sqlite3_step(stmt) != SQLITE_ROW || sqlite3_column_int(stmt, 0) == 0) {
        printf("Error: No existe un libro con el ID %d.\n", idLibro);
        sqlite3_finalize(stmt);
        return;
    }

    sqlite3_finalize(stmt);

    printf("Nuevo título (deje en blanco para no cambiar): ");
    fflush(stdout);
    fgets(nuevoTitulo, sizeof(nuevoTitulo), stdin);
    nuevoTitulo[strcspn(nuevoTitulo, "\n")] = 0;

    printf("Nuevo autor (deje en blanco para no cambiar): ");
    fflush(stdout);
    fgets(nuevoAutor, sizeof(nuevoAutor), stdin);
    nuevoAutor[strcspn(nuevoAutor, "\n")] = 0;

    printf("Nuevo género (deje en blanco para no cambiar): ");
    fflush(stdout);
    fgets(nuevoGenero, sizeof(nuevoGenero), stdin);
    nuevoGenero[strcspn(nuevoGenero, "\n")] = 0;

    const char *sql_obtener = "SELECT nombre, autor, genero FROM Libro WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql_obtener, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al obtener datos del libro: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, idLibro);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        if (strlen(nuevoTitulo) == 0) strcpy(nuevoTitulo, (const char *)sqlite3_column_text(stmt, 0));
        if (strlen(nuevoAutor) == 0) strcpy(nuevoAutor, (const char *)sqlite3_column_text(stmt, 1));
        if (strlen(nuevoGenero) == 0) strcpy(nuevoGenero, (const char *)sqlite3_column_text(stmt, 2));
    }

    sqlite3_finalize(stmt);

    const char *sql_actualizar = "UPDATE Libro SET nombre = ?, autor = ?, genero = ? WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql_actualizar, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar actualización: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoTitulo, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nuevoAutor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nuevoGenero, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 4, idLibro);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Libro editado correctamente :) .\n");
        fflush(stdout);
    } else {
        printf("Error al editar el libro :(.\n");
        fflush(stdout);
    }

    sqlite3_finalize(stmt);
}

void eliminarLibro(sqlite3 *db) {
    int id;
    printf("\nID del libro a eliminar: ");
    fflush(stdout);
    scanf("%d", &id);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Libro WHERE id = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar eliminación: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Libro eliminado correctamente.\n");
    } else {
        printf("Error al eliminar libro.\n");
    }
    sqlite3_finalize(stmt);
}

// --------------------------
// GESTIÓN DE USUARIOS
// --------------------------

void gestionarUsuarios(sqlite3 *db) {
    char opcion;
    do {
        printf("\n--- GESTIÓN DE USUARIOS ---\n");
        printf("a. Registrar nuevo usuario\n");
        printf("b. Editar datos de usuario\n");
        printf("c. Eliminar usuarios\n");
        printf("0. Volver\n");
        printf("Opción: ");
        fflush(stdout);

        scanf(" %c", &opcion);
        while (getchar() != '\n');

        switch (opcion) {
        case 'a':
            registrarNuevoUsuario(db);
            break;
        case 'b':
            editarDatosUsuario(db);
            break;
        case 'c':
            eliminarUsuario(db);
            break;
        case '0':
            return;
        default:
            printf("Opción no válida\n");
        }
    } while (1);
}

void registrarNuevoUsuario(sqlite3 *db) {
    Usuario nuevoUsuario;
    printf("\nREGISTRO DE USUARIO\n");
    printf("----------------------\n");
    fflush(stdout);

    printf("Nombre: ");
    fflush(stdout);
    fgets(nuevoUsuario.nombre, sizeof(nuevoUsuario.nombre), stdin);
    nuevoUsuario.nombre[strcspn(nuevoUsuario.nombre, "\n")] = 0;

    printf("Apellidos: ");
    fflush(stdout);
    fgets(nuevoUsuario.apellidos, sizeof(nuevoUsuario.apellidos), stdin);
    nuevoUsuario.apellidos[strcspn(nuevoUsuario.apellidos, "\n")] = 0;

    printf("DNI: ");
    fflush(stdout);
    scanf("%19s", nuevoUsuario.dni);
    while (getchar() != '\n');

    printf("Direccion: ");
    fflush(stdout);
    fgets(nuevoUsuario.direccion, sizeof(nuevoUsuario.direccion), stdin);
    nuevoUsuario.direccion[strcspn(nuevoUsuario.direccion, "\n")] = 0;

    printf("Email: ");
    fflush(stdout);
    scanf("%99s", nuevoUsuario.email);
    while (getchar() != '\n');

    printf("Telefono: ");
    fflush(stdout);
    scanf("%14s", nuevoUsuario.telefono);
    while (getchar() != '\n');

    printf("Contraseña: ");
    fflush(stdout);
    scanf("%14s", nuevoUsuario.contrasena);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO Usuario (nombre, apellidos, dni, direccion, email, telefono, contrasena, es_Admin) "
                     "VALUES (?, ?, ?, ?, ?, ?, ?, 0);";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoUsuario.nombre, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, nuevoUsuario.apellidos, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, nuevoUsuario.dni, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, nuevoUsuario.direccion, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 5, nuevoUsuario.email, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 6, nuevoUsuario.telefono, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 7, nuevoUsuario.contrasena, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Usuario registrado exitosamente.\n");
        fflush(stdout);
    } else {
        printf("Error al registrar usuario: %s\n", sqlite3_errmsg(db));
        fflush(stdout);
    }

    sqlite3_finalize(stmt);
}

void editarDatosUsuario(sqlite3 *db) {
    char dni[MAX_STR];
    printf("\nDNI del usuario a editar: ");
    fflush(stdout);
    scanf("%19s", dni);
    while (getchar() != '\n');

    sqlite3_stmt *stmt;
    const char *sql = "SELECT nombre, apellidos, direccion, email, telefono FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) != SQLITE_ROW) {
        printf("Usuario no encontrado.\n");
        sqlite3_finalize(stmt);
        return;
    }

    Usuario usuario;
    strcpy(usuario.dni, dni);
    strcpy(usuario.nombre, (const char*)sqlite3_column_text(stmt, 0));
    strcpy(usuario.apellidos, (const char*)sqlite3_column_text(stmt, 1));
    strcpy(usuario.direccion, (const char*)sqlite3_column_text(stmt, 2));
    strcpy(usuario.email, (const char*)sqlite3_column_text(stmt, 3));
    strcpy(usuario.telefono, (const char*)sqlite3_column_text(stmt, 4));

    sqlite3_finalize(stmt);

    printf("\nEditando usuario: %s %s (DNI: %s)\n", usuario.nombre, usuario.apellidos, usuario.dni);
    printf("1. Nombre: %s\n", usuario.nombre);
    printf("2. Apellidos: %s\n", usuario.apellidos);
    printf("3. Dirección: %s\n", usuario.direccion);
    printf("4. Email: %s\n", usuario.email);
    printf("5. Teléfono: %s\n", usuario.telefono);
    printf("6. Contraseña: ********\n");
    printf("0. Cancelar\n");
    printf("Seleccione el campo a modificar: ");
    fflush(stdout);

    char opcion;
    scanf(" %c", &opcion);
    while (getchar() != '\n');

    char nuevoValor[MAX_STR];
    const char *sqlUpdate = "";

    switch (opcion) {
        case '1':
            printf("Nuevo nombre: ");
            fflush(stdout);
            fgets(nuevoValor, sizeof(nuevoValor), stdin);
            nuevoValor[strcspn(nuevoValor, "\n")] = 0;
            strcpy(usuario.nombre, nuevoValor);
            sqlUpdate = "UPDATE Usuario SET nombre = ? WHERE dni = ?;";
            break;
        case '2':
            printf("Nuevos apellidos: ");
            fflush(stdout);
            fgets(nuevoValor, sizeof(nuevoValor), stdin);
            nuevoValor[strcspn(nuevoValor, "\n")] = 0;
            strcpy(usuario.apellidos, nuevoValor);
            sqlUpdate = "UPDATE Usuario SET apellidos = ? WHERE dni = ?;";
            break;
        case '3':
            printf("Nueva dirección: ");
            fflush(stdout);
            fgets(nuevoValor, sizeof(nuevoValor), stdin);
            nuevoValor[strcspn(nuevoValor, "\n")] = 0;
            strcpy(usuario.direccion, nuevoValor);
            sqlUpdate = "UPDATE Usuario SET direccion = ? WHERE dni = ?;";
            break;
        case '4':
            printf("Nuevo email: ");
            fflush(stdout);
            fgets(nuevoValor, sizeof(nuevoValor), stdin);
            nuevoValor[strcspn(nuevoValor, "\n")] = 0;
            strcpy(usuario.email, nuevoValor);
            sqlUpdate = "UPDATE Usuario SET email = ? WHERE dni = ?;";
            break;
        case '5':
            printf("Nuevo teléfono: ");
            fflush(stdout);
            fgets(nuevoValor, sizeof(nuevoValor), stdin);
            nuevoValor[strcspn(nuevoValor, "\n")] = 0;
            strcpy(usuario.telefono, nuevoValor);
            sqlUpdate = "UPDATE Usuario SET telefono = ? WHERE dni = ?;";
            break;
        case '6':
            printf("Nueva contraseña: ");
            fflush(stdout);
            fgets(nuevoValor, sizeof(nuevoValor), stdin);
            nuevoValor[strcspn(nuevoValor, "\n")] = 0;
            strcpy(usuario.contrasena, nuevoValor);
            sqlUpdate = "UPDATE Usuario SET contrasena = ? WHERE dni = ?;";
            break;
        case '0':
            printf("Edición cancelada.\n");
            return;
        default:
            printf("Opción no válida.\n");
            return;
    }

    if (sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar actualización: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, nuevoValor, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Usuario actualizado correctamente.\n");
    } else {
        printf("Error al actualizar usuario: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void eliminarUsuario(sqlite3 *db) {
    char dni[MAX_STR];
    printf("\nDNI del usuario a eliminar: ");
    fflush(stdout);
    scanf("%19s", dni);
    while (getchar() != '\n');

    if (strcmp(dni, "00000000A") == 0) {
        printf("No se puede eliminar al administrador principal.\n");
        return;
    }

    sqlite3_stmt *checkStmt;
    const char *checkSql = "SELECT COUNT(*) FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, checkSql, -1, &checkStmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar verificación: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(checkStmt, 1, dni, -1, SQLITE_STATIC);

    int userExists = 0;
    if (sqlite3_step(checkStmt) == SQLITE_ROW) {
        userExists = sqlite3_column_int(checkStmt, 0);
    }
    sqlite3_finalize(checkStmt);

    if (userExists == 0) {
        printf("El DNI ingresado no está registrado.\n");
        return;
    }

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM Usuario WHERE dni = ?;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar eliminación: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, dni, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Usuario eliminado correctamente.\n");
    } else {
        printf("Error al eliminar usuario.\n");
    }
    sqlite3_finalize(stmt);
}

// --------------------------
// REGISTROS DE PRÉSTAMOS Y DEVOLUCIONES
// --------------------------

void gestionarPrestamosDevoluciones(sqlite3 *db) {
    char opcion;
    do {
        printf("\n--- REGISTROS DE PRÉSTAMOS Y DEVOLUCIONES ---\n");
        printf("a. Registrar préstamo\n");
        printf("b. Registrar devolución\n");
        printf("c. Mostrar préstamos activos\n");
        printf("0. Volver\n");
        printf("Opción: ");
        fflush(stdout);

        scanf(" %c", &opcion);
        while (getchar() != '\n');

        switch (opcion) {
        case 'a':
            registrarPrestamo(db);
            break;
        case 'b':
            registrarDevolucion(db);
            break;
        case 'c':
            mostrar_prestamos_activos(db);
            break;
        case '0':
            return;
        default:
            printf("Opción no válida\n");
        }
    } while (1);
}

void registrarPrestamo(sqlite3 *db) {
    Prestamo nuevoPrestamo;
    printf("\n--- AGREGAR PRÉSTAMO ---\n");

    printf("DNI del usuario: ");
    fflush(stdout);
    fgets(nuevoPrestamo.usuario_dni, sizeof(nuevoPrestamo.usuario_dni), stdin);
    nuevoPrestamo.usuario_dni[strcspn(nuevoPrestamo.usuario_dni, "\n")] = 0;

    printf("ID del libro: ");
    fflush(stdout);
    char input[20];
    fgets(input, sizeof(input), stdin);
    nuevoPrestamo.libro_id = atoi(input);

    printf("Fecha de préstamo (YYYY-MM-DD): ");
    fflush(stdout);
    fgets(nuevoPrestamo.fecha_prestamo, sizeof(nuevoPrestamo.fecha_prestamo), stdin);
    nuevoPrestamo.fecha_prestamo[strcspn(nuevoPrestamo.fecha_prestamo, "\n")] = 0;

    int idReciclado = -1;
    sqlite3_stmt *stmtBuscarID;
    const char *sqlBuscarID = "SELECT MIN(t1.id + 1) FROM Prestamo t1 LEFT JOIN Prestamo t2 ON t1.id + 1 = t2.id WHERE t2.id IS NULL;";

    if (sqlite3_prepare_v2(db, sqlBuscarID, -1, &stmtBuscarID, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmtBuscarID) == SQLITE_ROW) {
            idReciclado = sqlite3_column_int(stmtBuscarID, 0);
        }
    }
    sqlite3_finalize(stmtBuscarID);

    const char *sqlInsert = idReciclado > 0
        ? "INSERT INTO Prestamo (id, usuario_dni, libro_id, fecha_Prestamo, fecha_Devolucion) VALUES (?, ?, ?, ?, ?);"
        : "INSERT INTO Prestamo (usuario_dni, libro_id, fecha_Prestamo, fecha_Devolucion) VALUES (?, ?, ?, ?);";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar inserción: %s\n", sqlite3_errmsg(db));
        return;
    }

    int bindIndex = 1;
    if (idReciclado > 0) {
        sqlite3_bind_int(stmt, bindIndex++, idReciclado);
    }
    sqlite3_bind_text(stmt, bindIndex++, nuevoPrestamo.usuario_dni, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, bindIndex++, nuevoPrestamo.libro_id);
    sqlite3_bind_text(stmt, bindIndex++, nuevoPrestamo.fecha_prestamo, -1, SQLITE_STATIC);
    
    if (strlen(nuevoPrestamo.fecha_devolucion) > 0) {
        sqlite3_bind_text(stmt, bindIndex, nuevoPrestamo.fecha_devolucion, -1, SQLITE_STATIC);
    } else {
        sqlite3_bind_null(stmt, bindIndex);
    }

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Préstamo agregado correctamente. ID: %lld\n", (long long)(idReciclado > 0 ? idReciclado : sqlite3_last_insert_rowid(db)));
    } else {
        fprintf(stderr, "Error al agregar préstamo: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void registrarDevolucion(sqlite3 *db) {
    char dniUsuario[20];
    printf("\n--- REGISTRAR DEVOLUCIÓN ---\n");
    printf("Ingrese DNI del usuario: ");
    fgets(dniUsuario, sizeof(dniUsuario), stdin);
    dniUsuario[strcspn(dniUsuario, "\n")] = 0;

    const char *sql = "SELECT p.id, l.nombre, p.fecha_Prestamo "
                     "FROM Prestamo p "
                     "JOIN Libro l ON p.libro_id = l.id "
                     "WHERE p.usuario_dni = ? AND p.fecha_Devolucion IS NULL;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_text(stmt, 1, dniUsuario, -1, SQLITE_STATIC);

    bool tienePrestamos = false;
    printf("\nPréstamos activos para DNI %s:\n", dniUsuario);
    printf("--------------------------------\n");
    printf("ID\tLibro\t\tFecha Préstamo\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tienePrestamos = true;
        int idPrestamo = sqlite3_column_int(stmt, 0);
        const char *nombreLibro = (const char *)sqlite3_column_text(stmt, 1);
        const char *fechaPrestamo = (const char *)sqlite3_column_text(stmt, 2);

        printf("%d\t%s\t%s\n", idPrestamo, nombreLibro, fechaPrestamo);
    }

    sqlite3_finalize(stmt);

    if (!tienePrestamos) {
        printf("No hay préstamos activos para este usuario.\n");
        return;
    }

    int idPrestamoDevolucion;
    printf("\nIngrese ID del préstamo a devolver: ");
    scanf("%d", &idPrestamoDevolucion);
    getchar();

    const char *sqlUpdate = "UPDATE Prestamo SET fecha_Devolucion = date('now') "
                           "WHERE id = ? AND usuario_dni = ? AND fecha_Devolucion IS NULL;";

    if (sqlite3_prepare_v2(db, sqlUpdate, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar actualización: %s\n", sqlite3_errmsg(db));
        return;
    }

    sqlite3_bind_int(stmt, 1, idPrestamoDevolucion);
    sqlite3_bind_text(stmt, 2, dniUsuario, -1, SQLITE_STATIC);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        int cambios = sqlite3_changes(db);
        if (cambios > 0) {
            printf("\n¡Devolución registrada con éxito!\n");
        } else {
            printf("\nError: No se pudo registrar la devolución.\n");
            printf("Verifique que el ID y DNI sean correctos.\n");
        }
    } else {
        fprintf(stderr, "Error al ejecutar actualización: %s\n", sqlite3_errmsg(db));
    }

    sqlite3_finalize(stmt);
}

void mostrar_prestamos_activos(sqlite3 *db) {
    if (db == NULL) {
        printf("Error: Conexión a la base de datos no inicializada.\n");
        return;
    }

    const char *sql =
        "SELECT p.id, l.nombre, u.nombre, p.fecha_Prestamo, "
        "CASE WHEN p.fecha_Devolucion IS NULL THEN 'Activo' ELSE 'Finalizado' END "
        "FROM Prestamo p "
        "LEFT JOIN Libro l ON p.libro_id = l.id "
        "LEFT JOIN Usuario u ON p.usuario_dni = u.dni "
        "WHERE p.fecha_Devolucion IS NULL "  // FILTRAR SOLO ACTIVOS
        "ORDER BY p.fecha_Prestamo DESC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n%-4s | %-25s | %-15s | %-12s | %-10s\n",
           "ID", "Libro", "Usuario", "F. Préstamo", "Estado");
    printf("-----+---------------------------+-----------------+--------------+------------\n");

    int total = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        total++;
        int id = sqlite3_column_int(stmt, 0);
        const char *libro = (const char *)sqlite3_column_text(stmt, 1);
        const char *usuario = (const char *)sqlite3_column_text(stmt, 2);
        const char *fecha_prestamo = (const char *)sqlite3_column_text(stmt, 3);
        const char *estado = (const char *)sqlite3_column_text(stmt, 4);

        printf("%-4d | %-25s | %-15s | %-12s | %-10s\n",
               id,
               libro ? libro : "Desconocido",
               usuario ? usuario : "Desconocido",
               fecha_prestamo ? fecha_prestamo : "N/A",
               estado);
    }

    if (total == 0) {
        printf("\nNo hay préstamos activos en este momento.\n");
    }

    sqlite3_finalize(stmt);
}

// --------------------------
// GENERAR INFORMES
// --------------------------

void generarInformes(sqlite3 *db) {
    char opcion;
    do {
        printf("\n--- GENERAR INFORMES ---\n");
        printf("a. Ver estadísticas\n");
        printf("b. Libros disponibles\n");
        printf("c. Usuarios con más préstamos\n");
        printf("d. Libro más prestado\n");
        printf("e. Préstamos vencidos\n");
        printf("0. Volver\n");
        printf("Opción: ");
        fflush(stdout);

        scanf(" %c", &opcion);
        while (getchar() != '\n');

        switch (opcion) {
        case 'a':
            verEstadisticas(db);
            break;
        case 'b':
            listarLibrosDisponibles(db);
            break;
        case 'c':
            mostrarUsuarioConMasPrestamos(db);
            break;
        case 'd':
            mostrarLibroMasPrestado(db);
            break;
        case 'e':
            mostrar_prestamos_vencidos(db);
            break;
        case '0':
            return;
        default:
            printf("Opción no válida\n");
        }
    } while (1);
}

void verEstadisticas(sqlite3 *db) {
    printf("\n--- ESTADÍSTICAS DE LA BIBLIOTECA ---\n");

    sqlite3_stmt *stmt;

    const char *sql = "SELECT COUNT(*) FROM Usuario;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Total usuarios: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sql = "SELECT COUNT(*) FROM Libro;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Total libros: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sql = "SELECT COUNT(*) FROM Libro WHERE estado = 0;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Libros prestados: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }

    sql = "SELECT COUNT(*) FROM Prestamo WHERE fecha_Devolucion IS NULL;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            printf("Préstamos activos: %d\n", sqlite3_column_int(stmt, 0));
        }
        sqlite3_finalize(stmt);
    }
}


void mostrarUsuarioConMasPrestamos(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT u.nombre, u.apellidos, u.dni, COUNT(p.id) AS total_prestamos "
        "FROM Usuario u "
        "JOIN Prestamo p ON u.dni = p.usuario_dni "
        "GROUP BY u.dni "
        "ORDER BY total_prestamos DESC "
        "LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- USUARIO CON MÁS PRÉSTAMOS ---\n");

    if(sqlite3_step(stmt) == SQLITE_ROW) {
        const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
        const char *apellidos = (const char *)sqlite3_column_text(stmt, 1);
        const char *dni = (const char *)sqlite3_column_text(stmt, 2);
        int total_prestamos = sqlite3_column_int(stmt, 3);

        printf("Nombre:    %s %s\n", nombre, apellidos);
        printf("DNI:       %s\n", dni);
        printf("Préstamos: %d\n", total_prestamos);
    } else {
        printf("No hay préstamos registrados.\n");
    }

    sqlite3_finalize(stmt);
}

void mostrarLibroMasPrestado(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT l.nombre, l.autor, COUNT(p.id) AS total_prestamos "
        "FROM Libro l "
        "JOIN Prestamo p ON l.id = p.libro_id "
        "GROUP BY l.id "
        "ORDER BY total_prestamos DESC "
        "LIMIT 1;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al preparar la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- LIBRO MÁS PRESTADO ---\n");

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const char *nombre = (const char *)sqlite3_column_text(stmt, 0);
        const char *autor = (const char *)sqlite3_column_text(stmt, 1);
        int total_prestamos = sqlite3_column_int(stmt, 2);

        printf("Título:   %s\n", nombre);
        printf("Autor:    %s\n", autor);
        printf("Préstamos: %d\n", total_prestamos);
    } else {
        printf("No hay préstamos registrados.\n");
    }

    sqlite3_finalize(stmt);
}

void mostrar_prestamos_vencidos(sqlite3 *db) {
    if (db == NULL) {
        printf("Error: Conexión a la base de datos no inicializada.\n");
        return;
    }

    // Consulta para seleccionar préstamos vencidos (con fecha de devolución)
    const char *sql =
        "SELECT p.id, l.nombre, u.nombre, p.fecha_Prestamo, p.fecha_Devolucion "
        "FROM Prestamo p "
        "LEFT JOIN Libro l ON p.libro_id = l.id "
        "LEFT JOIN Usuario u ON p.usuario_dni = u.dni "
        "WHERE p.fecha_Devolucion IS NOT NULL " // FILTRAR SOLO LOS QUE TIENEN FECHA DE DEVOLUCIÓN
        "ORDER BY p.fecha_Prestamo DESC;";

    sqlite3_stmt *stmt;
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        printf("Error preparando la consulta: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n%-4s | %-25s | %-15s | %-12s | %-12s\n",
           "ID", "Libro", "Usuario", "F. Préstamo", "F. Devolución");
    printf("-----+---------------------------+-----------------+--------------+--------------\n");

    int total = 0;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        total++;
        int id = sqlite3_column_int(stmt, 0);
        const char *libro = (const char *)sqlite3_column_text(stmt, 1);
        const char *usuario = (const char *)sqlite3_column_text(stmt, 2);
        const char *fecha_prestamo = (const char *)sqlite3_column_text(stmt, 3);
        const char *fecha_devolucion = (const char *)sqlite3_column_text(stmt, 4);

        printf("%-4d | %-25s | %-15s | %-12s | %-12s\n",
               id,
               libro ? libro : "Desconocido",
               usuario ? usuario : "Desconocido",
               fecha_prestamo ? fecha_prestamo : "N/A",
               fecha_devolucion ? fecha_devolucion : "N/A");
    }

    if (total == 0) {
        printf("\nNo hay préstamos vencidos en este momento.\n");
    }

    sqlite3_finalize(stmt);
}
void listarLibros(sqlite3 *db) {
    sqlite3_stmt *stmt;
    const char *sql =
        "SELECT id, nombre, autor, genero, estado FROM Libro ORDER BY id;";

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "Error al listar libros: %s\n", sqlite3_errmsg(db));
        return;
    }

    printf("\n--- CATÁLOGO DE LIBROS ---\n");
    printf("%-5s %-30s %-20s %-15s %-10s\n", "ID", "TÍTULO", "AUTOR", "GÉNERO",
           "ESTADO");
    printf("----------------------------------------------------------------\n");

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("%-5d %-30s %-20s %-15s %-10s\n", sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1), sqlite3_column_text(stmt, 2),
               sqlite3_column_text(stmt, 3),
               sqlite3_column_int(stmt, 4) ? "Disponible" : "Prestado");
    }
    sqlite3_finalize(stmt);
}
