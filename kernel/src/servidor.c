#include "servidor.h"

//********* DESARROLLO SERVER KERNEL PARA RECIBIR A IO *****
void server_para_io(kernel_config* config_kernel,t_log* log_kernel){
    int server_kernel = iniciar_servidor(config_kernel->puerto_escucha, log_kernel);
	 if (server_kernel == -1)
    {
	    log_info(log_kernel, "ERROR: No se pudo iniciar Kernel como servidor para IO");
        exit(1);
    }

    log_info(log_kernel, "Kernel listo para recibir a IO");
    int client_io= esperar_cliente(server_kernel, log_kernel);

    t_list* lista;

	int cod_op = recibir_operacion(client_io);
	switch (cod_op) {
	case MENSAJE:
		recibir_mensaje(client_io, log_kernel);
		break;
	case PAQUETE:
		lista = recibir_paquete(client_io);
		log_info(log_kernel, "Me llegaron los siguientes valores:\n");
		list_iterate(lista, (void*) iterator);
		break;
	case -1:
		log_error(log_kernel, "El cliente se desconecto. Terminando servidor");
		//return EXIT_FAILURE;
        exit(1);
	default:
		log_warning(log_kernel,"Operacion desconocida. No quieras meter la pata");
		break;
	}
}