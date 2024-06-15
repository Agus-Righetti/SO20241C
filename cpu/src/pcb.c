#include "pcb.h"

// PCB -----------------------------------------------------------------------------------------------------------------------------

void recibir_pcb(t_buffer* buffer, pcb** pcb_recibido)
{
    pcb* pcb_temp = recibir_estructura_del_buffer(buffer);

    if (pcb_temp == NULL) {
        log_error(log_cpu, "No se pudo recibir el PCB. La estructura recibida es NULL.");
        return;
    }

    *pcb_recibido = malloc(sizeof(pcb));

    if (*pcb_recibido == NULL) {
        log_error(log_cpu, "No se pudo asignar memoria para pcb_recibido");
        free(pcb_temp);
        return;
    }

    // Asignar los valores recibidos a la estructura pasada por referencia
    (*pcb_recibido)->pid = pcb_temp->pid;
    (*pcb_recibido)->program_counter = pcb_temp->program_counter;

    // Acordarse de sacarlos!!!!!!
    log_info(log_cpu, "El PID es: %d", (*pcb_recibido)->pid);
    log_info(log_cpu, "El PC es: %d", (*pcb_recibido)->program_counter);

    // Liberar la estructura temporal si es necesario
    free(pcb_temp);
}

void enviar_pcb(int conexion, pcb *proceso, op_code codigo, char* recurso)
{
    t_paquete *paquete = crear_paquete_personalizado(PCB_CPU_A_KERNEL);
    agregar_estructura_al_paquete_personalizado(paquete, proceso, sizeof(pcb));
    
    if (recurso == NULL) {
        return; // No hacer nada si el recurso es NULL
    }
    else
    {
        int recurso_valor = atoi(recurso);

        if(recurso_valor == 1) {
            agregar_int_al_paquete_personalizado(paquete, 0);
        } else if (recurso_valor == 2) {
            agregar_int_al_paquete_personalizado(paquete, 1); 
        } else if (recurso_valor == 3) {
            agregar_int_al_paquete_personalizado(paquete, 2); 
        } else {
            return; // No hacer nada si el recurso no coincide
        }
    }
    enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
}

// Diccionario ---------------------------------------------------------------------------------------------------------------------

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
	dictionary_put(registros, "PC", &registro->pc);
	dictionary_put(registros, "AX", &registro->ax);
	dictionary_put(registros, "BX", &registro->bx);
	dictionary_put(registros, "CX", &registro->cx);
	dictionary_put(registros, "DX", &registro->dx);
	dictionary_put(registros, "EAX", &registro->eax);
	dictionary_put(registros, "EBX", &registro->ebx);
	dictionary_put(registros, "ECX", &registro->ecx);
	dictionary_put(registros, "EDX", &registro->edx);
	dictionary_put(registros, "SI", &registro->si);
	dictionary_put(registros, "DI", &registro->di);
}

void destruir_diccionarios(void) 
{
	dictionary_destroy(instrucciones);
	dictionary_destroy(registros);
}

// Instrucciones memoria -----------------------------------------------------------------------------------------------------------

void solicitar_instrucciones_a_memoria(int socket_cliente_cpu, pcb** pcb_recibido) // Tendrian que ir **?
{   
    if (pcb_recibido == NULL) 
    {
        log_error(log_cpu, "No se pudo reservar memoria para el PCB al recibir el PCB");
    }

    // Creo el paquete
    t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_INSTRUCCION_A_MEMORIA); 

    // Agregamos el pc y el pid al paquete
    agregar_int_al_paquete_personalizado(paquete, (*pcb_recibido)->pid); 
    agregar_int_al_paquete_personalizado(paquete, (*pcb_recibido)->program_counter);

    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
    
    // Acordarse de sacarlo!!!!!!
    log_info(log_cpu, "Le pedi instruccion a Memoria");
    printf("Verificación fuera de la función:\n");
    printf("El PID es: %d\n", (*pcb_recibido)->pid);
    printf("El PC es: %d\n", (*pcb_recibido)->program_counter);
}

// Instrucciones -------------------------------------------------------------------------------------------------------------------

