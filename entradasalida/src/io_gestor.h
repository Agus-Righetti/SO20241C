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
    int puerto_kernel;
    char* ip_memoria;
    int puerto_memoria;
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

typedef struct 
{
    Interfaz interfaz;
    int tiempo_unidad_trabajo;
} InterfazGenerica;

extern t_log* log_io;
extern io_config* config_io;
extern pcb* proceso;

extern int conexion_io_kernel;
extern int conexion_io_memoria;

#endif