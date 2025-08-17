#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

t_log* logger;

int iniciar_servidor(void)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	// assert(!"no implementado!");

	int socket_servidor = -1;
	int yes = 1;

	struct addrinfo hints, *servinfo, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	int rv = getaddrinfo(NULL, PUERTO, &hints, &servinfo);
	if (rv != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		abort();
	}

	for (p = servinfo; p != NULL; p = p->ai_next) {

		// Creamos el socket de escucha del servidor
		socket_servidor = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if (socket_servidor == -1) {
			continue;
		}
		setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

		// Asociamos el socket a un puerto
		if (bind(socket_servidor, p->ai_addr, p->ai_addrlen) == -1) {
			close(socket_servidor);
			socket_servidor = -1;
			continue;
		}

		// Escuchamos las conexiones entrantes
		if (listen(socket_servidor, SOMAXCONN) == -1) {
			close(socket_servidor);
			socket_servidor = -1;
			continue;
		}

		// si llegamos acá: socket listo
		break;
	}

	freeaddrinfo(servinfo);

	if (socket_servidor == -1) {
		fprintf(stderr, "No pude iniciar el servidor en el puerto %s\n", PUERTO);
		abort();
	}

	log_trace(logger, "Listo para escuchar a mi cliente");
	return socket_servidor;
}

int esperar_cliente(int socket_servidor)
{
	// Quitar esta línea cuando hayamos terminado de implementar la funcion
	// assert(!"no implementado!");

	// Aceptamos un nuevo cliente
	struct sockaddr_storage cliente_addr;
	socklen_t addr_size = sizeof(cliente_addr);

	int socket_cliente = accept(socket_servidor, (struct sockaddr*)&cliente_addr, &addr_size);
	if (socket_cliente == -1) {
		perror("accept");
		abort();
	}

	log_info(logger, "Se conecto un cliente!");
	return socket_cliente;
}
