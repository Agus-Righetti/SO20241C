#include "pcb.h"

// PCB -----------------------------------------------------------------------------------------------------------------------------

void recibir_pcb(){
    
    //log_info(log_cpu, "entre a recibir pcb");
    
    t_buffer* buffer_pcb = recibiendo_paquete_personalizado(socket_cliente_kernel);
    pcb_recibido = NULL;
    pcb_recibido = recibir_estructura_del_buffer(buffer_pcb);
    pcb_recibido->registros = recibir_estructura_del_buffer(buffer_pcb);

    log_info(log_cpu , "el valor del registro ax del pid %d es:%u", pcb_recibido->pid, pcb_recibido->registros->ax);
    
    if (pcb_recibido == NULL) 
    {
        log_error(log_cpu, "No se pudo recibir el PCB. La estructura recibida es NULL.");
        return;
    }
    if(pcb_recibido->registros == NULL){
        log_error(log_cpu, "No están inicializados los registros del pcb_recibido");
    }
    // Acordarse de sacarlos!!!!!!
    //log_info(log_cpu, "El PID es: %d", pcb_recibido->pid);
    //og_info(log_cpu, "El PC es: %u", pcb_recibido->registros->pc);

    solicitar_instrucciones_a_memoria(socket_cliente_cpu); 

    free(buffer_pcb);
}

void enviar_pcb(int conexion, argumentos_cpu* argumentos_a_mandar){
    
    t_paquete *paquete = crear_paquete_personalizado(argumentos_a_mandar->operacion);

    agregar_estructura_al_paquete_personalizado(paquete, argumentos_a_mandar->proceso, sizeof(pcb));
    agregar_estructura_al_paquete_personalizado(paquete, argumentos_a_mandar->proceso->registros, sizeof(registros_cpu));

    switch(argumentos_a_mandar->operacion) // Según el código de operación voy a agregar cosas diferentes al paquete (siempre siendo el pcb + otras cosas)
    {
        case FIN_DE_QUANTUM: // Si es el caso más sencillo, entonces no agrego nada, sólo mando el pcb
        case INTERRUPTED_BY_USER:
        case OUT_OF_MEMORY:  
            break;

        case WAIT:
        case SIGNAL:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->recurso);

            break;

        case IO_GEN_SLEEP:

            //log_info(log_cpu, "El nombre de la interfaz justo antes de meterlo al paquete es: %s",argumentos_a_mandar->nombre_interfaz);
            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->unidades_de_trabajo);

            break;

        case IO_STDIN_READ:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_lista_al_paquete_personalizado(paquete, argumentos_a_mandar->direcciones_fisicas);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_tamano);

            break;

        case IO_STDOUT_WRITE:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_direccion);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_tamano);

            break;

        case IO_FS_CREATE:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_archivo);

            break;

        case IO_FS_DELETE:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_archivo);

            break;

        case IO_FS_TRUNCATE:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_archivo);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_tamano);

            break;

        case IO_FS_WRITE:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);
            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_archivo);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_direccion);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_tamano);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_puntero_archivo);

            break;
        
        case IO_FS_READ:

            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_interfaz);    
            agregar_string_al_paquete_personalizado(paquete, argumentos_a_mandar->nombre_archivo);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_direccion);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_tamano);
            agregar_int_al_paquete_personalizado(paquete, argumentos_a_mandar->registro_puntero_archivo);

            break;	
        
        case CPU_TERMINA_EJECUCION_PCB:
            break;
    }
    
   // log_info(log_cpu, "Voy a enviar el pcb\n");

    enviar_paquete(paquete, conexion);

    //log_info(log_cpu, "Ya envie el pcb\n");

    free(argumentos_a_mandar); //libero los args

    // free(pcb_recibido->registros);
    // free(pcb_recibido);

	eliminar_paquete(paquete);
    return;
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

void iniciar_diccionario_registros()
{
    //aca deberia ser los registros del proceso q estamos ejecutando, no registros cualqiueras.
	registros = dictionary_create();
	dictionary_put(registros, "PC", &pcb_recibido->registros->pc);
	dictionary_put(registros, "AX", &pcb_recibido->registros->ax);
	dictionary_put(registros, "BX", &pcb_recibido->registros->bx);
	dictionary_put(registros, "CX", &pcb_recibido->registros->cx);
	dictionary_put(registros, "DX", &pcb_recibido->registros->dx);
	dictionary_put(registros, "EAX", &pcb_recibido->registros->eax);
	dictionary_put(registros, "EBX", &pcb_recibido->registros->ebx);
	dictionary_put(registros, "ECX", &pcb_recibido->registros->ecx);
	dictionary_put(registros, "EDX", &pcb_recibido->registros->edx);
	dictionary_put(registros, "SI", &pcb_recibido->registros->si);
	dictionary_put(registros, "DI", &pcb_recibido->registros->di);
}

