#include "servidor.h"

// Server para recibir a kernel ------------------------------------------------------------------------------------------------------- 

// int esperar_cliente_de_kernel(int socket_servidor, t_log * log_cpu)
// {
//     int socket_cliente;

//     while(1) 
//     {
//         socket_cliente = accept(socket_servidor, NULL, NULL);
//         if (socket_cliente == -1) {
//             log_error(log_cpu, "Error al aceptar la conexión del cliente.");
//             continue; // Continuar esperando conexiones
//         }

//         log_info(log_cpu, "Se conectó un cliente!");

//         // Llamar a la función para atender al cliente en el mismo hilo
//         atender_memoria(socket_cliente, log_cpu);

// 		// Liberar cosas
//     }
	
//     return socket_cliente;
// }

void server_para_kernel() // Atiendo al cliente
{
    int socket_servidor_cpu = iniciar_servidor(config_cpu->puerto_escucha_dispatch, log_cpu);
    if (socket_servidor_cpu == -1)
    {
	    log_info(log_cpu, "ERROR: No se pudo iniciar CPU como servidor para KERNEL");
        exit(1);
    }

	log_info(log_cpu, "Esperando a KERNEL...");

    int socket_cliente_kernel = esperar_cliente(socket_servidor_cpu, log_cpu);
}

// Server para recibir interrupciones de Kernel ---------------------------------------------------------------------------------------

void interrupcion_para_kernel()
{	
    socket_servidor_cpu = iniciar_servidor(config_cpu->puerto_escucha_interrupt, log_cpu);
    if (socket_servidor_cpu == -1)
    {
	    log_info(log_cpu, "Error: No se pudo iniciar CPU como servidor para Kernel");
        exit(1);
    }

	log_info(log_cpu, "CPU listo para recibir interrupcion de Kernel");
    int client_kernel = esperar_cliente(socket_servidor_cpu, log_cpu);

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

