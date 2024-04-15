#include "../include/general.h"

// ********* DESARROLLO DEL ARMADO DE LA CONFIGURACIÓN DEL KERNEL *****
kernel_config* armar_config(t_log* log_kernel)
 {
     t_config* config_aux;
     kernel_config* aux_kernel_config = malloc(sizeof(kernel_config)); // Se inicializa la estructura que armamos 

     config_aux = config_create("kernel.config");

    if(config_aux == NULL){
        log_info(log_kernel, "ERROR: No se pudo crear el struct config_kernel");
        exit(1);
    }

     aux_kernel_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
     aux_kernel_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
     aux_kernel_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
     aux_kernel_config->ip_cpu = strdup(config_get_string_value(config_aux, "IP_CPU"));
     aux_kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_CPU_DISPATCH"));
     aux_kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_CPU_INTERRUPT"));
     aux_kernel_config->algoritmo_planificacion = strdup(config_get_string_value(config_aux, "ALGORITMO_PLANIFICACION"));
     aux_kernel_config->quantum = strdup(config_get_string_value(config_aux, "QUANTUM"));
     aux_kernel_config->recursos = strdup(config_get_string_value(config_aux, "RECURSOS"));
     aux_kernel_config->instancias_recursos = strdup(config_get_string_value(config_aux, "INSTANCIAS_RECURSOS"));
     aux_kernel_config->grado_multiprogramacion = config_get_int_value(config_aux, "GRADO_MULTIPROGRAMACION");   

     log_info(log_kernel, "Se creo el struct config_kernel con exito");

     config_destroy(config_aux);

     return aux_kernel_config;
 }