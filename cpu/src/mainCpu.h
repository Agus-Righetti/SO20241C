#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include "cpu_gestor.h"
#include "configuracion.h"
#include "cliente.h"
#include "servidor.h"
#include "pcb.h"

t_log* log_cpu;
cpu_config* config_cpu;

int conexion_cpu_kernel; // Socket servidor-cpu
int conexion_cpu_memoria; // Socket cliente-memoria
int server_cpu;

pcb* proceso;
t_dictionary* instrucciones;
t_dictionary* registros;

#endif