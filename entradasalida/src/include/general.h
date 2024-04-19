#ifndef GENERAL_H_
#define GENERAL_H_

#include "mainEntradaSalida.h"

// ************* CONFIGURACION *************

typedef struct 
{
    char* tipo_interfaz;
    int tiempo_unidad_trabajo;
    char* ip_kernel;
    char* puerto_kernel;
    char* ip_memoria;
    char* puerto_memoria;
    char* path_base_dialfs;
    int block_size;
    int block_count;
    int retraso_compactacion;
    
}io_config;

io_config* armar_config(t_log* log_io);

// ************* CONEXION A MEMORIA (CLIENTE) *************

int conexion_a_memoria(t_log* log_io, io_config* config_io);


// ************* CONEXION A KERNEL (CLIENTE) *************

int conexion_a_kernel(t_log* log_io, io_config* config_io);

#endif