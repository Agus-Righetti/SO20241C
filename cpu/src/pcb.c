#include "pcb.h"

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

void enviar_pcb(int conexion, pcb *proceso, op_code codigo)
{
    // Creamos un paquete
	t_paquete *paquete = crear_paquete(codigo);

    // Agregamos el pid al paquete
	agregar_a_paquete(paquete, &(proceso->pid), sizeof(unsigned int));

	int cantidad_instrucciones = list_size(proceso->instrucciones);
	agregar_a_paquete(paquete, &cantidad_instrucciones, sizeof(int));

    // Agregamos las instrucciones al paquete
	for (int i = 0; i < cantidad_instrucciones; i++)
	{
		char *instruccion = list_get(proceso->instrucciones, i);
		agregar_a_paquete(paquete, instruccion, strlen(instruccion) + 1);
	}

    // Agregamos el program counter al paquete
	agregar_a_paquete(paquete, &(proceso->program_counter), sizeof(int));

    // Agregamos los registros al paquete
	agregar_a_paquete(paquete, proceso->registros->ax, 8);
	agregar_a_paquete(paquete, proceso->registros->bx, 8);
	agregar_a_paquete(paquete, proceso->registros->cx, 8);
	agregar_a_paquete(paquete, proceso->registros->dx, 8);
	agregar_a_paquete(paquete, proceso->registros->eax, 32);
	agregar_a_paquete(paquete, proceso->registros->ebx, 32);
	agregar_a_paquete(paquete, proceso->registros->ecx, 32);
	agregar_a_paquete(paquete, proceso->registros->edx, 32);
	agregar_a_paquete(paquete, proceso->registros->si, 32);
   	agregar_a_paquete(paquete, proceso->registros->di, 32);

    enviar_paquete(paquete, conexion);
    
	eliminar_paquete(paquete);
}

// void recibir_instruccion(t_list *paquete, t_instruccion *proceso)
// {
//     int i = 0;

//     // Obtenemos el pid del paquete (a partir de una copia de memoria)
//     memcpy(&(proceso->pid), list_get(paquete, i++), sizeof(int));

//     // Obtenemos la instruccion, la sacamos del paquete y la asignamos a nuestra estructura
// 	proceso->instruccion = (char*)list_remove(paquete, i);
// }

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
	dictionary_put(instrucciones, "SIGNAL", (void*)(intptr_t)I_SIGNAL);
	dictionary_put(instrucciones, "IO_GEN_SLEEP", (void*)(intptr_t)I_IO_GEN_SLEEP);
	dictionary_put(instrucciones, "IO_STDIN_READ", (void*)(intptr_t)I_IO_STDIN_READ);
	dictionary_put(instrucciones, "IO_STDOUT_WRITE", (void*)(intptr_t)I_IO_STDOUT_WRITE);
    dictionary_put(instrucciones, "IO_FS_CREATE", (void*)(intptr_t)I_IO_FS_CREATE);
    dictionary_put(instrucciones, "IO_FS_DELETE", (void*)(intptr_t)I_IO_FS_DELETE);
    dictionary_put(instrucciones, "IO_FS_TRUNCATE", (void*)(intptr_t)I_IO_FS_TRUNCATE);
    dictionary_put(instrucciones, "IO_FS_WRITE", (void*)(intptr_t)I_IO_FS_WRITE);
    dictionary_put(instrucciones, "IO_FS_READ", (void*)(intptr_t)I_IO_FS_READ);
    dictionary_put(instrucciones, "EXIT", (void*)(intptr_t)I_EXIT);
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
{   
    iniciar_diccionario_instrucciones();
    iniciar_diccionario_registros(&proceso->registros);
    
    char** parte = string_split((char*)list_get(proceso->instrucciones, proceso->program_counter), " "); // Partes de la instruccion actual
	int instruccion_enum = (int)(intptr_t)dictionary_get(instrucciones, parte[0]);

    switch (instruccion_enum) 
    {
    case I_SET:
        instruccion_set(parte);
        break;
    // case I_MOV_IN:
    //     instruccion_mov_in(parte);
    //     break;
    // case I_MOV_OUT:
    //     instruccion_mov_out(parte);
    //     break;
    case I_SUM:
        instruccion_sum(parte);
        break;
    case I_SUB:
        instruccion_sub(parte);
        break;
    case I_JNZ:
        instruccion_jnz(parte);
        break;
    // case I_RESIZE:
    //     instruccion_resize(parte);
    //     break;
    // case I_COPY_STRING:
    //     instruccion_copy_string(parte);
    //     break;
    case I_WAIT:
        instruccion_wait(parte);
        break;
    case I_SIGNAL:
        instruccion_signal(parte);
        break;
    case I_IO_GEN_SLEEP:
        instruccion_io_gen_sleep(parte);
        break;
    // case I_IO_STDIN_READ:
    //     instruccion_io_stdin_read(parte);
    //     break;
    // case I_IO_STDOUT_WRITE:
    //     instruccion_io_stdout_write(parte);
    //     break;
    // case I_IO_FS_CREATE:
    //     instruccion_io_fs_create(parte);
    //     break;
    // case I_IO_FS_DELETE:
    //     instruccion_io_fs_delete(parte);
    //     break;
    // case I_IO_FS_TRUNCATE:
    //     instruccion_io_fs_truncate(parte);
    //     break;
    // case I_IO_FS_WRITE:
    //     instruccion_io_fs_write(parte);
    //     break;
    // case I_IO_FS_READ:
    //     instruccion_io_fs_read(parte);
    //     break;
    case I_EXIT:
        instruccion_exit(parte);
        break;
    case -1:
		log_warning(log_cpu, "PID: %d - Advertencia: No se pudo interpretar la instrucción - Ejecutando: EXIT", proceso->pid);
		error_exit(EXIT);
		return;
    }
    log_warning(log_cpu, "PID: %d - Advertencia: Sin instrucciones por ejecutar - Ejecutando: EXIT", proceso->pid);
	error_exit(EXIT);
    return;
}