void destruir_diccionarios(void) 
{
	dictionary_destroy(instrucciones);
	dictionary_destroy(registros);
}

// Instrucciones memoria -----------------------------------------------------------------------------------------------------------

// --------------- ESTO ES FETCH ---------------
void solicitar_instrucciones_a_memoria(int socket_cliente_cpu)
{   
    if (pcb_recibido == NULL) 
    {
        log_error(log_cpu, "No se pudo reservar memoria para el PCB al recibir el PCB");
    }

    iniciar_diccionario_registros();

    // Creo el paquete
    t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_INSTRUCCION_A_MEMORIA); 

    // Agregamos el pc y el pid al paquete
    agregar_int_al_paquete_personalizado(paquete, pcb_recibido->pid); 
    //cambie aca
    agregar_uint32_al_paquete_personalizado(paquete, pcb_recibido->registros->pc);

    // Envio el paquete a memoria
    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
    
    // Acordarse de sacarlo!!!!!!
    //log_info(log_cpu, "Le pedi instruccion a Memoria");

    // LOG OBLIGATORIO - FETCH INSTRUCCIÓN
    log_info(log_cpu, "PID: <%d> - FETCH - Program Counter: <%u>", pcb_recibido->pid, pcb_recibido->registros->pc); 
}

// Instrucciones -------------------------------------------------------------------------------------------------------------------



void interpretar_instruccion_de_memoria()
{   
    while(1)
    {
        //MOV_IN AX BX
    
        sem_wait(&sem_hay_instruccion);
        iniciar_diccionario_instrucciones(); // Esto va a iniciar el diccionario por cada instrucción. Se lo podría llamar una sola vez desde otro lado

        char** parte = string_split(instruccion_recibida, " "); // Divido la instrucción (que es un string) en partes   

        int instruccion_enum = (int)(intptr_t)dictionary_get(instrucciones, parte[0]); // Acá se obtiene la instrucción (el enum) a partir del diccionario

        switch (instruccion_enum) // Según la instrucción que sea, entonces realizo como tal cada instrucción (execute)
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
            case I_IO_FS_TRUNCATE:
                instruccion_io_fs_truncate(parte);
                break;
            case I_IO_FS_WRITE:
                instruccion_io_fs_write(parte);
                break;
            case I_IO_FS_READ:
                instruccion_io_fs_read(parte);
                break;
            case I_EXIT:
                instruccion_exit(parte);
                destruir_diccionarios();
                break;
            case -1:
                log_warning(log_cpu, "PID: %d - Advertencia: No se pudo interpretar la instrucción - Ejecutando: EXIT", pcb_recibido->pid);
                error_exit(EXIT);
                destruir_diccionarios();
                return;
        }
    }
    
    return;
}

void instruccion_set(char **parte) {
    
    // Ejemplo: SET AX 1
    
    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: <%d> - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);
    //log_info(log_cpu, "esto es pc antes de ejecutar: %u", pcb_recibido->registros->pc);
    char *registro = parte[1];
    if(es_Registro_de_1B(registro)){
        // El registro es de 1B
        //log_info(log_cpu,"estamos en registro de 1B");

        uint8_t *valor_registro = dictionary_get(registros, parte[1]);
        // uint8_t valor_registro_imprimir = *valor_registro;

        if (valor_registro == NULL)
        {
            log_info(log_cpu, "Error: El registro %s no se encontró en el diccionario.", registro);
            return;
        }
        // SEGMENTATION FAULT RECURRENTE
        //  REVISAR GRUPO, incorporamos otra variable, puede estar solucionado
        // log_info(log_cpu, "Registro: %s - Valor inicial: %u", registro, valor_registro_imprimir);

        if (parte[2] == NULL)
        {
            log_info(log_cpu, "Error: parte[2] es NULL.");
            return;
        }

        *valor_registro = (uint8_t)atoi(parte[2]); //le asigno el nuevo valor al registro

        log_info(log_cpu, "Registro: <%s> - Valor final: <%u>", registro, (uint8_t)atoi(parte[2]));
      
                
    } else {
        // El registro es de 4B
        uint32_t* valor_registro = dictionary_get(registros, parte[1]);

        if (valor_registro == NULL) {
            log_info(log_cpu, "Error: El registro %s no se encontró en el diccionario.", registro);
            return;
        }

        if (parte[2] == NULL) {
            log_info(log_cpu, "Error: parte[2] es NULL.");
            return;
        }

        *valor_registro = (uint32_t) atoi(parte[2]); // Asigno el nuevo valor al registro

        log_info(log_cpu, "Registro: <%s> - Valor final: <%u>", registro, (uint32_t)atoi(parte[2]));
    }

	// Aumento el PC
    //pcb_recibido->program_counter++; 
    pcb_recibido->registros->pc++;

    //log_info(log_cpu, "esto es pc dsp de ejecutar, antes de check interrupt: %u", pcb_recibido->registros->pc);
    
    check_interrupt();
    
    //log_info(log_cpu, "esto es pc dsp de checkinterrupt: %u", pcb_recibido->registros->pc);
    
    liberar_array_strings(parte);

    return;
}

