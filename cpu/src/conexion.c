#include "conexion.h"

// MEMORIA es server de CPU
// CPU es cliente de MEMORIA

void atender_memoria() 
{
    t_list* lista;
    t_instruccion* instruccion;

    int cod_op_memoria = recibir_operacion(socket_cliente_cpu);
    while(1)
    {
        switch (cod_op_memoria) 
        {
        case MENSAJE:
            recibir_mensaje(socket_cliente_cpu, log_cpu);
            break;
        case PAQUETE:
            lista = recibir_paquete(socket_cliente_cpu);
            log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void*) iterator);
            list_destroy_and_destroy_elements(lista, free);
            break;
        case CPU_RECIBE_INSTRUCCION_DE_MEMORIA:
            log_info(log_cpu, "Recibi una instruccion de memoria");
            t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
            log_info(log_cpu, "PID: %d - FETCH - Program Counter: %d", proceso->pid, proceso->program_counter);
            proceso->program_counter++;
			interpretar_instruccion_de_memoria(buffer);
            free(buffer);
            break;
        case EXIT:
            error_exit(EXIT);
            list_destroy_and_destroy_elements(lista, free); 
            break;
        case -1:
            log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
            free(socket_cliente_cpu);
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
            case PCB_KERNEL_A_CPU: // Execute
                t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_kernel);
                recibir_pcb(buffer, pcb_recibido);
                solicitar_instrucciones_a_memoria(socket_cliente_cpu, pcb_recibido);
                free(buffer);
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
    
	int cod_op = recibir_operacion(socket_interrupt_kernel);
	switch (cod_op) 
    {
        case MENSAJE:
            recibir_mensaje(socket_interrupt_kernel, log_cpu);
            break;
        case PAQUETE:
            lista = recibir_paquete(socket_interrupt_kernel);
            log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void*) iterator);
            break;
        case CPU_TERMINA_EJECUCION_PCB: // INTERRUPCION
            log_info(log_cpu, "Me llego una interrupcion de KERNEL");
            enviar_pcb(socket_cliente_kernel, proceso, DESALOJO);
            break;
        case -1:
            log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
            return EXIT_FAILURE;
            exit(1);
        default:
            log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
            break;
	}
}

void escuchar_memoria()
{
    log_info(log_cpu, "Estoy escuchando memoria\n");
    pthread_t hilo_memoria;
    pthread_create(&hilo_memoria, NULL, (void*) atender_memoria, NULL);
    pthread_join(hilo_memoria, NULL);
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