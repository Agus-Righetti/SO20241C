#ifndef GENERAL_H_
#define GENERAL_H_

#include "mainCpu.h"

typedef struct 
{
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;

    // Todo esta como char pero hay que cambiarlo al tipo q sea necesario
    // cuando identifiquemos en que lo vamos a usar

}cpu_config;

cpu_config* armar_config(t_log* log_cpu);

int conexion_a_memoria(t_log* log_cpu, cpu_config* config_cpu);

#endif