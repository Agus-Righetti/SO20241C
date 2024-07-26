#include <utils/utilsShare.h>

// Utils del server ---------------------------------------------------

int iniciar_servidor(char* puerto, t_log* logger)
{
	// Preparamos al receptor para recibir mensajes
	int socket_servidor;

// Cansado de que tu TP no conecte cuando lo ejecutás por segunda vez? da broken pipe, o simplemente rompe. Esperás 5-10 mins y de repente vuelve a andar bien? 
 
// Meté esto después de la declaración de tu socket y antes de bindearlo/conectarlo 
 
// if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
//     error("setsockopt(SO_REUSEADDR) failed");

// // siendo sockfd tu socket. 

// Voilá. Tu socket ya se puede reutilizar sin esperar.

	struct addrinfo hints, *servinfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	getaddrinfo(NULL, puerto, &hints, &servinfo);


	// Creamos el socket de escucha del servidor
	socket_servidor = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	if (setsockopt(socket_servidor, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0) 
	{
    	log_error(logger, "setsockopt(SO_REUSEADDR) failed 2");
	}
	// Asociamos el socket a un puerto
	//chequear que no sea -1
	if(bind(socket_servidor, servinfo->ai_addr, servinfo->ai_addrlen) == -1)
	{
		log_error(logger, "No se pudo hacer correctamente el bind");
		log_error(logger, "Terminando servidor ...");
		exit(1);
	}

	

	// Escuchamos las conexiones entrantes
	//chequear que no sea -1
	if(listen(socket_servidor, SOMAXCONN) == -1)
	{
		log_error(logger, "No se pudo hacer correctamente el listen");
		log_error(logger, "Terminando servidor ...");
		exit(1);
	}

	freeaddrinfo(servinfo);
	log_trace(logger, "Listo para escuchar a mi cliente");

	return socket_servidor;
}

int esperar_cliente(int socket_servidor, t_log* logger)
{
	// Una vez que se conecta el cliente
	// Aceptamos un nuevo cliente

	int socket_cliente = accept(socket_servidor,NULL,NULL);

	//log_info(logger, "Se conecto un cliente!");

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
	if(*size != 0)
	{
		recv(socket_cliente, buffer, *size, MSG_WAITALL);

	}

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



// Utils del cliente ----------------------------------------------------

void* serializar_paquete(t_paquete* paquete, int bytes)
{
	void * magic = malloc(bytes);
	int desplazamiento = 0;

	memcpy(magic + desplazamiento, &(paquete->codigo_operacion), sizeof(int));
	desplazamiento+= sizeof(int);
	memcpy(magic + desplazamiento, &(paquete->buffer->size), sizeof(int));
	desplazamiento+= sizeof(int);
	if(paquete->buffer->size != 0)
	{

		memcpy(magic + desplazamiento, paquete->buffer->stream, paquete->buffer->size);
		desplazamiento+= paquete->buffer->size;
	}

	return magic;
}


int crear_conexion(char *ip, char *puerto) {
    struct addrinfo hints;
    struct addrinfo *server_info;
    struct addrinfo *p;
    int socket_cliente;
    int conexion;
    int status;

    // Inicializamos la estructura hints
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;       // IPv4
    hints.ai_socktype = SOCK_STREAM; // TCP
    hints.ai_flags = 0;              // No usamos flags adicionales

    // Obtenemos información de la dirección del servidor
    if ((status = getaddrinfo(ip, puerto, &hints, &server_info)) != 0) {
        fprintf(stderr, "Error en getaddrinfo: %s\n", gai_strerror(status));
        return -1;
    }

    // Recorremos las direcciones posibles
    for (p = server_info; p != NULL; p = p->ai_next) {
        // Creamos el socket del cliente
        socket_cliente = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_cliente == -1) {
            perror("Error al crear el socket");
            continue; // Intentamos con la siguiente dirección
        }

        // Conectamos el socket del cliente con el del servidor
        if (connect(socket_cliente, p->ai_addr, p->ai_addrlen) == -1) {
            perror("Error al conectar");
            close(socket_cliente);
            continue; // Intentamos con la siguiente dirección
        }

        // Conexión exitosa
        break;
    }

    // Liberamos la memoria de la información de la dirección
    freeaddrinfo(server_info);

    // Verificamos si hemos salido del loop sin éxito
    if (p == NULL) {
        fprintf(stderr, "Error: No se pudo conectar a ningún servidor\n");
        return -1;
    }

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

// t_paquete* crear_paquete(void)
// {
// 	t_paquete* paquete = malloc(sizeof(t_paquete));
// 	paquete->codigo_operacion = PAQUETE;
// 	crear_buffer(paquete);
// 	return paquete;
// }

void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio)
{
	paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + tamanio + sizeof(int));

	memcpy(paquete->buffer->stream + paquete->buffer->size, &tamanio, sizeof(int));
	memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), valor, tamanio);

	paquete->buffer->size += tamanio + sizeof(int);
}

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

