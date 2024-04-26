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

#endif