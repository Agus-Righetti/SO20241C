#include <mainMemoria.h>

int main() {
    // ************* LOG Y CONFIG DE USO GENERAL *************
    log_memoria = log_create("memoria.log", "Memoria", 1, LOG_LEVEL_DEBUG); 
    config_memoria = armar_config();

    // ********* INICIALIZO MEMORIA COMO SERVIDOR *********
    inicializar_servidor();

    // ************* ESPERO QUE CPU SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_cpu();

    // ************* ESPERO QUE KERNEL SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_kernel();

    // ************* ESPERO QUE IO SE CONECTE COMO CLIENTE Y CREO HILO PARA ESCUCHA ACTIVA *************
    recibir_escuchar_io();

    //******************************************************************************************************
    // FALTA IMPLEMENTAR: MEMORIA RECIBE CREACION_PROCESO DE KERNEL, ENTONCES LEE EL ARCHIVO
    //ESTA FUNCION SE DEBE IMPLEMENTAR EN OTRO LADO, ESTA ACA PARA PROBAR
    t_list* instrucciones = leer_archivo_y_cargar_instrucciones("path_instrucciones.txt"); //el archivo lo recibe de kernel
    //FALTA IMPLEMENTAR CREACION DE ESTRUCTURAS PARA PROCESOS (TABLAS, ESPACIOS CONTIGUOS MEMORIA)

    // CPU SOLICITA UNA INSTRUCCION INDICANDO PROGRAM COUNTER
    cpu_pide_instruccion(instrucciones);

    // ********* LIBERO TODO *********
	log_destroy(log_memoria);
	free(config_memoria);
	
    return EXIT_SUCCESS;
}