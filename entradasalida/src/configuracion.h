#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

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

io_config* armar_config(t_log* log_io);
void iterator(char* value);

#endif