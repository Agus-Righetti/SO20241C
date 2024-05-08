#ifndef ENTRADASALIDA_MAIN_H_
#define ENTRADASALIDA_MAIN_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

#include "configuracion.h"
#include "cliente.h"

t_log* log_io;
io_config* config_io;

void terminar_programa(log_io, config_io);

#endif