void interpretar_instruccion_de_memoria(t_buffer* buffer)
{   
    if (buffer == NULL) 
    {
        error_show("No se recibio el proceso por parte de MEMORIA");
        exit(1);
    }

    iniciar_diccionario_instrucciones();
    iniciar_diccionario_registros(&proceso->registros);
    
    // [MOV_IN EAX EBX] -> UNA instruccion la recibimos como una lista de string
    char** parte = string_split((char*)list_get(proceso->instrucciones, proceso->program_counter), " "); // Partes de la instruccion actual
	int instruccion_enum = (int)(intptr_t)dictionary_get(instrucciones, parte[0]);

    switch (instruccion_enum) 
    {
        case I_SET:
            instruccion_set(parte);
            break;
        case I_MOV_IN:
            instruccion_mov_in(parte);
            break;
        case I_MOV_OUT:
            instruccion_mov_out(parte);
            break;
        case I_SUM:
            instruccion_sum(parte);
            break;
        case I_SUB:
            instruccion_sub(parte);
            break;
        case I_JNZ:
            instruccion_jnz(parte);
            break;
        case I_RESIZE:
            instruccion_resize(parte);
            break;
        case I_COPY_STRING:
            instruccion_copy_string(parte);
            break;
        case I_WAIT:
            instruccion_wait(parte);
            break;
        case I_SIGNAL:
            instruccion_signal(parte);
            break;
        case I_IO_GEN_SLEEP:
            instruccion_io_gen_sleep(parte);
            break;
        case I_IO_STDIN_READ:
            instruccion_io_stdin_read(parte);
            break;
        case I_IO_STDOUT_WRITE:
            instruccion_io_stdout_write(parte);
            break;
        case I_IO_FS_CREATE:
            instruccion_io_fs_create(parte);
            break;
        case I_IO_FS_DELETE:
            instruccion_io_fs_delete(parte);
            break;
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
            destruir_diccionarios();
            break;
        case -1:
            log_warning(log_cpu, "PID: %d - Advertencia: No se pudo interpretar la instrucción - Ejecutando: EXIT", proceso->pid);
            error_exit(EXIT);
            destruir_diccionarios();
            return;
    }
    log_warning(log_cpu, "PID: %d - Advertencia: Sin instrucciones por ejecutar - Ejecutando: EXIT", proceso->pid);
	error_exit(EXIT);
    destruir_diccionarios();
    return;
}

void instruccion_set(char **parte)
{
    // SET AX 1
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    
	// Obtiene el valor del registro desde el diccionario 'registros' usando 'parte[1]' como clave y copia el valor de 'parte[2]' en el registro encontrado.
    memcpy(dictionary_get(registros, parte[1]), parte[2], strlen(parte[2]));
	proceso->program_counter++; 
}

void instruccion_mov_in(char **parte)
{
    // MOV_IN registro_datos registro_direccion
    // MOV_IN EDX ECX

	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    
    // Traducimos la direccion del registro direccion
	int direccion_fisica = traducir_direccion_logica_a_fisica(parte[2]); 
	
	char* instruccion = string_from_format("%s %s %d", parte[0], (char*)dictionary_get(registros, parte[1]), direccion_fisica);
	
    // Pido espacio de memoria para la instruccion + /0
    char* instruccion_alloc = malloc(strlen(instruccion) + 1);

	strcpy(instruccion_alloc, instruccion); // Le copio la instruccion a instruccion_alloc

	list_replace_and_destroy_element(proceso->instrucciones, proceso->program_counter, instruccion_alloc, free);
	
    log_trace(log_cpu, "PID: %d - Instruccion traducida: %s", proceso->pid, (char*)list_get(proceso->instrucciones, proceso->program_counter));
	
    t_instruccion* instruccion_proceso = malloc(sizeof(t_instruccion));
	generar_instruccion(proceso, instruccion_proceso, list_get(proceso->instrucciones, proceso->program_counter));

    // Enviamos la instruccion a memoria
    enviar_instruccion(socket_cliente_cpu, instruccion_proceso, MOV_IN);
	// log_warning(log_cpu, "PID: %d - Advertencia: Instruccion sin realizar", proceso->pid);
	proceso->program_counter++;
}

