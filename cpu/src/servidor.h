#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "cpu_gestor.h"

int esperar_cliente_de_kernel();
void server_para_kernel();
void interrupcion_para_kernel();

#endif