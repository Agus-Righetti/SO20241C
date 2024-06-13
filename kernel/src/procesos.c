#include "procesos.h"

// -------------------------------------------------------------------------------
// ------------- INICIO HILOS QUE SE QUEDAN CONSTANTEMENTE PRENDIDOS -------------
// -------------------------------------------------------------------------------

// ************* CREO HILO PARA DEJAR LA CONSOLA ABIERTA SIEMPRE *************
pthread_t hilo_consola (){ 

    pthread_t thread_consola; 
            
    pthread_create(&thread_consola, NULL, (void*)leer_consola, NULL);

    return thread_consola;
}

// ************* CREO HILO PARA ENVIAR PROCESOS A CPU CONSTANTEMENTE *************
pthread_t hilo_enviar_procesos_cpu (){ 

    pthread_t thread_enviar_procesos_cpu;
            
    pthread_create(&thread_enviar_procesos_cpu, NULL, (void*)enviar_proceso_a_cpu, NULL);

    return thread_enviar_procesos_cpu;
}

// ************* CREO HILO PARA PASAR PROCESOS A LA COLA DE READY SI DA EL GRADO DE MULTIPROGRAMACION *************
pthread_t hilo_pasar_de_new_a_ready(){ 

    pthread_t thread_pasar_de_new_a_ready;
            
    pthread_create(&thread_pasar_de_new_a_ready, NULL, (void*)pasar_procesos_de_new_a_ready, NULL);

    return thread_pasar_de_new_a_ready;
}

// ----------------------------------------------------------------------------
// ------------- FIN HILOS QUE SE QUEDAN CONSTANTEMENTE PRENDIDOS -------------
// ----------------------------------------------------------------------------





// --------------------------------------------------------------------------------------------
// ------------- INICIO FUNCIONES LLAMADAS POR LOS HILOS CONSTANTEMENTE PRENDIDOS -------------
// --------------------------------------------------------------------------------------------

// ************* SIRVE PARA LEER LOS COMANDOS DESDE LA CONSOLA (DESDE HILO_CONSOLA) *************
void leer_consola(void* arg){
    
     while(1) { // Para mantener esperando comandos constantemente
        
        char* lectura = readline("Ingrese comando: "); // Pido que se ingrese un comando

        // Partes me separa segun los espacios lo que hay, en partes[0] esta INICIAR_PROCESO
        // En partes[1] va a estar el path
        char **partes = string_split(lectura, " "); 

        // Dependiendo el comando llamo a la función correspondiente
        if (strcmp(partes[0], "INICIAR_PROCESO") == 0) { 

            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
            iniciar_proceso(partes[1]); // Esta es la funcion a la que llamo

        } else if (strcmp(lectura, "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");
            finalizar_proceso(partes[1]); // Acá mando el PID del proceso que quiero finalizar

        }else if (strcmp(partes[0], "PROCESO_ESTADO") == 0){
            printf("Ha seleccionado la opción PROCESO_ESTADO\n");
            listar_procesos_por_estado();
        
        }else if (strcmp(partes[0], "MULTIPROGRAMACION") == 0){
            printf("Ha seleccionado la opción MULTIPROGRAMACION\n");
            cambiar_grado_multiprogramacion(partes[1]);
        }else if{
            printf("Ha seleccionado la opción EJECUTAR_SCRIPT\n");
            ejecutar_script(partes[1]);
        }else {
            
            printf("Opción no válida\n");
        }
        
        liberar_array_strings(partes); // Libero la memoria asignada por string_split

        free(lectura); // Libera la memoria asignada por readline
    }
}

// ************* PARA ENVIAR UN PROCESO (PCB) A CPU (DESDE HILO_ENVIAR_PROCESOS_CPU) *************
void enviar_proceso_a_cpu(){

    while(1){ // Para estar constantemente intentando enviar un proceso

        sem_wait(&sem_puedo_mandar_a_cpu); // Espero si ya hay otro proceso ejecutando en CPU (solo se ejecuta de a un proceso)
        pcb* proceso_seleccionado;
        if((strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0) && (queue_is_empty(cola_prioridad_vrr) == false))
        { 
            // Verifico si es VRR el algoritmo y si la cola de prioridad tiene algo, entonces voy a usar lo de esa cola prioritaria

            pthread_mutex_lock(&mutex_cola_prioridad_vrr); 
            proceso_seleccionado = queue_pop(cola_prioridad_vrr); // Saco el proceso siguiente de la cola de prioridad
            pthread_mutex_unlock(&mutex_cola_prioridad_vrr);

        }else{
            
            sem_wait(&sem_cola_de_ready); // Si no hay nada en la cola de Ready no avanzo
            pthread_mutex_lock(&mutex_cola_de_ready);
            proceso_seleccionado = queue_pop(cola_de_ready); // Saco el proceso siguiente de la cola de Ready
            pthread_mutex_unlock(&mutex_cola_de_ready);

        }

        if (proceso_seleccionado->estado_del_proceso == READY) // Verifico que el estado del proceso sea READY
        {
            
            pthread_mutex_lock(&proceso_seleccionado->mutex_pcb);
            proceso_seleccionado->estado_del_proceso = EXECUTE; // Cambio el estado del proceso sacado de la cola de READY
            pthread_mutex_unlock(&proceso_seleccionado->mutex_pcb);

            log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso_seleccionado->pid); // Hago un log obligatorio 
            
            enviar_pcb(proceso_seleccionado); // Envio el pcb a CPU

            crear_hilo_proceso (proceso_seleccionado); // Inicio un hilo que maneje el algoritmo del proceso

        }else
        {
            
            error_show("El estado seleccionado no se encuentra en estado 'READY'"); // Si el proceso seleccionado no se encuentra en READY muestro un error

        }
    }
}

