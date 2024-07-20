#ifndef ATENDERCPU_IO_H_
#define ATENDERCPU_IO_H_

#include "m_gestor.h"
#include "paginacion.h"

// *************** DECLARACIÓN DE FUNCIONES **********
// **************** PETICIÓN INSTRUCCIÓN *************
void cpu_pide_instruccion(t_buffer* un_buffer);
t_proceso* obtener_proceso_por_id(int pid);
bool comparar_pid(int pid, t_proceso* proceso);
char* obtener_instruccion_por_indice(t_list* instrucciones, uint32_t indice_instruccion);
void enviar_una_instruccion_a_cpu(char* instruccion);

// **************** ACCESO ESPACIO USUARIO ***********
// ****************** LECTURA DE MEMORIA *************
void cpu_pide_leer_1B(int pid, t_list* direcciones_fisicas);
void cpu_pide_leer_4B(t_buffer* un_buffer);
void cpu_pide_leer_string(t_buffer* un_buffer);
void io_pide_lectura(int socket, int pid, int tamanio, t_list* direcciones_fisica);

// ***************** ESCRITURA DE MEMORIA ************
void cpu_pide_guardar_1B(t_buffer* un_buffer);
void cpu_pide_guardar_4B(t_buffer* un_buffer);
void cpu_pide_guardar_string(t_buffer* un_buffer);
void io_pide_escritura( int socket, int pid, int tamanio, char* valor, t_list* direcciones_fisicas);

// **************** ACCESO A TABLA DE PAG ************
void cpu_pide_numero_de_marco(t_buffer* un_buffer);
int obtener_marco_segun_pagina (int pid, int nro_pag);
void enviar_marco_consultado_a_cpu(int marco_consultado);

// ******************* RESIZE ************************
void cpu_pide_resize(t_buffer* un_buffer);
void enviar_out_of_memory_a_cpu();
void enviar_ok_del_resize_a_cpu();

#endif