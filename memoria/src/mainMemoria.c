#include <mainMemoria.h>

// void iterator(char* value) 
// {
// 	log_info(log_memoria, "%s", value);
// }

int main() {
    // ************* LOG Y CONFIG DE USO GENERAL *************
    t_log* log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG); 
    memoria_config* config_memoria = armar_config(log_memoria);

    // ********* CREO HILOS PARA CREAR LAS CONEXIONES *********
    crear_hilos_servidor (log_memoria, config_memoria) ;
  
    // ********* LIBERO TODO *********
	log_destroy(log_memoria);
	free(config_memoria);
	
    return EXIT_SUCCESS;
}