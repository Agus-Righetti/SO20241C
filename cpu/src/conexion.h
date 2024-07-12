#ifndef CONEXION_H_
#define CONEXION_H_

#include "cpu_gestor.h"
#include "servidor.h"
#include "pcb.h"

pthread_t escuchar_kernel();
pthread_t escuchar_memoria();
pthread_t escuchar_kernel_interrupcion();
void atender_kernel();
void atender_memoria();
void atender_interrupcion();



#endif