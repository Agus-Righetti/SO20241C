#include "mainKernel.h"

t_log* log_kernel;
kernel_config* config_kernel;

void iterator(char* value) 
{
	log_info(log_kernel,"%s", value);
}

//Armo el config del Kernel
kernel_config* armar_config(void)
{
    t_config* config_aux;
    kernel_config* aux_kernel_config = malloc(sizeof(kernel_config)); // Se inicializa la estructura que armamos 

    config_aux = config_create("kernel.config");
    aux_kernel_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
    aux_kernel_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
    aux_kernel_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
    aux_kernel_config->ip_cpu = strdup(config_get_string_value(config_aux, "IP_CPU"));
    aux_kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_CPU_DISPATCH"));
    aux_kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_CPU_INTERRUPT"));
    aux_kernel_config->algoritmo_planificacion = strdup(config_get_string_value(config_aux, "ALGORITMO_PLANIFICACION"));
    aux_kernel_config->quantum = strdup(config_get_string_value(config_aux, "QUANTUM"));
    aux_kernel_config->recursos = strdup(config_get_string_value(config_aux, "RECURSOS"));
    aux_kernel_config->instancias_recursos = strdup(config_get_string_value(config_aux, "INSTANCIAS_RECURSOS"));
    aux_kernel_config->grado_multiprogramacion = config_get_int_value(config_aux, "GRADO_MULTIPROGRAMACION");   

    log_info(log_kernel, "Se creo el struct config_kernel con exito");

    config_destroy(config_aux);

    return aux_kernel_config;
}

int main(int argc, char* argv[]) 
{
    decir_hola("Kernel");

    log_kernel = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);
    config_kernel = armar_config();
    int conexion_kernel_cpu;
    conexion_kernel_cpu = crear_conexion(config_kernel->ip_cpu, config_kernel->puerto_cpu_dispatch);
    log_info(log_kernel , "Conexion con el servidor CPU creada");
    enviar_mensaje("Hola CPU soy Kernel",conexion_kernel_cpu);

    int server_kernel = iniciar_servidor(config_kernel->puerto_escucha, log_kernel);
	log_info(log_kernel, "Kernel listo para recibir a IO");
    int client_IO = esperar_cliente(server_kernel);

    t_list* lista;
	while (1) {
		int cod_op = recibir_operacion(client_IO);
		switch (cod_op) {
		case MENSAJE:
			recibir_mensaje(client_IO, log_kernel);
			break;
		case PAQUETE:
			lista = recibir_paquete(client_IO);
			log_info(log_kernel, "Me llegaron los siguientes valores:\n");
			list_iterate(lista, (void*) iterator);
			break;
		case -1:
			log_error(log_kernel, "el cliente se desconecto. Terminando servidor");
			return EXIT_FAILURE;
		default:
			log_warning(log_kernel,"Operacion desconocida. No quieras meter la pata");
			break;
		}
	}
	return EXIT_SUCCESS;

    log_destroy(log_kernel);
    return 0;
}
