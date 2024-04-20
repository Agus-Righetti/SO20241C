#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

#include "configuracion.h"

//  *************** DECLARACIÓN DE FUNCIONES **********
int conexion_a_cpu(t_log* log_kernel, kernel_config* config_kernel);
int conexion_a_memoria(t_log* log_kernel, kernel_config* config_kernel);

#endif