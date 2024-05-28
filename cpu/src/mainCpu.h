#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include "cpu_gestor.h"
#include "configuracion.h"
#include "cliente.h"
#include "servidor.h"
#include "pcb.h"
//#include "conexion.h"

t_log* log_cpu;
cpu_config* config_cpu;

int socket_servidor_memoria; 
int socket_servidor_cpu;
int socket_cliente_kernel;
int socket_cliente_cpu;
int socket_interrupt_kernel;

pcb* proceso;
t_dictionary* instrucciones;
t_dictionary* registros;
pcb* pcb_recibido; 

void terminar_programa();

#endif