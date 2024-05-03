#ifndef PCB_H_
#define PCB_H_

#include "cpu_gestor.h"

void iniciar_diccionario_instrucciones(void);
void iniciar_diccionario_registros(registros_cpu* registro);
void destruir_diccionarios(void);

typedef enum
{
	I_SET,
	I_MOV_IN,
	I_MOV_OUT,
    I_SUM,
    I_SUB,
    I_JNZ,
    I_RESIZE,
    I_COPY_STRING,
    I_WAIT,
    I_SIGNAL,
	I_IO_GEN_SLEEP,
    I_IO_STDIN_READ,
    I_IO_STDOUT_WRITE,
    I_IO_FS_CREATE,
    I_IO_FS_DELETE,
    I_IO_FS_TRUNCATE,
    I_IO_FS_WRITE,
    I_IO_FS_READ,
	I_EXIT
}enum_instrucciones;

// Ver que mas le falta a la estructura de instrucciones 
typedef struct 
{
    int pid;
    char* instruccion;
    int tamanio;
} t_instruccion;

void recibir_pcb(t_list *lista, pcb *proceso); // Creemos que deberia ir el el utilsShare.h
void recibir_instruccion(t_list *paquete, t_instruccion *proceso); // Creemos que deberia ir el el utilsShare.h

#endif

