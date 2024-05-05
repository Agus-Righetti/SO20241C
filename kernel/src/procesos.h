#ifndef PROCESOS_H_
#define PROCESOS_H_

#include "configuracion.h"
#include "cliente.h"
#include "k_gestor.h"

//  *************** DECLARACIÓN DE FUNCIONES **********
void leer_consola ();
pthread_t hilo_consola ();
void iniciar_proceso(char* path );
void enviar_proceso_a_cpu();
void enviar_pcb(pcb* proceso);
void recibir_pcb(pcb* proceso);
void desalojar_proceso(pcb* proceso);
void desalojar_proceso_hilo(void* arg);
void crear_hilo_proceso(pcb* proceso);
void algoritmo_round_robin (void* arg);
void accionar_segun_estado (pcb* proceso);
void recibir_pcb_hilo(void* arg);


//  *************** DECLARACIÓN DE ESTRUCTURAS ***************

typedef struct {
    pcb* proceso;
} thread_args_procesos_kernel; 

#endif