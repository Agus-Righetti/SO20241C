#include "cliente.h"

// Conexion a memoria -----------------------------------------------------------------------------------------------------------------

int conexion_a_memoria(t_log* log_io, io_config* config_io)
{    
    conexion_io_memoria = crear_conexion(config_io->ip_memoria,config_io->puerto_memoria);
    if(conexion_io_memoria == -1)
    {
        log_info(log_io, "Error: No se pudo crear conexion IO --> MEMORIA");
        exit(1);
    }
    
    log_info(log_io, "Conexion con el servidor MEMORIA creada");
    enviar_mensaje("Hola MEMORIA soy IO",conexion_io_memoria);

    return conexion_io_memoria;
}

// Conexion a Kernel ------------------------------------------------------------------------------------------------------------------

int conexion_a_kernel(t_log* log_io, io_config* config_io)
{    
    conexion_io_kernel = crear_conexion(config_io->ip_kernel,config_io->puerto_kernel);
    if(conexion_io_kernel == -1)
    {
        log_info(log_io, "Error: No se pudo crear conexion IO --> KERNEL");
        exit(1);
    }
    
    log_info(log_io, "Conexion con el servidor KERNEL creada");
    enviar_mensaje("Hola KERNEL soy IO",conexion_io_kernel);

    return conexion_io_kernel;
}