// ************* PARA PASAR PROCESOS NEW -> READY (DESDE HILO_PASAR_DE_NEW_A_READY) *************
void pasar_procesos_de_new_a_ready(){
    
    //Parte del planificador a largo plazo
    //Espera que haya procesos en la cola de New, si el grado de multiprogramacion lo permite, pasa los procesos a Ready para que se ejecuten
    
    pcb * proceso_a_mandar_a_ready;
    while(1)
    {
        sem_wait(&sem_cola_de_new); // Uso semaforos sobre la cola y el grado de multiprogramacion porque son cosas compartidas
        sem_wait(&sem_multiprogramacion); // Tanto para ver que haya procesos en New como para ver de no pasarme del grado de multiprogramacion
        pthread_mutex_lock(&mutex_cola_de_new);
        proceso_a_mandar_a_ready = queue_pop(cola_de_new); // Saco un proceso de la cola de New para despues pasarlo a Ready
        pthread_mutex_unlock(&mutex_cola_de_new);

        pthread_mutex_lock(&proceso_a_mandar_a_ready->mutex_pcb);
        proceso_a_mandar_a_ready->estado_del_proceso = READY; // Le asigno el nuevo estado "Ready"
        pthread_mutex_unlock(&proceso_a_mandar_a_ready->mutex_pcb);

        pthread_mutex_lock(&mutex_cola_de_ready); // Tmbn semaforo porque es seccion critica 
        queue_push(cola_de_ready,proceso_a_mandar_a_ready); // Ingreso el proceso a la cola de Ready. 
        pthread_mutex_unlock(&mutex_cola_de_ready);
        sem_post(&sem_cola_de_ready); // Agrego 1 al semaforo contador de la cola
    }
}

// -----------------------------------------------------------------------------------------
// ------------- FIN FUNCIONES LLAMADAS POR LOS HILOS CONSTANTEMENTE PRENDIDOS -------------
// -----------------------------------------------------------------------------------------





// -------------------------------------------------------------------------------------------
// ------------- INICIO FUNCIONES PROPIAS DE LOS COMANDOS INGRESADOS POR CONSOLA -------------
// -------------------------------------------------------------------------------------------

