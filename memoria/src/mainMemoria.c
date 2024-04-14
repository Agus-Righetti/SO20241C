#include <include/mainMemoria.h>

t_log* log_memoria;
memoria_config* config_memoria;

// void iterator(char* value) 
// {
// 	log_info(log_memoria, "%s", value);
// }

int main(int argc, char* argv[]) 
{
	decir_hola("Memoria");
    
	// ************* LOG Y CONFIG DE USO GENERAL *************
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG);
    config_memoria = armar_config(log_memoria);
    
	// ********* SERVER PARA RECIBIR A CPU *********
	server_para_cpu(config_memoria, log_memoria);

	// ********* SERVER PARA RECIBIR A KERNEL *********
	server_para_kernel(config_memoria, log_memoria);
	
	// ********* SERVER PARA RECIBIR A I/O *********
	server_para_io(config_memoria, log_memoria);
	
	//Despues podemos agregar una funcion y liberar memoria ahi
	log_destroy(log_memoria);
	free(config_memoria);

    return EXIT_SUCCESS;
}





