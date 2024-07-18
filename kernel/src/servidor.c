#include "servidor.h"
#include "manejo_io.h"


//********* DESARROLLO SERVER KERNEL PARA RECIBIR A IO *****
void server_para_io(){
	
    int server_kernel = iniciar_servidor(config_kernel->puerto_escucha, log_kernel);
	
	if (server_kernel == -1)
    {
	    log_info(log_kernel, "ERROR: No se pudo iniciar Kernel como servidor para IO");
        exit(1);
    }

    log_info(log_kernel, "Kernel listo para recibir a IO");
	int client_io;
	t_buffer* buffer;
	int cod_op;

	while(1)
	{
		log_info(log_kernel, "estoy en el while de esperar q se conecte alguien");
		client_io = esperar_cliente(server_kernel, log_kernel); //espera que se conecte alguna interfaz
		log_info(log_kernel, "ya se conecto alguien nuevo");
    	cod_op = recibir_operacion(client_io); //me habla la interfaz

		switch (cod_op) {
			case MENSAJE:
				recibir_mensaje(client_io, log_kernel);
				break;
			// case PAQUETE:
			// 	lista = recibir_paquete(client_io);
			// 	log_info(log_kernel, "Me llegaron los siguientes valores:\n");
			// 	list_iterate(lista, (void*) iterator);
			// 	break;
			case NUEVA_INTERFAZ: //pedir que manden esto con el nombre de la interfaz

				buffer = recibiendo_paquete_personalizado(client_io);
				log_info(log_kernel, "se conecto una nueva interfaz");
				
				op_code tipo_interfaz = recibir_int_del_buffer(buffer); //va a decir si es gen, stdin, etc
				//falta iniciar nombre_interfaz

				char* nombre_interfaz = recibir_string_del_buffer(buffer);

				//free(buffer);

				crear_interfaz(tipo_interfaz, client_io, nombre_interfaz);
				
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