// ************* INICIALIZA UN NUEVO PCB Y LO MANDA A LA COLA CORRESPONDIENTE *************
void iniciar_proceso(char* path ){    

    // Creo una estructura de pcb e inicializo todos los campos
    pcb* nuevo_pcb = malloc(sizeof(pcb)); 
    pid_contador += 1;
    nuevo_pcb->estado_del_proceso = NEW;
    nuevo_pcb->program_counter = 0;
    nuevo_pcb->direccion_instrucciones = path; // Este path lo enviamos a memoria
    nuevo_pcb->pid = pid_contador;
    nuevo_pcb->quantum = config_kernel->quantum; // Al iniciar el proceso tiene todo su quantum disponible
    nuevo_pcb->recursos_asignados = queue_create();
    nuevo_pcb->registros = malloc(sizeof(registros_cpu)); 
    nuevo_pcb->registros->ax = 0;
    nuevo_pcb->registros->bx = 0;
    nuevo_pcb->registros->cx = 0;
    nuevo_pcb->registros->dx = 0;
    nuevo_pcb->registros->pc= 0;
    nuevo_pcb->registros->eax= 0;
    nuevo_pcb->registros->ebx= 0;
    nuevo_pcb->registros->ecx= 0;
    nuevo_pcb->registros->edx= 0;
    nuevo_pcb->registros->si= 0;
    nuevo_pcb->registros->di= 0;
    pthread_mutex_init(&nuevo_pcb->mutex_pcb,NULL);

    pthread_mutex_lock(&mutex_cola_general_de_procesos);
    queue_push(cola_general_de_procesos,nuevo_pcb); // Agrego a la cola de todos los procesos el nuevo PCB
    pthread_mutex_unlock(&mutex_cola_general_de_procesos);


    log_info(log_kernel, "Se crea el proceso %d en NEW", nuevo_pcb->pid); // Log obligatorio
    
    //Le mandamos el path de las instrucciones a memoria
    t_paquete* paquete_path = crear_paquete_personalizado(CREACION_PROCESO_KERNEL_A_MEMORIA);
    agregar_string_al_paquete_personalizado(paquete_path, path);
    agregar_int_al_paquete_personalizado(paquete_path, nuevo_pcb->pid);
    enviar_paquete(paquete_path, conexion_kernel_memoria);
    eliminar_paquete(paquete_path);

    // Si el grado de multiprogramacion me lo permite, modifico el estado a READY
    int multiprogramacion_actual;
    sem_getvalue(&sem_multiprogramacion, &multiprogramacion_actual); // Obtengo valor del grado de multiprogramacion
    
    if (multiprogramacion_actual > 0 )
    {   
        //Chequeo si tengo lugar para aceptar otro proceso en base al grado de multiprogramacion actual q tengo
        
        // Modifico el estado del proceso - Uso semaforo porque es una variable que tocan muchos hilos
        pthread_mutex_lock(&nuevo_pcb->mutex_pcb);
        nuevo_pcb->estado_del_proceso = READY; 
        pthread_mutex_unlock(&nuevo_pcb->mutex_pcb);

        // Ingreso el proceso a la cola de READY - Tambien semaforo porque es seccion critica 
        pthread_mutex_lock(&mutex_cola_de_ready);
        queue_push(cola_de_ready,nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_ready);
        sem_post(&sem_cola_de_ready); //Agrego 1 al semaforo contador
        
        //Bajo el grado de programacion actual, ya que agregue un proceso mas a ready
        sem_wait(&sem_multiprogramacion); //Resto 1 al grado de multiprogramacion

    }else {
        // Si no hay espacio para un nuevo proceso en ready lo sumo a la cola de NEW - Semaforo SC
        pthread_mutex_lock(&mutex_cola_de_new);
        queue_push(cola_de_new,nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_new);
        sem_post(&sem_cola_de_new);
    }
}

// ************* MUESTRA EN FORMA DE TABLA EN QUÉ ESTADO SE ENCUENTRA CADA PROCESO ACTUALMENTE ************* 
void listar_procesos_por_estado(){
    
    // Armo colas auxiliares de cada estado para mostrar por pantalla, asi pueden seguir las colas normales sin problema
    t_queue* cola_aux_ready = queue_create();
    t_queue* cola_aux_execute = queue_create();
    t_queue* cola_aux_blocked = queue_create();
    t_queue* cola_aux_exit = queue_create();
    t_queue* cola_aux_new = queue_create();

    pthread_mutex_lock(&mutex_cola_general_de_procesos); // Bloqueo la cola general
    
    pcb* primer_pcb_cola_gral = queue_pop(cola_general_de_procesos); // Saco el 1er PCB de la cola gral

    pcb* aux = primer_pcb_cola_gral; // Esta variable tendrá los procesos de la cola

    pcb* primero_aux = queue_peek(cola_general_de_procesos);
    
    // Este while recorre la cola gral, agregando cada proceso a la cola aux q corresponda
    
    while(primer_pcb_cola_gral != primero_aux) // Salgo cuando dí toda la vuelta
    {
        switch(aux->estado_del_proceso)
        {
            case READY:
                queue_push(cola_aux_ready, aux);
                break;
            case EXECUTE:
                queue_push(cola_aux_execute, aux);
                break;
            case BLOCKED:
                queue_push(cola_aux_blocked, aux);
                break;
            case EXIT:
                queue_push(cola_aux_exit, aux);
                break;
            case NEW:
                queue_push(cola_aux_new, aux);
                break;
        }
        primero_aux = queue_peek(cola_general_de_procesos);
        queue_push(cola_general_de_procesos, aux); // Agrego el q saque de la cola gral
        aux = queue_pop(cola_general_de_procesos); // Saco el proximo de la cola
    }

    queue_push(cola_general_de_procesos, aux); // Agrego el ultimo q saque (no entró al while)
    pthread_mutex_unlock(&mutex_cola_general_de_procesos); // Desbloqueo la cola general

    // Recorro cada cola y hago los logs de cada una

    log_info(log_kernel, "Procesos en estado NEW: \n");
    while(queue_is_empty(cola_aux_new)!= true)
    {
        aux = queue_pop(cola_aux_new);
        log_info(log_kernel, "PID: %d\n" , aux->pid );
    }

    log_info(log_kernel, "Procesos en estado READY: \n");
    while(queue_is_empty(cola_aux_ready)!= true)
    {
        aux = queue_pop(cola_aux_ready);
        log_info(log_kernel, "PID: %d\n" , aux->pid );
    }

    log_info(log_kernel, "Procesos en estado BLOCKED: \n");
    while(queue_is_empty(cola_aux_blocked)!= true)
    {
        aux = queue_pop(cola_aux_blocked);
        log_info(log_kernel, "PID: %d\n" , aux->pid );
    }

    log_info(log_kernel, "Procesos en estado EXIT: \n");
    while(queue_is_empty(cola_aux_exit)!= true)
    {
        aux = queue_pop(cola_aux_exit);
        log_info(log_kernel, "PID: %d\n" , aux->pid );
    }

    log_info(log_kernel, "Procesos en estado EXECUTE: \n");
    while(queue_is_empty(cola_aux_execute)!= true)
    {
        aux = queue_pop(cola_aux_execute);
        log_info(log_kernel, "PID: %d\n" , aux->pid );
    }

    // Libero las colas auxiliares
    queue_destroy(cola_aux_exit);
    queue_destroy(cola_aux_ready);
    queue_destroy(cola_aux_blocked);
    queue_destroy(cola_aux_execute);
    queue_destroy(cola_aux_new);
}

