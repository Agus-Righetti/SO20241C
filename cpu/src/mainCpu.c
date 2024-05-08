#include <mainCpu.h>

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

int main(int argc, char* argv[]) 
{
    decir_hola("CPU");
    
	// Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    config_cpu = armar_config(log_cpu);

    // Conexion CPU --> Memoria -------------------------------------------------------------------------------------------------------

	escuchar_memoria();

	// Server para recibir a Kernel ---------------------------------------------------------------------------------------------------
    
	escuchar_kernel();

    // liberar_cliente(conexion_cpu_memoria); -> VER
    // terminar_programa(log_cpu, config_cpu);

	return EXIT_SUCCESS;
}