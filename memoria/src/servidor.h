#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>
#include "configuracion.h"

// *************** DECLARACIÓN DE FUNCIONES **********
// ********* SERVER PARA RECIBIR A CPU *********
void server_para_cpu(memoria_config* config_memoria, t_log* log_memoria);

// ********* SERVER PARA RECIBIR A KERNEL *********
void server_para_kernel(memoria_config* config_memoria,t_log* log_memoria);

// ********* SERVER PARA RECIBIR A I/O *********
void server_para_io(memoria_config* config_memoria,t_log* log_memoria);

#endif