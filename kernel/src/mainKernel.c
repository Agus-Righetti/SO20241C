#include "mainKernel.h"

t_log* log_kernel;
kernel_config* config_kernel;
t_queue* cola_de_new;
t_queue* cola_de_ready;
int pid_contador;
int conexion_kernel_cpu;
int interrupcion_kernel_cpu;
int conexion_kernel_memoria;
pthread_mutex_t mutex_cola_de_ready;
pthread_mutex_t mutex_cola_de_new;
pthread_mutex_t mutex_grado_programacion;
sem_t sem_cola_de_ready;
sem_t sem_cola_de_new;
sem_t sem_multiprogramacion;


void iterator(char* value) 
{
	log_info(log_kernel, "%s", value);
}

int main(int argc, char* argv[]) 
{
    decir_hola("Kernel");
    pid_contador = 0; //Va incrementando a meidda que arrancamos un nuevo proceso
    
    pthread_mutex_init(&mutex_cola_de_new,NULL);
    pthread_mutex_init(&mutex_cola_de_ready,NULL);
    cola_de_new = queue_create();
    cola_de_ready = queue_create();
    sem_init(&sem_cola_de_new,0,0);
    sem_init(&sem_cola_de_ready,0,0);

    // ************* Creo el log y el config del kernel para uso general *************
    log_kernel = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);
    config_kernel = armar_config(log_kernel);

    //semaforo para manejar el grado de multiprogramacion
    sem_init(&sem_multiprogramacion,0,config_kernel->grado_multiprogramacion); 

    // ************* Esto es para funcionar como cliente con el CPU *************
    conexion_kernel_cpu = conexion_a_cpu(log_kernel, config_kernel);
    interrupcion_kernel_cpu = interrupcion_a_cpu(log_kernel, config_kernel);

    // ************* Esto es para funcionar como cliente con la Memoria *************
    conexion_kernel_memoria = conexion_a_memoria(log_kernel, config_kernel);
    // ************* Esto es para funcionar como servidor para el I/O *************
    server_para_io(config_kernel, log_kernel);

    //************* HILO CONSOLA *************
    pthread_t thread_consola = hilo_consola ();

    pthread_join(thread_consola, NULL);
    //*************HILO GESTOR DE LOS PROCESOS A ENVIAR A CPU*************

    pthread_t thread_enviar_procesos_cpu = hilo_enviar_procesos_cpu();

    //************* Destruyo el log y cierro programa *************
    log_destroy(log_kernel);
    queue_destroy(cola_de_new);
    queue_destroy(cola_de_ready);
    pthread_mutex_destroy(&mutex_cola_de_new);
    pthread_mutex_destroy(&mutex_cola_de_ready);
    sem_destroy(&sem_cola_de_ready);
    sem_destroy(&sem_cola_de_new);
    sem_destroy(&sem_multiprogramacion);


	return EXIT_SUCCESS;
}