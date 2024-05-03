#include <utils/utilsShare.h>

// Utils del server ---------------------------------------------------

int iniciar_servidor(char* puerto, t_log* logger)
{
	// Preparamos al receptor para recibir mensajes
	int socket_servidor;
	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);

	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	// Asociamos el socket a un puerto
	bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen);

	// Escuchamos las conexiones entrantes
	listen(socket_servidor, SOMAXCONN);

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger)
{
	// Una vez que se conecta el cliente
	// Aceptamos un nuevo cliente

	int socket_cliente = accept(socket_servidor,NULL,NULL);

	log_info(logger, "Se conecto un cliente!");

	return socket_cliente;
}

int recibir_operacion(int socket_cliente)
{
	int cod_op;
	if(recv(socket_cliente, &cod_op, sizeof(int), MSG_WAITALL) > 0)
		return cod_op;
	else
	{
		close(socket_cliente);
		return -1;
	}
}

void* recibir_buffer(int* size, int socket_cliente)
{
	void * buffer;

	recv(socket_cliente, size, sizeof(int), MSG_WAITALL);
	buffer = malloc(*size);
	recv(socket_cliente, buffer, *size, MSG_WAITALL);

	return buffer;
}

void recibir_mensaje(int socket_cliente, t_log* logger)
{
	int size;
	char* buffer = recibir_buffer(&size, socket_cliente);
	log_info(logger, "Me llego el mensaje: %s", buffer);
	free(buffer);
}

t_list* recibir_paquete(int socket_cliente)
{
	int size;
	int desplazamiento = 0;
	void * buffer;
	t_list* valores = list_create();
	int tamanio;

	buffer = recibir_buffer(&size, socket_cliente);
	while(desplazamiento < size)
	{
		memcpy(&tamanio, buffer + desplazamiento, sizeof(int));
		desplazamiento+=sizeof(int);
		char* valor = malloc(tamanio);
		memcpy(valor, buffer+desplazamiento, tamanio);
		desplazamiento+=tamanio;
		list_add(valores, valor);
	}
	free(buffer);
	return valores;
}


// Utils del client ----------------------------------------------------

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
	desplazamiento+= paquete->buffer->size;

	return magic;
}

int crear_conexion(char *ip, char *puerto)
{
	struct addrinfo hints;
	struct addrinfo *server_info;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(ip, puerto, &hints, &server_info);

	// creamos el socket del cliente
	int socket_cliente = socket(server_info->ai_family,
								server_info->ai_socktype,
								server_info->ai_protocol);

	//conectamos el socket del cliente con el del servidor

	
	int conexion = connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	
	// COMENTAR LAS SIGUIENTES LINEAS SI QUEREMOS PROBAR UN MODULO SOLO
	 if (conexion == -1){
	 	printf("ERROR DE CONEXION\n");
	 	printf("Esperando 5 segundos para buscar al cliente...\n");
     	sleep(5);
	 	connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen);
	 	if (connect(socket_cliente, server_info->ai_addr, server_info->ai_addrlen) == -1){
	 		perror("Error al intentar conectar");
         	exit(1);
	 	}
	 }

	freeaddrinfo(server_info);

	return socket_cliente;
}

void enviar_mensaje(char* mensaje, int socket_cliente)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));

	paquete->codigo_operacion = MENSAJE;
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = strlen(mensaje) + 1;
	paquete->buffer->stream = malloc(paquete->buffer->size);
	memcpy(paquete->buffer->stream, mensaje, paquete->buffer->size);

	int bytes = paquete->buffer->size + 2*sizeof(int);

	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
	eliminar_paquete(paquete);
}

void crear_buffer(t_paquete* paquete)
{
	paquete->buffer = malloc(sizeof(t_buffer));
	paquete->buffer->size = 0;
	paquete->buffer->stream = NULL;
}

t_paquete* crear_paquete(void)
{
	t_paquete* paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = PAQUETE;
	crear_buffer(paquete);
	return paquete;
}

// t_paquete* crear_paquete_personalizado(op_code code_op){
// 	t_paquete* paquete_personalizado = malloc(sizeof(t_paquete));
// 	paquete_personalizado->codigo_operacion = code_op;
// 	crear_buffer(paquete_personalizado);
// 	return  paquete_personalizado;
// }

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

// void cargar_string_a_paquete_personalizado(t_paquete* paquete, char* string){
// 	int tamanio_string = strlen(string)+1;

// 	if(paquete->buffer->size == 0){
// 		paquete->buffer->stream = malloc(sizeof(int) + sizeof(char)*tamanio_string);
// 		memcpy(paquete->buffer->stream, &tamanio_string, sizeof(int));
// 		memcpy(paquete->buffer->stream + sizeof(int), string, sizeof(char)*tamanio_string);

// 	}else {
// 		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int) + sizeof(char)*tamanio_string);
// 		memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio_string, sizeof(int));
// 		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char)*tamanio_string);

// 	}
// 	paquete->buffer->size += sizeof(int);
// 	paquete->buffer->size += sizeof(char)*tamanio_string;
// }

// t_buffer* recibiendo_paquete_personalizado(int conexion){
// 	t_buffer* unBuffer = malloc(sizeof(t_buffer));
// 	int size;
// 	unBuffer->stream = recibir_buffer(&size, conexion);
// 	unBuffer->size = size;
// 	return unBuffer;
// }

void enviar_paquete(t_paquete* paquete, int socket_cliente)
{
	int bytes = paquete->buffer->size + 2*sizeof(int);
	void* a_enviar = serializar_paquete(paquete, bytes);

	send(socket_cliente, a_enviar, bytes, 0);

	free(a_enviar);
}

void eliminar_paquete(t_paquete* paquete)
{
	free(paquete->buffer->stream);
	free(paquete->buffer);
	free(paquete);
}

void liberar_conexion(int socket_cliente)
{
	close(socket_cliente);
}
