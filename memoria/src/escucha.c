#include "escucha.h"

// ************* ESCUCHA ACTIVA DE CPU *************
void atender_cpu(){
    bool control = 1;
    t_buffer* buffer;
    int cod_op_cpu;
    t_list* direcciones_fisicas;
    int pid;
    uint8_t valor_uint_8;
    uint32_t valor_uint_32;

    while(control){
        cod_op_cpu = recibir_operacion(socket_cliente_cpu);

        switch (cod_op_cpu) {

            case CPU_PIDE_INSTRUCCION_A_MEMORIA:
                usleep(config_memoria->retardo_respuesta *1000);
    
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
				cpu_pide_instruccion(buffer);
                free(buffer);
                break;

            case CPU_PIDE_GUARDAR_REGISTRO_1B:
        
                usleep(config_memoria->retardo_respuesta *1000);
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                pid = recibir_int_del_buffer(buffer);
	            direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));
	            valor_uint_8 = recibir_uint8_del_buffer(buffer);
            	guardar_uint8_en_memoria (pid, direcciones_fisicas, valor_uint_8);
				// cpu_pide_guardar_1B(buffer);
                free(buffer);
                break;

            case CPU_PIDE_GUARDAR_REGISTRO_4B:
                usleep(config_memoria->retardo_respuesta *1000);
                
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);                
                pid = recibir_int_del_buffer(buffer);
	            direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));
	            //int tamanio = recibir_int_del_buffer(un_buffer);
	            valor_uint_32 = recibir_uint32_del_buffer(buffer);
	            
	            guardar_uint32_en_memoria (pid, direcciones_fisicas, valor_uint_32);

				//cpu_pide_guardar_4B(buffer);
                free(buffer);
                list_destroy(direcciones_fisicas);

                break;

            case CPU_PIDE_LEER_REGISTRO_1B:
                
                usleep(config_memoria->retardo_respuesta *1000);
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
            

                pid = recibir_int_del_buffer(buffer);
	            direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));

                leer_uint8_en_memoria (pid, direcciones_fisicas); // Hago directamente este y listo

				// cpu_pide_leer_1B(pid, direcciones_fisicas);

                // free(buffer->stream);

                free(buffer);
                break;

            case CPU_PIDE_LEER_REGISTRO_4B:
                usleep(config_memoria->retardo_respuesta *1000);
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);

                pid = recibir_int_del_buffer(buffer);
	            direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));
                
                if(direcciones_fisicas == NULL){
                    log_error(log_memoria, "Direcciones_fisicas es null");
                }

            	leer_uint32_en_memoria (pid, direcciones_fisicas); // Hago directamente este y listo

				// cpu_pide_leer_4B(buffer);
                free(buffer);
                break;

            case CPU_PIDE_MARCO_A_MEMORIA:
            // ME DAN UNA PAG RESPONDO NUMERO DE MARCO
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                usleep(config_memoria->retardo_respuesta *1000);
				cpu_pide_numero_de_marco(buffer);
                free(buffer);
                break;

            case CPU_MANDA_RESIZE_A_MEMORIA: 
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                usleep(config_memoria->retardo_respuesta *1000);
                cpu_pide_resize(buffer); 
                free(buffer);
                break; 

            case CPU_PIDE_LEER_STRING:
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                usleep(config_memoria->retardo_respuesta *1000);
                
                cpu_pide_leer_string(buffer); 
                free(buffer);
                break; 
                

            case CPU_PIDE_GUARDAR_STRING:
                buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                usleep(config_memoria->retardo_respuesta *1000);
                cpu_pide_guardar_string(buffer); 
                free(buffer);
                break; 

            case -1:
                log_error(log_memoria, "CPU se desconecto.");
                control = 0; 
                break;

            default:
                log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

// ************* ESCUCHA ACTIVA DE KERNEL *************
void atender_kernel(){
    bool control = 1;
    t_buffer* buffer;
    int cod_op_kernel;

    while(control){
        cod_op_kernel = recibir_operacion(socket_cliente_kernel);
        switch (cod_op_kernel) {
            case CREACION_PROCESO_KERNEL_A_MEMORIA:
                buffer = recibiendo_paquete_personalizado(socket_cliente_kernel);
                usleep(config_memoria->retardo_respuesta *1000);
				iniciar_estructura_para_un_proceso_nuevo(buffer);
                free(buffer);
                break;

            case FINALIZAR_PROCESO_KERNEL_A_MEMORIA:
                buffer = recibiendo_paquete_personalizado(socket_cliente_kernel);
                usleep(config_memoria->retardo_respuesta *1000);
				liberar_memoria_proceso(buffer);
                free(buffer);
                break;

            case -1:
                log_error(log_memoria, "KERNEL se desconecto.");
                control = 0;
                break;

            default:
                log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

// ************* ESCUCHA ACTIVA DE IO************* //la comento porque ahora hay otra q es un hilo
// void atender_io(){
//     bool control = 1;
//     t_buffer* buffer;
//     int cod_op_io;

//     while(control){
        
//         cod_op_io = recibir_operacion(socket_cliente_io);

//         switch (cod_op_io) {
            
//             case IO_PIDE_LECTURA_MEMORIA:
                
//                 log_info(log_memoria, "IO me pide la lectura de un espacio de memoria");
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_io);
                
//                 usleep(config_memoria->retardo_respuesta *1000);
                
//                 io_pide_lectura(buffer); 
                
//                 free(buffer);
//                 break;

//             case IO_PIDE_ESCRITURA_MEMORIA:
                
//                 log_info(log_memoria, "IO me pide la escritura de un espacio de memoria");
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_io);
                
//                 usleep(config_memoria->retardo_respuesta *1000);
                
//                 io_pide_escritura(buffer);  
                
//                 free(buffer);

//                 break;

//             case -1:
                
//                 log_error(log_memoria, "IO se desconecto.");
//                 control = 0; 
//                 break;

//             default:
                
//                 log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
//                 break;
//         }
//     }
// }



// *************************************** CPU ***************************************
// ************* RECIBIR CPU COMO CLIENTE, CREAR HILO PARA ESCUCHA ACTIVA *************
void recibir_escuchar_cpu(){
    server_para_cpu();
    pthread_t hilo_escucha_cpu;
    pthread_create(&hilo_escucha_cpu, NULL, (void*)atender_cpu, NULL);
    pthread_detach(hilo_escucha_cpu);
}

// *************************************** KERNEL ***************************************
// ************* RECIBIR KERNEL COMO CLIENTE, CREAR HILO PARA ESCUCHA ACTIVA *************
void recibir_escuchar_kernel(){
    server_para_kernel();
    pthread_t hilo_escucha_kernel;
    pthread_create(&hilo_escucha_kernel, NULL, (void*)atender_kernel, NULL);
    pthread_detach(hilo_escucha_kernel);
}

//*************************************** IO ***************************************
//************* RECIBIR IO COMO CLIENTE, CREAR HILO PARA ESCUCHA ACTIVA *************
void recibir_escuchar_io(){
    //server_para_io();
    pthread_t hilo_escucha_io;
    pthread_create(&hilo_escucha_io, NULL, (void*)server_para_io, NULL);
    pthread_join(hilo_escucha_io, NULL);
}