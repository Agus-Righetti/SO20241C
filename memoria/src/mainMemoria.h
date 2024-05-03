#ifndef MEMORIA_MAIN_H_
#define MEMORIA_MAIN_H_

#include "m_gestor.h"

#include "configuracion.h"
#include "servidor.h"
#include "escucha.h"
#include "atenderCPU.h"

//************** VARIABLES GLOBALES *******************
//************** CONFIG Y LOG ******************
t_log* log_memoria;
memoria_config* config_memoria;

//******** SOCKET DE MI SERVIDOR: MEMORIA ***********
int socket_servidor_memoria;

//******** SOCKETS DE TODOS LOS CLIENTES ********
int socket_cliente_cpu;
int socket_cliente_kernel;
int socket_cliente_io;

#endif