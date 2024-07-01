#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

#include "configuracion.h"



//  *************** DECLARACIÓN DE FUNCIONES **********
void server_para_io(kernel_config* config_kernel, t_log* log_kernel);
void crear_interfaz(op_code interfaz_nueva, int socket);

#endif