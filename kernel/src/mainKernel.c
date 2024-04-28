#include "mainKernel.h"

t_log* log_kernel;
kernel_config* config_kernel;
t_list* cola_de_new;
t_list* cola_de_ready;
int grado_multiprogramacion_actual; //dice cual es el grado actual de multip. hay que incrementarlo al pasar un proceso a ready y decrementarlo al pasar uno a exit 
int pid_contador;
int conexion_kernel_cpu;

void iterator(char* value) 
{
	log_info(log_kernel, value);
}

int main(int argc, char* argv[]) 
{
    decir_hola("Kernel");
    pid_contador = 0; //Va incrementando a meidda que arrancamos un nuevo proceso
    
    cola_de_new = list_create();
    cola_de_ready = list_create();

    grado_multiprogramacion_actual = 0;

    // ************* Creo el log y el config del kernel para uso general *************
    log_kernel = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);
    config_kernel = armar_config(log_kernel);

    // ************* Esto es para funcionar como cliente con el CPU *************
    //conexion_kernel_cpu = conexion_a_cpu(log_kernel, config_kernel);


    // ************* Esto es para funcionar como cliente con la Memoria *************
    int conexion_kernel_memoria = conexion_a_memoria(log_kernel, config_kernel);
    // ************* Esto es para funcionar como servidor para el I/O *************
    server_para_io(config_kernel, log_kernel);

    //************* HILO CONSOLA *************
    pthread_t thread_consola = hilo_consola (log_kernel, config_kernel);

    pthread_join(thread_consola, NULL);
    //************* Destruyo el log y cierro programa *************
    log_destroy(log_kernel);
    list_destroy(cola_de_new);
    list_destroy(cola_de_ready);
	return EXIT_SUCCESS;
}