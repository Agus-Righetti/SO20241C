#include "conexion.h"

// MEMORIA es server de CPU
// CPU es cliente de MEMORIA

void atender_memoria() {

    t_buffer* buffer;

    t_direccion_fisica* dir_fisica;
    int pid;
    uint8_t valor_escrito;
    uint8_t valor_completo;

    int cod_op_memoria;

    while(1) {
        cod_op_memoria = recibir_operacion(socket_cliente_cpu);

        //log_info(log_cpu, "estoy en el while de atender memoria22222");

        //log_info(log_cpu, "me llego un codigo %d", cod_op_memoria);

        switch (cod_op_memoria)  {
            
            case MENSAJE:  
            log_info(log_cpu, "case 1 ");
                recibir_mensaje(socket_cliente_cpu, log_cpu);
                break;
                
            case CPU_RECIBE_TAMAÑO_PAGINA_DE_MEMORIA:
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                tamanio_pagina = recibir_int_del_buffer(buffer);
                // free(buffer->stream);
                free(buffer);
                break; 

            case CPU_RECIBE_INSTRUCCION_DE_MEMORIA:
                //log_info(log_cpu, "case 3 ");
                log_info(log_cpu, "Recibi una instruccion de memoria");
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                //log_info(log_cpu, "PID: %d - FETCH - Program Counter: %d", proceso->pid, proceso->program_counter);
                //proceso->program_counter++; // Esto hay que sacarlo porque usan la variable global, y aumentarlo en pcb.c
                instruccion_recibida = recibir_string_del_buffer(buffer); // Obtengo la instrucción posta del buffer
                //interpretar_instruccion_de_memoria(instruccion_recibida); // Mando la instrucción para hacer un decode
                // free(buffer->stream);
                free(buffer); // Libero el buffer
                sem_post(&sem_hay_instruccion);
                break;

            case CPU_RECIBE_OUT_OF_MEMORY_DE_MEMORIA: // VACIO
            //log_info(log_cpu, "case 4 ");
                printf("Error: No se pudo ajustar el tamaño del proceso. Out of Memory.\n");
                    
                argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
                args->proceso = pcb_recibido; //Este proceso es global, no deberia ser global
                args->operacion = OUT_OF_MEMORY;
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                // free(buffer->stream);
                free(buffer);
                enviar_pcb(socket_cliente_kernel, args);
                log_info(log_cpu, "entre a out of  ");
            
                
                break;

            case CPU_RECIBE_NUMERO_DE_MARCO_DE_MEMORIA:
                log_info(log_cpu, "Me llego un marco de memoria");
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                marco = recibir_int_del_buffer(buffer);
                log_info(log_cpu, "me llego el marco: ,%d", marco);
                
                sem_post(&sem_tengo_el_marco);
            
                // free(buffer->stream);
                free(buffer);

                break; 

            case CPU_RECIBE_OK_DEL_RESIZE:
                //log_info(log_cpu, "case 5 ");
                printf("El tamaño del proceso se ha ajustado correctamente.\n");
                pcb_recibido->program_counter++; 
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                // free(buffer->stream);
                free(buffer);
                //log_info(log_cpu, "recibi buffer");

                sem_post(&sem_tengo_ok_resize);
                //check_interrupt();
                //log_info(log_cpu, "volvi de checkinterrupt  ");
                
                break;
            case CPU_RECIBE_LECTURA_1B:

                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer); // Este pid creo que ya lo puedo tener directo del pcb recibido
                dir_fisica = recibir_estructura_del_buffer(buffer);
                valor_leido_de_memoria_8 = recibir_uint8_del_buffer(buffer);


                log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_leido_de_memoria_8);
                log_info(log_cpu, "ACCIÓN COMPLETADA: LEER %u EN MEMORIA", valor_leido_de_memoria_8);
                // free(buffer->stream);
                free(buffer);

                sem_post(&sem_valor_leido_de_memoria);

                break;

            case CPU_RECIBE_LECTURA_4B:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_32]
                // Pueden ser mas de una

                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer); // Lo mismo, creo que no hace falta 
                dir_fisica = recibir_estructura_del_buffer(buffer);
                valor_leido_de_memoria_32 = recibir_uint32_del_buffer(buffer);
                log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_leido_de_memoria_32);
                //free(buffer->stream);
                free(buffer);

                sem_post(&sem_valor_leido_de_memoria);

                break; 

            case CPU_RECIBE_LECTURA_U_4B:   // [PID, DF, VALOR, VALOR FINAL] -> [Int, Direccion_fisica, uint_32, uint_32]

                // Esta es la ultima
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer); // Lo mismo, creo que no hace falta
                dir_fisica = recibir_estructura_del_buffer(buffer);
                valor_leido_de_memoria_32 = recibir_uint32_del_buffer(buffer);
                valor_reconstruido = recibir_uint32_del_buffer(buffer);
                log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_leido_de_memoria_32);
                log_info(log_cpu, "ACCIÓN COMPLETADA: LEER %u EN MEMORIA", valor_reconstruido);
                // control = 1;
                //free(buffer->stream);
                
                free(buffer);

                sem_post(&sem_valor_leido_de_memoria);

                break; 
            
            case CPU_RECIBE_OK_1B_DE_ESCRITURA:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_8]

                // Esta va a ser una sola
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer);
                dir_fisica = recibir_estructura_del_buffer(buffer);
                valor_escrito = recibir_uint8_del_buffer(buffer);

                log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_escrito);
                log_info(log_cpu, "ACCIÓN COMPLETADA: ESCRIBIR %u EN MEMORIA", valor_escrito);
                
                free(buffer);

                sem_post(&sem_ok_escritura);

                break; 
            
            case CPU_RECIBE_OK_4B_DE_ESCRITURA:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_32]
                // Pueden ser mas de una
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer);
                dir_fisica = recibir_estructura_del_buffer(buffer);
                valor_escrito = recibir_uint32_del_buffer(buffer);
                log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_escrito);
                
                free(buffer);

                sem_post(&sem_ok_escritura);

                break; 

            case CPU_RECIBE_ULT_OK_4B_DE_ESCRITURA:   // [PID, DF, VALOR, VALOR FINAL] -> [Int, Direccion_fisica, uint_32, uint_32]

                // Esta es la ultima
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer);
                dir_fisica = recibir_estructura_del_buffer(buffer);
                valor_escrito = recibir_uint32_del_buffer(buffer);
                valor_completo = recibir_uint32_del_buffer(buffer);
                log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_escrito);
                log_info(log_cpu, "ACCIÓN COMPLETADA: ESCRIBIR %u EN MEMORIA", valor_completo);
                //control = 1;
                free(buffer);

                sem_post(&sem_ok_escritura);
                
                break; 


            case EXIT:
            //log_info(log_cpu, "case 6 ");
                error_exit(EXIT);
                break;
            case -1:
            //log_info(log_cpu, "case 7 ");
                log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
                exit(1);
            default:
                //log_info(log_cpu, "case 8 ");
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
               // free(buffer->stream);
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
                
            case FIN_DE_QUANTUM:
                log_info(log_cpu, "Me llego una interrupcion de KERNEL, ahora voy a enviar el pcb");  
                flag_interrupcion = true; // Este flag me marca que HAY una interrupción, entonces desde el final de cada instrucción voy a devolver el pcb a kernel
                motivo_interrupcion = FIN_DE_QUANTUM;

                break;
            
            case INTERRUPTED_BY_USER:
                log_info(log_cpu, "Me llego una interrupcion de KERNEL, ahora voy a enviar el pcb");  
                flag_interrupcion = true; // Este flag me marca que HAY una interrupción, entonces desde el final de cada instrucción voy a devolver el pcb a kernel
                motivo_interrupcion = INTERRUPTED_BY_USER;
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

pthread_t crear_hilo_interpretar_instruccion()
{
    pthread_t hilo_interp_instruccion;
    
    pthread_create(&hilo_interp_instruccion, NULL,(void*)interpretar_instruccion_de_memoria, NULL);

    return hilo_interp_instruccion;
}