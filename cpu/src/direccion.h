#ifndef DIRECCION_H_
#define DIRECCION_H_

#include "cpu_gestor.h"

int traducir_direccion_logica_a_fisica(int direccion_logica);
int recibir_marco(int socket_cliente, int* marco);
TLB_Entrada buscar(int numero_pagina);
void actualizar_tlb(TLB_Entrada* nueva_entrada);
void inicializar_tlb();
// void manejar_solicitud_traduccion(int socket_cliente_kernel);
void enviar_direccion_fisica_a_kernel(int socket_cliente_kernel, int direccion_fisica);

#endif