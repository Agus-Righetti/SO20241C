#ifndef ATENDERKERNEL_H_
#define ATENDERKERNEL_H_

#include "m_gestor.h"
#include "atenderCPU-IO.h"

// *************** DECLARACIÓN DE FUNCIONES **********
//**************** INICIAR_PROCESO *******************
void iniciar_estructura_para_un_proceso_nuevo(t_buffer* buffer);
void crear_proceso(int pid, char* path_instruc);
t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo); 

//**************** FINALIZAR_PROCESO *******************
void liberar_memoria_proceso(t_buffer* buffer);

#endif