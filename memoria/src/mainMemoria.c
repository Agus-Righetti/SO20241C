#include <mainMemoria.h>

int main() {
    // ************* LOG Y CONFIG DE USO GENERAL *************
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG); 
    config_memoria = armar_config();

    // ********* INICIALIZO MEMORIA COMO SERVIDOR *********
    inicializar_servidor();

    // ************* ESPERO QUE CPU SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_cpu();

    // ************* ESPERO QUE KERNEL SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_kernel();
    
    // ************* ESPERO QUE IO SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_io();

    // ********* LIBERO TODO *********
	log_destroy(log_memoria);
    list_destroy(lista_procesos_recibidos);
	free(config_memoria);
	
    return EXIT_SUCCESS;
}