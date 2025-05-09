#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PUERTO 8080

int main() {
    int servidor_fd, cliente_fd;
    struct sockaddr_in direccion;
    socklen_t addrlen = sizeof(direccion);
    char buffer[1024];

    servidor_fd = socket(AF_INET, SOCK_STREAM, 0);
    direccion.sin_family = AF_INET;
    direccion.sin_addr.s_addr = INADDR_ANY;
    direccion.sin_port = htons(PUERTO);

    bind(servidor_fd, (struct sockaddr *)&direccion, sizeof(direccion));
    listen(servidor_fd, 3);
    printf("Servidor escuchando en el puerto %d...\n", PUERTO);

    cliente_fd = accept(servidor_fd, (struct sockaddr *)&direccion, &addrlen);
    printf("Cliente conectado\n");

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        read(cliente_fd, buffer, sizeof(buffer));

        if (strncmp(buffer, "SALIR", 5) == 0) break;
        else if (strncmp(buffer, "LOGIN", 5) == 0)
            write(cliente_fd, "Login recibido\n", 15);
        else if (strncmp(buffer, "LISTAR_LIBROS", 13) == 0)
            write(cliente_fd, "Lista de libros:\n1. Cien Años de Soledad\n2. Don Quijote\n", 56);
        else
            write(cliente_fd, "Comando no reconocido\n", 23);
    }

    close(cliente_fd);
    close(servidor_fd);
    return 0;
}
