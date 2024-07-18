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
// #include "DIALFS.h"

t_log* log_io;
io_config* config_io;
metadatos_config* config_metadatos;
pcb* proceso;

sem_t sem_ok_escritura_memoria;
sem_t sem_ok_lectura_memoria;

// int socket_servidor_kernel;
int conexion_io_kernel;
int conexion_io_memoria;
Interfaz *configuracion;

size_t bitarray_size;
t_queue* cola_archivos_en_fs;

void terminar_programa(t_log* log_io, t_config* config_io);

#endif