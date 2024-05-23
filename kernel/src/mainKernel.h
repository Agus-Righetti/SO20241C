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
int pid_contador = 0;
int conexion_kernel_cpu;
int interrupcion_kernel_cpu;
int conexion_kernel_memoria;
pthread_mutex_t mutex_cola_de_ready;
pthread_mutex_t mutex_cola_de_new;
pthread_mutex_t mutex_grado_programacion;
sem_t sem_cola_de_ready;
sem_t sem_cola_de_new;
sem_t sem_multiprogramacion;

#endif