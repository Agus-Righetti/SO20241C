#ifndef CPU_MAIN_H_
#define CPU_MAIN_H_

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
    //Todo esta como char pero hay que cambiarlo al tipo q sea necesario
    // cuando identifiquemos en que lo vamos a usar
}cpu_config;

//Arma un struct del tipo memoria_config con los datos del archivo config
cpu_config* armar_config(void);


#endif