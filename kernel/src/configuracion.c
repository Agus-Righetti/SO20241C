#include "configuracion.h"

// ********* DESARROLLO DEL ARMADO DE LA CONFIGURACIÓN DEL KERNEL *****
kernel_config* armar_config(t_log* log_kernel)
 {
    t_config* config_aux;
    kernel_config* aux_kernel_config = malloc(sizeof(kernel_config)); // Se inicializa la estructura que armamos 

    config_aux = config_create("kernel.config");

    aux_kernel_config->recursos = list_create();
    aux_kernel_config->instancias_recursos = list_create();

    if(config_aux == NULL){
        log_info(log_kernel, "ERROR: No se pudo crear el struct config_kernel");
        exit(1);
    }

     aux_kernel_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA")); //tipo: NUMÉRICO
     aux_kernel_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA")); //tipo: STRING
     aux_kernel_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA")); //tipo: NUMÉRICO
     aux_kernel_config->ip_cpu = strdup(config_get_string_value(config_aux, "IP_CPU")); //tipo: STRING
     aux_kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_CPU_DISPATCH")); //tipo: NUMÉRICO
     aux_kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_CPU_INTERRUPT")); //tipo: NUMÉRICO
     aux_kernel_config->algoritmo_planificacion = strdup(config_get_string_value(config_aux, "ALGORITMO_PLANIFICACION")); //tipo: STRING
     aux_kernel_config->quantum = config_get_int_value(config_aux, "QUANTUM"); //tipo: NUMÉRICO
     aux_kernel_config->recursos = config_get_array_value(config_aux, "RECURSOS"); //tipo: LISTA
     char** aux_instancias_recursos = config_get_array_value(config_aux, "INSTANCIAS_RECURSOS"); //tipo: LISTA
     aux_kernel_config->grado_multiprogramacion = config_get_int_value(config_aux, "GRADO_MULTIPROGRAMACION"); //tipo: NUMÉRICO

    cantidad_recursos = 0; 
	while(aux_instancias_recursos[cantidad_recursos]!= NULL){
		cantidad_recursos++;
	}

    aux_kernel_config->instancias_recursos = (int*)malloc(cantidad_recursos * sizeof(int));

	for(int i= 0; i<cantidad_recursos; i++){
		aux_kernel_config->instancias_recursos[i] = atoi(aux_instancias_recursos[i]);
	}

     log_info(log_kernel, "Se creo el struct config_kernel con exito");

     log_info(log_kernel, "La cantidad de instancias del primer recurso son: %d" , aux_kernel_config->instancias_recursos[0]);
     log_info(log_kernel, "La cantidad de instancias del segundo recurso son: %d" , aux_kernel_config->instancias_recursos[1]);
     log_info(log_kernel, "La cantidad de instancias del tercer recurso son: %d" , aux_kernel_config->instancias_recursos[2]);

     config_destroy(config_aux);

     return aux_kernel_config;
 }