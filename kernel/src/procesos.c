#include "procesos.h"
#include "configuracion.h"

pthread_t hilo_consola (t_log* log_kernel, kernel_config* config_kernel){ 
    // Creo lel hilo de consola
    pthread_t thread_consola;

    //Defino la estrcutura del argumento
    thread_args_kernel args = {config_kernel, log_kernel};
        
    // ********* CREO EL HILO SERVER PARA RECIBIR A CPU *********
    pthread_create(&thread_consola, NULL, leer_consola, (void*) &args);

    return thread_consola;
}

void leer_consola(){
    
     while(1) {
        char* lectura = readline("Ingrese comando: ");

        if (strcmp(lectura, "INICIAR_PROCESO") == 0) {
            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
        } else if (strcmp(lectura, "FINALIZAR_PROCESO") == 0) {
            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");
        } else {
            printf("Opción no válida\n");
        }

        // Libera la memoria asignada por readline
        free(lectura);
    }
}