void generar_instruccion(pcb* proceso, t_instruccion* instruccion_proceso, char* instruccion) 
{
	instruccion_proceso->pid = proceso->pid;
	instruccion_proceso->instruccion = instruccion;
}

void instruccion_mov_out(char **parte)
{
    // MOV_OUT registro_direccion registro_datos
    // MOV_OUT EDX ECX

    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
	int direccion_fisica = traducir_direccion_logica_a_fisica(parte[1]);
	
	char* instruccion = string_from_format("%s %d %s", parte[0], direccion_fisica, parte[2]);
	char* instruccion_alloc = malloc(strlen(instruccion) + 1);
	strcpy(instruccion_alloc, instruccion);
    
	list_replace_and_destroy_element(proceso->instrucciones, proceso->program_counter, instruccion_alloc, free);
	log_trace(log_cpu, "PID: %d - Instruccion traducida: %s", proceso->pid, (char*)list_get(proceso->instrucciones, proceso->program_counter));
	t_instruccion* instruccion_proceso = malloc(sizeof(t_instruccion));
	
    generar_instruccion(proceso, instruccion_proceso, list_get(proceso->instrucciones, proceso->program_counter));
	enviar_instruccion(socket_cliente_cpu, instruccion_proceso, MOV_OUT);
    
	// log_warning(log_cpu, "PID: %d - Advertencia: Instruccion sin realizar", proceso->pid);
	proceso->program_counter++;
}

void enviar_instruccion(int conexion, t_instruccion* instruccion, op_code codigo)
{
    // Creamos un paquete
	t_paquete *paquete = crear_paquete_personalizado(codigo);
    agregar_estructura_al_paquete_personalizado(paquete, instruccion, sizeof(t_instruccion));
    enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);
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

void instruccion_resize(char **parte)
{
    // RESIZE 128
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", proceso->pid, parte[0], parte[1]);

    // Verificar si se proporcionó el tamaño como parámetro
    if (parte[1] == NULL) 
    {
        printf("Error: Se debe proporcionar un tamaño para la instrucción RESIZE\n");
        return;
    }

    // Convertir el tamaño de la instrucción a un entero
    int nuevo_tamanio = atoi(parte[1]);

    // Solicitar a la memoria ajustar el tamaño del proceso
    t_paquete *paquete = crear_paquete_personalizado(CPU_MANDA_RESIZE_A_MEMORIA); // [PID, TAMAÑO] -> [Int, Int]
    agregar_int_al_paquete_personalizado(paquete, proceso->pid);
    agregar_int_al_paquete_personalizado(paquete, nuevo_tamanio);
    enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);

    // Escucha para recibir el ok o el out of memory
    int cod_op_memoria = recibir_operacion(socket_cliente_cpu);
    while(1) {
        t_buffer* buffer;
        switch (cod_op_memoria){
            case CPU_RECIBE_OUT_OF_MEMORY_DE_MEMORIA: // VACIO
                printf("Error: No se pudo ajustar el tamaño del proceso. Out of Memory.\n");
                enviar_pcb(socket_cliente_kernel, proceso, OUT_OF_MEMORY, NULL);
                free(buffer);
                break;
            case CPU_RECIBE_OK_DEL_RESIZE:
                printf("El tamaño del proceso se ha ajustado correctamente a %d\n", nuevo_tamanio);
                free(buffer);
                break;
            case -1:
                log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
                free(socket_cliente_cpu);
                exit(1);
                return;
            default:
                log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
                break;
            }
    } 
    proceso->program_counter++; 
}
    

void instruccion_copy_string(char **parte)
{
    // COPY_STRING 8
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", proceso->pid, parte[0], parte[1]);

    // Verificar si se proporcionó el tamaño como parámetro
    if (parte[1] == NULL) 
    {
        printf("Error: Se debe proporcionar un tamaño para la instrucción COPY_STRING\n");
        return;
    }

    // Obtener el tamaño de la copia
    int tamanio = atoi(parte[1]);

    registros_cpu* registros;

    uint32_t direccion_origen = (uint32_t)dictionary_get(registros, "SI");
    uint32_t direccion_destino = (uint32_t)dictionary_get(registros, "DI");

    if (copiar_bytes(direccion_origen, direccion_destino, tamanio) == 1) {
        printf("Se han copiado correctamente %d bytes desde la dirección de memoria apuntada por SI hacia la dirección de memoria apuntada por DI\n", tamanio);
    } else {
        printf("Error al copiar los bytes en la dirección de memoria apuntada por DI\n");
    }

    proceso->program_counter++;
}

