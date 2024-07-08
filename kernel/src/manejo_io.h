#ifndef MANEJO_IO_H_
#define MANEJO_IO_H_

#include "k_gestor.h"
#include "servidor.h"
#include "procesos.h"


typedef struct{
    interfaz_kernel* interfaz;
}thread_args_escucha_io;

typedef struct{
    int unidades_de_trabajo;
    registros_existentes registro_direccion;
    registros_existentes registro_tamano;
    char* nombre_archivo;
    registros_existentes registro_puntero_archivo;
}argumentos_para_io;

void crear_interfaz(op_code interfaz_nueva, int socket, char* nombre_interfaz);
void escucha_interfaz(thread_args_escucha_io* args);
void desconectar_interfaz(interfaz_kernel* interfaz);
int io_gen_sleep(char* nombre_interfaz, int unidades_de_trabajo, pcb* proceso);
void enviar_instruccion_io(int socket, op_code instruccion, argumentos_para_io args);
void* verificar_interfaz(char* nombre_interfaz_buscada, op_code tipo_interfaz_buscada);






#endif