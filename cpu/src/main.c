#include <stdlib.h>
#include <stdio.h>
#include <utils/hello.h>
#include <utils/utils.h>
#include <main.h>

t_log* log_cpu;
cpu_config* config_cpu;

int main(int argc, char* argv[]) {
    decir_hola("CPU");
    
    log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    config_cpu = armar_config();
    int conexion_cpu_memoria;
    conexion_cpu_memoria = crear_conexion(config_cpu->ip_memoria,config_cpu->puerto_memoria);
    log_info(log_cpu , "Conexion con el servidor memoria creada");
    enviar_mensaje("Hola memoria como estas?",conexion_cpu_memoria);
	paquete(conexion_cpu_memoria);


    return 0;
}

cpu_config* armar_config(void)
{
    t_config* config_aux;
    cpu_config* aux_cpu_config = malloc(sizeof(cpu_config)); // Se inicializa la estructura que armamos 

    config_aux = config_create("cpu.config");
    aux_cpu_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
    aux_cpu_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
    aux_cpu_config->puerto_escucha_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA_DISPATCH"));
    aux_cpu_config->puerto_escucha_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA_INTERRUPT"));
    aux_cpu_config->cantidad_entradas_tlb = config_get_int_value(config_aux, "CANTIDAD_ENTRADAS_TLB");
    aux_cpu_config->algoritmo_tlb = strdup(config_get_string_value(config_aux, "ALGORITMO_TLB"));

    log_info(log_cpu, "Se creo el struct config_cpu con exito");

    config_destroy(config_aux);

    return aux_cpu_config;
}

// Esta funcion paquete es para capturar las lineas que escribamos en la consola y mandarla a 
//memoria, es solo para probar si las recibe bien
void paquete(int conexion)
{
	
	char* leido;
	t_paquete* paquete;

	
	leido = readline("< Escribi lo que quieras enviar a memoria... escribi vacio para terminar");
	paquete = crear_paquete();
	while(strcmp(leido,""))
	{
		agregar_a_paquete(paquete,leido,strlen(leido)+1);
		free(leido);
		leido = readline("< ");
		
	};
	free(leido);

	enviar_paquete(paquete,conexion);

	eliminar_paquete(paquete);
	
	
}