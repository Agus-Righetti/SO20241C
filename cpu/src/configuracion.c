#include "configuracion.h"

// Configuracion ---------------------------------------------------------------------------------------------------------------------

cpu_config* armar_config(char* nombre_config)
{
    t_config* config_aux;
    cpu_config* aux_cpu_config = malloc(sizeof(cpu_config)); // Se inicializa la estructura que armamos 

    char* filepath = string_new();

    string_append(&filepath, "../cpu/config_files/");

    string_append(&filepath, nombre_config);

    config_aux = config_create(filepath);

    free(filepath);

    if (config_aux == NULL)
    {
        log_info(log_cpu, "Error: No se pudo crear el config de CPU");
        exit(1);
    }
    
    aux_cpu_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
    aux_cpu_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
    aux_cpu_config->puerto_escucha_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA_DISPATCH"));
    aux_cpu_config->puerto_escucha_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA_INTERRUPT"));
    aux_cpu_config->cantidad_entradas_tlb = config_get_int_value(config_aux, "CANTIDAD_ENTRADAS_TLB");
    aux_cpu_config->algoritmo_tlb = strdup(config_get_string_value(config_aux, "ALGORITMO_TLB"));

    log_info(log_cpu, "Se creo el struct config_cpu con exito");

    config_destroy(config_aux);

    return aux_cpu_config;

    free(aux_cpu_config);
}

void iterator(char* value) 
{
	log_info(log_cpu,"%s", value);
}