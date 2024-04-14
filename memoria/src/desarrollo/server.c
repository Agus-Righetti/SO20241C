#include "../include/general.h"

int server_memoria;

// Despues lo vamos a tener que usar si hacemos paquetes
// void iterator(char* value) 
// {
// 	log_info(log_memoria, "%s", value);
// }

// ********* SERVER PARA RECIBIR A CPU *********
void server_para_cpu(memoria_config* config_memoria, t_log* log_memoria){

    server_memoria = iniciar_servidor(config_memoria->puerto_escucha, log_memoria);
    if (server_memoria == -1)
    {
	    log_info(log_memoria, "Error: No se pudo iniciar Memoria como servidor para CPU");
        exit(1);
    }
    
	log_info(log_memoria, "Memoria lista para recibir a CPU");
	int client_cpu = esperar_cliente(server_memoria, log_memoria);

    // t_list* listaCpu;

	int cod_op_cpu = recibir_operacion(client_cpu);
	switch (cod_op_cpu) {
	case MENSAJE:
		recibir_mensaje(client_cpu, log_memoria);
		break;
	// case PAQUETE:
	// 	listaCpu = recibir_paquete(client_cpu);
	// 	log_info(log_memoria, "Me llegaron los siguientes valores:\n");
	// 	list_iterate(listaCpu, (void*) iterator);
	// 	break;
	case -1:
		log_error(log_memoria, "el cliente se desconecto. Terminando servidor");
		// return EXIT_FAILURE;
        exit(1);
	default:
		log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
		break;
	}
}

// ********* SERVER PARA RECIBIR A KERNEL *********
void server_para_kernel(memoria_config* config_memoria,t_log* log_memoria){

    log_info(log_memoria, "Memoria lista para recibir a Kernel");
	int client_kernel = esperar_cliente(server_memoria, log_memoria);
    
	// t_list* listaKernel;

	int cod_op_kernel = recibir_operacion(client_kernel);
	switch (cod_op_kernel) {
	case MENSAJE:
		recibir_mensaje(client_kernel, log_memoria);
		break;
	// case PAQUETE:
	// 	listaKernel = recibir_paquete(client_kernel);
	// 	log_info(log_memoria, "Me llegaron los siguientes valores:\n");
	// 	list_iterate(listaKernel, (void*) iterator);
	// 	break;
	case -1:
		log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
		// return EXIT_FAILURE;
        exit(1);
	default:
		log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
		break;
	}
}

// ********* SERVER PARA RECIBIR A I/O *********
void server_para_io(memoria_config* config_memoria,t_log* log_memoria){
    log_info(log_memoria, "Memoria lista para recibir a I/O");
	int client_entradasalida = esperar_cliente(server_memoria, log_memoria);
    
	// t_list* listaentradasalida;

	int cod_op_entradasalida = recibir_operacion(client_entradasalida);
	switch (cod_op_entradasalida) {
	case MENSAJE:
		recibir_mensaje(client_entradasalida, log_memoria);
		break;
	// case PAQUETE:
	// 	listaentradasalida = recibir_paquete(client_entradasalida);
	// 	log_info(log_memoria, "Me llegaron los siguientes valores:\n");
	// 	list_iterate(listaentradasalida, (void*) iterator);
	// 	break;
	case -1:
		log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
		// return EXIT_FAILURE;
        exit(1);
	default:
		log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
		break;
	}
}