//************ SERIALIZACION PERSONALIZADA *******************
t_paquete* crear_paquete_personalizado(op_code code_op){
	t_paquete* paquete_personalizado = malloc(sizeof(t_paquete));
	// indicamos el codigo de operacion que queramos
	paquete_personalizado->codigo_operacion = code_op;

	crear_buffer(paquete_personalizado);
	return  paquete_personalizado;
}

//AYUDITA DEL CHATI------------------------------------------------------------------

// void agregar_lista_al_paquete_personalizado(t_paquete* paquete, t_list* lista, int size_data) {
//     // Serializar el tamaño de la lista
//     if (paquete->buffer->size == 0) {
//         paquete->buffer->stream = malloc(sizeof(int));
//     } else {
//         paquete->buffer->stream = realloc(paquete->buffer->stream, 
//                                           paquete->buffer->size + sizeof(int));
//     }
//     memcpy(paquete->buffer->stream + paquete->buffer->size, &(lista->elements_count), sizeof(int));
//     paquete->buffer->size += sizeof(int);

//     // Serializar cada elemento de la lista
//     t_link_element* current = lista->head;
//     while (current != NULL) {
//         // Redimensionar el buffer para el nuevo elemento
//         paquete->buffer->stream = realloc(paquete->buffer->stream, 
//                                           paquete->buffer->size + sizeof(int) + size_data);
//         // Serializar el tamaño del elemento
//         memcpy(paquete->buffer->stream + paquete->buffer->size, &size_data, sizeof(int));
//         // Serializar el elemento
//         memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), current->data, size_data);
        
//         paquete->buffer->size += sizeof(int) + size_data;

//         current = current->next;
//     }
// }

void agregar_lista_al_paquete_personalizado(t_paquete* paquete, t_list* lista, int size_data) { //hace lo mismo pero desde las commons
    // Serializar el tamaño de la lista
    if (paquete->buffer->size == 0) {
        paquete->buffer->stream = malloc(sizeof(int));
    } else {
        paquete->buffer->stream = realloc(paquete->buffer->stream, 
                                          paquete->buffer->size + sizeof(int));
    }
	int test = list_size(lista);
    memcpy(paquete->buffer->stream + paquete->buffer->size, &test, sizeof(int));
    paquete->buffer->size += sizeof(int);

    // Serializar cada elemento de la lista
    t_list_iterator* iter = list_iterator_create(lista);
    if (iter == NULL) {
        printf("\n[ERROR] No se pudo crear el iterador de la lista\n\n");
        exit(EXIT_FAILURE);
    }

    while (list_iterator_has_next(iter)) {
        void* elemento = list_iterator_next(iter);
        if (elemento == NULL) {
            printf("\n[ERROR] Elemento nulo encontrado durante la serialización\n\n");
            list_iterator_destroy(iter);
            exit(EXIT_FAILURE);
        }

        // Redimensionar el buffer para el nuevo elemento
        paquete->buffer->stream = realloc(paquete->buffer->stream, 
                                          paquete->buffer->size + sizeof(int) + size_data);
        // Serializar el tamaño del elemento
        memcpy(paquete->buffer->stream + paquete->buffer->size, &size_data, sizeof(int));
        // Serializar el elemento
        memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), elemento, size_data);
        
        paquete->buffer->size += sizeof(int) + size_data;
    }

    list_iterator_destroy(iter);
}

// void agregar_lista_al_paquete_personalizado(t_paquete* paquete, t_list* lista) {
    
