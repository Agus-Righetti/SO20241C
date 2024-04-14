#ifndef GENERAL_H_
#define GENERAL_H_

#include "mainEntradaSalida.h"

// ************* CONFIGURACION *************

typedef struct 
{
    char* tipo_interfaz;
    char* tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
    char* path_base_dialfs;
    char* block_size;
    char* block_count;
    
    //Todo esta como char pero hay que cambiarlo al tipo q sea necesario
    // cuando identifiquemos en que lo vamos a usar
    
}io_config;

io_config* armar_config(t_log* log_io);

// ************* CONEXION A MEMORIA *************

int conexion_a_memoria(t_log* log_io, io_config* config_io);


// ************* CONEXION A KERNEL *************

int conexion_a_kernel(t_log* log_io, io_config* config_io);

#endif