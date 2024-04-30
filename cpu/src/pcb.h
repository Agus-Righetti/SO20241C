#ifndef PCB_H_
#define PCB_H_

#include "utils/utilsShare.h"

void recibir_pcb(t_list *lista, pcb *proceso); // Tendria que ir en el utils?
void interpretar_instrucciones(void);

typedef enum
{
	SET,
	MOV_IN,
	MOV_OUT,
    SUM,
    SUB,
    JNZ,
    RESIZE,
    COPY_STRING,
    WAIT,
    SIGNAL,
	IO_GEN_SLEEP,
    IO_STDIN_READ,
    IO_STDOUT_WRITE,
    IO_FS_CREATE,
    IO_FS_DELETE,
    IO_FS_TRUNCATE,
    IO_FS_WRITE,
    IO_FS_READ,
	EXIT
}enum_instrucciones;

#endif

