#ifndef CONFIGURACION_H_
#define CONFIGURACION_H_

#include <utils/hello.h>
#include <utils/utilsShare.h>

// ************* CONFIGURACION *************
typedef struct 
{
    char* puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char* path_instrucciones;
    int retardo_respuesta;
}memoria_config;

// *************** DECLARACIÓN DE FUNCIONES **********
memoria_config* armar_config(t_log* log_memoria);

#endif