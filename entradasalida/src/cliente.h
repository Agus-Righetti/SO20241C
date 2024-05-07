#ifndef CLIENTE_H_
#define CLIENTE_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

#include "configuracion.h"

// Declaracion de funciones ----------------------------------------------------------------------------------------------------------

int conexion_a_memoria(t_log* log_io, io_config* config_io);
int conexion_a_kernel(t_log* log_io, io_config* config_io);

#endif