#ifndef PROCESOS_H_
#define PROCESOS_H_

#include "configuracion.h"
#include "cliente.h"
#include "k_gestor.h"
#include "manejo_io.h"

//  *************** DECLARACIÓN DE FUNCIONES **********
void leer_consola ();
pthread_t hilo_consola ();
pthread_t hilo_enviar_procesos_cpu ();
pthread_t hilo_pasar_de_new_a_ready();
pthread_t hilo_io();
void iniciar_proceso(char* path );
void enviar_proceso_a_cpu();
void enviar_pcb(pcb* proceso);
void recibir_pcb(pcb* proceso);
void desalojar_proceso(pcb* proceso, int operacion);
void desalojar_proceso_hilo(void* arg, int operacion);
void crear_hilo_proceso(pcb* proceso);
void algoritmo_round_robin (void* arg);
void accionar_segun_estado(pcb* proceso, int flag, int motivo);
void recibir_pcb_hilo(void* arg);
void pasar_proceso_a_exit(pcb* proceso, int motivo);
void pasar_proceso_a_blocked(pcb* proceso);
void pasar_procesos_de_new_a_ready();
int hacer_signal(char* recurso, pcb* proceso);
int hacer_wait(char* recurso, pcb* proceso);
void listar_procesos_por_estado();
char* obtener_char_de_estado(estados estado_a_convertir);
void cambiar_grado_multiprogramacion(char* nuevo_valor_formato_char);
void finalizar_proceso(char* pid_formato_char);
void ejecutar_script(char* script_path);
void liberar_array_strings(char **array);
void sacar_de_cola_de_ready(int pid);
void sacar_de_cola_de_new(int pid);
void sacar_de_execute(int pid);
void sacar_de_blocked(int pid);
int buscar_indice_recurso_segun_nombre(char* recurso);
char* pasar_a_string(int valor);
char *agregar_al_final(char *buffer, const char *informacion);
void hacer_el_log_obligatorio_de_ingreso_a_ready(pcb* proceso_a_mandar_a_ready);
void hacer_el_log_obligatorio_de_ingreso_a_ready_prioridad(pcb* proceso_a_mandar_a_ready);
void detener_planificacion();
void iniciar_planificacion();
void machear_con_cola_gral(pcb* proceso);
void actualizar_pcb(pcb* proceso_original, pcb* nuevo);


//  *************** DECLARACIÓN DE ESTRUCTURAS ***************

typedef struct {
    pcb* proceso;
} thread_args_procesos_kernel; 

#endif