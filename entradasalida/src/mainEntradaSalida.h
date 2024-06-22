#ifndef ENTRADASALIDA_MAIN_H_
#define ENTRADASALIDA_MAIN_H_

#include "io_gestor.h"
#include "configuracion.h"
#include "cliente.h"
#include "interfazGENERICA.h"
#include "interfazSTDIN.h"
#include "interfazSTDOUT.h"
#include "interfazDIALFS.h"
#include "conexion.h"

t_log* log_io;
io_config* config_io;
metadatos_config* config_metadatos;
pcb* proceso;

// int socket_servidor_kernel;
int conexion_io_kernel;
int conexion_io_memoria;
Interfaz *configuracion;

void terminar_programa(log_io, config_io);

#endif