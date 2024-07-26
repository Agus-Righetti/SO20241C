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
}TLB_Entrada;

// Para LRU, se necesita un campo adicional para mantener el orden de uso
typedef struct {
    int cantidad_entradas_libres;
    TLB_Entrada* entradas;
    int* uso_lru; // Para LRU se puede usar un array para rastrear el uso
} TLB;

extern t_log* log_cpu;
extern cpu_config* config_cpu;
extern int tamanio_pagina;

extern char* instruccion_recibida;
extern sem_t sem_hay_instruccion;
extern sem_t sem_tengo_ok_resize;
extern sem_t sem_valor_leido_de_memoria;
extern sem_t sem_ok_escritura;
extern sem_t sem_string_memoria;
extern sem_t sem_ok_escritura_string;

//extern int socket_servidor_cpu;
extern int socket_servidor_dispatch;
extern int socket_servidor_interrupt;
extern int socket_cliente_kernel;
extern int socket_servidor_memoria;
extern int socket_cliente_cpu;
extern int socket_interrupt_kernel;
extern bool flag_interrupcion;          // flag para chequear interrupciones o pedir otra instruccion
extern int motivo_interrupcion;
extern sem_t sem_tengo_el_marco;
extern int marco;
extern uint8_t valor_leido_de_memoria_8;

extern uint32_t valor_leido_de_memoria_32;  
extern uint32_t valor_reconstruido;  

extern char* string_leido;
extern char* string_leido_reconstruido;

extern t_dictionary* instrucciones;
extern t_dictionary* registros;

extern pcb* pcb_recibido;

extern TLB* tlb;
extern int cantidad_entradas_tlb; // es el dato de config
extern char* algoritmo_tlb;

#endif