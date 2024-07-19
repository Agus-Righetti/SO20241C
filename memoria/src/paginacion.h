#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "m_gestor.h"

// *************** DECLARACIÓN DE FUNCIONES **********
void ocupar_marco(int posicion);
void liberar_marco(int posicion);
t_pagina* crear_pagina(t_frame* un_frame);
void agregar_pag_a_tabla (t_proceso* proceso, t_pagina* pagina);
bool hay_lugar_en_memoria(int paginasNecesarias);
t_list* buscar_marcos_libres();
void eliminar_lista(t_list* lista);
void eliminar_algo(void* algo);

//******************************************************************
//********************** ESCRITURA EN PAG **************************
void leer_uint32_en_memoria (int pid, t_list* direcciones_fisicas);
void enviar_lectura_4B_a_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor);
void enviar_lectura_ult_4B_a_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor, uint32_t valor_leido_reconstruido);
void leer_uint8_en_memoria (int pid, t_list* direcciones_fisicas);
void enviar_lectura_1B_a_cpu(int pid, t_direccion_fisica* dir_actual, uint8_t valor);
void guardar_uint32_en_memoria (int pid, t_list* direcciones_fisicas, uint32_t valor);
void enviar_ok_4B_escritura_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor);
void enviar_ult_ok_4B_escritura_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor, uint32_t valor_completo);
void guardar_uint8_en_memoria (int pid, t_list* direcciones_fisicas, uint8_t valor);
void enviar_ok_1B_escritura_cpu(int pid, t_direccion_fisica* dir_actual, uint8_t valor);

void guardar_string_en_memoria (int pid, t_list* direcciones_fisicas, char* valor, int tamanio);
void enviar_ok_string_escritura_cpu(int pid, t_direccion_fisica* dir_actual, char* valor);
void enviar_ult_ok_string_escritura_cpu(int pid, t_direccion_fisica* dir_actual, char* valor, char* valor_completo);

void leer_string_en_memoria (int pid, t_list* direcciones_fisicas, int tamanio);
void enviar_lectura_string_a_cpu(int pid, t_direccion_fisica* dir_actual, char* valor);
void enviar_lectura_ult_string_a_cpu(int pid, t_direccion_fisica* dir_actual, char* valor, char* valor_leido_reconstruido);

void guardar_string_io_en_memoria (int pid, t_list* direcciones_fisicas, char* valor, int tamanio, int socket);
void enviar_ult_ok_string_escritura_io(int socket);

void leer_string_io_en_memoria(int pid, t_list* direcciones_fisicas, int tamanio, int socket);
void enviar_lectura_ult_string_a_io(char* valor_leido_reconstruido, int socket);

#endif