#ifndef GENERAL_H_
#define GENERAL_H_

#include "mainKernel.h"

// ***************** CREACIÓN DE STRUCT PARA LA CONFIGURACIÓN DEL KERNEL *******
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
int conexion_a_cpu(t_log* log_kernel, kernel_config* config_kernel);
int conexion_a_memoria(t_log* log_kernel, kernel_config* config_kernel);
void server_para_io(kernel_config* config_kernel, t_log* log_kernel);

#endif