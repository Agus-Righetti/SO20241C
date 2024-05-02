#include "pcb.h"

extern int server_para_kernel;
extern int conexion_cpu_memoria;

pcb* proceso;

void recibir_pcb(t_list *paquete, pcb *proceso)
{
    // void memcpy(destino, elemento que obtenemos del paquete, size);
    
    // Del paquete, obtengo el pid
    int i = 0;
	memcpy(&(proceso->pid), list_get(paquete, i++), sizeof(int));

    // Obtenemos las instrucciones
	int cantidad_instrucciones;
	memcpy(&(cantidad_instrucciones), list_get(paquete, i++), sizeof(int));

    // Si proceso->instruccion es distinto de NULL, destruye el paquete y sus elementos (del pcb) 
	if(proceso->instrucciones) 
    {
		list_destroy_and_destroy_elements(proceso->instrucciones, free); // Para asegurar que no haya fuga de memoria
	}

    // Obtenemos las intrucciones
	proceso->instrucciones = list_slice_and_remove(paquete, i, cantidad_instrucciones);

    // Obtenemos el program counter
	memcpy(&(proceso->program_counter), list_get(paquete, i++), sizeof(int));
	
    // Obtenemos los registros
    memcpy(proceso->registros.ax, list_get(paquete, i++), 8);
	memcpy(proceso->registros.bx, list_get(paquete, i++), 8);
	memcpy(proceso->registros.cx, list_get(paquete, i++), 8);
	memcpy(proceso->registros.dx, list_get(paquete, i++), 8);
	memcpy(proceso->registros.eax, list_get(paquete, i++), 32);
	memcpy(proceso->registros.ebx, list_get(paquete, i++), 32);
	memcpy(proceso->registros.ecx, list_get(paquete, i++), 32);
	memcpy(proceso->registros.edx, list_get(paquete, i++), 32);
	memcpy(proceso->registros.si, list_get(paquete, i++), 32);
	memcpy(proceso->registros.di, list_get(paquete, i++), 32);
}

void recibir_instruccion(t_list *paquete, t_instruccion *proceso)
{
    int i = 0;

    // Obtenemos el pid del paquete (a partir de una copia de memoria)
    memcpy(&(proceso->pid), list_get(paquete, i++), sizeof(int));

    // Obtenemos la instruccion, la sacamos del paquete y la asignamos a nuestra estructura
	proceso->instruccion = (char*)list_remove(paquete, i);
}

void interpretar_instrucciones(void)
{
    switch (expresion)
    {
    case SET:
        instruccion_set(registro, valor);
        break;
    case MOV_IN:
        instruccion_mov_in();
        break;
    case MOV_OUT:
        instruccion_mov_out();
        break;
    case SUM:
        instruccion_sum(destino, origen);
        break;
    case SUB:
        instruccion_sub(destino, origen);
        break;
    case JNZ:
        instruccion_jnz(registro, instruccion);
        break;
    case RESIZE:
        instruccion_resize();
        break;
    case COPY_STRING:
        instruccion_copy_string();
        break;
    case WAIT:
        instruccion_wait();
        break;
    case SIGNAL:
        instruccion_signal();
        break;
    case IO_GEN_SLEEP:
        instruccion_io_gen_sleep(interfaz, unidades_de_trabajo);
        break;
    case IO_STDIN_READ:
        instruccion_io_stdin_read();
        break;
    case IO_STDOUT_WRITE:
        instruccion_io_stdout_write();
        break;
    case IO_FS_CREATE:
        instruccion_io_fs_create();
        break;
    case IO_FS_DELETE:
        instruccion_io_fs_delete();
        break;
    case IO_FS_TRUNCATE:
        instruccion_io_fs_truncate();
        break;
    case IO_FS_WRITE:
        instruccion_io_fs_write();
        break;
    case IO_FS_READ:
        instruccion_io_fs_read();
        break;
    case EXIT:
        instruccion_exit();
        break;
    case -1:
		log_warning(logger, "PID: %d - Advertencia: No se pudo interpretar la instrucción - Ejecutando: EXIT", proceso->pid);
		error_exit(EXIT);
		return;
    }
    log_warning(logger, "PID: %d - Advertencia: Sin instrucciones por ejecutar - Ejecutando: EXIT", proceso->pid);
	error_exit(EXIT);
    return;
}

void instruccion_set(registros_cpu* registro, int valor)
{
    registro->ax = valor;
}

void instruccion_sum()
{

}

void instruccion_sub()
{

}

void instruccion_jnz()
{

}

void instruccion_io_gen_sleep()
{

}