// ************* CAMBIA EL GRADO DE MULTIPROGRAMCION DEL SISTEMA POR EL ENVIADO POR PARAMETRO ************* 
void cambiar_grado_multiprogramacion(char* nuevo_valor_formato_char){
    
    int nuevo_valor = atoi(nuevo_valor_formato_char); // Se pasa el nuevo valor que es un string a tipo int
    config_kernel->grado_multiprogramacion = nuevo_valor; // Asigno el nuevo grado de multiprogramación
    
    int valor_semaforo;
    sem_getvalue(&sem_multiprogramacion, &valor_semaforo); // Obtengo el valor actual del semáforo del grado de multiprogramación y lo guardo en valor_semaforo

    while (valor_semaforo > nuevo_valor) // Mientras que el valor de multiprogramación sea mayor que el nuevo
    {
        sem_wait(&sem_multiprogramacion); // Le voy restando de a uno al semáforo del grado de multiprogramación
        sem_getvalue(&sem_multiprogramacion, &valor_semaforo);
    } 
    // Hago eso hasta que el semáforo quede con el mismo valor que el nuevo determinado por parámetro
}

// ************* FINALIZA EL PROCESO SELECCIONADO ************* 
void finalizar_proceso(char* pid_formato_char){

    int pid = atoi(pid_formato_char); // Paso a int el pid mandado por consola

    pthread_mutex_lock(&mutex_cola_general_de_procesos); // Bloqueo la cola gral para sacar el proceso q paso a exit

    pcb* proceso_cola_gral = queue_pop(cola_general_de_procesos); // Saco el primer elemento de la cola

    while(pid != proceso_cola_gral->pid){ // Mientras el valor del pid del proceso obtenido no sea el mismo que el que quiero finalizar, entonces sigo

        queue_push(cola_general_de_procesos, proceso_cola_gral); // Voy guardando y poniendo el proceso
        proceso_cola_gral = queue_pop(cola_general_de_procesos); // Hasta que obtenga el que quiero
    }

    queue_push(cola_general_de_procesos, proceso_cola_gral); // Meto de nuevo el proceso que voy a pasar a exit, porque ahí se encargará de sacarlo de la cola

    pthread_mutex_unlock(&mutex_cola_general_de_procesos); // Desbloqueo la cola gral

    pasar_proceso_a_exit(proceso_cola_gral); // Paso el proceso a exit, ahí lo saco de la cola y libero recursos

}

// ************* EJECUTA UN SCRIPT DE COMANDOS ************* 
// Tengo que terminar los ifs cuando termine todos los comandos
void ejecutar_script(char* script_path){
    
    FILE *archivo; // Declaro un archivo
    char linea[100]; // Declaro un tamaño de la línea de 100 caracteres

    archivo = fopen(path, "r") // Abro el archivo en modo lectura
    
    if(archivo == NULL){ // Si no lo puedo abrir porque no existe o está mal el path
        error_show("Error al abrir archivo de comandos"); // Entonces muestro un error
        return;
    }

    while (fgets(linea, sizeof(linea), archivo) != NULL) { // Voy obteniendo las líneas del archivo una por una

        linea[strcspn(linea, "\n")] = 0; // Obtengo la longitud de la línea hasta antes que haya un "\n", y ahí, lo reemplazo por un caracter nulo (elimino el salto de línea)

        char **partes = string_split(linea, " "); 

        // Dependiendo el comando llamo a la función correspondiente
        if (strcmp(partes[0], "INICIAR_PROCESO") == 0) { 

            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
            iniciar_proceso(partes[1]); // Esta es la funcion a la que llamo

        } else if (strcmp(linea, "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");
            finalizar_proceso(partes[1]); // Acá mando el PID del proceso que quiero finalizar

        }else if (strcmp(partes[0], "PROCESO_ESTADO") == 0){
            printf("Ha seleccionado la opción PROCESO_ESTADO\n");
            listar_procesos_por_estado();
        
        }else if (strcmp(partes[0], "MULTIPROGRAMACION") == 0){
            printf("Ha seleccionado la opción MULTIPROGRAMACION\n");
            cambiar_grado_multiprogramacion(partes[1]);
        }else if{
            printf("Ha seleccionado la opción EJECUTAR_SCRIPT\n");
            ejecutar_script(partes[1]);
        }else {
            
            printf("Opción no válida\n");
        }

        liberar_array_strings(partes); // Libero la memoria asignada por string_split después de usarla

    }

    fclose(archivo); // Cierro el archivo
    
}


