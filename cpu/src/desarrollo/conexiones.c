#include "../include/general.h"

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

    return conexion_cpu_memoria;
}