void instruccion_mov_in(char **parte) {
    
    // MOV_IN (Registro Datos, Registro Dirección)

    // Lee el valor de memoria correspondiente a la Dirección Lógica que se encuentra en el Registro Dirección y lo almacena en el Registro Datos.

    // Todos los MOV_IN van a ser de 4 bytes, con la excepción de que el registro datos sea AX, BX, CX, DX donde pasan a ser de 1 byte

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
	log_info(log_cpu, "PID: <%d> - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);

    // Traducimos la direccion del registro direccion
    char *registro_direccion = parte[2]; // Direccion logica
    int direccion_logica = *(int*)dictionary_get(registros, registro_direccion);
    
    //log_info(log_cpu, "la direccion logica es: %d", direccion_logica);
    
    char *registro_dato = parte[1];

    //log_info(log_cpu, "El registro_dato es %s", registro_dato);

    t_list* direcciones_fisicas = malloc(sizeof(t_list));

    if(es_Registro_de_1B(registro_dato)){

        //log_info(log_cpu, "ESTOY JUSTO ANTES DE LA FUNCION TRADUCIR COMPLETO");

        //segm fault
        
        direcciones_fisicas = traducir_dl_a_df_completa(direccion_logica, 1);

        

        //log_info(log_cpu, "volvi de traducir las direcciones completo");

        // Ahora tengo todas las DF -> necesito leer en memoria el dato
        peticion_lectura_a_memoria(CPU_PIDE_LEER_REGISTRO_1B, pcb_recibido->pid, direcciones_fisicas);

        //log_info(log_cpu, "ya hice la peticion de lectura a memoria");

        // uint8_t valor_leido_de_memoria = espero_rta_lectura_1B_de_memoria();

        // REVISAR ESTO
        uint8_t* valor_registro_dato = dictionary_get(registros, registro_dato);

        sem_wait(&sem_valor_leido_de_memoria);

        *valor_registro_dato = valor_leido_de_memoria_8;

        //log_info(log_cpu, "(1B)El valor despues de mov_in es: %d", *valor_registro_dato);

    } else {
        // la lectura sera de 4 bytes
        //segm fault
        direcciones_fisicas = traducir_dl_a_df_completa(direccion_logica, 4);

        // Ahora tengo todas las DF -> necesito leer en memoria el dato
        peticion_lectura_a_memoria(CPU_PIDE_LEER_REGISTRO_4B, pcb_recibido->pid, direcciones_fisicas);

        // Esto tendria que ser una variable global y obtenerlo en la escucha general

        // uint32_t valor_leido_de_memoria = espero_rta_lectura_4B_de_memoria();

        // REVISAR ESTO
        uint32_t* valor_registro_dato = dictionary_get(registros, registro_dato);

        sem_wait(&sem_valor_leido_de_memoria);

        *valor_registro_dato = valor_leido_de_memoria_32; // Este ultimo es el que ahora es global

        //log_info(log_cpu, "(4B)El valor despues de mov_in es: %d", *valor_registro_dato);

    }
    
    // Aumento el PC para que lea la proxima instruccion
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    liberar_array_strings(parte);

    check_interrupt();
    
    return;
}

void instruccion_mov_out(char **parte) {
    // MOV_OUT registro_direccion registro_datos
    // Lee el valor del Registro Datos y lo escribe en la dirección física de memoria obtenida a partir de la Dirección Lógica almacenada en el Registro Dirección.
    // Todos los MOV_OUT van a ser de 4 bytes, con la excepción de que el registro datos sea AX, BX, CX, DX donde pasan a ser de 1 byte  

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);

    // Leo el registro de datos
    char *registro_dato = parte[2];

	// Traducimos la direccion del registro direccion
    // ver por que lo tratan como un char
    char *registro_direccion = parte[1]; // Direccion logica
    int direccion_logica = obtener_valor_registro_segun_nombre(registro_direccion);
    
    log_info(log_cpu, "La direccion logica es: %d", direccion_logica);
    
    t_list* direcciones_fisicas;

    if(es_Registro_de_1B(registro_dato))
    {
        //log_info(log_cpu, "Llegue a la traduccion");

        direcciones_fisicas = traducir_dl_a_df_completa(direccion_logica, 1);
        
        log_info(log_cpu, "el registro dato es: %s", parte[2]);
        
        uint8_t valor_registro_dato = (uint8_t) obtener_valor_registro_segun_nombre(registro_dato);
        
        log_info(log_cpu, "El valor de AX es -> %u", pcb_recibido->registros->ax);
        log_info(log_cpu, "El valor de EAX es -> %u", pcb_recibido->registros->eax);

        log_info(log_cpu, "El valor que quiero escribir es %u", valor_registro_dato);

        // Ahora tengo todas las DF -> necesito escribir en memoria el dato
        peticion_escritura_1B_a_memoria(pcb_recibido->pid, direcciones_fisicas, valor_registro_dato); 
        // [PID, DFs, VALOR] _> [Int, lista, uint8]

    } 
    else 
    {
        // la escritura sera de 4 bytes
        direcciones_fisicas = traducir_dl_a_df_completa(direccion_logica, 4);
        int vicky = list_size(direcciones_fisicas);
        log_info(log_cpu, "Escrituras por hacer -> %d", vicky);

        t_direccion_fisica* dir1 = list_get(direcciones_fisicas, 0);
        log_info(log_cpu, "1era escritura -> %d", dir1->bytes_a_operar);

        t_direccion_fisica* dir2 = list_get(direcciones_fisicas, 1);
        log_info(log_cpu, "2nda escritura -> %d", dir2->bytes_a_operar);
        
        uint32_t valor_registro_dato = (uint32_t) obtener_valor_registro_segun_nombre(registro_dato);
        
        log_info(log_cpu, "El valor que quiero escribir es %u", valor_registro_dato);

        // Ahora tengo todas las DF -> necesito leer en memoria el dato
        peticion_escritura_4B_a_memoria(pcb_recibido->pid, direcciones_fisicas, valor_registro_dato);

    }

    log_info(log_cpu, "Esperando que el valor se escriba en memoria...");

    sem_wait(&sem_ok_escritura);

    //log_info(log_cpu, "Valor escrito con exito");

    // Aumento el PC para que lea la proxima instruccion
	//pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;
    
    //log_info(log_cpu, "A rezar que ande bien este");

    liberar_array_strings(parte);

    check_interrupt();

    return;
}

