#ifndef PROCESOS_H_
#define PROCESOS_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>
#include "configuracion.h"
#include "cliente.h"

// Esto habria que sacarlo por lo mismo que dice en el .c
typedef struct {
    kernel_config* config_kernel;
    t_log* log_kernel;
} thread_args_kernel; 

//  *************** DECLARACIÓN DE FUNCIONES **********
void leer_consola ();
pthread_t hilo_consola (t_log* log_kernel, kernel_config* config_kernel);
void iniciar_proceso(char* path );
void enviar_proceso_a_cpu();
void enviar_pcb(pcb* proceso);
void recibir_pcb_(nte, pcb* proceso);
void desalojar_proceso(pcb* proceso);


//  *************** DECLARACIÓN DE FUNCIONES ***************

typedef struct {
    pcb* proceso;
} thread_args_procesos_kernel; 

#endif