#include "servidor.h"

int server_memoria;

// Despues lo vamos a tener que usar si hacemos paquetes
// void iterator(char* value) 
// {
// 	log_info(log_memoria, "%s", value);
// }

// ********* SERVER PARA RECIBIR A CPU *********
void* server_para_cpu(void* args) {
    thread_args_memoria* hilos_args = (thread_args_memoria*)args;
    server_memoria = iniciar_servidor(hilos_args->config_memoria->puerto_escucha, hilos_args->log_memoria);
    if (server_memoria == -1)
    {
        log_info(hilos_args->log_memoria, "Error: No se pudo iniciar Memoria como servidor para CPU");
        exit(1);
    }

    log_info(hilos_args->log_memoria, "Memoria lista para recibir a CPU");
    int client_cpu = esperar_cliente(server_memoria, hilos_args->log_memoria);

    int cod_op_cpu = recibir_operacion(client_cpu);
    switch (cod_op_cpu) {
        case MENSAJE:
            recibir_mensaje(client_cpu, hilos_args->log_memoria);
            break;
        case -1:
            log_error(hilos_args->log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(hilos_args->log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }

    return NULL;
}

// ********* SERVER PARA RECIBIR A KERNEL *********
void* server_para_kernel(void* args) {
    thread_args_memoria* hilos_args = (thread_args_memoria*) args;
    log_info(hilos_args->log_memoria, "Memoria lista para recibir a Kernel");
    int client_kernel = esperar_cliente(server_memoria, hilos_args->log_memoria);

    int cod_op_kernel = recibir_operacion(client_kernel);
    switch (cod_op_kernel) {
        case MENSAJE:
            recibir_mensaje(client_kernel, hilos_args->log_memoria);
            break;
        case -1:
            log_error(hilos_args->log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(hilos_args->log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }

    return NULL;
}

// ********* SERVER PARA RECIBIR A I/O *********  
void* server_para_io(void* args) {
    thread_args_memoria* hilos_args = (thread_args_memoria*) args;
    log_info(hilos_args->log_memoria, "Memoria lista para recibir a I/O");
    int client_entradasalida = esperar_cliente(server_memoria, hilos_args->log_memoria);

    int cod_op_entradasalida = recibir_operacion(client_entradasalida);
    switch (cod_op_entradasalida) {
        case MENSAJE:
            recibir_mensaje(client_entradasalida, hilos_args->log_memoria);
            break;
        case -1:
            log_error(hilos_args->log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(hilos_args->log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }

    return NULL;
}

// ********* CREACION DE HILOS PARA SER SERVIDOR *********  
void crear_hilos_servidor (t_log* log_memoria, memoria_config* config_memoria){
    
    // Creo los 3 hilos:
    pthread_t thread_cpu, thread_kernel, thread_io;

    //Defino la estrcutura del argumento
    thread_args_memoria args = {config_memoria, log_memoria};
        
    // ********* CREO EL HILO SERVER PARA RECIBIR A CPU *********
    pthread_create(&thread_cpu, NULL, server_para_cpu, (void*) &args);
        
    // ********* CREO EL HILO SERVER PARA RECIBIR A KERNEL *********
    pthread_create(&thread_kernel, NULL, server_para_kernel, (void*) &args);

    // ********* CREO EL HILO SERVER PARA RECIBIR A I/O *********
    pthread_create(&thread_io, NULL, server_para_io, (void*) &args);

    //ESTO LO VAMOS A TENER QUE CAMBIAR, LOS JOIN VAN EN OTRO LADO
    pthread_join(thread_cpu, NULL);
    pthread_join(thread_kernel, NULL);
    pthread_join(thread_io, NULL);

    return ;
}