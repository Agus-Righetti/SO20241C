#include "mainMemoria.h"

int main() {
    // ************* LOG Y CONFIG DE USO GENERAL *************
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG); 
    config_memoria = armar_config();

    // ************* PAGINACIÓN *************
    paginar_memoria();
    pthread_mutex_init(&mutex_bitmap_marcos, NULL);
    pthread_mutex_init(&mutex_espacio_usuario, NULL);

    // *******************
    // Ejemplo con un entero
    int dato_int = 42;
    guardar_en_memoria(&dato_int, sizeof(dato_int), 0);
    
    int* resultado_int = (int*)leer_desde_memoria(sizeof(dato_int), 0);
    if (resultado_int != NULL) {
        printf("Dato entero leído: %d\n", *resultado_int);
        free(resultado_int);  // Liberamos la memoria asignada
    }

    // Ejemplo con un flotante
    float dato_float = 3.14;
    guardar_en_memoria( &dato_float, sizeof(dato_float), sizeof(dato_int));  // Almacenamos después del entero
    
    float* resultado_float = (float*)leer_desde_memoria(sizeof(dato_float), sizeof(dato_int));
    if (resultado_float != NULL) {
        printf("Dato flotante leído: %.2f\n", *resultado_float);
        free(resultado_float);  // Liberamos la memoria asignada
    }
    //***********************

    // ********* INICIALIZO MEMORIA COMO SERVIDOR *********
    inicializar_servidor();
    
    // ************* ESPERO QUE CPU SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_cpu();

    // ************* ESPERO QUE KERNEL SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_kernel();
    
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

    return EXIT_SUCCESS;
}