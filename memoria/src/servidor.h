#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>
#include "configuracion.h"

typedef struct {
    memoria_config* config_memoria;
    t_log* log_memoria;
} thread_args_memoria;  //argumentos de los hilos

// *************** DECLARACIÓN DE FUNCIONES **********
// ********* SERVER PARA RECIBIR A CPU *********
void* server_para_cpu(void* args);

// ********* SERVER PARA RECIBIR A KERNEL *********
void* server_para_kernel(void* args);

// ********* SERVER PARA RECIBIR A I/O *********
void* server_para_io(void* args);

// ********* CREACION DE HILOS PARA SER SERVIDOR *********
void crear_hilos_servidor (t_log* log_memoria,memoria_config* config_memoria);

#endif