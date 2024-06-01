#include "escucha.h"

// ************* ESCUCHA ACTIVA DE CPU *************
void atender_cpu(){
    bool control = 1;
    while(control){
        int cod_op_cpu = recibir_operacion(socket_cliente_cpu);
        switch (cod_op_cpu) {
            case MENSAJE:
                recibir_mensaje(socket_cliente_cpu, log_memoria);
                break;

            case CPU_PIDE_INSTRUCCION_A_MEMORIA:
                log_info(log_memoria, "CPU me pide una instruccion");
                t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
				cpu_pide_instruccion(buffer);
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
    while(control){
        int cod_op_kernel = recibir_operacion(socket_cliente_kernel);
        switch (cod_op_kernel) {
            case MENSAJE:
                recibir_mensaje(socket_cliente_kernel, log_memoria);
                break;

            case CREACION_PROCESO_KERNEL_A_MEMORIA:
                log_info(log_memoria, "Kernel pide creacion de un nuevo proceso");
                buffer = recibiendo_paquete_personalizado(socket_cliente_kernel);
				iniciar_estructura_para_un_proceso_nuevo(buffer);
                free(buffer);
                break;

            case FINALIZAR_PROCESO_KERNEL_A_MEMORIA:
                log_info(log_memoria, "Kernel quiere finalizar un proceso");
                buffer = recibiendo_paquete_personalizado(socket_cliente_kernel);
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

// ************* ESCUCHA ACTIVA DE IO*************
void atender_io(){
    bool control = 1;
    while(control){
        int cod_op_io = recibir_operacion(socket_cliente_io);
        switch (cod_op_io) {
            case MENSAJE:
                recibir_mensaje(socket_cliente_io, log_memoria);
                break;
            case -1:
                log_error(log_memoria, "IO se desconecto.");
                control = 0; 
                break;
            default:
                log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}


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

// *************************************** IO ***************************************
// ************* RECIBIR IO COMO CLIENTE, CREAR HILO PARA ESCUCHA ACTIVA *************
void recibir_escuchar_io(){
    server_para_io();
    pthread_t hilo_escucha_io;
    pthread_create(&hilo_escucha_io, NULL, (void*)atender_io, NULL);
    pthread_join(hilo_escucha_io, NULL);
}