void enviar_instruccion(int conexion, t_instruccion* instruccion, op_code codigo)
{
    // Creamos un paquete
	t_paquete *paquete = crear_paquete_personalizado(codigo);
    agregar_estructura_al_paquete_personalizado(paquete, instruccion, sizeof(t_instruccion));
    enviar_paquete(paquete, conexion);
	eliminar_paquete(paquete);

    return;
}

void instruccion_sum(char **parte) {
    // SUM (Registro Destino, Registro Origen)
    // Suma al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.

    // SUM AX BX

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);

    
    char *registro_destino = parte[1];
    char *registro_origen = parte[2];


    if(es_Registro_de_1B(registro_destino) && es_Registro_de_1B(registro_origen)){
        // Hago la suma de 1B
        uint8_t* valor_registro_destino = dictionary_get(registros, registro_destino);
        uint8_t* valor_registro_origen = dictionary_get(registros, registro_origen);

        log_info(log_cpu, "Registro origen: %u", *valor_registro_origen);
        log_info(log_cpu, "Registro destino: %u", *valor_registro_destino);

        *valor_registro_destino += *valor_registro_origen;
        log_info(log_cpu, "Registro destino luego de la suma: %u", *valor_registro_destino);

    } else {
        // Hago la suma de 4B
        uint32_t* valor_registro_destino = dictionary_get(registros, registro_destino);
        uint32_t* valor_registro_origen = dictionary_get(registros, registro_origen);

        log_info(log_cpu, "Registro origen: %u", *valor_registro_origen);
        log_info(log_cpu, "Registro destino: %u", *valor_registro_destino);

        *valor_registro_destino += *valor_registro_origen;
        log_info(log_cpu, "Registro destino luego de la suma: %u", *valor_registro_destino);
    }

    
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    liberar_array_strings(parte);

    check_interrupt();

    return;
}