// 	// Serializar el tamaño de la lista
//     if (paquete->buffer->size == 0) {
//         paquete->buffer->stream = malloc(sizeof(int));
//         memcpy(paquete->buffer->stream, &(lista->elements_count), sizeof(int));
//     } else {
//         paquete->buffer->stream = realloc(paquete->buffer->stream, 
//                                           paquete->buffer->size + sizeof(int));
//         memcpy(paquete->buffer->stream + paquete->buffer->size, &(lista->elements_count), sizeof(int));
//     }
//     paquete->buffer->size += sizeof(int);

//     // Serializar cada elemento de la lista
//     t_link_element* current = lista->head;
//     while (current != NULL) {
//         int size_data = sizeof(current->data); // Asumiendo que data es un int para simplificar
//         if (paquete->buffer->size == 0) {
//             paquete->buffer->stream = malloc(sizeof(int) + size_data);
//             memcpy(paquete->buffer->stream, &size_data, sizeof(int));
//             memcpy(paquete->buffer->stream + sizeof(int), current->data, size_data);
//         } else {
//             paquete->buffer->stream = realloc(paquete->buffer->stream, 
//                                               paquete->buffer->size + sizeof(int) + size_data);
//             memcpy(paquete->buffer->stream + paquete->buffer->size, &size_data, sizeof(int));
//             memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), current->data, size_data);
//         }
//         paquete->buffer->size += sizeof(int) + size_data;

//         current = current->next;
//     }
// }
//FIN AYUDITA DEL CHATI------------------------------------------------------------------

void agregar_int_al_paquete_personalizado(t_paquete* paquete, int valor){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int));
		memcpy(paquete->buffer->stream, &valor, sizeof(int));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(int));
	}
	paquete->buffer->size += sizeof(int);
}

void agregar_uint32_al_paquete_personalizado(t_paquete* paquete, uint32_t valor){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(uint32_t));
		memcpy(paquete->buffer->stream, &valor, sizeof(uint32_t));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint32_t));
		memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(uint32_t));
	}
	paquete->buffer->size += sizeof(uint32_t);
}


void agregar_uint8_al_paquete_personalizado(t_paquete* paquete, uint8_t valor){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(uint8_t));
		memcpy(paquete->buffer->stream, &valor, sizeof(uint8_t));
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream, paquete->buffer->size + sizeof(uint8_t));
		memcpy(paquete->buffer->stream + paquete->buffer->size, &valor, sizeof(uint8_t));
	}
	paquete->buffer->size += sizeof(uint8_t);
}

void agregar_string_al_paquete_personalizado(t_paquete* paquete, char* string){
	int size_string = strlen(string)+1;

	//si esta vacio el paquete
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + sizeof(char)*size_string);
		memcpy(paquete->buffer->stream, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), string, sizeof(char)*size_string);

	//el paquete no esta vacio
	}else {
		paquete->buffer->stream = realloc(paquete->buffer->stream,
										paquete->buffer->size + sizeof(int) + sizeof(char)*size_string);
		/**/
		memcpy(paquete->buffer->stream + paquete->buffer->size, &size_string, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), string, sizeof(char)*size_string);

	}
	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += sizeof(char)*size_string;
}

void agregar_estructura_al_paquete_personalizado(t_paquete* paquete, void* estructura, int size){
	if(paquete->buffer->size == 0){
		paquete->buffer->stream = malloc(sizeof(int) + size);
		memcpy(paquete->buffer->stream, &size, sizeof(int));
		memcpy(paquete->buffer->stream + sizeof(int), estructura, size);
	}else{
		paquete->buffer->stream = realloc(paquete->buffer->stream,
												paquete->buffer->size + sizeof(int) + size);

		memcpy(paquete->buffer->stream + paquete->buffer->size, &size, sizeof(int));
		memcpy(paquete->buffer->stream + paquete->buffer->size + sizeof(int), estructura, size);
	}

	paquete->buffer->size += sizeof(int);
	paquete->buffer->size += size;
}

t_buffer* recibiendo_paquete_personalizado(int socket_conexion){
	t_buffer* unBuffer = malloc(sizeof(t_buffer));
	int size;
	unBuffer->stream = recibir_buffer(&size, socket_conexion);
	unBuffer->size = size;
	return unBuffer;
}

