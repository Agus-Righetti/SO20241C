#include "servidor.h"

// ********* INICIALIZO MEMORIA COMO SERVIDOR *********
void inicializar_servidor(){
    socket_servidor_memoria = iniciar_servidor(config_memoria->puerto_escucha, log_memoria);
    if (socket_servidor_memoria == -1){
        log_info(log_memoria, "Error: No se pudo iniciar Memoria como servidor");
        exit(1);
    }

    log_info(log_memoria, "Memoria lista para recibir clientes");
}

// ********* SERVER PARA RECIBIR A CPU *********
void server_para_cpu() {
    log_info(log_memoria, "Esperando a CPU...");
    socket_cliente_cpu = esperar_cliente(socket_servidor_memoria, log_memoria);

    int cod_op_cpu = recibir_operacion(socket_cliente_cpu);
    switch (cod_op_cpu) {
        case MENSAJE:
            recibir_mensaje(socket_cliente_cpu, log_memoria);
            // Conexión creada con éxito -> ENVIO TAMAÑO DE LOS MARCOS DE MEMORIA PARA QUE CPU PUEDA HACER PÁGINAS DE PROCESOS
            enviar_tam_marco_a_cpu();
            log_info(log_memoria, "Tamaño de marcos enviado a CPU");
            break;
        case -1:
            log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }
}

// ********* SERVER PARA RECIBIR A KERNEL *********
void server_para_kernel() {
    log_info(log_memoria, "Esperando a KERNEL...");
    socket_cliente_kernel = esperar_cliente(socket_servidor_memoria, log_memoria);

    int cod_op_kernel = recibir_operacion(socket_cliente_kernel);
    switch (cod_op_kernel) {
        case MENSAJE:
            recibir_mensaje(socket_cliente_kernel, log_memoria);
            break;
        case -1:
            log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }
}

// ********* SERVER PARA RECIBIR A I/O *********  
void server_para_io() {
    log_info(log_memoria, "Esperando a I/O...");
    int socket_io;
    t_buffer* buffer;

    //armo un while 1 para que se pueda conectar mas de una interfaz

    while(1)
    {
        socket_io = esperar_cliente(socket_servidor_memoria, log_memoria);

        int cod_op_io = recibir_operacion(socket_io);
        switch (cod_op_io) {
            case MENSAJE:
                recibir_mensaje(socket_io, log_memoria);
                break;
            case NUEVA_INTERFAZ:
                log_info(log_memoria,"Se conecto una interfaz");
                crear_hilo_escucha_interfaz(socket_io);
                buffer = recibiendo_paquete_personalizado(socket_io);
                free(buffer);
                break;
            case -1:
                log_error(log_memoria, "El cliente se desconecto.");
                //exit(1);
            default:
                log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
    
}

void enviar_tam_marco_a_cpu(){
    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_TAMAÑO_PAGINA_DE_MEMORIA);

	agregar_int_al_paquete_personalizado(paquete,config_memoria->tam_pagina);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void crear_hilo_escucha_interfaz(int socket_io)
{
    pthread_t hilo_escucha_interfaz;

    pthread_create(&hilo_escucha_interfaz, NULL, (void*)escucha_interfaz,(void*)(intptr_t)socket_io);

    pthread_detach(hilo_escucha_interfaz);
    return;
}

void escucha_interfaz(void* arg) //es un hilo porque asi puedo escuchar a varias interfaces
{
    int socket = (intptr_t)arg;

    bool control = 1;
    t_buffer* buffer;
    int cod_op_io;
    int pid;
    int tamanio;
    char* valor;
    t_list* direcciones_fisicas;

    while(control){

        cod_op_io = recibir_operacion(socket);

        switch (cod_op_io) {
            
            case IO_PIDE_LECTURA_MEMORIA:
                
                log_info(log_memoria, "IO me pide la lectura de un espacio de memoria");
                buffer = recibiendo_paquete_personalizado(socket);
                pid = recibir_int_del_buffer(buffer);
	            tamanio = recibir_int_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer , sizeof(t_direccion_fisica));
                
                usleep(config_memoria->retardo_respuesta *1000);

                
                io_pide_lectura(socket,pid, tamanio, direcciones_fisicas); 
                
                free(buffer);
                break;

            case IO_PIDE_ESCRITURA_MEMORIA:
                
                log_info(log_memoria, "IO me pide la escritura de un espacio de memoria");
                buffer = recibiendo_paquete_personalizado(socket);

                pid = recibir_int_del_buffer(buffer);
	            tamanio = recibir_int_del_buffer(buffer);
                valor = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer , sizeof(t_direccion_fisica));
                t_direccion_fisica* dir;
                if(direcciones_fisicas == NULL){
                    log_error(log_memoria, "Direcciones_fisicas es null");
                }
                for(int i = 0; i < list_size(direcciones_fisicas); i++)
                {
                    dir = list_get(direcciones_fisicas, i);
                    log_info(log_memoria, "marco q recibi de io %d", dir->nro_marco);
                    //free(dir);
                }
                usleep(config_memoria->retardo_respuesta *1000);

            
                
                guardar_string_io_en_memoria(pid, direcciones_fisicas, valor, tamanio, socket);
                
                free(buffer);

                break;

            case -1:
                
                log_error(log_memoria, "Una IO se desconecto.");
                control = 0; 
                break;

            default:
                
                log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }

    return ;
}


