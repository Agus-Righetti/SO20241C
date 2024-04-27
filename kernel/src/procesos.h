#ifndef PROCESOS_H
#define PROCESOS_H
#include <utils/hello.h>
#include <utils/utilsShare.h>
#include "configuracion.h"

typedef struct {
    kernel_config* config_kernel;
    t_log* log_kernel;
} thread_args_kernel;

//  *************** DECLARACIÓN DE FUNCIONES ***************

typedef struct {
    t_log* log_kernel;
    pcb* proceso;
    int socket_cliente;
} thread_args_procesos_kernel; 

#endif