// Estos son los comandos por consola que faltan hacer
void detener_planificacion();
void iniciar_planificacion();
    
// ----------------------------------------------------------------------------------------
// ------------- FIN FUNCIONES PROPIAS DE LOS COMANDOS INGRESADOS POR CONSOLA -------------
// ----------------------------------------------------------------------------------------






// -------------------------------------------------------------------------
// ------------- INICIO FUNCIONES PARA ENVIAR UN PROCESO A CPU -------------
// -------------------------------------------------------------------------

// ************* ARMA EL PAQUETE NECESARIO PARA ENVIAR EL PCB A CPU *************
void enviar_pcb(pcb* proceso) {

    // Funcion llamada por enviar_proceso_a_cpu

    t_paquete* paquete_pcb = crear_paquete_personalizado(PCB_KERNEL_A_CPU); // Creo un paquete personalizado con un codop para que CPU reconozca lo que le estoy mandando

    agregar_estructura_al_paquete_personalizado(paquete_pcb, proceso, sizeof(pcb)); // Agrego el struct pcb al paquete

    enviar_paquete(paquete_pcb, conexion_kernel_cpu); // Envio el paquete a través del socket

    eliminar_paquete(paquete_pcb); // Libero el paquete
}

// ----------------------------------------------------------------------
// ------------- FIN FUNCIONES PARA ENVIAR UN PROCESO A CPU -------------
// ----------------------------------------------------------------------





// ---------------------------------------------------------------------------
// ------------- INICIO FUNCIONES PARA RECIBIR UN PROCESO DE CPU -------------
// ---------------------------------------------------------------------------

// ENTENDER EL CANCEL DE ESTE

// ************* CREA HILOS SEGUN EL ALGORITMO DE PLANIFICACION Y LLAMA FUNCIONES PARA MANEJAR LA RECEPCION DEL PCB *************
void crear_hilo_proceso(pcb* proceso){

    // Funcion llamada por enviar_proceso_a_cpu

    pthread_t hilo_recibe_proceso, hilo_interrupcion; // Creo un hilo
    thread_args_procesos_kernel args_hilo = {proceso}; // En sus args le cargo el proceso

    if(strcmp(config_kernel->algoritmo_planificacion, "FIFO") == 0)
    {
        // Si el algoritmo de planificacion es FIFO entonces recibo el pcb normalmente desde CPU

        pthread_create(&hilo_recibe_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); // Recibir un pcb normalmente, sin interrupciones
        pthread_join(hilo_recibe_proceso, NULL); // No sigo hasta que no haya terminado de recibirlo, y ahí libero el hilo
    }
    else if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0 || strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
    {

        // Si el algoritmo de planificacion es RR o VRR entonces permito la posibilidad de recibirlo por interrupcion de fin de quantum

        pthread_create(&hilo_recibe_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); // Recibir pcb normalmente
        pthread_create(&hilo_interrupcion, NULL, (void*)algoritmo_round_robin,(void*)&args_hilo); // Recibir pcb por interrupcion de fin de quantum
        
        pthread_join(hilo_recibe_proceso, NULL); // Espero que termine de recibir normalmente
        
        sem_wait(&destruir_hilo_interrupcion);
        pthread_cancel(hilo_interrupcion); 

    }else {
        log_error(log_kernel, "Estan mal las configs capo");
    }
}

// ************* LLAMA A RECIBIR_PCB (RECIBO NORMAL) A TRAVES DE UN HILO *************
void recibir_pcb_hilo(void* arg){
    
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg; // Creo un hilo
    pcb* proceso = args->proceso; // Le cargo el proceso en los args

    recibir_pcb(proceso);
}

// NO ME CONVENCE / NO TERMINO DE ENTENDER EL CANCEL DEL HILO DE INTERRUPCION

