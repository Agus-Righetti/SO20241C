#include "mainKernel.h"

t_log* log_kernel;
kernel_config* config_kernel;

void iterator(char* value) 
{
	log_info(log_kernel, value);
}

int main(int argc, char* argv[]) 
{
    decir_hola("Kernel");

    // ************* Creo el log y el config del kernel para uso general *************
    log_kernel = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);
    config_kernel = armar_config(log_kernel);

    // ************* Esto es para funcionar como cliente con el CPU *************
    int conexion_kernel_cpu = conexion_a_cpu(log_kernel, config_kernel);
    // Agrego esta conexion de interrupcion solo para tener disponible el socket, pero hay que manejarlo de otra manera
    int interrupcion_kernel_cpu = interrupcion_a_cpu(log_kernel, config_kernel);
    
    // ************* Esto es para funcionar como cliente con la Memoria *************
    int conexion_kernel_memoria = conexion_a_memoria(log_kernel, config_kernel);
    
    // ************* Esto es para funcionar como servidor para el I/O *************
    server_para_io(config_kernel, log_kernel);

    // ************* Esto es para funcionar como servidor para el I/O *************
    enviar_proceso_a_cpu();

    //************* Destruyo el log y cierro programa *************
    log_destroy(log_kernel);
	return EXIT_SUCCESS;
}