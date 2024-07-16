#ifndef CONEXION_H_
#define CONEXION_H_

#include "io_gestor.h"
#include "cliente.h"
#include "interfazGENERICA.h"
#include "interfazSTDIN.h"
#include "interfazSTDOUT.h"
#include "interfazDIALFS.h"
// #include "DIALFS.h"

pthread_t escuchar_memoria();
pthread_t escuchar_kernel();
void atender_a_memoria();
void atender_kernel();
// int solicitar_traduccion_direccion(int direccion_logica); 

#endif