#include "utils.h"

t_log* logger;

/* ====================== PARTE SERVIDOR ====================== */

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
	printf("[SERVER] Pasó accept()\n");
	if (socket_cliente == -1) {
		perror("accept");
		abort();
	}

	log_info(logger, "Se conecto un cliente!");
	return socket_cliente;
}

/* ====================== RECEPCIÓN DE DATOS ====================== */

void* recibir_buffer(int* size, int socket_cliente)
{
	void* buffer = NULL;

	// Primero recibimos el tamaño del payload
	if (recv(socket_cliente, size, sizeof(int), MSG_WAITALL) <= 0) {
		*size = 0;
		return NULL;
	}

	// Luego el contenido de ese tamaño
	buffer = malloc(*size);
	if (!buffer) return NULL;

	if (recv(socket_cliente, buffer, *size, MSG_WAITALL) <= 0) {
		free(buffer);
		*size = 0;
		return NULL;
	}

	return buffer;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	int res = recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL);

	if (res > 0) return cod_op;

	// Si falla o el cliente cerró, devolvemos -1
	return -1;
}

void recibir_mensaje(int socket_cliente)
{
	int size = 0;
	char* buffer = (char*) recibir_buffer(&size, socket_cliente);
	if (buffer == NULL) {
		log_warning(logger, "No se pudo recibir el mensaje (cliente cerró o error).");
		return;
	}

	// Aseguramos terminación por las dudas (si el emisor no envió \0)
	char* msg = malloc(size + 1);
	memcpy(msg, buffer, size);
	msg[size] = '\0';

	log_info(logger, "Me llegó el mensaje: %s", msg);

	free(msg);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size_total = 0;
	int desplazamiento = 0;

	char* buffer = (char*) recibir_buffer(&size_total, socket_cliente);
	if (buffer == NULL) {
		log_warning(logger, "No se pudo recibir el paquete.");
		return NULL;
	}

	t_list* valores = list_create();
	while (desplazamiento < size_total) {
		int tamanio = 0;

		// cada elemento viene como: [int tamanio][tamanio bytes de data]
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento += sizeof(int);

		char* valor = malloc(tamanio);
		memcpy(valor, buffer + desplazamiento, tamanio);
		desplazamiento += tamanio;

		list_add(valores, valor);
	}

	free(buffer);
	return valores;
}
