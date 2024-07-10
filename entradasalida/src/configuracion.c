#include "configuracion.h"

// Configuracion ----------------------------------------------------------------------------------------------------------------------

io_config* armar_config(t_log* log_io, char* nombre_config) 
{
    t_config* config_aux;
    io_config* aux_io_config = malloc(sizeof(io_config));  

    config_aux = config_create(nombre_config);
    if (config_aux == NULL)
    {
        log_error(log_io, "Error: No se pudo crear el config de I/O");
        exit(1);
    }

    char* tipo_interfaz = strdup(config_get_string_value(config_aux, "TIPO_INTERFAZ"));

    aux_io_config->nombre = nombre_config;
    aux_io_config->tipo_interfaz = tipo_interfaz;
    aux_io_config->ip_kernel = strdup(config_get_string_value(config_aux, "IP_KERNEL"));
    aux_io_config->puerto_kernel = strdup(config_get_string_value(config_aux, "PUERTO_KERNEL"));
    
    switch (obtener_tipo_interfaz(tipo_interfaz))
    {
    case GENERICA:

        aux_io_config->tiempo_unidad_trabajo = config_get_int_value(config_aux, "TIEMPO_UNIDAD_TRABAJO");

        break;
        
    case STDIN:
    case STDOUT:
        
        aux_io_config->ip_memoria= strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
        aux_io_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
        break;

    case DIALFS:

        aux_io_config->ip_memoria= strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
        aux_io_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
        aux_io_config->tiempo_unidad_trabajo = config_get_int_value(config_aux, "TIEMPO_UNIDAD_TRABAJO");
        aux_io_config->path_base_dialfs = strdup(config_get_string_value(config_aux, "PATH_BASE_DIALFS"));
        aux_io_config->block_size = config_get_int_value(config_aux, "BLOCK_SIZE");
        aux_io_config->block_count = config_get_int_value(config_aux, "BLOCK_COUNT");
        aux_io_config->retraso_compactacion = config_get_int_value(config_aux, "RETRASO_COMPACTACION");

        crear_archivos_gestion_fs();

        cola_archivos_en_fs = queue_create();

        
        break;

    default:
        break;
    }

    log_info(log_io, "Se creo el struct config_io con exito");

    config_destroy(config_aux);

    return aux_io_config;
}

// metadatos_config* armar_config_metadatos()
// {
//     t_config* config_aux;
//     metadatos_config* aux_metadatos_config = malloc(sizeof(metadatos_config));  

//     config_aux = config_create("metadatos.config");
//     if (config_aux == NULL)
//     {
//         log_error(log_io, "Error: No se pudo crear el config de metadatos");
//         exit(1);
//     }

//     aux_metadatos_config->bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
//     aux_metadatos_config->tamanio_archivo = config_get_int_value(config_aux, "TAMAÑO_ARCHIVO");
//     aux_metadatos_config->max_path = config_get_int_value(config_aux, "MAX_PATH");

//     config_destroy(config_aux);
//     return aux_metadatos_config;
// }

// void iterator(char* value) 
// {
// 	log_info(log_io, "%s", value);
// }