void instruccion_sub(char **parte) {
    //SUB (Registro Destino, Registro Origen)
    // Resta al Registro Destino el Registro Origen y deja el resultado en el Registro Destino.

    // SUB AX BX

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);

    char *registro_destino = parte[1];
    char *registro_origen = parte[2];


    if(es_Registro_de_1B(registro_destino) && es_Registro_de_1B(registro_origen)){
        // Hago la resta de 1B
        uint8_t* valor_registro_destino = dictionary_get(registros, registro_destino);
        uint8_t* valor_registro_origen = dictionary_get(registros, registro_origen);

        log_info(log_cpu, "Registro origen: %u", *valor_registro_origen);
        log_info(log_cpu, "Registro destino: %u", *valor_registro_destino);

        *valor_registro_destino -= *valor_registro_origen;
        log_info(log_cpu, "Registro destino luego de la resta: %u", *valor_registro_destino);

    } else {
        // Hago la resta de 4B
        uint32_t* valor_registro_destino = dictionary_get(registros, registro_destino);
        uint32_t* valor_registro_origen = dictionary_get(registros, registro_origen);

        log_info(log_cpu, "Registro origen: %u", *valor_registro_origen);
        log_info(log_cpu, "Registro destino: %u", *valor_registro_destino);

        *valor_registro_destino -= *valor_registro_origen;
        log_info(log_cpu, "Registro destino luego de la resta: %u", *valor_registro_destino);
    }

    
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    liberar_array_strings(parte);

    check_interrupt();

    return;
}

void instruccion_jnz(char **parte) {
    // JNZ (Registro, Instrucción)
    // Si el valor del registro es distinto de cero, actualiza el program counter al número de instrucción pasada por parámetro.

    // JNZ AX 4

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);
    
    int registro = *(int*)dictionary_get(registros, parte[1]);
    int instruccion = atoi(parte[2]); // Convertir la instrucción a un entero

    log_info(log_cpu, "PC antes del jnz: %d", pcb_recibido->registros->pc);

    if (registro != 0) {
        // Actualizo el PC al pasado por parametro
        //pcb_recibido->program_counter = instruccion;
        pcb_recibido->registros->pc = instruccion;

    } else {
        // pcb_recibido->program_counter++;
        pcb_recibido->registros->pc++;
    }

    log_info(log_cpu, "PC después del jnz: %d", pcb_recibido->registros->pc);

    liberar_array_strings(parte);

    check_interrupt();
    
    return;
}


void instruccion_resize(char **parte) {
    // ACA HAY QUE MANEJAR UN ENVIO DE PCB QUE ESTA COMENTADO EN EL SWITCH
    // RESIZE (Tamaño)
    // Solicitará a la Memoria ajustar el tamaño del proceso al tamaño pasado por parámetro. En caso de que la respuesta de la memoria sea Out of Memory, se deberá devolver el contexto de ejecución al Kernel informando de esta situación.

    // RESIZE 128

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", pcb_recibido->pid, parte[0], parte[1]);

    // Verificar si se proporcionó el tamaño como parámetro
    if (parte[1] == NULL) 
    {
        printf("Error: Se debe proporcionar un tamaño para la instrucción RESIZE\n");
        return;
    }

    // Convertir el tamaño de la instrucción a un entero
    int nuevo_tamanio = atoi(parte[1]);

    liberar_array_strings(parte);

    // Solicitar a la memoria ajustar el tamaño del proceso
    t_paquete *paquete = crear_paquete_personalizado(CPU_MANDA_RESIZE_A_MEMORIA); // [PID, TAMAÑO] -> [Int, Int]
    agregar_int_al_paquete_personalizado(paquete, pcb_recibido->pid);
    agregar_int_al_paquete_personalizado(paquete, nuevo_tamanio);
    enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);

    pcb_recibido->registros->pc++;
    
    sem_wait(&sem_tengo_ok_resize);

    check_interrupt();


    return;
}
    

void instruccion_copy_string(char **parte) {

    // COPY_STRING (Tamaño): Toma del string apuntado por el registro SI y copia la cantidad 
    //de bytes indicadas en el parámetro tamaño a la posición de memoria apuntada por 
    //el registro DI. 

    // COPY_STRING (TAMAÑO EN BYTES)

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", pcb_recibido->pid, parte[0], parte[1]);

    // Verificar si se proporcionó el tamaño como parámetro
    if (parte[1] == NULL) 
    {
        printf("Error: Se debe proporcionar un tamaño para la instrucción COPY_STRING\n");
        return;
    }

    // Obtener el tamaño de la copia
    int tamanio = atoi(parte[1]);

    // 1ero necesito obtener el STRING
    // registro "SI" tiene DL de donde esta almacenado en memoria 
    uint32_t dl_origen = pcb_recibido->registros->si;

    // Busco dirección física origen
    t_list* df_origen = traducir_dl_a_df_completa(dl_origen, tamanio);

    // Tengo que ir a leer el dato -> el string
    peticion_lectura_string_a_memoria(pcb_recibido->pid, df_origen, tamanio);


    sem_wait(&sem_string_memoria);


    // registro "DI" tiene DL en donde tengo que guardar el dato
    uint32_t dl_destino = pcb_recibido->registros->di;
    
    // Busco dirección física destino
    t_list* df_destino = traducir_dl_a_df_completa(dl_destino, tamanio);

    peticion_escritura_string_a_memoria(pcb_recibido->pid, df_destino, string_leido); 

    sem_wait(&sem_ok_escritura_string);


    pcb_recibido->registros->pc++;
    
    liberar_array_strings(parte);

    check_interrupt();


    return;
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

    return 0; // Agrego eso para que, si no entra en ningun if, devuelva algo
}

