#include "servidor.h"

// Server para recibir a kernel ------------------------------------------------------------------------------------------------------- 

int esperar_cliente_de_kernel()
{
	while(1) // Servidor en un estado de espera constante
	{
		pthread_t thread; // Almacenar el identificador del hilo que se creará más adelante.
		int socket_cliente = malloc(sizeof(int)); // Reservo espacio en memoria para el socket del cliente 
		socket_cliente = accept(socket_servidor, NULL, NULL);
		recv_handshake(socket_cliente);
		pthread_create(&thread, NULL,(void*) server_para_kernel,socket_cliente);
		log_info(log_cpu, "Se conecto un cliente!");
		pthread_detach(thread); // El hilo creado no necesita ser esperado o unido al hilo principal. Los recursos asociados se liberarán automáticamente
	}
	return socket_cliente;
}

void server_para_kernel() // Atiendo al cliente
{
    server_cpu = iniciar_servidor(config_cpu->puerto_escucha_dispatch, log_cpu);
    if (server_cpu == -1)
    {
	    log_info(log_cpu, "Error: No se pudo iniciar CPU como servidor para Kernel");
        exit(1);
    }

	log_info(log_cpu, "CPU listo para recibir a Kernel");
    int client_kernel = esperar_cliente_de_kernel(server_cpu, log_cpu);
   	t_list* lista;
    
	int cod_op = recibir_operacion(client_kernel);
	switch (cod_op) 
    {
	case MENSAJE:
		recibir_mensaje(client_kernel, log_cpu);
		break;
	case PAQUETE:
		lista = recibir_paquete(client_kernel);
		log_info(log_cpu, "Me llegaron los siguientes valores:\n");
		list_iterate(lista, (void*) iterator);
		list_destroy_and_destroy_elements(lista, free);
		break;
	case EXECUTE:
		lista = recibir_paquete(client_kernel);
		proceso = malloc(sizeof(pcb));
		proceso->instruccion = NULL;
		recibir_pcb(lista, proceso);
		interpretar_instrucciones();
		list_destroy_and_destroy_elements(lista, free);
		break;
	case EXIT:
		error_exit(EXIT);
		list_destroy_and_destroy_elements(lista, free); 
		break;
	case -1:
		log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
		return EXIT_FAILURE;
        exit(1);
	default:
		log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
		break;
	}
}

// Server para recibir interrupciones de kernel --------------------------------------------------------------------------------------

void interrupcion_para_kernel(){

    server_cpu = iniciar_servidor(config_cpu->puerto_escucha_interrupt, log_cpu);
    if (server_cpu == -1)
    {
	    log_info(log_cpu, "Error: No se pudo iniciar CPU como servidor para Kernel");
        exit(1);
    }

	log_info(log_cpu, "CPU listo para recibir interrupcion de Kernel");
    int client_kernel = esperar_cliente_de_kernel(server_cpu, log_cpu);

    t_list* lista;
    
	int cod_op = recibir_operacion(client_kernel);
	switch (cod_op) 
    {
        case MENSAJE:
            recibir_mensaje(client_kernel, log_cpu);
            break;
        case PAQUETE:
            lista = recibir_paquete(client_kernel);
            log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void*) iterator);
            break;
        case -1:
            log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
            return EXIT_FAILURE;
            exit(1);
        default:
            log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
            break;
	}
}