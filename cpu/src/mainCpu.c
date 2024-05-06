#include <mainCpu.h>

int main(int argc, char* argv[]) {
    decir_hola("CPU");
    
	// Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    config_cpu = armar_config(log_cpu);

    // Conexion CPU --> Memoria -------------------------------------------------------------------------------------------------------

    int conexion_cpu_memoria = conexion_a_memoria(log_cpu, config_cpu);

	// Server para recibir a Kernel ---------------------------------------------------------------------------------------------------
    
	server_para_kernel(config_cpu, log_cpu);

    // liberar_cliente(conexion_cpu_memoria); -> VER
    terminar_programa(log_cpu, config_cpu);
	return EXIT_SUCCESS;
}

void terminar_programa(t_log* log_cpu, t_config* config_cpu)
{
	if (log_cpu != NULL) 
    {
		log_destroy(log_cpu);
	}
	if (config_cpu != NULL) 
    {
		config_destroy(config_cpu);
	}
}