#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

#include "configuracion.h"

// ************* DECLARACIÓN DE FUNCIONES *************
// ************* CONEXION A MEMORIA (CLIENTE) *************
int conexion_a_memoria(t_log* log_io, io_config* config_io);

// ************* CONEXION A KERNEL (CLIENTE) *************
int conexion_a_kernel(t_log* log_io, io_config* config_io);

#endif