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
    socket_cliente_io = esperar_cliente(socket_servidor_memoria, log_memoria);

    int cod_op_io = recibir_operacion(socket_cliente_io);
    switch (cod_op_io) {
        case MENSAJE:
            recibir_mensaje(socket_cliente_io, log_memoria);
            break;
        case -1:
            log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }
}

void enviar_tam_marco_a_cpu(){
    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_TAMAÑO_PAGINA_DE_MEMORIA);

	agregar_int_al_paquete_personalizado(paquete,config_memoria->tam_pagina);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}