#ifndef PROCESOS_H_
#define PROCESOS_H_

#include "configuracion.h"
#include "cliente.h"
#include "k_gestor.h"

//  *************** DECLARACIÓN DE FUNCIONES **********
void leer_consola ();
pthread_t hilo_consola ();
pthread_t hilo_enviar_procesos_cpu ();
pthread_t hilo_pasar_de_new_a_ready();
void iniciar_proceso(char* path );
void enviar_proceso_a_cpu();
void enviar_pcb(pcb* proceso);
void recibir_pcb(pcb* proceso);
void desalojar_proceso(pcb* proceso);
void desalojar_proceso_hilo(void* arg);
void crear_hilo_proceso(pcb* proceso);
void algoritmo_round_robin (void* arg);
void accionar_segun_estado(pcb* proceso, int flag);
void recibir_pcb_hilo(void* arg);
void pasar_proceso_a_exit(pcb* proceso);
void pasar_procesos_de_new_a_ready();
int hacer_signal(int indice_recurso, pcb* proceso);
int hacer_wait(int indice_recurso);



//  *************** DECLARACIÓN DE ESTRUCTURAS ***************

typedef struct {
    pcb* proceso;
} thread_args_procesos_kernel; 

#endif