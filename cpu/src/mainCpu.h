#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include "cpu_gestor.h"
#include "configuracion.h"
#include "cliente.h"
#include "servidor.h"
#include "pcb.h"

t_log* log_cpu;
cpu_config* config_cpu;

int conexion_cpu_kernel; 
int socket_servidor_memoria; 
int socket_servidor_cpu;
int socket_cliente_kernel;
int socket_cliente_cpu;

pcb* proceso;
t_dictionary* instrucciones;
t_dictionary* registros;

void terminar_programa(t_log* log_cpu, t_config* config_cpu);

#endif