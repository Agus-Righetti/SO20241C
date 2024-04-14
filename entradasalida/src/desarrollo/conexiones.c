#include "../include/general.h"

// ************* CONEXION A MEMORIA *************
int conexion_a_memoria(t_log* log_io, io_config* config_io)
{    
    int conexion_io_memoria = crear_conexion(config_io->ip_memoria,config_io->puerto_memoria);
    if(conexion_io_memoria == -1)
    {
        log_info(log_io, "Error: No se pudo crear conexion I/O --> Memoria");
        exit(1);
    }
    
    log_info(log_io , "Conexion con el servidor Memoria creada");
    enviar_mensaje("Hola Memoria soy I/O",conexion_io_memoria);

    return conexion_io_memoria;
}


// ************* CONEXION A KERNEL *************
int conexion_a_kernel(t_log* log_io, io_config* config_io)
{    
    int conexion_io_kernel = crear_conexion(config_io->ip_kernel,config_io->puerto_kernel);
    if(conexion_io_kernel == -1)
    {
        log_info(log_io, "Error: No se pudo crear conexion I/O --> Kernel");
        exit(1);
    }
    
    log_info(log_io , "Conexion con el servidor Kernel creada");
    enviar_mensaje("Hola Kernel soy I/O",conexion_io_kernel);

    return conexion_io_kernel;
}