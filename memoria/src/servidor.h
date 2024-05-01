#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "m_gestor.h"

// *************** DECLARACIÓN DE FUNCIONES **********
// ********* INICIO MEMORIA COMO SERVIDOR *********
void inicializar_servidor();

// ********* ESPERO QUE CPU SE CONECTE COMO CLIENTE *********
void server_para_cpu();

// ********* ESPERO QUE KERNEL SE CONECTE COMO CLIENTE *********
void server_para_kernel();

// ********* ESPERO QUE IO SE CONECTE COMO CLIENTE *********
void server_para_io();

#endif