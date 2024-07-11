#ifndef EXTRA_H_
#define EXTRA_H_

#include "cpu_gestor.h"
#include "direccion.h"

// ****************** DECLARACIÓN DE FUNCIONES ************************** 
t_list* traducir_dl_a_df_completa(int direccion_logica, int bytes_a_operar) ;
int minimo(int a, int b) ;
t_direccion_fisica* traducir_una_dl_a_df(int numero_pagina, int desplazamiento, int bytes_operar_pag);
bool es_Registro_de_1B(const char* registro) ;

void peticion_lectura_a_memoria(op_code code_op, int pid, t_list* direcciones_fisicas);
uint8_t espero_rta_lectura_1B_de_memoria();
uint32_t espero_rta_lectura_4B_de_memoria();

void espero_rta_escritura_1B_de_memoria();
void espero_rta_escritura_4B_de_memoria();
void peticion_escritura_1B_a_memoria(int pid, t_list* direcciones_fisicas, uint8_t valor_por_escribir);
void peticion_escritura_4B_a_memoria(int pid, t_list* direcciones_fisicas, uint32_t valor_por_escribir);


#endif