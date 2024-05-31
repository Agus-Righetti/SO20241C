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
}cpu_config;

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

typedef struct 
{
    uint32_t pid;
    uint32_t pagina;
    uint32_t marco;
}tlb;

#endif