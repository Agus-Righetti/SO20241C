#include "servidor.h"

// ********* INICIALIZO MEMORIA COMO SERVIDOR *********
void inicializar_servidor(){
    socket_servidor_memoria = iniciar_servidor(config_memoria->puerto_escucha, log_memoria);
    if (socket_servidor_memoria == -1)
    {
        log_info(log_memoria, "Error: No se pudo iniciar Memoria como servidor");
        exit(1);
    }
    log_info(log_memoria, "Memoria lista para recibir clientes");
}

//********* SERVER PARA RECIBIR A CPU *********
void server_para_cpu() {
    log_info(log_memoria, "Esperando a CPU...");
    socket_cliente_cpu = esperar_cliente(socket_servidor_memoria, log_memoria);

    int cod_op_cpu = recibir_operacion(socket_cliente_cpu);
    switch (cod_op_cpu) {
        case MENSAJE:
            recibir_mensaje(socket_cliente_cpu, log_memoria);
            break;
        case -1:
            log_error(log_memoria, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(log_memoria,"Operacion desconocida. No quieras meter la pata");
            break;
    }
}

// // ********* SERVER PARA RECIBIR A KERNEL *********
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

// // ********* SERVER PARA RECIBIR A I/O *********  
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