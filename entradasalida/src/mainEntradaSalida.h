#ifndef ENTRADASALIDA_MAIN_H_
#define ENTRADASALIDA_MAIN_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

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

#endif