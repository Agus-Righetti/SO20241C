#include "conexion.h"

// MEMORIA es server de CPU
// CPU es cliente de MEMORIA

void atender_memoria() 
{
    t_list* lista;
    t_instruccion* instruccion;

    // Ver que recibe recibir_operacion(socket_servidor_memoria);
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
            recibir_instruccion_de_memoria(socket_servidor_memoria);
            proceso->program_counter++;
            log_info(log_cpu, "PID: %d - FETCH - Program Counter: %d", proceso->pid, proceso->program_counter);
            interpretar_instrucciones();
            list_destroy_and_destroy_elements(lista, free);
            free(instruccion);
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

    while(1)
    { 
        int cod_op_kernel = recibir_operacion(socket_cliente_kernel);

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

void atender_interrupcion()
{
   	t_list* lista;

    while(1)
    { 
        int cod_op_kernel = recibir_operacion(socket_interrupt_kernel);

        switch (cod_op_kernel) 
        {
            case MENSAJE:
                recibir_mensaje(socket_interrupt_kernel, log_cpu);
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
    // solicitar_instrucciones_a_memoria(socket_cliente_cpu);
    esperar_memoria(socket_cliente_cpu);
}

void escuchar_kernel()
{
    pthread_t hilo_dispatch, hilo_interrupt;
    server_para_kernel();
    interrupcion_para_kernel();
    pthread_create(&hilo_dispatch, NULL,(void*)atender_kernel, NULL);
    pthread_create(&hilo_interrupt, NULL, (void*)atender_interrupcion, NULL);
    pthread_join(hilo_dispatch, NULL);
    pthread_join(hilo_interrupt, NULL);
}