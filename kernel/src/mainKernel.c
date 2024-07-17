#include "mainKernel.h"


void iterator(char* value) 
{
	log_info(log_kernel, "%s", value);
}


int main(int argc, char* argv[]) 
{
    decir_hola("Kernel");
    pid_contador = 0; //Va incrementando a medida que arrancamos un nuevo proceso
    
    interrupcion_por_fin_de_proceso = false;

    pthread_mutex_init(&mutex_cola_de_new,NULL);
    pthread_mutex_init(&mutex_cola_de_ready,NULL);
    pthread_mutex_init(&mutex_cola_de_execute,NULL);
    pthread_mutex_init(&mutex_cola_de_exit,NULL);
    pthread_mutex_init(&mutex_cola_de_blocked,NULL);
    pthread_mutex_init(&mutex_cola_prioridad_vrr,NULL);
    pthread_mutex_init(&mutex_cola_general_de_procesos,NULL);
    pthread_mutex_init(&mutex_planificacion_activa,NULL);
    pthread_mutex_init(&mutex_enviando_instruccion_a_io, NULL);
    pthread_mutex_init(&mutex_cola_de_interfaces,NULL);

    cola_de_new = queue_create();
    cola_de_ready = queue_create();
    cola_de_execute = queue_create();
    cola_de_exit = queue_create();
    cola_de_blocked = queue_create();
    cola_prioridad_vrr = queue_create();
    cola_general_de_procesos = queue_create();
    cola_interfaces_conectadas = queue_create();
    cola_de_ready_prioridad = queue_create();

    sem_init(&sem_cola_de_new,0,0);
    sem_init(&sem_cola_de_ready,0,0);
    sem_init(&destruir_hilo_interrupcion,0,0);
    sem_init(&sem_cola_prioridad_vrr,0,0);
    sem_init(&sem_puedo_mandar_a_cpu,0,1);
    sem_init(&sem_planificacion_activa,0,1);



    // ************* Creo el log y el config del kernel para uso general *************
    log_kernel = log_create("kernel.log", "Kernel", 1, LOG_LEVEL_DEBUG);
    config_kernel = armar_config(log_kernel, argv[1]);

    // ************* creo las colas por recursos con sus mutex *************

    // cantidad_recursos = sizeof(config_kernel->instancias_recursos) / sizeof(config_kernel->instancias_recursos[0]);

    colas_por_recurso = malloc(cantidad_recursos * sizeof(t_queue*));
    mutex_por_recurso = malloc(cantidad_recursos * sizeof(pthread_mutex_t));

    for (int i = 0; i < cantidad_recursos; i++) {
        colas_por_recurso[i] = queue_create();
        pthread_mutex_init(&mutex_por_recurso[i], NULL);
    }

    //semaforo para manejar el grado de multiprogramacion
    sem_init(&sem_multiprogramacion,0,config_kernel->grado_multiprogramacion); 

    // ************* Esto es para funcionar como cliente con el CPU *************
    conexion_kernel_cpu = conexion_a_cpu(log_kernel, config_kernel);
    interrupcion_kernel_cpu = interrupcion_a_cpu(log_kernel, config_kernel);

    // ************* Esto es para funcionar como cliente con la Memoria *************
    conexion_kernel_memoria = conexion_a_memoria(log_kernel, config_kernel);
    // ************* Esto es para funcionar como servidor para el I/O *************
    
    log_info(log_kernel, "Estoy justo antes de server_para_io");
    pthread_t thread_escucha_conexion_io = hilo_io();
    
    //server_para_io(config_kernel, log_kernel);

    //************* HILO CONSOLA *************
    log_info(log_kernel, "Estoy por crear el hilo consola");
    pthread_t thread_consola = hilo_consola ();

    //*************HILO GESTOR DE LOS PROCESOS A ENVIAR A CPU*************
    pthread_t thread_enviar_procesos_cpu = hilo_enviar_procesos_cpu();

    pthread_t thread_pasar_procesos_new_a_ready = hilo_pasar_de_new_a_ready();

    pthread_detach(thread_enviar_procesos_cpu);

    pthread_detach(thread_pasar_procesos_new_a_ready);

    pthread_detach(thread_escucha_conexion_io);

    pthread_join(thread_consola, NULL);


    //************* Destruyo el log y cierro programa *************
    //Podriamos ahcer una funcion q termine el programa con todo esto
    //faltaria hacer el free de todos los procesos
    log_destroy(log_kernel);
    queue_destroy(cola_de_new);
    queue_destroy(cola_de_ready);
    queue_destroy(cola_de_ready_prioridad);

    for (int i = 0; i < cantidad_recursos; i++) {
        queue_destroy(colas_por_recurso[i]);
        pthread_mutex_destroy(&mutex_por_recurso[i]);
    }
    free(colas_por_recurso);
    free(mutex_por_recurso);
    
    pthread_mutex_destroy(&mutex_cola_de_new);
    pthread_mutex_destroy(&mutex_cola_de_ready);
    pthread_mutex_destroy(&mutex_cola_de_exit);
    pthread_mutex_destroy(&mutex_cola_de_blocked);
    pthread_mutex_destroy(&mutex_cola_de_execute);
    pthread_mutex_destroy(&mutex_cola_prioridad_vrr);
    pthread_mutex_destroy(&mutex_planificacion_activa);
    pthread_mutex_destroy(&mutex_enviando_instruccion_a_io);

    sem_destroy(&sem_cola_de_ready);
    sem_destroy(&sem_cola_de_new);
    sem_destroy(&sem_multiprogramacion);
    sem_destroy(&destruir_hilo_interrupcion);
    sem_destroy(&sem_puedo_mandar_a_cpu);
    sem_destroy(&sem_cola_prioridad_vrr);
    sem_destroy(&sem_planificacion_activa);

    liberar_conexion(conexion_kernel_cpu);
    liberar_conexion(interrupcion_kernel_cpu);
    liberar_conexion(conexion_kernel_memoria);

    liberar_conexion(conexion_kernel_cpu);
    liberar_conexion(interrupcion_kernel_cpu);
    liberar_conexion(conexion_kernel_memoria);

	return EXIT_SUCCESS;
}