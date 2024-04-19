#ifndef GENERAL_H_
#define GENERAL_H_

#include "mainMemoria.h"

// ************* CONFIGURACION *************
typedef struct 
{
    char* puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char* path_instrucciones;
    int retardo_respuesta;
}memoria_config;

memoria_config* armar_config(t_log* log_memoria);

// ********* SERVER PARA RECIBIR A CPU *********
void server_para_cpu(memoria_config* config_memoria, t_log* log_memoria);

// ********* SERVER PARA RECIBIR A KERNEL *********
void server_para_kernel(memoria_config* config_memoria,t_log* log_memoria);

// ********* SERVER PARA RECIBIR A I/O *********
void server_para_io(memoria_config* config_memoria,t_log* log_memoria);


#endif