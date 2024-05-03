#ifndef CPU_GESTOR_H_
#define CPU_GESTOR_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

// Estructuras globales ---------------------------------------
typedef struct 
{
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;
}cpu_config;

// ************* VARIABLES GLOBALES *************
extern t_log* log_cpu;
extern cpu_config* config_cpu;

extern int server_cpu;
extern int conexion_cpu_kernel;
extern int conexion_cpu_memoria;

extern pcb* proceso;
extern t_dictionary* instrucciones;
extern t_dictionary* registros;


#endif