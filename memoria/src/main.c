#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include <main.h>

t_log* log_memoria;
memoria_config* config;

int main(int argc, char* argv[]) {
    decir_hola("Memoria");
    
    
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);

    config = armar_config();
    
	int server_memoria = iniciar_servidor(config->puerto_escucha, log_memoria);
	log_info(log_memoria, "Memoria lista para recibir a CPU");
	int client_cpu = esperar_cliente(server_memoria);
    log_info(log_memoria, "AHORA RECIBI CPU");

    return 0;
}

memoria_config* armar_config(void)
{
    t_config* config_aux;
    memoria_config* aux_memoria_config = malloc(sizeof(memoria_config)); // Se inicializa la estructura

    config_aux = config_create("memoria.config");
    aux_memoria_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
    aux_memoria_config->tam_memoria = config_get_int_value(config_aux, "TAM_MEMORIA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->tam_pagina = config_get_int_value(config_aux, "TAM_PAGINA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->path_instrucciones = strdup(config_get_string_value(config_aux, "PATH_INSTRUCCIONES"));
    aux_memoria_config->retardo_respuesta = config_get_int_value(config_aux, "RETARDO_RESPUESTA"); // Se usa config_get_int_value para obtener un entero

    
    log_info(log_memoria, "Se creo el struct config_memoria con exito");

    config_destroy(config_aux);

    return aux_memoria_config;
}