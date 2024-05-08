#include <mainEntradaSalida.h>

int main(int argc, char* argv[]) 
{
    decir_hola("Una Interfaz de IO");
   
    // Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config(log_io);

    // Conexion IO --> Memoria --------------------------------------------------------------------------------------------------------

    int conexion_io_memoria = conexion_a_memoria(log_io, config_io); 

    // Conexion IO --> Kernel ---------------------------------------------------------------------------------------------------------
    
    int conexion_io_kernel = conexion_a_kernel(log_io, config_io);

    terminar_programa(log_io, config_io);

    return 0;
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