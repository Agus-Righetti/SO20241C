#include "configuracion.h"

// ************* CONFIGURACION *************
memoria_config* armar_config(t_log* log_memoria)
{
    t_config* config_aux;
    memoria_config* aux_memoria_config = malloc(sizeof(memoria_config)); // Se inicializa la estructura

    config_aux = config_create("memoria.config");
    if (config_aux == NULL)
    {
        log_info(log_memoria, "Error: No se pudo crear el config de Memoria");
        exit(1);
    }
    aux_memoria_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
    aux_memoria_config->tam_memoria = config_get_int_value(config_aux, "TAM_MEMORIA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->tam_pagina = config_get_int_value(config_aux, "TAM_PAGINA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->path_instrucciones = strdup(config_get_string_value(config_aux, "PATH_INSTRUCCIONES"));
    aux_memoria_config->retardo_respuesta = config_get_int_value(config_aux, "RETARDO_RESPUESTA"); // Se usa config_get_int_value para obtener un entero
    
    log_info(log_memoria, "Se creo el struct config_memoria con exito");

    config_destroy(config_aux);

    return aux_memoria_config;
}