#ifndef PCB_H_
#define PCB_H_

#include "cpu_gestor.h"

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

void recibir_pcb(t_buffer* buffer); 
void enviar_pcb(int conexion, pcb *proceso, op_code codigo);
void recibir_instruccion(t_list *paquete, t_instruccion *proceso);

void iniciar_diccionario_instrucciones(void);
void iniciar_diccionario_registros(registros_cpu* registro);
void destruir_diccionarios(void); 
void interpretar_instrucciones(void);

void instruccion_set(char **parte);
void instruccion_sum(char **parte);
void instruccion_sub(char **parte);
void instruccion_jnz(char **parte);
void instruccion_io_gen_sleep(char **parte);
void instruccion_exit(char** parsed); 
void instruccion_wait(char** parte);
void instruccion_signal(char **parte);

void error_exit(char** parte);
void solicitar_instrucciones_a_memoria(int socket_servidor_memoria); 
void recibir_instruccion_de_memoria(int socket_servidor_memoria);

#endif