#include "pcb.h"

// Creemos que deberia ir en utilsShare.c
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
    memcpy(proceso->registros->ax, list_get(paquete, i++), 8);
	memcpy(proceso->registros->bx, list_get(paquete, i++), 8);
	memcpy(proceso->registros->cx, list_get(paquete, i++), 8);
	memcpy(proceso->registros->dx, list_get(paquete, i++), 8);
	memcpy(proceso->registros->eax, list_get(paquete, i++), 32);
	memcpy(proceso->registros->ebx, list_get(paquete, i++), 32);
	memcpy(proceso->registros->ecx, list_get(paquete, i++), 32);
	memcpy(proceso->registros->edx, list_get(paquete, i++), 32);
	memcpy(proceso->registros->si, list_get(paquete, i++), 32);
	memcpy(proceso->registros->di, list_get(paquete, i++), 32);
}

// Creemos que deberia ir en utilsShare.c
void recibir_instruccion(t_list *paquete, t_instruccion *proceso)
{
    int i = 0;

    // Obtenemos el pid del paquete (a partir de una copia de memoria)
    memcpy(&(proceso->pid), list_get(paquete, i++), sizeof(int));

    // Obtenemos la instruccion, la sacamos del paquete y la asignamos a nuestra estructura
	proceso->instruccion = (char*)list_remove(paquete, i);
}

void iniciar_diccionario_instrucciones(void)
{
	instrucciones = dictionary_create(); // Creo el diccionario

    // dictionary_put(diccionario al que se agrega el par clave-valor, clave para acceder al valor en el diccionario, valor que se va a asociar con la clave en el diccionario);
	dictionary_put(instrucciones, "SET", (void*)(intptr_t)I_SET);
	dictionary_put(instrucciones, "MOV_IN", (void*)(intptr_t)I_MOV_IN);
	dictionary_put(instrucciones, "MOV_OUT", (void*)(intptr_t)I_MOV_OUT);
	dictionary_put(instrucciones, "SUM", (void*)(intptr_t)I_SUM);
	dictionary_put(instrucciones, "SUB", (void*)(intptr_t)I_SUB);
	dictionary_put(instrucciones, "JNZ", (void*)(intptr_t)I_JNZ);
	dictionary_put(instrucciones, "RESIZE", (void*)(intptr_t)I_RESIZE);
	dictionary_put(instrucciones, "COPY_STRING", (void*)(intptr_t)I_COPY_STRING);
    dictionary_put(instrucciones, "WAIT", (void*)(intptr_t)I_WAIT);
    dictionary_put(instrucciones, "COPY_STRING", (void*)(intptr_t)I_COPY_STRING);
	dictionary_put(instrucciones, "WAIT", (void*)(intptr_t)I_WAIT);
	dictionary_put(instrucciones, "SIGNAL", (void*)(intptr_t)I_SIGNAL);
	dictionary_put(instrucciones, "IO_GEN_SLEEP", (void*)(intptr_t)I_IO_GEN_SLEEP);
	dictionary_put(instrucciones, "IO_STDIN_READ", (void*)(intptr_t)I_IO_STDIN_READ);
	dictionary_put(instrucciones, "IO_STDOUT_WRITE", (void*)(intptr_t)I_IO_STDOUT_WRITE);
    dictionary_put(instrucciones, "IO_FS_CREATE,", (void*)(intptr_t)I_IO_FS_CREATE);
    dictionary_put(instrucciones, "IO_FS_DELETE,", (void*)(intptr_t)I_IO_FS_DELETE);
    dictionary_put(instrucciones, "IO_FS_TRUNCATE,", (void*)(intptr_t)I_IO_FS_TRUNCATE);
    dictionary_put(instrucciones, "IO_FS_WRITE,", (void*)(intptr_t)I_IO_FS_WRITE);
    dictionary_put(instrucciones, "IO_FS_READ,", (void*)(intptr_t)I_IO_FS_READ);
    dictionary_put(instrucciones, "EXIT,", (void*)(intptr_t)I_EXIT);
}

void iniciar_diccionario_registros(registros_cpu* registro)
{
	registros = dictionary_create();
	dictionary_put(registros, "PC", (void*)registro->pc);
	dictionary_put(registros, "AX", (void*)registro->ax);
	dictionary_put(registros, "BX", (void*)registro->bx);
	dictionary_put(registros, "CX", (void*)registro->cx);
	dictionary_put(registros, "DX", (void*)registro->dx);
	dictionary_put(registros, "EAX", (void*)registro->eax);
	dictionary_put(registros, "EBX", (void*)registro->ebx);
	dictionary_put(registros, "ECX", (void*)registro->ecx);
	dictionary_put(registros, "EDX", (void*)registro->edx);
	dictionary_put(registros, "SI", (void*)registro->si);
	dictionary_put(registros, "DI", (void*)registro->di);
}

void destruir_diccionarios(void) 
{
	dictionary_destroy(instrucciones);
	dictionary_destroy(registros);
}

void interpretar_instrucciones(void)
{   int expresion = recibir_operacion(conexion_cpu_memoria);
    switch (expresion) {
    // case I_SET:
    //     instruccion_set(registro, valor);
    //     break;
    // case I_I_MOV_IN:
    //     instruccion_mov_in();
    //     break;
    // case I_MOV_OUT:
    //     instruccion_mov_out();
    //     break;
    // case I_SUM:
    //     instruccion_sum(destino, origen);
    //     break;
    // case I_SUB:
    //     instruccion_sub(destino, origen);
    //     break;
    // case I_JNZ:
    //     instruccion_jnz(registro, instruccion);
    //     break;
    // case I_RESIZE:
    //     instruccion_resize();
    //     break;
    // case I_COPY_STRING:
    //     instruccion_copy_string();
    //     break;
    // case I_WAIT:
    //     instruccion_wait();
    //     break;
    // case I_SIGNAL:
    //     instruccion_signal();
    //     break;
    // case I_IO_GEN_SLEEP:
    //     instruccion_io_gen_sleep(interfaz, unidades_de_trabajo);
    //     break;
    // case I_IO_STDIN_READ:
    //     instruccion_io_stdin_read();
    //     break;
    // case I_IO_STDOUT_WRITE:
    //     instruccion_io_stdout_write();
    //     break;
    // case I_IO_FS_CREATE:
    //     instruccion_io_fs_create();
    //     break;
    // case I_IO_FS_DELETE:
    //     instruccion_io_fs_delete();
    //     break;
    // case I_IO_FS_TRUNCATE:
    //     instruccion_io_fs_truncate();
    //     break;
    // case I_IO_FS_WRITE:
    //     instruccion_io_fs_write();
    //     break;
    // case I_IO_FS_READ:
    //     instruccion_io_fs_read();
    //     break;
    // case I_EXIT:
    //     instruccion_exit();
    //     break;
    case -1:
		log_warning(log_cpu, "PID: %d - Advertencia: No se pudo interpretar la instrucción - Ejecutando: EXIT", proceso->pid);
		//error_exit(EXIT);
		return;
    }
    log_warning(log_cpu, "PID: %d - Advertencia: Sin instrucciones por ejecutar - Ejecutando: EXIT", proceso->pid);
	//error_exit(EXIT);
    return;
}

void instruccion_set(registros_cpu* registro, int valor)
{
    
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