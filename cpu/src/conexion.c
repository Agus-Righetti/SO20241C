#include "conexion.h"

// MEMORIA es server de CPU
// CPU es cliente de MEMORIA

void atender_memoria() 
{
    t_list* lista;
    t_instruccion* instruccion;

    int cod_op_memoria = recibir_operacion(socket_servidor_memoria);
    while(1)
    {
        switch (cod_op_memoria) 
        {
        case MENSAJE:
            recibir_mensaje(socket_servidor_memoria, log_cpu);
            break;
        case PAQUETE:
            lista = recibir_paquete(socket_servidor_memoria);
            log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void*) iterator);
            list_destroy_and_destroy_elements(lista, free);
            break;
        case INSTRUCCION:
            lista = recibir_paquete(socket_servidor_memoria);
            instruccion = malloc(sizeof(t_instruccion));
            recibir_instruccion(lista, instruccion);
            interpretar_instrucciones();
            list_destroy_and_destroy_elements(lista, free);
            free(instruccion);
            break;
        case OK:
			proceso->program_counter++; // Avanza a la siguiente instrucción
			interpretar_instrucciones();
			list_destroy_and_destroy_elements(lista, free);
			break;
        case EXIT:
            error_exit(EXIT);
            list_destroy_and_destroy_elements(lista, free); 
            break;
        case -1:
            log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
            free(socket_servidor_memoria);
            exit(1);
            return;
        default:
            log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
            break;
    
        }
    }
}

// KERNEL es cliente de CPU
// CPU es servidor de KERNEL

void atender_kernel()
{
   	t_list* lista;
    
    int socket_cliente_kernel = esperar_cliente(server_cpu, log_cpu);
    int cod_op_kernel = recibir_operacion(socket_cliente_kernel);

    while(1)
    { 
        switch (cod_op_kernel) 
        {
            case MENSAJE:
                recibir_mensaje(socket_cliente_kernel, log_cpu);
                break;
            case PAQUETE:
                lista = recibir_paquete(socket_cliente_kernel);
                log_info(log_cpu, "Me llegaron los siguientes valores:\n");
                list_iterate(lista, (void*) iterator);
                list_destroy_and_destroy_elements(lista, free);
                break;
            case EXECUTE:
                lista = recibir_paquete(socket_cliente_kernel);
                proceso = malloc(sizeof(pcb));
                proceso->instrucciones = NULL;
                recibir_pcb(lista, proceso);
                interpretar_instrucciones();
                list_destroy_and_destroy_elements(lista, free);
                break;
            case EXIT:
                error_exit(EXIT);
                list_destroy_and_destroy_elements(lista, free); 
                break;
            case -1:
                log_error(log_cpu, "KERNEL se desconecto. Terminando servidor");
                return EXIT_FAILURE;
                exit(1);
            default:
                log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

void esperar_memoria(int conexion)
{
	pthread_t thread;
	int socket_servidor_memoria = malloc(sizeof(int));
	socket_servidor_memoria = conexion;
	pthread_create(&thread,NULL,(void*) atender_memoria, socket_servidor_memoria);
	pthread_detach(thread);
}

void escuchar_memoria()
{
    int socket_cliente_cpu = conexion_a_memoria();

    // No se si tenemos que hacer un atender_memoria
    // Quizas en esta parte se pueden pedir las instrucciones -> ver case INSTRUCCION
    // esperar_memoria(socket_cliente_cpu);
}

void escuchar_kernel()
{
    server_para_kernel();
    atender_kernel();
}