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
	int client_io;
	t_list* lista;
	t_buffer* buffer;
	int cod_op;

	while(1)
	{
		client_io = esperar_cliente(server_kernel, log_kernel); //espera que se conecte alguna interfaz

    	cod_op = recibir_operacion(client_io); //me habla la interfaz

		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(client_io, log_kernel);
				break;
			case PAQUETE:
				lista = recibir_paquete(client_io);
				log_info(log_kernel, "Me llegaron los siguientes valores:\n");
				list_iterate(lista, (void*) iterator);
				break;
			case NUEVA_INTERFAZ: //pedir que manden esto con el nombre de la interfaz
				buffer = recibiendo_paquete_personalizado(client_io);
				op_code interfaz_nueva = recibir_int_del_buffer(buffer); //va a decir si es gen, stdin, etc
				crear_interfaz(interfaz_nueva, client_io);
				break;
			//case -1:
				//log_error(log_kernel, "El cliente se desconecto. Terminando servidor");
				//return EXIT_FAILURE;
				//exit(1);
			default:
				log_warning(log_kernel,"Operacion desconocida. No quieras meter la pata");
				break;

	};
	}
}

void crear_interfaz(op_code interfaz_nueva, int socket)
{
	interfaz_kernel* nueva_interfaz = malloc(sizeof(interfaz_kernel)); 
	nueva_interfaz->nombre_interfaz= interfaz_nueva;
	nueva_interfaz->cola_de_espera = queue_create();
	nueva_interfaz->socket = socket;
	nueva_interfaz->en_uso = false;
	queue_push(cola_interfaces_conectadas, nueva_interfaz); 

	//aca deberiamos crear un hilo que le mande cosas a la interfaz y reciba cosas de la interfaz, o quizas que el hilo se cree una vez que se le quiere mandar algo a la interfaz, habria que pensarlo
	//se me ocurre un hilo q quede prendido con un while(1) siempre q la interfaz este prendida, para ver lo q devuelve, y que al pasarle cosas a la interfaz se haga desde la funcion q corresponda
	return;
}