void instruccion_wait(char** parte)
{
    // Verificar que se haya pasado el nombre del recurso como argumento
    if (parte[1] == NULL) 
    {
        log_error(log_cpu, "No se ha proporcionado el nombre del recurso.");
        return;
    }

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", pcb_recibido->pid, parte[0], parte[1]);

	//pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;
    argumentos_cpu* argumentos_a_mandar = malloc(sizeof(argumentos_cpu));
    
    log_info(log_cpu, "El nombre del recurso es: %s", parte[1]);
    argumentos_a_mandar->proceso = pcb_recibido; 
    argumentos_a_mandar->recurso = parte[1];
    argumentos_a_mandar->operacion = WAIT;

    // Enviar solicitud de SIGNAL al kernel
    enviar_pcb(socket_cliente_kernel, argumentos_a_mandar);
	
    liberar_array_strings(parte);

	//free(proceso);
    return;
}

void instruccion_signal(char **parte)
{
    // Verificar que se haya pasado el nombre del recurso como argumento
    if (parte[1] == NULL) 
    {
        log_error(log_cpu, "No se ha proporcionado el nombre del recurso.");
        return;
    }
    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
	log_info(log_cpu, "PID: %d - Ejecutando: %s - %s", pcb_recibido->pid, parte[0], parte[1]);

    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido; 
    args->recurso = parte[1];
    args->operacion = SIGNAL;

    // Enviar solicitud de SIGNAL al kernel
    enviar_pcb(socket_cliente_kernel, args);

    liberar_array_strings(parte);
    
    //free(proceso);
    return;
}

void instruccion_io_gen_sleep(char **parte)
{
    // IO_GEN_SLEEP Int1 10

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: <%d> - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);

    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->unidades_de_trabajo = atoi(parte[2]);
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;
    
    args->proceso = pcb_recibido;
    args->operacion = IO_GEN_SLEEP;

    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Unidades de trabajo: %d", args->unidades_de_trabajo);

    enviar_pcb(socket_cliente_kernel, args);

    liberar_array_strings(parte);

    return;
}

void instruccion_io_stdin_read(char** parte)
{
    // IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
    // Esta instrucción solicita al Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado) 
    // un valor cuyo tamaño está delimitado por el valor del Registro Tamaño y el mismo se guarde a partir de la 
    // Dirección Lógica almacenada en el Registro Dirección.

    // IO_STDIN_READ Int2 EAX AX

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL || parte[3] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_STDIN_READ.");
        return;
    }

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2], parte[3]);

    // Extraer los registros de la instrucción
    // char* interfaz = parte[1];
    char* direccion = parte[2];

    // Obtener la dirección lógica y el tamaño desde los registros
    int direccion_logica = obtener_valor_registro_segun_nombre(registros, direccion);
    int registro_tamano = obtener_valor_registro_segun_nombre(registros, parte[3]);

    t_list* direcciones_fisicas = traducir_dl_a_df_completa(direccion_logica, registro_tamano);
    
    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->direcciones_fisicas = direcciones_fisicas;
    args->registro_tamano = registro_tamano;

    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_STDIN_READ;

    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Registro direccion: %d", args->registro_direccion);
    log_info(log_cpu, "Registro tamanio: %d", args->registro_tamano);

    enviar_pcb(socket_cliente_kernel, args);
    
    liberar_array_strings(parte);
}

void instruccion_io_stdout_write(char **parte) // ESTE NO LO ENTIENDO PORQUE MANDA BANDA DE COSAS
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

    // char *interfaz = parte[1];
    char *registro_direccion = parte[2]; // Direccion logica
    char *registro_tamano = parte[3];

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2], parte[3]);

    int valor_registro = *(int*)dictionary_get(registros, registro_tamano);
    int direccion_logica = *(int*)dictionary_get(registros, registro_direccion);
    int direccion_fisica = traducir_direccion_logica_a_fisica(direccion_logica);     

    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->registro_direccion = direccion_fisica;
    args->registro_tamano = valor_registro;

    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_STDOUT_WRITE;

    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Registro direccion: %d", args->registro_direccion);
    log_info(log_cpu, "Registro tamanio: %d", args->registro_tamano);

    enviar_pcb(socket_cliente_kernel, args);

    liberar_array_strings(parte);
}

