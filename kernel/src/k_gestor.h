#ifndef K_GESTOR_H
#define K_GESTOR_H

// ************* INCLUDES GENERALES *************
#include <utils/hello.h>
#include <utils/utilsShare.h>

// ************* ESTRUCTURAS GLOBALES *************
typedef struct
{
    char *puerto_escucha;
    char *ip_memoria;
    char *puerto_memoria;
    char *ip_cpu;
    char *puerto_cpu_dispatch;
    char *puerto_cpu_interrupt;
    char *algoritmo_planificacion;
    char *quantum;
    char *recursos;
    char *instancias_recursos;
    int grado_multiprogramacion;

} kernel_config;

// ************* VARIABLES GLOBALES *************
extern t_log *log_kernel;
extern kernel_config *config_kernel;
extern t_queue *cola_de_new;
extern t_queue *cola_de_ready;
extern t_queue *cola_de_execute;
extern t_queue *cola_de_exit;
extern t_queue *cola_de_blocked;
extern t_queue *cola_prioridad_vrr;
extern t_queue **colas_por_recurso;
extern t_queue *cola_general_de_procesos;
extern t_queue *cola_interfaces_conectadas;
extern int pid_contador;
extern int conexion_kernel_cpu;
extern int interrupcion_kernel_cpu;
extern int conexion_kernel_memoria;
extern int cantidad_recursos;
extern int planificacion_activa;
extern int pid_eliminar;
extern bool interrupcion_por_fin_de_proceso;
extern pthread_mutex_t **mutex_por_recurso;
extern pthread_mutex_t mutex_cola_de_ready;
extern pthread_mutex_t mutex_cola_de_new;
extern pthread_mutex_t mutex_grado_programacion;
extern pthread_mutex_t mutex_cola_de_exit;
extern pthread_mutex_t mutex_cola_de_blocked;
extern pthread_mutex_t mutex_cola_de_execute;
extern pthread_mutex_t mutex_cola_prioridad_vrr;
extern pthread_mutex_t mutex_cola_general_de_procesos;
extern pthread_mutex_t mutex_planificacion_activa;
extern pthread_mutex_t mutex_enviando_instruccion_a_io;
extern pthread_mutex_t mutex_cola_de_interfaces;

extern sem_t sem_cola_de_ready;
extern sem_t sem_cola_de_new;
extern sem_t sem_multiprogramacion;
extern sem_t destruir_hilo_interrupcion;
extern sem_t sem_puedo_mandar_a_cpu;
extern sem_t sem_cola_prioridad_vrr;
extern sem_t sem_planificacion_activa;

extern t_queue **colas_por_recurso;
extern pthread_mutex_t **mutex_por_recurso;

#endif