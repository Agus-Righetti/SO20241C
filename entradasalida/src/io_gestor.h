#ifndef IO_GESTOR_H_
#define IO_GESTOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>


//#include <cliente.h>

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
    char* nombre;
} io_config;

typedef struct 
{
    char* nombre;
    io_config* archivo;
} Interfaz;

typedef struct
{
    int bloque_inicial;
    int tamanio_archivo;
    int max_path;
} metadatos_config;

typedef struct 
{
    Interfaz interfaz;
    int tiempo_unidad_trabajo;
} InterfazGenerica;

typedef struct{
    char* nombre_archivo;
    int bloque_inicial;
    int tamanio_archivo;
}t_metadata;

extern t_log* log_io;
extern io_config* config_io;
extern metadatos_config* config_metadatos;
extern pcb* proceso;

extern int conexion_io_kernel;
extern int conexion_io_memoria;

extern t_queue* cola_archivos_en_fs;
extern size_t bitarray_size;

#endif