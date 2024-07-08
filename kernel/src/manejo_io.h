#ifndef MANEJO_IO_H_
#define MANEJO_IO_H_

#include "k_gestor.h"
#include "servidor.h"


typedef struct{
    interfaz_kernel* interfaz;
}thread_args_escucha_io;

void crear_interfaz(op_code interfaz_nueva, int socket, char* nombre_interfaz);
void escucha_interfaz(thread_args_escucha_io* args);
void desconectar_interfaz(interfaz_kernel* interfaz);





#endif