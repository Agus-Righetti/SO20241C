#ifndef K_GESTOR_H
#define K_GESTOR_H

// ************* INCLUDES GENERALES *************
#include <utils/hello.h>
#include <utils/utilsShare.h>

// ************* ESTRUCTURAS GLOBALES *************
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

// ************* VARIABLES GLOBALES *************
extern t_log* log_kernel;
extern kernel_config* config_kernel;
extern t_queue* cola_de_new;
extern t_queue* cola_de_ready;
extern t_queue* cola_de_execute;
extern t_queue* cola_de_exit;
extern t_queue* cola_de_blocked;
extern t_queue* cola_prioridad_vrr;
extern int pid_contador; 
extern int conexion_kernel_cpu;
extern int interrupcion_kernel_cpu;
extern int conexion_kernel_memoria;
extern pthread_mutex_t mutex_cola_de_ready;
extern pthread_mutex_t mutex_cola_de_new;
extern pthread_mutex_t mutex_grado_programacion;
extern pthread_mutex_t mutex_cola_de_exit;
extern pthread_mutex_t mutex_cola_de_blocked;
extern pthread_mutex_t mutex_cola_de_execute;
extern pthread_mutex_t mutex_cola_prioridad_vrr;
extern sem_t sem_cola_de_ready;
extern sem_t sem_cola_de_new;
extern sem_t sem_multiprogramacion;
extern sem_t destruir_hilo_interrupcion;
extern sem_t sem_puedo_mandar_a_cpu;
extern sem_t sem_cola_prioridad_vrr;


#endif