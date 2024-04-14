#include <include/mainEntradaSalida.h>

t_log* log_io;
io_config* config_io;

void iterator(char* value) {
	log_info(log_io, "%s", value);
}

int main(int argc, char* argv[]) 
{
    decir_hola("una Interfaz de Entrada/Salida");
   
    // ************* LOG Y CONFIG DE USO GENERAL *************
    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config(log_io);

    // ************* CONEXION I/O --> MEMORIA *************
    int conexion_io_memoria = conexion_a_memoria(log_io, config_io); 

    // ************* CONEXION I/O --> KERNEL *************
    int conexion_io_kernel = conexion_a_kernel(log_io, config_io);

    return 0;
}