// ************* RECIBE EL PCB NORMALMENTE (SIN INTERRUPCION) SEGUN ALGORITMO *************
void recibir_pcb(pcb* proceso) {
    
    clock_t inicio, fin; // Inicio un reloj, cuenta el tiempo que estuvo esperando hasta que llegue el paquete (sirve para VRR)
    int tiempo_que_tardo_en_recibir;

    inicio = clock(); // En este momento comienzo a esperar

    int codigo_operacion = recibir_operacion(conexion_kernel_cpu); // Recibo el codigo de operacion para ver como actúo según eso

    int flag_estado; // Declaro un flag que me va a servir para manejar el estado del proceso posteriormente
    
    t_buffer* buffer; // Buffer para recibir el paquete
    
    switch(codigo_operacion) // Segun el codigo de operacion actuo 
    {
        case PCB_CPU_A_KERNEL: 

            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
            fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
            flag_estado = 0; // El proceso todavia no termino

            break;

        case CPU_TERMINA_EJECUCION_PCB:

            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
            fin = clock(); // Termino el tiempo desde que empecé a esperar la recepción
            flag_estado = 1; // El proceso ya finalizo, no quedan rafagas por ejecutar

            break;

        case SIGNAL:

            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
            fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 

            break;

        case WAIT:
     
            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
            fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 

            break;
        
        default:
            log_error(log_kernel, "El codigo de operacion no es reconocido :(");
            break;
    }

    tiempo_que_tardo_en_recibir = (int)((double)(fin - inicio) * 1000.0 / CLOCKS_PER_SEC); // Calculo el tiempo que me tarde en recibir el PCB

    if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0 || strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
    {
        //VER SI FUNCA
        sem_post(&destruir_hilo_interrupcion);
    }

    proceso = recibir_estructura_del_buffer(buffer); // Asigno al proceso lo que viene del buffer

    // Si estoy haciendo un Wait o Signal, entonces tambien tengo que recibir el indice del recurso necesitado para poder manejarlo

    if(codigo_operacion == WAIT)
    {
        int indice_recurso = recibir_int_del_buffer(buffer); // Obtengo el indice del recurso que se usa para manejarlo
        flag_estado = hacer_wait(indice_recurso, proceso); // Asigno un flag que sirve para manejar el estado del proceso

    }else if(codigo_operacion == SIGNAL)
    {
        int indice_recurso = recibir_int_del_buffer(buffer); // Obtengo el indice del recurso que se usa para manejarlo
        flag_estado = hacer_signal(indice_recurso, proceso); // Asigno un flag que sirve para manejar el estado del proceso
    }
    
    if(strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0) // Si estoy recibiendo a traves del algoritmo VRR
    {
        proceso->quantum = proceso->quantum - tiempo_que_tardo_en_recibir; // Le asigno el quantum que le queda disponible
    };

    log_info(log_kernel, "Recibi PID: %d", proceso->pid); 
    
    free(buffer->stream); // Libero directamente el buffer, no arme paquete asi que no hace falta
    free(buffer);

    sem_post(&sem_puedo_mandar_a_cpu); // Aviso que ya volvio el proceso que estaba en CPU, puedo mandar otro

    accionar_segun_estado(proceso, flag_estado); // Mando el proceso a Ready o Exit segun corresponda
    
    return;
}

// NO ENTIENDO BIEN EL TEMA DEL HILO DE INTERRUPCION NI EL FUNCIONAMIENTO DE ESTA FUNCION - VER DESPUES

void algoritmo_round_robin (void* arg){

    //aca vamos a armar dos hilos, uno que maneje solo cuando mandarle
    //la interrupcion y otro que siemp este esperando a q cpu le mande el pcb
    thread_args_procesos_kernel*args = (thread_args_procesos_kernel*)arg;
    pcb* proceso_actual = args->proceso;

    usleep(proceso_actual->quantum); //aca usamos el quantum del proceso, asi podemos reutilziar la funcion para VRR
    desalojar_proceso_hilo(args);
    
    return;
}

// ************* LLAMA A DESALOJAR_PROCESO (NOTIFICAR INTERRUPCION) A TRAVES DE UN HILO *************
void desalojar_proceso_hilo(void* arg){
    
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg; // Creo un hilo
    pcb* proceso = args->proceso; // Le cargo el proceso en los args

    desalojar_proceso(proceso); 
}

// ************* NOTIFICA A CPU QUE HAY UNA INTERRUPCION Y DEBE MANDAR INMEDIATAMENTE EL PCB DE REGRESO *************
void desalojar_proceso(pcb* proceso){
   
    //Esto capaz se puede poner desde donde se la llama a la funcion
    log_info(log_kernel, "PID: %d - Desalojado por fin de Quantum", proceso->pid);

    t_paquete* paquete_a_enviar = crear_paquete_personalizado(INTERRUPCION); // Creo un paquete con el codop especifico
    enviar_paquete(paquete_a_enviar, interrupcion_kernel_cpu); // Lo mando vacio porque lo que interesa es el codop

    eliminar_paquete(paquete_a_enviar); // Libero el paquete

    return;
}

// ------------------------------------------------------------------------
// ------------- FIN FUNCIONES PARA RECIBIR UN PROCESO DE CPU -------------
// ------------------------------------------------------------------------




// --------------------------------------------------------------------
// ------------- INICIO FUNCIONES PARA MANEJAR UN PROCESO -------------
// --------------------------------------------------------------------

