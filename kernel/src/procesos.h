#ifndef PROCESOS_H_
#define PROCESOS_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>
#include "configuracion.h"

typedef struct {
    kernel_config* config_kernel;
    t_log* log_kernel;
} thread_args_kernel; 

//  *************** DECLARACIÓN DE FUNCIONES **********
void leer_consola ();
pthread_t hilo_consola (t_log* log_kernel, kernel_config* config_kernel);
void iniciar_proceso(char* path );
void enviar_proceso_a_cpu(pcb* proceso_seleccionado, int socket_cliente);
void enviar_pcb(int socket_cliente, pcb* proceso);
void recibir_pcb_(int socket_cliente, pcb* proceso);

//  *************** DECLARACIÓN DE FUNCIONES ***************

// typedef struct {
//     // t_log* log_kernel;
//     pcb* proceso;
//     int socket_cliente;
// } thread_args_procesos_kernel; 

#endif