#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_


#include <utils/hello.h>
#include <utils/utilsShare.h>
#include <utilsKernel.h>

//Creo struct para poder tener todos los atributos del kernel.config a mano
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
    
    //Todo esta como char pero hay que cambiarlo al tipo q sea necesario
    // cuando identifiquemos en que lo vamos a usar
    
}kernel_config;

// void iterator(char* value) 
// {
// 	log_info(log_kernel,"%s", value);
// }

// PUERTO_ESCUCHA=8003
// IP_MEMORIA=127.0.0.1
// PUERTO_MEMORIA=8002
// IP_CPU=127.0.0.1
// PUERTO_CPU_DISPATCH=8006
// PUERTO_CPU_INTERRUPT=8007
// ALGORITMO_PLANIFICACION=VRR
// QUANTUM=2000
// RECURSOS=[RA,RB,RC]
// INSTANCIAS_RECURSOS=[1,2,1]
// GRADO_MULTIPROGRAMACION=10

#endif