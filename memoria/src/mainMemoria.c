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

    // // Supongamos que este es el string original
    // char* string_original = "Hello, World!";
    // // Puntero al string original
    // //void* string_original_puntero = string_original;

    // // Partes para reconstruir el string
    // char string_reconstruido[50] = {0};
    // void* string_reconstruido_puntero = string_reconstruido;

    // // Tamaño del string original
    // size_t string_size = strlen(string_original);

    // // Copiar la primera parte del string al espacio de usuario
    // memcpy(espacio_usuario, string_original, 5);
    // // Copiar la segunda parte del string al espacio de usuario en una posición diferente
    // memcpy(espacio_usuario + 28, string_original + 5, string_size - 5);

    // // Mostrar los valores antes de la reconstrucción
    // printf("El string original es: %s\n", string_original);
    // printf("El string reconstruido antes de reconstruirlo: %s\n", string_reconstruido);

    // // Reconstruir el string desde el espacio de usuario
    // memcpy(string_reconstruido_puntero, espacio_usuario, 5);
    // memcpy(string_reconstruido_puntero + 5, espacio_usuario + 28, string_size - 5);

    // // Mostrar los valores después de la reconstrucción
    // printf("El string reconstruido después de reconstruirlo: %s\n", string_reconstruido);


    // ************* ESPERO QUE IO SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_io();

    // ********* LIBERO TODO *********
    // Despues armar funcion que haga todo
    
	log_destroy(log_memoria);

    // Chequear que esto este bien
    liberar_conexion(socket_cliente_io);
    liberar_conexion(socket_cliente_cpu);
    liberar_conexion(socket_cliente_kernel);

    list_destroy(lista_procesos_recibidos);
	free(config_memoria);
    bitarray_destroy(bitmap_marcos);
    pthread_mutex_destroy(&mutex_bitmap_marcos);
    pthread_mutex_destroy(&mutex_espacio_usuario);
    
    free(espacio_usuario);
    sem_destroy(&sem_lista_procesos);

    return EXIT_SUCCESS;
}