#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include "io_gestor.h"
#include "cliente.h"

// Declaracion de funciones ----------------------------------------------------------------------------------------------------------

io_config* armar_config(t_log* log_io, char* nombre_config) ;
//t_queue* cola_archivos_en_fs;

void iterator(char* value);
//size_t bitarray_size;

#endif