void instruccion_io_fs_create(char **parte)
{
    // IO_FS_CREATE Interfaz NombreArchivo

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instruccion IO_FS_CREATE.");
        return;
    }

    // if(strcmp(parte[1], "DIALFS") != 0) 
    // {
    //     log_error(log_cpu, "La interfaz indicada no es DIALFS.");
    // }


    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);


    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->nombre_archivo = parte[2];
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_FS_CREATE;

    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Nombre archivo: %s", args->nombre_archivo);
    
    enviar_pcb(socket_cliente_kernel, args);

    liberar_array_strings(parte); 
}

void instruccion_io_fs_delete(char **parte)
{
    // IO_FS_DELETE Int4 notas.txt
    
    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_FS_DELETE.");
        return;
    }

    // if(strcmp(parte[1], "DIALFS") != 0) 
    // {
    //     log_error(log_cpu, "La interfaz indicada no es DIALFS.");
    // }

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2]);
    
    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->nombre_archivo = parte[2];
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_FS_DELETE;

    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Nombre archivo: %s", args->nombre_archivo);

    enviar_pcb(socket_cliente_kernel, args);

    liberar_array_strings(parte);  
}

void instruccion_io_fs_truncate(char **parte)
{
    // IO_FS_TRUNCATE Int4 notas.txt ECX

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL || parte[3] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_FS_TRUNCATE.");
        return;
    }

    // if(strcmp(parte[1], "DIALFS") != 0) 
    // {
    //     log_error(log_cpu, "La interfaz indicada no es DIALFS.");
    // }
    
    // Log de la ejecucion de la instruccion

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2], parte[3]);
    
    int nuevo_tamanio = *(int*)dictionary_get(registros, parte[3]);
    
    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->nombre_archivo = parte[2];
    args->registro_tamano = nuevo_tamanio;
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_FS_TRUNCATE;
    
    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Nombre archivo: %s", args->nombre_archivo);
    log_info(log_cpu, "Registro tamanio: %s", args->registro_tamano);

    enviar_pcb(socket_cliente_kernel, args);

    liberar_array_strings(parte);
}

void instruccion_io_fs_write(char **parte)
{
    // IO_FS_WRITE Int4 notas.txt AX ECX EDX

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL || parte[3] == NULL || parte[4] == NULL || parte[5] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_FS_WRITE.");
        return;
    }

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2], parte[3], parte[4], parte[5]);

    // // Extrae los parámetros de la instrucción
    // char *interfaz = parte[1];
    // char *nombre_archivo = parte[2];
    int registro_direccion = *(int*)dictionary_get(registros, parte[3]); // Registro que contiene la posición inicial
    int registro_tamanio = *(int*)dictionary_get(registros, parte[4]); // Registro que contiene el tamaño de los datos
    int registro_puntero_archivo = *(int*)dictionary_get(registros, parte[5]); // Registro que contiene los datos a escribir

    int direccion_fisica = traducir_direccion_logica_a_fisica(registro_direccion);

    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->nombre_archivo = parte[2];
    args->registro_direccion = direccion_fisica; 
    args->registro_tamano = registro_tamanio;
    args->registro_puntero_archivo = registro_puntero_archivo;
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_FS_WRITE;

    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Nombre archivo: %s", args->nombre_archivo);
    log_info(log_cpu, "Registro dirección: %s", args->registro_direccion);
    log_info(log_cpu, "Registro tamanio: %s", args->registro_tamano);
    log_info(log_cpu, "Registro puntero archivo: %s", args->registro_puntero_archivo);

    enviar_pcb(socket_cliente_kernel, args);
    
    liberar_array_strings(parte);
}