int recibir_int_del_buffer(t_buffer* buffer){
	if(buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int valor_a_devolver;
	memcpy(&valor_a_devolver, buffer->stream, sizeof(int));

	int nuevo_size = buffer->size - sizeof(int);
	if(nuevo_size == 0){
		free(buffer->stream);
		buffer->stream = NULL;
		buffer->size = 0;
		return valor_a_devolver;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_INT]: BUFFER CON TAMANIO NEGATIVO\n\n");
		//free(valor_a_devolver);
		//return 0;
		exit(EXIT_FAILURE);
	}
	void* nuevo_buffer = malloc(nuevo_size);
	memcpy(nuevo_buffer, buffer->stream + sizeof(int), nuevo_size);
	free(buffer->stream);
	buffer->stream = nuevo_buffer;
	buffer->size = nuevo_size;

	return valor_a_devolver;
}

char* recibir_string_del_buffer(t_buffer* buffer){
	if(buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_string;
	char* string;
	memcpy(&size_string, buffer->stream, sizeof(int));
	//string = malloc(sizeof(size_string));
	string = malloc(size_string);
	memcpy(string, buffer->stream + sizeof(int), size_string);

	int nuevo_size = buffer->size - sizeof(int) - size_string;
	if(nuevo_size == 0){
		free(buffer->stream);
		buffer->stream = NULL;
		buffer->size = 0;
		return string;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_STRING]: BUFFER CON TAMANIO NEGATIVO\n\n");
		free(string);
		//return "[ERROR]: BUFFER CON TAMAÑO NEGATIVO";
		exit(EXIT_FAILURE);
	}
	void* nuevo_buffer = malloc(nuevo_size);
	memcpy(nuevo_buffer, buffer->stream + sizeof(int) + size_string, nuevo_size);
	free(buffer->stream);
	buffer->stream = nuevo_buffer;
	buffer->size = nuevo_size;

	return string;
}