// Función para copiar bytes desde una dirección de memoria a otra
int copiar_bytes(uint32_t direccion_origen, uint32_t direccion_destino, int tamanio) 
{
    // Verificar si las direcciones de memoria son válidas
    if (direccion_origen == 0 || direccion_destino == 0) 
    {
        printf("Error: Direcciones de memoria inválidas\n");
        return 0;
    }

    if (tamanio != 0) 
    {
        memcpy((void *)direccion_destino, (void *)direccion_origen, tamanio);
        return 1;
    } 
}

void instruccion_wait(char** parte)
{
    // Verificar que se haya pasado el nombre del recurso como argumento
    if (parte[1] == NULL) 
    {
        log_error(log_cpu, "No se ha proporcionado el nombre del recurso.");
        return;
    }

	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", proceso->pid, parte[0], parte[1]);
	proceso->program_counter++;
    
    // Indico al kernel que el proceso está esperando algo
	enviar_pcb(socket_cliente_kernel, proceso, WAIT, parte[1]);
	
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

    // Log de ejecucion de la instruccion
	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", proceso->pid, parte[0], parte[1]);
    proceso->program_counter++;

    // Enviar solicitud de SIGNAL al kernel
    enviar_pcb(socket_cliente_kernel, proceso, SIGNAL, parte[1]);

    list_destroy_and_destroy_elements(proceso->instrucciones, free);
    free(proceso);
}

void instruccion_io_gen_sleep(char **parte)
{
    // IO_GEN_SLEEP Int1 10
    char *interfaz = parte[1];
    int unidades_trabajo = atoi(parte[2]);

    if(parte[1] != GENERICA) 
    {
        log_error(log_cpu, "La interfaz indicada no es GENERICA.");
    }
    
    printf("Solicitando a la interfaz %s que realice un sleep por %s unidades de trabajo...\n", interfaz, parte[2]);
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    solicitar_sleep_io(parte[1], unidades_trabajo, proceso->pid); // Esta función enviará la solicitud de sleep al Kernel
    printf("El sleep en la interfaz %s se ha completado.\n", interfaz);
    proceso->program_counter++;
}

void solicitar_sleep_io(const char *interfaz, int unidades_trabajo, int pid)
{
    // Crear y enviar el paquete al Kernel
    t_paquete *paquete = crear_paquete_personalizado(IO_GEN_SLEEP);

    // Agregar la información necesaria al paquete
    agregar_estructura_al_paquete_personalizado(paquete, &proceso, sizeof(pcb)); 
    agregar_estructura_al_paquete_personalizado(paquete, &unidades_trabajo, sizeof(int));
    agregar_estructura_al_paquete_personalizado(paquete, interfaz, strlen(interfaz) + 1);

    // Enviar el paquete al Kernel
    enviar_paquete(paquete, socket_cliente_kernel);

    // Liberar el paquete
    eliminar_paquete(paquete);
}

void instruccion_io_stdin_read(char** parte)
{
    // IO_STDIN_READ Int2 EAX AX

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL || parte[3] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_STDIN_READ.");
        return;
    }

    if(strcmp(parte[1], "STDIN") != 0) 
    {
        log_error(log_cpu, "La interfaz indicada no es STDIN.");
    }

    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s", proceso->pid, parte[0], parte[1], parte[2], parte[3]);

    // Extraer los registros de la instrucción
    char* interfaz = parte[1];
    char* direccion = parte[2];

    // Obtener la dirección lógica y el tamaño desde los registros
    int direccion_fisica = traducir_direccion_logica_a_fisica(direccion);
    int tamanio = atoi(parte[3]);

    // Enviar la solicitud al kernel
    enviar_solicitud_a_kernel(interfaz, direccion_fisica, tamanio, socket_cliente_kernel);
    proceso->program_counter++;
}

