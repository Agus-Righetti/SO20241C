#ifndef IO_GESTOR_H_
#define IO_GESTOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

// Estructuras globales ---------------------------------------------------------------------------------------------------------------

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
} io_config;

typedef struct 
{
    char* nombre;
    io_config* archivo;
} Interfaz;

extern t_log* log_io;
extern io_config* config_io;

extern int socket_servidor_kernel;

#endif