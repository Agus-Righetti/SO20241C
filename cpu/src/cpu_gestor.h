#ifndef CPU_GESTOR_H_
#define CPU_GESTOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

// Estructuras globales ---------------------------------------------------------------------------------------------------------------

typedef struct 
{
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;
} cpu_config;

typedef struct {
    int pid;
    int numero_pagina;
    int numero_marco;
} TLB_Entrada;

// Para LRU, se necesita un campo adicional para mantener el orden de uso
typedef struct {
    int cantidad_entradas;
    TLB_Entrada* entradas;
    int* uso_lru; // Para LRU se puede usar un array para rastrear el uso
} TLB;


extern t_log* log_cpu;
extern cpu_config* config_cpu;

extern int socket_servidor_cpu;
extern int socket_cliente_kernel;
extern int socket_servidor_memoria;
extern int socket_cliente_cpu;
extern int socket_interrupt_kernel;

extern pcb* proceso;
extern t_dictionary* instrucciones;
extern t_dictionary* registros;
extern pcb** pcb_recibido;
extern TLB* tlb;
extern int cantidad_entradas_tlb;
extern int algoritmo_tlb;

#endif