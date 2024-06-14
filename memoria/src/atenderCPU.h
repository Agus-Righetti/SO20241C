#ifndef ATENDERCPU_H_
#define ATENDERCPU_H_

#include "m_gestor.h"
#include "paginacion.h"

// *************** DECLARACIÓN DE FUNCIONES **********
// **************** PETICIÓN INSTRUCCIÓN *************
void cpu_pide_instruccion(t_buffer* un_buffer);
t_proceso* obtener_proceso_por_id(int pid);
char* obtener_instruccion_por_indice(t_list* instrucciones, int indice_instruccion);
void enviar_una_instruccion_a_cpu(char* instruccion);

// **************** ACCESO ESPACIO USUARIO ***********
// ****************** LECTURA DE MEMORIA *************
// ***************** ESCRITURA DE MEMORIA ************
// FALTA
// void* dato;
// // Asignacion de valor a dato
// memcpy(memoria_general+DF, dato, tamanio);

// **************** ACCESO A TABLA DE PAG ************
void cpu_pide_numero_de_marco(t_buffer* un_buffer);
int obtener_marco_segun_pagina (int pid, int nro_pag);
void enviar_marco_consultado_a_cpu(int marco_consultado);

// ******************* RESIZE ************************
void cpu_pide_resize(t_buffer* un_buffer);
void enviar_out_of_memory_a_cpu();
void enviar_ok_del_resize_a_cpu();

#endif