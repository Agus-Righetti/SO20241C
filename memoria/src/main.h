#ifndef MEMORIA_MAIN_H_
#define MEMORIA_MAIN_H_

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

//Arma un struct del tipo memoria_config con los datos del archivo config
memoria_config* armar_config (void);


#endif