#include <mainMemoria.h>

int main() {
    // ************* LOG Y CONFIG DE USO GENERAL *************
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG); 
    config_memoria = armar_config();

    // ************* PAGINACIÓN *************
    paginar_memoria();
    pthread_mutex_init(&mutex_bitmap_marcos, NULL);
    pthread_mutex_init(&mutex_espacio_usuario, NULL);

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
    list_destroy(lista_procesos_recibidos);
	free(config_memoria);
    bitarray_destroy(bitmap_marcos);
    pthread_mutex_destroy(&mutex_bitmap_marcos);
    pthread_mutex_destroy(&mutex_espacio_usuario);

    // ELIMINAR COSAS
    //pthread_mutex_destroy (pthread_mutex_t *__mutex);
	//sem_destroy (sem_t *__sem) __THROW __nonnull ((1));

    return EXIT_SUCCESS;
}