// ************* SEGUN EL FLAG QUE TENGA EL PROCESO VOY A CAMBIARLE EL ESTADO *************
void accionar_segun_estado(pcb* proceso, int flag){

    //flag =  1, ya ejecutó todo, tengo q pasarlo a exit
    //flag =  0, aun no ejecutó del todo, lo mando a la cola de ready
    //flag =  2, tengo que bloquear el proceso
    //flag = -1, no hago nada, ya lo hicieron 
    
    if(flag == 2){
        pasar_proceso_a_blocked(proceso);
    }
    else if(flag == 1){
        pasar_proceso_a_exit(proceso);
    }else if (flag == 0)
    { 
        char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
        pthread_mutex_lock(&proceso->mutex_pcb);
        proceso->estado_del_proceso = READY; 
        pthread_mutex_unlock(&proceso->mutex_pcb);

        if(strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
        {
            if(proceso->quantum > 0) // Si todavía me queda quantum disponible
            {
                pthread_mutex_lock(&mutex_cola_prioridad_vrr); 
                queue_push(cola_prioridad_vrr,proceso); // Entonces voy a la cola de prioridad
                pthread_mutex_unlock(&mutex_cola_prioridad_vrr);
                sem_post(&sem_cola_prioridad_vrr);

                log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY", proceso->pid, estado_anterior);

            }else{ // Si no le queda quantum

                proceso->quantum = config_kernel->quantum; // Le reinicio el quantum
                pthread_mutex_lock(&mutex_cola_de_ready);
                queue_push(cola_de_ready,proceso); // Y la sumo a la cola de Ready normal
                pthread_mutex_unlock(&mutex_cola_de_ready);
                sem_post(&sem_cola_de_ready);

                log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY", proceso->pid, estado_anterior);

            }
        }else{ // No estoy en vrr, siempre madno a cola de Ready normal
            
            pthread_mutex_lock(&mutex_cola_de_ready);
            queue_push(cola_de_ready,proceso); // Meto a la cola de Ready
            pthread_mutex_unlock(&mutex_cola_de_ready);
            sem_post(&sem_cola_de_ready);

            log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY", proceso->pid , estado_anterior);

        };
    }
    return;
}

// ************* PASA EL PROCESO SELECCIONADO A EXIT *************
void pasar_proceso_a_exit(pcb* proceso){

    //En esta funcion faltan liberar los recursos q tenia el proceso
    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso); // Devuelvo el estado como string

    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = EXIT; // Asigno el estado del proceso como Exit
    pthread_mutex_unlock(&proceso->mutex_pcb);

    while(queue_is_empty(proceso->recursos_asignados) == false) // Mientras la cola de recursos asignados no esta vacia
    {
        int recurso_a_liberar = (int)(intptr_t)queue_peek(proceso->recursos_asignados); // Entonces libero los recursos corresopndientes 
        hacer_signal(recurso_a_liberar, proceso);
    }
    
    pthread_mutex_lock(&mutex_cola_general_de_procesos); // Bloqueo la cola gral para sacar el proceso q paso a exit

    pcb* primer_pcb_cola_gral = queue_pop(cola_general_de_procesos);

    pcb* aux = primer_pcb_cola_gral;

    while(aux->pid != proceso->pid)
    {
        queue_push(cola_general_de_procesos, aux); // Agrego el q saque de la cola gral
        aux = queue_pop(cola_general_de_procesos); // Saco el proximo de la cola
    }
    pthread_mutex_unlock(&mutex_cola_general_de_procesos); // Desbloqueo la cola gral

    log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: EXIT", proceso->pid, estado_anterior);

    free(proceso->registros);
    free(proceso); 
    if(queue_size(cola_general_de_procesos) < config_kernel->grado_multiprogramacion)
    {
        sem_post(&sem_multiprogramacion);//agrego 1 al grado de multiprogramacion solo si puedo
    }
     
}

// ************* PASA EL PROCESO SELECCIONADO A BLOCKED *************
void pasar_proceso_a_blocked(pcb* proceso){
    
    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = BLOCKED; 
    pthread_mutex_unlock(&proceso->mutex_pcb);

    log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: BLOCKED", proceso->pid, estado_anterior);
}

// -----------------------------------------------------------------
// ------------- FIN FUNCIONES PARA MANEJAR UN PROCESO -------------
// -----------------------------------------------------------------





// -------------------------------------------------------------
// ------------- INICIO FUNCIONES DE INSTRUCCIONES -------------
// -------------------------------------------------------------

// ************* REALIZA EL SIGNAL DE UN RECURSO *************

