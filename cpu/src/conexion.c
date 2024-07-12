#include "conexion.h"

// MEMORIA es server de CPU
// CPU es cliente de MEMORIA

void atender_memoria() {

    t_buffer* buffer;

    int cod_op_memoria;

    while(1) {
        cod_op_memoria = recibir_operacion(socket_cliente_cpu);

        log_info(log_cpu, "estoy en el while de atender memoria22222");

        log_info(log_cpu, "me llego un codigo %d", cod_op_memoria);

        switch (cod_op_memoria)  {
            
            // case MENSAJE:  
            // log_info(log_cpu, "case 1 ");
            //     recibir_mensaje(socket_cliente_cpu, log_cpu);
            //     break;
                
            case CPU_RECIBE_TAMAÑO_PAGINA_DE_MEMORIA:
            log_info(log_cpu, "case 2 ");
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                tamanio_pagina = recibir_int_del_buffer(buffer);
                free(buffer);
                break; 

            case CPU_RECIBE_INSTRUCCION_DE_MEMORIA:
                log_info(log_cpu, "case 3 ");
                log_info(log_cpu, "Recibi una instruccion de memoria");
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                //log_info(log_cpu, "PID: %d - FETCH - Program Counter: %d", proceso->pid, proceso->program_counter);
                //proceso->program_counter++; // Esto hay que sacarlo porque usan la variable global, y aumentarlo en pcb.c
                char* instruccion_recibida = recibir_string_del_buffer(buffer); // Obtengo la instrucción posta del buffer
                interpretar_instruccion_de_memoria(instruccion_recibida); // Mando la instrucción para hacer un decode
                free(buffer); // Libero el buffer
                break;

            case CPU_RECIBE_OUT_OF_MEMORY_DE_MEMORIA: // VACIO
            log_info(log_cpu, "case 4 ");
                printf("Error: No se pudo ajustar el tamaño del proceso. Out of Memory.\n");
                    
                argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
                args->proceso = pcb_recibido; //Este proceso es global, no deberia ser global
                args->operacion = OUT_OF_MEMORY;
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                free(buffer);
                enviar_pcb(socket_cliente_kernel, args);
                log_info(log_cpu, "entre a out of  ");
                
                break;

            case CPU_RECIBE_OK_DEL_RESIZE:
                log_info(log_cpu, "case 5 ");
                printf("El tamaño del proceso se ha ajustado correctamente.\n");
                pcb_recibido->program_counter++; 
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                free(buffer);
                log_info(log_cpu, "recibi buffer");
                check_interrupt();
                log_info(log_cpu, "volvi de checkinterrupt  ");
                
                break;

            case EXIT:
            log_info(log_cpu, "case 6 ");
                error_exit(EXIT);
                break;
            case -1:
            log_info(log_cpu, "case 7 ");
                log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
                exit(1);
            default:
            log_info(log_cpu, "case 8 ");
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
        log_info(log_cpu, "Estoy en el while de atender kernel");

        int cod_op_kernel = recibir_operacion(socket_cliente_kernel);

        switch (cod_op_kernel) 
        {
            case MENSAJE:
                recibir_mensaje(socket_cliente_kernel, log_cpu);
                break;
            // case PAQUETE:
            //     lista = recibir_paquete(socket_cliente_kernel);
            //     log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            //     list_iterate(lista, (void*) iterator);
            //     list_destroy_and_destroy_elements(lista, free);
            //     break;
            case PCB_KERNEL_A_CPU: // Execute

                log_info(log_cpu, "Estoy antes de entrar a recibir_pcb()");

                recibir_pcb();

                log_info(log_cpu, "Estoy después de entrar a recibir_pcb()");

                break;
            case SOLICITAR_TRADUCCION:
                log_info(log_cpu, "Recibi una solicitud de traducción de dirección lógica a física del Kernel");
                t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_kernel);
                int direccion_logica = recibir_int_del_buffer(buffer);
                int direccion_fisica = traducir_direccion_logica_a_fisica(direccion_logica);
                enviar_direccion_fisica_a_kernel(socket_cliente_kernel, direccion_fisica);
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

void atender_interrupcion() // ACA HAY QUE MANEJAR EL ENVIAR PCB DENTRO DEL SWITCH (ESTA COMENTADO)
{
    t_list* lista;
    int cod_op ;

    while(1) {
        cod_op = recibir_operacion(socket_interrupt_kernel);
        switch (cod_op) {
            case MENSAJE:
                recibir_mensaje(socket_interrupt_kernel, log_cpu);
                break;
            // case PAQUETE:
            //     lista = recibir_paquete(socket_interrupt_kernel);
            //     log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            //     list_iterate(lista, (void*) iterator);
            //     break;
                
            case INTERRUPCION_KERNEL:
                log_info(log_cpu, "Me llego una interrupcion de KERNEL, ahora voy a enviar el pcb");  
                flag_interrupcion = true; // Este flag me marca que HAY una interrupción, entonces desde el final de cada instrucción voy a devolver el pcb a kernel
                break;
                
            case -1:
                log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
                exit(1);
                
            default:
                log_warning(log_cpu, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }

}

pthread_t escuchar_memoria()
{
    log_info(log_cpu, "Estoy escuchando memoria\n");
    pthread_t hilo_memoria;

    pthread_create(&hilo_memoria, NULL, (void*) atender_memoria, NULL);
    
    return hilo_memoria;
}

pthread_t escuchar_kernel()
{
    pthread_t hilo_dispatch;
    
    pthread_create(&hilo_dispatch, NULL,(void*)atender_kernel, NULL);

    return hilo_dispatch;
}

pthread_t escuchar_kernel_interrupcion(){
    
    pthread_t hilo_interrupt;
    
    pthread_create(&hilo_interrupt, NULL, (void*)atender_interrupcion, NULL);
    
    return hilo_interrupt;
}