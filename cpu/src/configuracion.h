#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

typedef struct 
{
    char* ip_memoria;
    char* puerto_memoria;
    char* puerto_escucha_dispatch;
    char* puerto_escucha_interrupt;
    int cantidad_entradas_tlb;
    char* algoritmo_tlb;

}cpu_config;

// ************* DECLARACION DE FUNCIONES *************
cpu_config* armar_config(t_log* log_cpu);

#endif