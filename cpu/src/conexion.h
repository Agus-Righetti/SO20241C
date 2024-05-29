#ifndef CONEXION_H_
#define CONEXION_H_

#include "cpu_gestor.h"
#include "servidor.h"
#include "pcb.h"

void escuchar_kernel();
void escuchar_memoria();
void atender_kernel();
void atender_memoria();
void atender_interrupcion();

#endif