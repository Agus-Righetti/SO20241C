#include "cliente.h"

// ************* CONEXION A MEMORIA *************
int conexion_a_memoria(t_log* log_cpu, cpu_config* config_cpu)
{    
    int conexion_cpu_memoria = crear_conexion(config_cpu->ip_memoria,config_cpu->puerto_memoria);
    if(conexion_cpu_memoria == -1)
    {
        log_info(log_cpu, "Error: No se pudo crear conexion CPU --> Memoria");
        exit(1);
    }
    
    log_info(log_cpu , "Conexion con el servidor Memoria creada");
    enviar_mensaje("Hola Memoria soy CPU",conexion_cpu_memoria);
    
    int cod_op_cpu = recibir_operacion(conexion_cpu_memoria);
    switch (cod_op_cpu) {
        case MENSAJE:
            recibir_mensaje(conexion_cpu_memoria, log_cpu);
            break;
        case -1:
            log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
            break;
    };

    return conexion_cpu_memoria;
}