void* recibir_estructura_del_buffer(t_buffer* buffer){
	if(buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un contenido de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	int size_estructura;
	void* estructura;
	memcpy(&size_estructura, buffer->stream, sizeof(int));
	estructura = malloc(size_estructura);
	memcpy(estructura, buffer->stream + sizeof(int), size_estructura);

	int nuevo_size = buffer->size - sizeof(int) - size_estructura;
	if(nuevo_size == 0){
		free(buffer->stream);
		buffer->stream = NULL;
		buffer->size = 0;
		return estructura;
	}
	if(nuevo_size < 0){
		printf("\nBUFFER CON TAMANIO NEGATIVO\n\n");
		//free(estructura);
		//return "";
		exit(EXIT_FAILURE);
	}
	void* nuevo_estructura = malloc(nuevo_size);
	memcpy(nuevo_estructura, buffer->stream + sizeof(int) + size_estructura, nuevo_size);
	free(buffer->stream);
	buffer->stream = nuevo_estructura;
	buffer->size = nuevo_size;

	return estructura;
}



// t_list* recibir_lista_del_buffer(t_buffer* buffer,int size_data) {
//     if (buffer->size == 0) {
//         printf("\n[ERROR] Al intentar extraer una lista de un t_buffer vacío\n\n");
//         exit(EXIT_FAILURE);
//     }

//     if (buffer->size < 0) {
//         printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
//         exit(EXIT_FAILURE);
//     }

//     // Recibir el tamaño de la lista
//     int size_lista;
//     memcpy(&size_lista, buffer->stream, size_data);

//     buffer->stream += size_data;
//     buffer->size -= size_data;

//     // Crear la lista
//     t_list* lista = malloc(sizeof(t_list));
//     lista->head = NULL;
//     lista->elements_count = size_lista;

//     t_link_element** current = &(lista->head);

//     for (int i = 0; i < size_lista; i++) {
//         // Recibir el tamaño del elemento
//         int size_elemento;
//         memcpy(&size_elemento, buffer->stream, size_data);

//         buffer->stream += size_data;
//         buffer->size -= size_data;

//         if (buffer->size < size_elemento) {
//             printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO INSUFICIENTE\n\n");
//             exit(EXIT_FAILURE);
//         }

//         // Recibir el elemento
//         void* elemento = malloc(size_elemento);
//         memcpy(elemento, buffer->stream, size_elemento);

//         buffer->stream += size_elemento;
//         buffer->size -= size_elemento;

//         // Agregar el elemento a la lista
//         t_link_element* nuevo_elemento = malloc(sizeof(t_link_element));
//         nuevo_elemento->data = elemento;
//         nuevo_elemento->next = NULL;

//         *current = nuevo_elemento;
//         current = &(nuevo_elemento->next);
//     }

//     return lista;
// }

//sugerenciadel chat
// t_list* recibir_lista_del_buffer(t_buffer* buffer, int size_data) {
//     if (buffer->size <= 0) {
//         printf("\n[ERROR] Al intentar extraer una lista de un t_buffer vacío o corrupto\n\n");
//         exit(EXIT_FAILURE);
//     }

//     // Recibir el tamaño de la lista
//     int size_lista;
//     if (buffer->size < size_data) {
//         printf("\n[ERROR] BUFFER CON TAMAÑO INSUFICIENTE PARA LEER EL TAMAÑO DE LA LISTA\n\n");
//         exit(EXIT_FAILURE);
//     }
//     memcpy(&size_lista, buffer->stream, size_data);

//     buffer->stream += size_data;
//     buffer->size -= size_data;

//     // Crear la lista
//     t_list* lista = malloc(sizeof(t_list));
//     if (lista == NULL) {
//         printf("\n[ERROR] No se pudo asignar memoria para la lista\n\n");
//         exit(EXIT_FAILURE);
//     }
//     lista->head = NULL;
//     lista->elements_count = size_lista;

//     t_link_element** current = &(lista->head);

//     for (int i = 0; i < size_lista; i++) {
//         // Recibir el tamaño del elemento
//         int size_elemento;
//         if (buffer->size < size_data) {
//             printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO INSUFICIENTE PARA LEER EL TAMAÑO DEL ELEMENTO\n\n");
//             exit(EXIT_FAILURE);
//         }
//         memcpy(&size_elemento, buffer->stream, size_data);

//         buffer->stream += size_data;
//         buffer->size -= size_data;

//         if (buffer->size < size_elemento) {
//             printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO INSUFICIENTE PARA LEER EL ELEMENTO\n\n");
//             exit(EXIT_FAILURE);
//         }

//         // Recibir el elemento
//         void* elemento = malloc(size_elemento);
//         if (elemento == NULL) {
//             printf("\n[ERROR] No se pudo asignar memoria para el elemento\n\n");
//             exit(EXIT_FAILURE);
//         }
//         memcpy(elemento, buffer->stream, size_elemento);

//         buffer->stream += size_elemento;
//         buffer->size -= size_elemento;

//         // Agregar el elemento a la lista
//         t_link_element* nuevo_elemento = malloc(sizeof(t_link_element));
//         if (nuevo_elemento == NULL) {
//             printf("\n[ERROR] No se pudo asignar memoria para el nuevo elemento de la lista\n\n");
//             exit(EXIT_FAILURE);
//         }
//         nuevo_elemento->data = elemento;
//         nuevo_elemento->next = NULL;

//         // Verificar que 'current' sea válido antes de usarlo
//         if (current == NULL) {
//             printf("\n[ERROR] El puntero 'current' es NULL\n\n");
//             exit(EXIT_FAILURE);
//         }

//         *current = nuevo_elemento;
//         current = &(nuevo_elemento->next);
//     }

//     return lista;
// }

t_list* recibir_lista_del_buffer(t_buffer* buffer, int size_data) {
    if (buffer->size == 0) {
        printf("\n[ERROR] Al intentar extraer una lista de un t_buffer vacío\n\n");
        exit(EXIT_FAILURE);
    }

    if (buffer->size < 0) {
        printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO\n\n");
        exit(EXIT_FAILURE);
    }

    // Leer el tamaño de la lista
    int size_lista;
    if (buffer->size < sizeof(int)) {
        printf("\n[ERROR] Buffer demasiado pequenio para leer el tamanio de la lista\n\n");
        exit(EXIT_FAILURE);
    }
    memcpy(&size_lista, buffer->stream, sizeof(int));

    // Crear la lista
    t_list* lista = list_create();
    if (lista == NULL) {
        printf("\n[ERROR] No se pudo crear la lista\n\n");
        exit(EXIT_FAILURE);
    }

    // Avanzar el puntero buffer->stream y ajustar buffer->size
    void* nuevo_stream = buffer->stream + sizeof(int);
    int nuevo_size = buffer->size - sizeof(int);

    for (int i = 0; i < size_lista; i++) {
        // Leer el tamaño del elemento
        int size_elemento;
        if (nuevo_size < sizeof(int)) {
            printf("\n[ERROR] Buffer demasiado pequenio para leer el tamanio del elemento\n\n");
            list_destroy(lista);
            exit(EXIT_FAILURE);
        }
        memcpy(&size_elemento, nuevo_stream, sizeof(int));
        nuevo_stream += sizeof(int);
        nuevo_size -= sizeof(int);

        if (nuevo_size < size_elemento) {
            printf("\n[ERROR_LISTA]: BUFFER CON TAMANIO INSUFICIENTE\n\n");
            list_destroy(lista);
            exit(EXIT_FAILURE);
        }

        // Leer el elemento
        void* elemento = malloc(size_elemento);
        if (elemento == NULL) {
            printf("\n[ERROR] No se pudo alocar memoria para el elemento\n\n");
            list_destroy(lista);
            exit(EXIT_FAILURE);
        }
        memcpy(elemento, nuevo_stream, size_elemento);
        nuevo_stream += size_elemento;
        nuevo_size -= size_elemento;

        // Agregar el elemento a la lista
        list_add(lista, elemento);
    }

    // Actualizar el stream y size del buffer
    memmove(buffer->stream, nuevo_stream, nuevo_size);
    buffer->size = nuevo_size;

    //printf("La lista que recibi del buffer tiene %d elementos", list_size(lista));
    return lista;
}

//capaz esta si///////////////ESTA ERA LA ULTIMA

// t_list* recibir_lista_del_buffer(t_buffer* buffer, int size_data) {
//     if (buffer->size == 0) {
//         printf("\n[ERROR] Al intentar extraer una lista de un t_buffer vacío\n\n");
//         exit(EXIT_FAILURE);
//     }

//     if (buffer->size < 0) {
//         printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO\n\n");
//         exit(EXIT_FAILURE);
//     }

//     // Leer el tamaño de la lista
//     int size_lista;
//     if (buffer->size < sizeof(int)) {
//         printf("\n[ERROR] Buffer demasiado pequeño para leer el tamaño de la lista\n\n");
//         exit(EXIT_FAILURE);
//     }
//     memcpy(&size_lista, buffer->stream, sizeof(int));
//     buffer->stream += sizeof(int);
//     buffer->size -= sizeof(int);

//     // Crear la lista
//     t_list* lista = list_create();
//     if (lista == NULL) {
//         printf("\n[ERROR] No se pudo crear la lista\n\n");
//         exit(EXIT_FAILURE);
//     }

//     for (int i = 0; i < size_lista; i++) {
//         // Leer el tamaño del elemento
//         int size_elemento;
//         if (buffer->size < sizeof(int)) {
//             printf("\n[ERROR] Buffer demasiado pequeño para leer el tamaño del elemento\n\n");
//             list_destroy(lista);
//             exit(EXIT_FAILURE);
//         }
//         memcpy(&size_elemento, buffer->stream, sizeof(int));
//         buffer->stream += sizeof(int);
//         buffer->size -= sizeof(int);

//         if (buffer->size < size_elemento) {
//             printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO INSUFICIENTE\n\n");
//             list_destroy(lista);
//             exit(EXIT_FAILURE);
//         }

//         // Leer el elemento
//         void* elemento = malloc(size_elemento);
//         if (elemento == NULL) {
//             printf("\n[ERROR] No se pudo alocar memoria para el elemento\n\n");
//             list_destroy(lista);
//             exit(EXIT_FAILURE);
//         }
//         memcpy(elemento, buffer->stream, size_elemento);
//         buffer->stream += size_elemento;
//         buffer->size -= size_elemento;

//         // Agregar el elemento a la lista
//         list_add(lista, elemento);


        

//     }

// 	printf("La lista que recibi del buffer tiene %d elementos", list_size(lista));
//     return lista;
// }
// t_list* recibir_lista_del_buffer(t_buffer* buffer) {
//     if(buffer->size == 0){
//         printf("\n[ERROR] Al intentar extraer una lista de un t_buffer vacio\n\n");
//         exit(EXIT_FAILURE);
//     }

//     if(buffer->size < 0){
//         printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
//         exit(EXIT_FAILURE);
//     }

//     // Recibir el tamaño de la lista
//     int size_lista;
//     memcpy(&size_lista, buffer->stream, sizeof(int));

//     int nuevo_size = buffer->size - sizeof(int);
//     if(nuevo_size < 0){
//         printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO NEGATIVO\n\n");
//         exit(EXIT_FAILURE);
//     }

//     void* nuevo_buffer = malloc(nuevo_size);
//     memcpy(nuevo_buffer, buffer->stream + sizeof(int), nuevo_size);
//     free(buffer->stream);
//     buffer->stream = nuevo_buffer;
//     buffer->size = nuevo_size;

//     // Crear la lista
//     t_list* lista = malloc(sizeof(t_list));
//     lista->head = NULL;
//     lista->elements_count = 0;

//     for(int i = 0; i < size_lista; i++){
//         // Recibir el tamaño del elemento
//         int size_elemento;
//         memcpy(&size_elemento, buffer->stream, sizeof(int));

//         nuevo_size = buffer->size - sizeof(int);
//         if(nuevo_size < 0){
//             printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO NEGATIVO\n\n");
//             exit(EXIT_FAILURE);
//         }

//         nuevo_buffer = malloc(nuevo_size);
//         memcpy(nuevo_buffer, buffer->stream + sizeof(int), nuevo_size);
//         free(buffer->stream);
//         buffer->stream = nuevo_buffer;
//         buffer->size = nuevo_size;

//         // Recibir el elemento
//         void* elemento = malloc(size_elemento);
//         memcpy(elemento, buffer->stream, size_elemento);

//         nuevo_size = buffer->size - size_elemento;
//         if(nuevo_size < 0){
//             printf("\n[ERROR_LISTA]: BUFFER CON TAMAÑO NEGATIVO\n\n");
//             free(elemento);
//             exit(EXIT_FAILURE);
//         }

//         nuevo_buffer = malloc(nuevo_size);
//         memcpy(nuevo_buffer, buffer->stream + size_elemento, nuevo_size);
//         free(buffer->stream);
//         buffer->stream = nuevo_buffer;
//         buffer->size = nuevo_size;

//         // Agregar el elemento a la lista
//         t_link_element* nuevo_elemento = malloc(sizeof(t_link_element));
//         nuevo_elemento->data = elemento;
//         nuevo_elemento->next = lista->head;
//         lista->head = nuevo_elemento;
//         lista->elements_count++;
//     }

//     return lista;
// }

uint32_t recibir_uint32_del_buffer(t_buffer* buffer){
	if(buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	uint32_t valor_a_devolver;
	memcpy(&valor_a_devolver, buffer->stream, sizeof(uint32_t));

	uint32_t nuevo_size = buffer->size - sizeof(uint32_t);
	if(nuevo_size == 0){
		// free(buffer->stream);
		buffer->stream = NULL;
		buffer->size = 0;
		return valor_a_devolver;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_uint32_t]: BUFFER CON TAMANIO NEGATIVO\n\n");
		//free(valor_a_devolver);
		//return 0;
		exit(EXIT_FAILURE);
	}
	void* nuevo_buffer = malloc(nuevo_size);
	memcpy(nuevo_buffer, buffer->stream + sizeof(uint32_t), nuevo_size);
	free(buffer->stream);
	buffer->stream = nuevo_buffer;
	buffer->size = nuevo_size;

	return valor_a_devolver;
}


uint8_t recibir_uint8_del_buffer(t_buffer* buffer){
	if(buffer->size == 0){
		printf("\n[ERROR] Al intentar extraer un INT de un t_buffer vacio\n\n");
		exit(EXIT_FAILURE);
	}

	if(buffer->size < 0){
		printf("\n[ERROR] Esto es raro. El t_buffer contiene un size NEGATIVO \n\n");
		exit(EXIT_FAILURE);
	}

	uint8_t valor_a_devolver;
	memcpy(&valor_a_devolver, buffer->stream, sizeof(uint8_t));

	uint8_t nuevo_size = buffer->size - sizeof(uint8_t);
	if(nuevo_size == 0){
		// free(buffer->stream);
		buffer->stream = NULL;
		buffer->size = 0;
		return valor_a_devolver;
	}
	if(nuevo_size < 0){
		printf("\n[ERROR_uint8_t]: BUFFER CON TAMANIO NEGATIVO\n\n");
		//free(valor_a_devolver);
		//return 0;
		exit(EXIT_FAILURE);
	}
	void* nuevo_buffer = malloc(nuevo_size);
	memcpy(nuevo_buffer, buffer->stream + sizeof(uint8_t), nuevo_size);
	free(buffer->stream);
	buffer->stream = nuevo_buffer;
	buffer->size = nuevo_size;

	return valor_a_devolver;
}
