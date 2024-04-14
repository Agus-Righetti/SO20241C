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
    char* retardo_respuesta;
    //Todo esta como char pero hay que cambuarlo al tipo q sea necesario
    // cuando identifiquemos en que lo vamos a usar
}memoria_config;

memoria_config* armar_config(t_log* log_memoria);


// ************* CONEXION A MEMORIA *************
// int conexion_a_memoria(t_log* log_cpu, cpu_config* config_cpu);

#endif