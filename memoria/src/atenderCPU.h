#ifndef ATENDERCPU_H_
#define ATENDERCPU_H_

#include "m_gestor.h"

// *************** DECLARACIÓN DE FUNCIONES **********
t_list* leer_archivo_y_cargar_instrucciones(const char* archivo_pseudocodigo);

void cpu_pide_instruccion(t_list* instrucciones);
char* obtener_instruccion_por_indice(t_list* instrucciones, int indice_instruccion);
void enviar_una_instruccion_a_cpu(char* instruccion);

#endif