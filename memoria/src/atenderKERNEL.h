#ifndef ATENDERKERNEL_H_
#define ATENDERKERNEL_H_

#include "m_gestor.h"

// *************** DECLARACIÓN DE FUNCIONES **********
void iniciar_estructura_para_un_proceso_nuevo(t_buffer* buffer);
t_proceso* crear_proceso(int pid, char* path_instruc);
t_list* leer_archivo_y_cargar_instrucciones(char* archivo_pseudocodigo); 

#endif