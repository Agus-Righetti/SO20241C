#ifndef ENTRADASALIDA_MAIN_H_
#define ENTRADASALIDA_MAIN_H_

#include "io_gestor.h"
#include "configuracion.h"
#include "cliente.h"
#include "interfazGenerica.h"
//#include "conexion.h"

t_log* log_io;
io_config* config_io;
pcb* proceso;

int socket_servidor_kernel;
Interfaz *configuracion;

void terminar_programa(log_io, config_io);

#endif