void enviar_solicitud_a_kernel(Interfaz* interfaz, int direccion_fisica, int tamanio, int conexion) 
{
    t_paquete* paquete = crear_paquete_personalizado(IO_STDIN_READ);

    // Agregar la estructura Interfaz al paquete
    agregar_estructura_al_paquete_personalizado(paquete, interfaz, sizeof(Interfaz));

    // Agregar la dirección fisica al paquete
    agregar_estructura_al_paquete_personalizado(paquete, &direccion_fisica, sizeof(int));

    // Agregar el tamaño al paquete
    agregar_estructura_al_paquete_personalizado(paquete, &tamanio, sizeof(int));

    // Enviar el paquete al socket cliente
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);
}

void instruccion_io_stdout_write(char **parte) 
{
    // IO_STDOUT_WRITE Int3 BX EAX

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL || parte[3] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_STDOUT_WRITE.");
        return;
    }

    if(strcmp(parte[1], "STDOUT") != 0) 
    {
        log_error(log_cpu, "La interfaz indicada no es STDOUT.");
    }

    char *interfaz = parte[1];
    char *registro_direccion = parte[2]; // Direccion logica
    char *registro_tamano = parte[3];

    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s", proceso->pid, parte[0], parte[1], parte[2], parte[3]);

    int valor_registro = (int)dictionary_get(registros, registro_tamano);
    int direccion_logica = (int)dictionary_get(registros, registro_direccion);
    // int direccion_fisica = traducir_direccion_logica_a_fisica(registro_direccion);

    // Crear paquete para enviar al Kernel
    t_paquete *paquete = crear_paquete_personalizado(IO_STDOUT_WRITE);

    // Agregar los datos al paquete
    agregar_string_al_paquete_personalizado(paquete, interfaz);
    agregar_string_al_paquete_personalizado(paquete, registro_direccion);
    agregar_string_al_paquete_personalizado(paquete, registro_tamano);
    agregar_int_al_paquete_personalizado(paquete, valor_registro);
    agregar_int_al_paquete_personalizado(paquete, direccion_logica);

    // Enviar paquete al Kernel
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);

    // Incrementar el program counter del proceso
    proceso->program_counter++;
}

void instruccion_io_fs_create(char **parte)
{
    // IO_FS_CREATE Interfaz NombreArchivo

    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);

    char *interfaz = parte[1];
    char *nombre_archivo = parte[2];

    // Crear paquete para enviar al kernel
    t_paquete* paquete = crear_paquete_personalizado(IO_FS_CREATE);
    agregar_string_al_paquete_personalizado(paquete, interfaz);
    agregar_string_al_paquete_personalizado(paquete, nombre_archivo);

    // Enviar paquete al kernel
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);

    // proceso->program_counter++;
}

void instruccion_io_fs_delete(char **parte)
{
    // IO_FS_DELETE Int4 notas.txt

    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", proceso->pid, parte[0], parte[1], parte[2]);
    
    char *interfaz = parte[1];
    char *nombre_archivo = parte[2];

    // Crear paquete para enviar al kernel
    t_paquete* paquete = crear_paquete_personalizado(IO_FS_DELETE);
    agregar_string_al_paquete_personalizado(paquete, interfaz);
    agregar_string_al_paquete_personalizado(paquete, nombre_archivo);

    // Enviar paquete al kernel
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);

    // proceso->program_counter++;
}

void instruccion_io_fs_truncate(char **parte)
{
    
}

void instruccion_io_fs_write(char **parte)
{
    
}

void instruccion_io_fs_read(char **parte)
{
    
}

void instruccion_exit(char** parte) 
{
	log_info(log_cpu, "PID: %d - Ejecutando: %s", proceso->pid, parte[0]);
	proceso->program_counter++;
	enviar_pcb(socket_cliente_kernel, proceso, CPU_TERMINA_EJECUCION_PCB, NULL);
	list_destroy_and_destroy_elements(proceso->instrucciones, free);
	free(proceso);
}

void error_exit(op_code codigo) 
{
	enviar_pcb(socket_cliente_kernel, proceso, CODIGO, NULL);
	list_destroy_and_destroy_elements(proceso->instrucciones, free);
	free(proceso);
}