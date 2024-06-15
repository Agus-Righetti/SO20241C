#include "interfazDIALFS.h"

void leer_configuracion_dialfs(Interfaz *configuracion)
{
    iniciar_config_dialfs(configuracion);

    // Loggeamos el valor de config
    log_info(log_io, "Lei el TIPO_INTERFAZ %s, el TIEMPO_UNIDAD_TRABAJO %d, el IP_KERNEL %s, el PUERTO_KERNEL %d, el IP_MEMORIA %s, el PUERTO_MEMORIA %d, el PATH_BASE_DIALFS %s, el BLOCK_SIZE %d, el BLOCK_COUNT %d y el RETRASO_COMPACTACION %d.", 
             configuracion->archivo->tipo_interfaz, 
             configuracion->archivo->tiempo_unidad_trabajo,
             configuracion->archivo->ip_kernel, 
             configuracion->archivo->puerto_kernel, 
             configuracion->archivo->ip_memoria,
             configuracion->archivo->puerto_memoria,
             configuracion->archivo->path_base_dialfs,
             configuracion->archivo->block_size,
             configuracion->archivo->block_count,
             configuracion->archivo->retraso_compactacion);
}

void iniciar_config_dialfs(Interfaz *configuracion)
{   
    if (configuracion == NULL) 
    {
        printf("El puntero de configuración es NULL\n");
        exit(2);
    }

    // Asignar memoria para configuracion->archivo
    configuracion->archivo = malloc(sizeof(io_config));
    if (configuracion->archivo == NULL) 
    {
        printf("No se puede crear la config archivo\n");
        exit(2);
    }

    // Inicializa la estructura del archivo de configuración desde el archivo de configuración
    t_config* config = config_create("./io.config");
    if (config == NULL) 
    {
        printf("No se puede leer el archivo de config\n");
        free(configuracion->archivo);
        exit(2);
    }
    
    configuracion->archivo->tipo_interfaz = strdup(config_get_string_value(config, "TIPO_INTERFAZ"));
    configuracion->archivo->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    configuracion->archivo->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
    configuracion->archivo->puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    configuracion->archivo->ip_memoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
    configuracion->archivo->puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    configuracion->archivo->path_base_dialfs = strdup(config_get_string_value(config, "PATH_BASE_DIALFS"));
    configuracion->archivo->block_size = config_get_int_value(config, "BLOCK_SIZE");
    configuracion->archivo->block_count = config_get_int_value(config, "BLOCK_COUNT");
    configuracion->archivo->retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");

    // Liberar el t_config
    config_destroy(config);
}

void liberar_configuracion_dialfs(Interfaz* configuracion)
{
    if(configuracion) 
    {
        free(configuracion->archivo->tipo_interfaz);
        // free(configuracion->archivo->tiempo_unidad_trabajo);
        free(configuracion->archivo->ip_kernel);
        // free(configuracion->archivo->puerto_kernel);
        free(configuracion->archivo->ip_memoria);
        // free(configuracion->archivo->puerto_memoria);
        free(configuracion->archivo->path_base_dialfs);
        // free(configuracion->archivo->block_size);
        // free(configuracion->archivo->block_count);
        // free(configuracion->archivo->retraso_compactacion);
        free(configuracion->archivo);
    }
}