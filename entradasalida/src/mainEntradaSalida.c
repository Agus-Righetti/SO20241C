#include <mainEntradaSalida.h>

t_log* log_io;
io_config* config_io;

void iterator(char* value) {
	log_info(log_io, "%s", value);
}

int main(int argc, char* argv[]) 
{
    decir_hola("una Interfaz de Entrada/Salida");
   
    // Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config(log_io);

    // Conexion IO --> Memoria --------------------------------------------------------------------------------------------------------

    int conexion_io_memoria = conexion_a_memoria(log_io, config_io); 

    // Conexion IO --> Kernel ---------------------------------------------------------------------------------------------------------
    
    int conexion_io_kernel = conexion_a_kernel(log_io, config_io);

    // Podemos agregar una funcion para liberar todo de un saque
    log_destroy(log_io);
	free(config_io);
    return 0;
}