#ifndef DIRECCION_H_
#define DIRECCION_H_

#include "cpu_gestor.h"

int recibir_marco(int socket_cliente, int* marco);
TLB_Entrada* buscar(int numero_pagina);
void actualizar_tlb(TLB_Entrada* nueva_entrada);
void inicializar_tlb();
void actualizar_tlb_HIT(int pid, int numero_pagina);
void actualizar_tlb_libre(int pid, int numero_pagina);

// void manejar_solicitud_traduccion(int socket_cliente_kernel);
void enviar_direccion_fisica_a_kernel(int socket_cliente_kernel, int direccion_fisica);

#endif