void instruccion_io_fs_read(char **parte)
{
    // IO_FS_READ Int4 notas.txt BX ECX EDX

    // Verificar que la cantidad de argumentos sea la correcta
    if (parte[1] == NULL || parte[2] == NULL || parte[3] == NULL || parte[4] == NULL || parte[5] == NULL) 
    {
        log_error(log_cpu, "Argumentos incorrectos para la instrucción IO_FS_READ.");
        return;
    }

    // if(strcmp(parte[1], "DIALFS") != 0) 
    // {
    //     log_error(log_cpu, "La interfaz indicada no es DIALFS.");
    // }

    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
    log_info(log_cpu, "PID: %d - Ejecutando: %s - %s %s %s %s %s", pcb_recibido->pid, parte[0], parte[1], parte[2], parte[3], parte[4], parte[5]);

    // // Extrae los parámetros de la instrucción
    // char *interfaz = parte[1];
    // char *nombre_archivo = parte[2];
    int registro_direccion = *(int*)dictionary_get(registros, parte[3]); // Registro que contiene la posición inicial
    int registro_tamanio = *(int*)dictionary_get(registros, parte[4]); // Registro que contiene el tamaño de los datos a leer
    int registro_puntero_archivo = *(int*)dictionary_get(registros, parte[5]); // Registro donde se almacenarán los datos leídos

    int direccion_fisica = traducir_direccion_logica_a_fisica(registro_direccion);

    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    args->nombre_interfaz = parte[1];
    args->nombre_archivo = parte[2];
    args->registro_direccion = direccion_fisica;
    args->registro_tamano = registro_tamanio;
    args->registro_puntero_archivo = registro_puntero_archivo;
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;

    args->proceso = pcb_recibido;
    args->operacion = IO_FS_READ;
    
    log_info(log_cpu, "Nombre interfaz: %s", args->nombre_interfaz);
    log_info(log_cpu, "Nombre archivo: %s", args->nombre_archivo);
    log_info(log_cpu, "Registro dirección: %s", args->registro_direccion);
    log_info(log_cpu, "Registro tamanio: %s", args->registro_tamano);
    log_info(log_cpu, "Registro puntero archivo: %s", args->registro_puntero_archivo);
    
    enviar_pcb(socket_cliente_kernel, args);
    
    liberar_array_strings(parte);
}

void instruccion_exit(char** parte) 
{
    // LOG OBLIGATORIO - INSTRUCCIÓN EJECUTADA
	log_info(log_cpu, "PID: %d - Ejecutando: %s", pcb_recibido->pid, parte[0]);

    argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
    //pcb_recibido->program_counter++;
    pcb_recibido->registros->pc++;
    
    args->proceso = pcb_recibido;
    args->operacion = CPU_TERMINA_EJECUCION_PCB;

    liberar_array_strings(parte);
    enviar_pcb(socket_cliente_kernel, args);

	// proceso->program_counter++;
	// enviar_pcb(socket_cliente_kernel, proceso, CPU_TERMINA_EJECUCION_PCB, NULL);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// free(proceso);
    
}

void error_exit(op_code codigo) 
{
	// enviar_pcb(socket_cliente_kernel, proceso, CODIGO, NULL);
	// list_destroy_and_destroy_elements(proceso->instrucciones, free);
	// free(proceso);
}

void generar_instruccion(pcb* proceso, t_instruccion* instruccion_proceso, char* instruccion) 
{
	instruccion_proceso->pid = proceso->pid;
	instruccion_proceso->instruccion = instruccion;

    return;
}

void check_interrupt(){
    // Chequeo si hay interrupciones
    
    if(flag_interrupcion){ //hay una interrupcion

        argumentos_cpu* args = malloc(sizeof(argumentos_cpu));
        args->proceso = pcb_recibido;
        args->operacion = motivo_interrupcion;

        enviar_pcb(socket_cliente_kernel, args);

        flag_interrupcion = false;
        motivo_interrupcion = -1;
        instruccion_recibida = NULL; 

        return;
        
        
    } else {
        instruccion_recibida = NULL; 
        solicitar_instrucciones_a_memoria(socket_cliente_cpu);
        return;

    }

}

void liberar_array_strings(char **array) {
    
    if (array == NULL) return; // Si ya es null, entonces termino la función
    
    for (int i = 0; array[i] != NULL; i++) { // Sino voy recorriendo y liberando
        free(array[i]);
    }
    free(array);
}







int obtener_valor_registro_segun_nombre(char* nombre_registro)
{
    if(strcmp(nombre_registro,"AX")==0)
    {
        return pcb_recibido->registros->ax;
    } else if(strcmp(nombre_registro,"BX")==0)
    {
        return pcb_recibido->registros->bx;
    } else if(strcmp(nombre_registro,"CX")==0)
    {
        return pcb_recibido->registros->cx;
        
    } else if(strcmp(nombre_registro,"DX")==0)
    {
        return pcb_recibido->registros->dx;
    } else if(strcmp(nombre_registro,"EAX")==0)
    {
        return pcb_recibido->registros->eax;
    } else if(strcmp(nombre_registro,"EBX")==0){
        return pcb_recibido->registros->ebx;
    }
     else if(strcmp(nombre_registro,"ECX")==0)
    {
        return pcb_recibido->registros->ecx;
    } else if(strcmp(nombre_registro,"EDX")==0)
    {
        return pcb_recibido->registros->edx;
    } else if(strcmp(nombre_registro,"PC")==0)
    {
        return pcb_recibido->registros->pc;
    } else if(strcmp(nombre_registro,"DI")==0)
    {
        return pcb_recibido->registros->di;
    }else if(strcmp(nombre_registro,"SI")==0)
    {
        return pcb_recibido->registros->si;
    } 
}