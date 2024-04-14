#include <include/mainCpu.h>

t_log* log_cpu;
cpu_config* config_cpu;

void iterator(char* value) 
{
	log_info(log_cpu,"%s", value);
}

int main(int argc, char* argv[]) 
{
    decir_hola("CPU");
    
	// ************* LOG Y CONFIG DE USO GENERAL *************
    log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    config_cpu = armar_config(log_cpu);

    // ************* CONEXION CPU --> MEMORIA *************
    int conexion_cpu_memoria = conexion_a_memoria(log_cpu, config_cpu);

    

	int server_cpu = iniciar_servidor(config_cpu->puerto_escucha_dispatch, log_cpu);
	log_info(log_cpu, "CPU listo para recibir a Kernel");
    int client_kernel = esperar_cliente(server_cpu, log_cpu);

    t_list* lista;
	while (1) 
    {
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
		default:
			log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;

    log_destroy(log_cpu);
    return 0;
}
