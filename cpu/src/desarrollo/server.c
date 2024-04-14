#include "../include/general.h"

int server_memoria;
// void iterator(char* value) 
// {
// 	log_info(log_cpu,"%s", value);
// }

// ********* SERVER PARA RECIBIR A KERNEL *********
void server_para_kernel(cpu_config* config_cpu,t_log* log_cpu){

    int server_cpu = iniciar_servidor(config_cpu->puerto_escucha_dispatch, log_cpu);
    if (server_cpu == -1)
    {
	    log_info(log_cpu, "Error: No se pudo iniciar CPU como servidor para Kernel");
        exit(1);
    }

	log_info(log_cpu, "CPU listo para recibir a Kernel");
    int client_kernel = esperar_cliente(server_cpu, log_cpu);

    // t_list* lista;
    
	int cod_op = recibir_operacion(client_kernel);
	switch (cod_op) 
    {
	case MENSAJE:
		recibir_mensaje(client_kernel, log_cpu);
		break;
	// case PAQUETE:
	// 	lista = recibir_paquete(client_kernel);
	// 	log_info(log_cpu, "Me llegaron los siguientes valores:\n");
	// 	list_iterate(lista, (void*) iterator);
	// 	break;
	case -1:
		log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
		// return EXIT_FAILURE;
        exit(1);
	default:
		log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
		break;
	}
}
