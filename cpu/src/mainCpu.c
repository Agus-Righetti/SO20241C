#include <mainCpu.h>

t_log* log_cpu;
cpu_config* config_cpu;

void iterator(char* value) 
{
	log_info(log_cpu,"%s", value);
}

int main(int argc, char* argv[]) {
    
    decir_hola("CPU");
    
    log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    config_cpu = armar_config();
    int conexion_cpu_memoria;
    conexion_cpu_memoria = crear_conexion(config_cpu->ip_memoria,config_cpu->puerto_memoria);
    log_info(log_cpu , "Conexion con el servidor memoria creada");
    enviar_mensaje("Hola Memoria soy CPU",conexion_cpu_memoria);

	int server_cpu = iniciar_servidor(config_cpu->puerto_escucha_dispatch, log_cpu);
	log_info(log_cpu, "CPU listo para recibir a Kernel");
    int client_kernel = esperar_cliente(server_cpu);

    t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(client_kernel);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(client_kernel, log_cpu);
			break;
		case PAQUETE:
			lista = recibir_paquete(client_kernel);
			log_info(log_cpu, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(log_cpu, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;

    log_destroy(log_cpu);
    return 0;
}

cpu_config* armar_config(void)
{
    t_config* config_aux;
    cpu_config* aux_cpu_config = malloc(sizeof(cpu_config)); // Se inicializa la estructura que armamos 

    config_aux = config_create("cpu.config");
    aux_cpu_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
    aux_cpu_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
    aux_cpu_config->puerto_escucha_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA_DISPATCH"));
    aux_cpu_config->puerto_escucha_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA_INTERRUPT"));
    aux_cpu_config->cantidad_entradas_tlb = config_get_int_value(config_aux, "CANTIDAD_ENTRADAS_TLB");
    aux_cpu_config->algoritmo_tlb = strdup(config_get_string_value(config_aux, "ALGORITMO_TLB"));

    log_info(log_cpu, "Se creo el struct config_cpu con exito");

    config_destroy(config_aux);

    return aux_cpu_config;
}
