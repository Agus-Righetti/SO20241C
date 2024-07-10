#ifndef CLIENTE_H_
#define CLIENTE_H_

#include "io_gestor.h"

// Declaracion de funciones ----------------------------------------------------------------------------------------------------------

int conexion_a_memoria();
int conexion_a_kernel();

op_code obtener_tipo_interfaz(char* char_interfaz);
void avisar_fin_io_a_kernel();

#endif