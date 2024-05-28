#include <mainCpu.h>

t_log* log_cpu;
cpu_config* config_cpu;

int main(int argc, char* argv[]) 
{
    decir_hola("CPU");
    
	// Log y config de uso general ----------------------------------------------------------------------------------------------------

	log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
	config_cpu = armar_config(log_cpu);

    // Conexion CPU --> Memoria -------------------------------------------------------------------------------------------------------

	int socket_cliente_cpu = conexion_a_memoria();

	// Server para recibir a Kernel ---------------------------------------------------------------------------------------------------
	
	escuchar_kernel();

	log_info(log_cpu, "El PID es: %d", pcb_recibido->pid);

	// Instrucciones con memoria
	
	escuchar_memoria();
   
	// terminar_programa();

	return EXIT_SUCCESS;
}

// void terminar_programa()
// {
// 	if (log_cpu != NULL) 
//     {
// 		log_destroy(log_cpu);
// 	}
// 	if (config_cpu != NULL) 
//     {
// 		config_destroy(config_cpu);
// 	}
// }