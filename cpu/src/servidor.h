#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "cpu_gestor.h"

// Declaracion de funciones -----------------------------------------------------------------------------------------------------------

void server_para_kernel();
void interrupcion_para_kernel();
int esperar_cliente_de_kernel();

#endif