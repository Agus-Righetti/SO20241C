#ifndef GENERAL_H_
#define GENERAL_H_

#include "mainCpu.h"

// ************* CONFIGURACION *************
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
// ************* CONFIGURACION.C *************
cpu_config* armar_config(t_log* log_cpu);

// ************* CLIENTE.C *************
int conexion_a_memoria(t_log* log_cpu, cpu_config* config_cpu);

// ********* SERVER.C *********
void server_para_kernel(cpu_config* config_cpu,t_log* log_cpu);
void interrupcion_para_kernel(cpu_config* config_cpu,t_log* log_cpu);

#endif