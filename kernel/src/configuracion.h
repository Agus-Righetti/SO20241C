#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

typedef struct 
{
    char* puerto_escucha;
    char* ip_memoria;
    char* puerto_memoria;
    char* ip_cpu;
    char* puerto_cpu_dispatch;
    char* puerto_cpu_interrupt;
    char* algoritmo_planificacion;
    char* quantum;
    char* recursos;
    char* instancias_recursos;
    int grado_multiprogramacion;
    
}kernel_config;

// *************** DECLARACIÓN DE FUNCIONES **********
kernel_config* armar_config(t_log* log_kernel);

#endif