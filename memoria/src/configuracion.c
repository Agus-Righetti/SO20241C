#include "configuracion.h"

// ************* CONFIGURACION *************
memoria_config* armar_config(char* nombre_config){
    t_config* config_aux;
    memoria_config* aux_memoria_config = malloc(sizeof(memoria_config)); // Se inicializa la estructura


    char* filepath = string_new();

    string_append(&filepath, "../memoria/config_files/");

    string_append(&filepath, nombre_config);

    config_aux = config_create(filepath);

    free(filepath);


    if (config_aux == NULL) {
        log_info(log_memoria, "Error: No se pudo crear el config de Memoria");
        exit(1);
    }
    aux_memoria_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
    aux_memoria_config->tam_memoria = config_get_int_value(config_aux, "TAM_MEMORIA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->tam_pagina = config_get_int_value(config_aux, "TAM_PAGINA"); // Se usa config_get_int_value para obtener un entero
    aux_memoria_config->path_instrucciones = strdup(config_get_string_value(config_aux, "PATH_INSTRUCCIONES"));
    aux_memoria_config->retardo_respuesta = config_get_int_value(config_aux, "RETARDO_RESPUESTA"); // Se usa config_get_int_value para obtener un entero
    
    lista_procesos_recibidos = list_create();

    log_info(log_memoria, "Se creo el struct config_memoria con exito");

    config_destroy(config_aux);

    return aux_memoria_config;
}

void paginar_memoria(){
    // Creo el espacio de usuario, como el tamaño total de memoria
	espacio_usuario = malloc(config_memoria->tam_memoria);
    // El espacio_usuario esta liberado en el main

	if (espacio_usuario == NULL) {
        log_info(log_memoria, "Error: No se pudo crear el espacio de usuario");
        exit(1);
    }

    cant_marcos = ceil((double)config_memoria->tam_memoria / (double)config_memoria->tam_pagina);
    // Como la (/) redondea para abajo, es necesario usar el CEIL que redondea para arriba
    // Tuve que agregar double porque el ceil opera sobre valores del tipo double
    
    // CREO EL BITMAP -> 
    // Divido por 8 porque está en Byte y lo necesito en bits
    void* espacio_bitarray = malloc(cant_marcos/8);
    bitmap_marcos = bitarray_create_with_mode(espacio_bitarray, cant_marcos/8, LSB_FIRST);
    

    // Seteo todo el bitmap en 0, para marcar que están todas libres
    for (int i = 0; i < cant_marcos; i++) {
        bitarray_clean_bit(bitmap_marcos, i);
    } 


    log_info(log_memoria, "Marcos en memoria creados con éxito. Cantidad de marcos: %d", cant_marcos);
}