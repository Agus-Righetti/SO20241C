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
    int quantum;
    //VER POR LAS DUDAS
    char** recursos; //Este tiene que ser una lista ordenada de los nombres de recursos compartidos por el sistema
    char** instancias_recursos; //Lista ordenadad de cantidad de unidades por recurso
    int grado_multiprogramacion;
}kernel_config;

// *************** DECLARACIÓN DE FUNCIONES **********

// *************** CONFIGURACION.C **********
kernel_config* armar_config(t_log* log_kernel);

// *************** CLIENTE.C **********
int conexion_a_cpu(t_log* log_kernel, kernel_config* config_kernel);
int interrupcion_a_cpu(t_log* log_kernel, kernel_config* config_kernel);
int conexion_a_memoria(t_log* log_kernel, kernel_config* config_kernel);

// *************** SERVIDOR.C **********
void server_para_io(kernel_config* config_kernel, t_log* log_kernel);

#endif