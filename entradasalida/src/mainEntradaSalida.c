#include <mainEntradaSalida.h>

int main(int argc, char* argv[]) 
{
    decir_hola("IO");
   
    // Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config(log_io);

    // Conexion IO --> Memoria --------------------------------------------------------------------------------------------------------

    escuchar_memoria(); 

    // Conexion IO --> Kernel ---------------------------------------------------------------------------------------------------------
    
    escuchar_kernel();
  
    // terminar_programa(log_io, config_io);

    return 0;
}

void terminar_programa(t_log* log_io, t_config* config_io)
{
    if (log_io != NULL) 
    {
		log_destroy(log_io);
    }
    if (config_io != NULL) 
    {
		config_destroy(config_io);
    }
}