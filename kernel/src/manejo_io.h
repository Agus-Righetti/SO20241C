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
    int registro_direccion;
    int registro_tamano;
    char* nombre_archivo;
    int registro_puntero_archivo;
    pcb* proceso;
    op_code operacion;
}argumentos_para_io;

void crear_interfaz(op_code interfaz_nueva, int socket, char* nombre_interfaz);
void escucha_interfaz(thread_args_escucha_io* args);
void desconectar_interfaz(interfaz_kernel* interfaz);
int io_gen_sleep(char* nombre_interfaz, int unidades_de_trabajo, pcb* proceso);
void enviar_instruccion_io(int socket, argumentos_para_io* args);
void* verificar_interfaz(char* nombre_interfaz_buscada, op_code tipo_interfaz_buscada);
int io_stdin_read(char* nombre_interfaz, uint32_t registro_direccion, uint32_t registro_tamano, pcb* proceso);
void envio_interfaz(thread_args_escucha_io* args);
int io_stdout_write(char* nombre_interfaz, uint32_t registro_direccion, uint32_t registro_tamano, pcb* proceso);






#endif