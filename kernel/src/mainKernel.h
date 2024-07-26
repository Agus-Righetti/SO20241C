#ifndef KERNEL_MAIN_H_
#define KERNEL_MAIN_H_

#include "k_gestor.h"

#include "configuracion.h"
#include "cliente.h"
#include "servidor.h"
#include "procesos.h"

// ************ VARIABLES GLOBALES **************
t_log* log_kernel;
kernel_config* config_kernel;
t_queue* cola_de_new;
t_queue* cola_de_ready;
t_queue* cola_de_execute;
t_queue* cola_de_exit;
t_queue* cola_de_blocked;
t_queue* cola_prioridad_vrr;
t_queue* cola_general_de_procesos; // Esta cola tendra a todos los procesos del sistema
t_queue* cola_interfaces_conectadas; //tendra las interfaces conectadas actualmente a kernel
// t_queue** colas_por_recurso = malloc(cantidad_recursos * sizeof(t_queue*)); -- Esto lo tuve que poner en .c porque sino rompía todo.
t_queue* cola_de_ready_prioridad;
int pid_contador = 0;
int conexion_kernel_cpu;
int interrupcion_kernel_cpu;
int conexion_kernel_memoria;
int cantidad_recursos;
int planificacion_activa = 1;
int pid_eliminar = -1;
bool interrupcion_por_fin_de_proceso;
pthread_mutex_t mutex_cola_de_ready;
pthread_mutex_t mutex_cola_de_new;
pthread_mutex_t mutex_cola_de_exit;
pthread_mutex_t mutex_cola_de_blocked;
pthread_mutex_t mutex_cola_de_execute;
pthread_mutex_t mutex_grado_programacion;
pthread_mutex_t mutex_cola_prioridad_vrr;
pthread_mutex_t mutex_cola_general_de_procesos;
pthread_mutex_t mutex_planificacion_activa;
pthread_mutex_t mutex_enviando_instruccion_a_io;
pthread_mutex_t mutex_cola_de_interfaces;

sem_t sem_cola_de_ready;
sem_t sem_cola_de_new;
sem_t sem_multiprogramacion;
sem_t destruir_hilo_interrupcion;
sem_t sem_puedo_mandar_a_cpu;
sem_t sem_cola_prioridad_vrr;
sem_t sem_planificacion_activa;
sem_t sem_hay_algo_en_ready;

sem_t sem_puertos;

t_queue** colas_por_recurso;
pthread_mutex_t** mutex_por_recurso;

t_temporal* tiempo_de_quantum;



#endif