void instruccion_set(char **parte)
{
    // SET AX 1
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
	memcpy(dictionary_get(registros, parte[1]), parte[2], strlen(parte[2]));
	proceso->program_counter++; 
}

void instruccion_sum(char **parte)
{
    // SUM AX BX
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    int *registro_destino = dictionary_get(registros, parte[1]);
    int *registro_origen = dictionary_get(registros, parte[2]);
    
    // Realizar la suma y almacenar el resultado en el registro destino
    *registro_destino += *registro_origen;
    
    proceso->program_counter++;
}

void instruccion_sub(char **parte)
{
    // SUB AX BX
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    int *registro_destino = dictionary_get(registros, parte[1]);
    int *registro_origen = dictionary_get(registros, parte[2]);
    
    // Realizar la resta y almacenar el resultado en el registro destino
    *registro_destino -= *registro_origen;
    
    proceso->program_counter++;
}

void instruccion_jnz(char **parte)
{
    // JNZ AX 4
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    
    int *registro = dictionary_get(registros, parte[1]);
    int instruccion = atoi(parte[2]); // Convertir la instrucción a un entero
    
    if (*registro != 0) 
    {
        proceso->program_counter = instruccion;
    } 
    else 
    {
        proceso->program_counter++;
    }
}

void instruccion_io_gen_sleep(char **parte)
{
    // IO_GEN_SLEEP Int1 10
    char *interfaz = parte[1];
    int unidades_trabajo = atoi(parte[2]);

    printf("Solicitando a la interfaz %s que realice un sleep por %d unidades de trabajo...\n", interfaz, parte[2]);
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    sleep(unidades_trabajo);
    // solicitar_sleep_io(parte[1], unidades_trabajo); // Esta función enviará la solicitud de sleep al Kernel
    printf("El sleep en la interfaz %s se ha completado.\n", interfaz);
    proceso->program_counter++;
}

void instruccion_exit(char** parte) 
{
	log_info(log_cpu, "PID: %d - Ejecutando: %s", proceso->pid, parte[0]);
	proceso->program_counter++;
	enviar_pcb(conexion_cpu_kernel, proceso, EXIT);
	list_destroy_and_destroy_elements(proceso->instrucciones, free);
	free(proceso);
}

// void intruccion_io_fs_create(char **parte)
// {

// }

// void intruccion_io_fs_delete(char **parte)
// {
    
// }

// void intruccion_io_fs_truncate(char **parte)
// {
    
// }

// void intruccion_io_fs_write(char **parte)
// {
    
// }

// void intruccion_io_fs_read(char **parte)
// {
    
// }

void instruccion_wait(char** parte)
{
	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", proceso->pid, parte[0], parte[1]);
	proceso->program_counter++;
    
    // Indico al kernel que el proceso está esperando algo
	enviar_pcb(conexion_cpu_kernel, proceso, WAIT);
	
    list_destroy_and_destroy_elements(proceso->instrucciones, free);
	free(proceso);
}

void instruccion_signal(char **parte)
{
    // Verificar que se haya pasado el nombre del recurso como argumento
    if (parte[1] == NULL) 
    {
        log_error(log_cpu, "No se ha proporcionado el nombre del recurso.");
        return;
    }

    // Log de ejecución de la instrucción
	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", proceso->pid, parte[0], parte[1]);

    // Enviar solicitud de SIGNAL al kernel
    enviar_pcb(conexion_cpu_kernel, proceso, SIGNAL);

    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    free(proceso);
}

void error_exit(char** parte) 
{
	enviar_pcb(conexion_cpu_kernel, proceso, CODIGO);
	list_destroy_and_destroy_elements(proceso->instrucciones, free);
	free(proceso);
}

void solicitar_instrucciones_a_memoria(int conexion_cpu_memoria) 
{   
    // Creo el paquete
    t_paquete* paquete = crear_paquete(); 

    // Agregamos el pc y el pid al paquete
    agregar_a_paquete(paquete, proceso->program_counter, sizeof(int));
    agregar_a_paquete(paquete, proceso->pid, sizeof(int));

    // Envio el paquete a memoria
    enviar_paquete(paquete, conexion_cpu_memoria);

    eliminar_paquete(paquete);
}

void recibir_instruccion_de_memoria(int socket_servidor_memoria)
{
    // Espero la respuesta de memoria
    t_paquete *respuesta = recibir_paquete(socket_servidor_memoria);

    // Verifico que se hayan recibidos valores
    if (respuesta == NULL) 
    {
        error_show("No se recibio el proceso por parte de MEMORIA");
        return;
    }

    // Extraigo las instrucciones del paquete
    memcpy(proceso, respuesta->buffer->stream, sizeof(respuesta));

    eliminar_paquete(respuesta);
    list_destroy_and_destroy_elements(respuesta, free);
}

// 3er checkpoint
// MOV_IN EDX ECX
// MOV_OUT EDX ECX
// RESIZE 128
// COPY_STRING 8
// IO_STDIN_READ Int2 EAX AX
// IO_STDOUT_WRITE Int3 BX EAX