#include <mainMemoria.h>

t_log* log_memoria;
memoria_config* config_memoria;

void iterator(char* value) 
{
	log_info(log_memoria, value);
}

memoria_config* armar_config(void)
{
    t_config* config_aux;
    memoria_config* aux_memoria_config = malloc(sizeof(memoria_config)); // Se inicializa la estructura

    config_aux = config_create("memoria.config");
    aux_memoria_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
    aux_memoria_config->tam_memoria = config_get_int_value(config_aux, "TAM_MEMORIA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->tam_pagina = config_get_int_value(config_aux, "TAM_PAGINA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->path_instrucciones = strdup(config_get_string_value(config_aux, "PATH_INSTRUCCIONES"));
    aux_memoria_config->retardo_respuesta = config_get_string_value(config_aux, "RETARDO_RESPUESTA"); // Se usa config_get_int_value para obtener un entero
    
    log_info(log_memoria, "Se creo el struct config_memoria con exito");

    config_destroy(config_aux);

    return aux_memoria_config;
}

int main(int argc, char* argv[]) 
{
	decir_hola("Memoria");
    
	// ********* Esto es de uso general, el log y el config *********
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);

    config_memoria = armar_config();
    
	// ********* Esto es para poder recibir mensajes del CPU *********
	int server_memoria = iniciar_servidor(config_memoria->puerto_escucha, log_memoria);
	log_info(log_memoria, "Memoria lista para recibir a CPU");
	int client_cpu = esperar_cliente(server_memoria, log_memoria);

    t_list* listaCpu;

	int cod_op_cpu = recibir_operacion(client_cpu);
	switch (cod_op_cpu) {
	case MENSAJE:
		recibir_mensaje(client_cpu, log_memoria);
		break;
	case PAQUETE:
		listaCpu = recibir_paquete(client_cpu);
		log_info(log_memoria, "Me llegaron los siguientes valores:\n");
		list_iterate(listaCpu, (void*) iterator);
		break;
	case -1:
		log_error(log_memoria, "el cliente se desconecto. Terminando servidor");
		return EXIT_FAILURE;
	default:
		log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
		break;
	}
	

	// ********* Esto es para recibir mensajes del Kernel *********

	log_info(log_memoria, "Memoria lista para recibir a Kernel");
	int client_kernel = esperar_cliente(server_memoria, log_memoria);
    
	t_list* listaKernel;

	int cod_op_kernel = recibir_operacion(client_kernel);
	switch (cod_op_kernel) {
	case MENSAJE:
		recibir_mensaje(client_kernel, log_memoria);
		break;
	case PAQUETE:
		listaKernel = recibir_paquete(client_kernel);
		log_info(log_memoria, "Me llegaron los siguientes valores:\n");
		list_iterate(listaKernel, (void*) iterator);
		break;
	case -1:
		log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
		return EXIT_FAILURE;
	default:
		log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
		break;
	}

	log_destroy(log_memoria);

    return EXIT_SUCCESS;
}





