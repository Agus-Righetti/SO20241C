#include "mainMemoria.h"

int main() {
    // ************* LOG Y CONFIG DE USO GENERAL *************
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG); 
    config_memoria = armar_config();
    

    // ************* PAGINACIÓN *************
    paginar_memoria();
    pthread_mutex_init(&mutex_bitmap_marcos, NULL);
    pthread_mutex_init(&mutex_espacio_usuario, NULL);

    // ************ LISTA PROCESOS ************
    sem_init(&sem_lista_procesos, 0, 1);

    // ********* INICIALIZO MEMORIA COMO SERVIDOR *********
    inicializar_servidor();

    // ************* ESPERO QUE CPU SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_cpu();
    
    // ************* ESPERO QUE KERNEL SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_kernel();

    // ************* ESPERO QUE IO SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_io();
    


    // ********* LIBERO TODO *********
	log_destroy(log_memoria);
    free(config_memoria);
    free(espacio_usuario);
    //free(espacio_bitarray);

    // Chequear que esto este bien
    liberar_conexion(socket_cliente_io);
    liberar_conexion(socket_cliente_cpu);
    liberar_conexion(socket_cliente_kernel);

    list_destroy(lista_procesos_recibidos);

    bitarray_destroy(bitmap_marcos);
    pthread_mutex_destroy(&mutex_bitmap_marcos);
    pthread_mutex_destroy(&mutex_espacio_usuario);
    
    sem_destroy(&sem_lista_procesos);

    
    
    return EXIT_SUCCESS;
}