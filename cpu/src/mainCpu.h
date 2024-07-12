#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

#include "cpu_gestor.h"
#include "configuracion.h"
#include "cliente.h"
#include "servidor.h"
#include "pcb.h"
#include "conexion.h"
#include "direccion.h"

t_log* log_cpu;
cpu_config* config_cpu;
int tamanio_pagina;

int socket_servidor_memoria; 
int socket_servidor_cpu;
int socket_cliente_kernel;
int socket_cliente_cpu;
int socket_interrupt_kernel;
bool flag_interrupcion;
int motivo_interrupcion;
sem_t sem_tengo_el_marco;

t_dictionary* instrucciones;
t_dictionary* registros;
int marco;

//pcb* proceso = NULL;
pcb* pcb_recibido ; // &proceso; // Asignación de la dirección de proceso a pcb_recibido

TLB* tlb; 
int cantidad_entradas_tlb;
int algoritmo_tlb;

void terminar_programa();

#endif