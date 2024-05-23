#ifndef ATENDERCPU_H_
#define ATENDERCPU_H_

#include "m_gestor.h"

// *************** DECLARACIÓN DE FUNCIONES **********
void cpu_pide_instruccion(t_buffer* un_buffer);
t_proceso* obtener_proceso_por_id(int pid);
t_instruccion_codigo* obtener_instruccion_por_indice(t_list* instrucciones, int indice_instruccion);
void enviar_una_instruccion_a_cpu(t_instruccion_codigo* instruccion);

#endif