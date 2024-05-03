#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include "cpu_gestor.h"

#include "configuracion.h"
#include "cliente.h"
#include "servidor.h"
#include "pcb.h"

// **************** VARIABLES GLOBALES **********************
t_log* log_cpu;
cpu_config* config_cpu;

//int server_para_kernel;//SOCKET SERVIDOR CPU
int conexion_cpu_memoria; // SOCKET CLIENTE MEMORIA
int server_cpu;

pcb* proceso;
t_dictionary* instrucciones;
t_dictionary* registros;

#endif