int hacer_signal(int indice_recurso, pcb* proceso){

    int flag;

    if(indice_recurso < cantidad_recursos) // Chequeo que existe el recurso
    {
        ++ config_kernel->instancias_recursos[indice_recurso]; // Sumo una instancia al recurso

        if(config_kernel->instancias_recursos[indice_recurso] <= 0){ // Verifico si el proceso está bloqueado o no, para desbloquearlo si hiciera falta

            flag = 0; // Esto es porque el proceso que llama al signal tiene que quedar en ready
            
            pthread_mutex_lock(mutex_por_recurso[indice_recurso]);
            pcb* proceso_desbloqueado = queue_pop(colas_por_recurso[indice_recurso]); // Saco de la cola de blocked de ese recurso
            pthread_mutex_unlock(mutex_por_recurso[indice_recurso]);

            queue_push(proceso_desbloqueado->recursos_asignados, (void*)(intptr_t)indice_recurso); // Agrego el recurso a la cola de recursos asignados del proceso
            
            accionar_segun_estado(proceso_desbloqueado , 0); // Con esto es con lo que desbloqueo lo que estaba bloqueado
            
        }else flag = 0; // Debo pasar este proceso de nuevo a ready, el signal no lo bloquea

        //Saco de la cola de recursos asignados del proceso el recurso
        int primer_recurso_asignado = (int)(intptr_t)queue_pop(proceso->recursos_asignados); // Acá podría fallar si no hice un wait antes, podría no haber nada en la cola
        int recurso_aux = primer_recurso_asignado;
        while(recurso_aux != indice_recurso) // Acá se está recorriendo la cola, hasta que saco el recurso que quiero, si no tengo el que quiero lo vuelvo a meter en la cola
        {
            queue_push(proceso->recursos_asignados, (void*)(intptr_t)recurso_aux);
            recurso_aux = (int)(intptr_t)queue_pop(proceso->recursos_asignados);
        }

    } else flag = 1;// no existe,  mando a exit el proceso

    return flag;
}

// ************* REALIZA EL WAIT DE UN RECURSO *************
int hacer_wait(int indice_recurso, pcb* proceso){
    int flag;

    if(indice_recurso < cantidad_recursos){ // Si existe el recurso solicitado
        
        -- config_kernel->instancias_recursos[indice_recurso]; // Quito una instancia de ese recurso

        if(config_kernel->instancias_recursos[indice_recurso] < 0){ // Si no tengo más instancias disponibles, entonces bloqueo el proceso
            
            flag = -1; // En accionar_segun_estado no hara nada porque ya lo bloqueo aca

            char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
            pthread_mutex_lock(&proceso->mutex_pcb);
            proceso->estado_del_proceso = BLOCKED; // Bloqueo el proceso
            pthread_mutex_unlock(&proceso->mutex_pcb);

            log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: BLOCKED", proceso->pid, estado_anterior);
            
            
            pthread_mutex_lock(mutex_por_recurso[indice_recurso]);
            queue_push(colas_por_recurso[indice_recurso], proceso); // Mando a la cola de blocked de ese recurso
            pthread_mutex_unlock(mutex_por_recurso[indice_recurso]);
            
        }else{ // Si no lo tengo que bloquear, entonces lo mando a Ready

            flag = 0; //flag 0 para que vuelva a la cola de ready
            queue_push(proceso->recursos_asignados, (void*)(intptr_t)indice_recurso); // Agrego el recurso a la cola de recursos asignados
        } 
         
    }else flag = 1; // Entonces no existe el recurso solicitado, y mando a exit al proceso
    
    return flag;
}

// ----------------------------------------------------------
// ------------- FIN FUNCIONES DE INSTRUCCIONES -------------
// ----------------------------------------------------------





// -----------------------------------------------------------------
// ------------- INICIO FUNCIONES PARA LIBERAR MEMORIA -------------
// -----------------------------------------------------------------

// ************* LIBERA LA MEMORIA DE "PARTES" QUE SIRVE PARA DIVIDIR UN TEXTO *************
void liberar_array_strings(char **array) {
    
    if (array == NULL) return; // Si ya es null, entonces termino la función
    
    for (int i = 0; array[i] != NULL; i++) { // Sino voy recorriendo y liberando
        free(array[i]);
    }
    free(array);
}

// --------------------------------------------------------------
// ------------- FIN FUNCIONES PARA LIBERAR MEMORIA -------------
// --------------------------------------------------------------





// ---------------------------------------------------
// ------------- INICIO FUNCIONES VARIAS -------------
// ---------------------------------------------------

// ************* DEVUELVE UN CHAR* (STRING) AL PASARLE UN ENUM DE ESTADOS *************
char* obtener_char_de_estado(estados estado_a_convertir){
    switch(estado_a_convertir)
    {
        case READY:
            return "READY";
        case BLOCKED:
            return "BLOCKED";
        case EXECUTE:
            return "EXECUTE";
        case EXIT:
            return "EXIT";
        case NEW:
            return "NEW";
        default:
            return "No entiendo";
    }

}

// ------------------------------------------------
// ------------- FIN FUNCIONES VARIAS -------------
// ------------------------------------------------

