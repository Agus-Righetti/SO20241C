#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

#include "configuracion.h"
#include "pcb.h"

// ************* DECLARACION DE FUNCIONES *************
void server_para_kernel(cpu_config* config_cpu,t_log* log_cpu);
void interrupcion_para_kernel(cpu_config* config_cpu,t_log* log_cpu);
int esperar_cliente_de_kernel(int socket_servidor, t_log* logger);

#endif