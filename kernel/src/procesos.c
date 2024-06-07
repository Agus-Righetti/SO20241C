#include "procesos.h"





// -----------------------------------------------------------------------------------------
// ------------- ACÁ EMPIEZAN LOS HILOS QUE SE QUEDAN CONSTANTEMENTE PRENDIDOS -------------
// -----------------------------------------------------------------------------------------

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

// -----------------------------------------------------------------------------------------
// ------------- ACÁ TERMINAN LOS HILOS QUE SE QUEDAN CONSTANTEMENTE PRENDIDOS -------------
// -----------------------------------------------------------------------------------------





// ------------------------------------------------------------------------------------------------------
// ------------- ACÁ EMPIEZAN LAS FUNCIONES LLAMADAS POR LOS HILOS CONSTANTEMENTE PRENDIDOS -------------
// ------------------------------------------------------------------------------------------------------

// ************* SIRVE PARA LEER LOS COMANDOS DESDE LA CONSOLA (DESDE HILO_CONSOLA) *************
void leer_consola(void* arg){
    
     while(1) { // Para mantener esperando comandos constantemente
        
        char* lectura = readline("Ingrese comando: "); // Pido que se ingrese un comando

        // partes me separa segun los espacios lo que hay, en partes[0] esta INICIAR_PROCESO
        // en partes[1] va a estar el path
        char **partes = string_split(lectura, " "); 

        // Dependiendo el comando llamo a la funcion correspondiente
        if (strcmp(partes[0], "INICIAR_PROCESO") == 0) { 

            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
            iniciar_proceso(partes[1]); // Esta es la funcion a la que llamo

        } else if (strcmp(lectura, "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");

        }else if (strcmp(partes[0], "PROCESO_ESTADO") == 0){
            printf("Ha seleccionado la opción PROCESO_ESTADO\n");
            listar_procesos_por_estado();
        
        }else if (strcmp(partes[0], "MULTIPROGRAMACION") == 0){
            printf("Ha seleccionado la opción MULTIPROGRAMACION\n");
            cambiar_grado_multiprogramacion(partes[1]);
        }else {
            
            printf("Opción no válida\n");
        }

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

            crear_hilo_proceso (proceso_seleccionado); // Inicio un hilo que maneje la ejecucion del proceso

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

// ------------------------------------------------------------------------------------------------------
// ------------- ACÁ TERMINAN LAS FUNCIONES LLAMADAS POR LOS HILOS CONSTANTEMENTE PRENDIDOS -------------
// ------------------------------------------------------------------------------------------------------





// -----------------------------------------------------------------------------------------------------
// ------------- ACÁ EMPIEZAN LAS FUNCIONES PROPIAS DE LOS COMANDOS INGRESADOS POR CONSOLA -------------
// -----------------------------------------------------------------------------------------------------

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
    queue_push(cola_general_de_procesos,nuevo_pcb); //agrego a la cola de todos los procesos el nuevo PCB
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

//************* Esta funcion mostrara e forma de tabla cada proceso en que estado esta actualmente ************* 
void listar_procesos_por_estado()
{
    //armo colas auxiliares para mostrar por pantalla, asi pueden seguir las colas normales sin problema
    t_queue* cola_aux_ready = queue_create();
    t_queue* cola_aux_execute = queue_create();
    t_queue* cola_aux_blocked = queue_create();
    t_queue* cola_aux_exit = queue_create();
    t_queue* cola_aux_new = queue_create();


    pthread_mutex_lock(&mutex_cola_general_de_procesos); //bloqueo la cola gral
    
    pcb* primer_pcb_cola_gral = queue_pop(cola_general_de_procesos); //saco el 1er PCB de la cola gral

    pcb* aux = primer_pcb_cola_gral; //esta variable tendra los procesos de la cola

    pcb* primero_aux = queue_peek(cola_general_de_procesos);
    //este while recorre la cola gral, agregando cada proceso a la cola aux q corresponda
    while(primer_pcb_cola_gral != primero_aux)
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
        queue_push(cola_general_de_procesos, aux); //agrego el q saque de la cola gral
        aux = queue_pop(cola_general_de_procesos); //saco el proximo de la cola
    }

    queue_push(cola_general_de_procesos, aux); //agrego el ultimo q saque (no entro al while)
    pthread_mutex_unlock(&mutex_cola_general_de_procesos); //desbloqueo la cola gral

    //hago los logs de cada cola
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

    //libero las colas auxiliares
    
    queue_destroy(cola_aux_exit);
    queue_destroy(cola_aux_ready);
    queue_destroy(cola_aux_blocked);
    queue_destroy(cola_aux_execute);
    queue_destroy(cola_aux_new);


}

//************* Falta el comentario de esta ************* 
void cambiar_grado_multiprogramacion(char* nuevo_valor_formato_char)
{
    int nuevo_valor = atoi(nuevo_valor_formato_char);
    config_kernel->grado_multiprogramacion = nuevo_valor;
    int valor_semaforo;
    sem_getvalue(&sem_multiprogramacion, &valor_semaforo);
    while (valor_semaforo > nuevo_valor)
    {
        sem_wait(&sem_multiprogramacion);
        sem_getvalue(&sem_multiprogramacion, &valor_semaforo);
    }
}

// -----------------------------------------------------------------------------------------------------
// ------------- ACÁ TERMINAN LAS FUNCIONES PROPIAS DE LOS COMANDOS INGRESADOS POR CONSOLA -------------
// -----------------------------------------------------------------------------------------------------





// ------------------------------------------------------------------------------------
// ------------- ACÁ COMIENZAN LAS FUNCIONES PARA ENVIAR UN PROCESO A CPU -------------
// ------------------------------------------------------------------------------------

// ************* ARMA EL PAQUETE NECESARIO PARA ENVIAR EL PCB A CPU *************
void enviar_pcb(pcb* proceso) {

    t_paquete* paquete_pcb = crear_paquete_personalizado(PCB_KERNEL_A_CPU); // Creo un paquete personalizado con un codop para que CPU reconozca lo que le estoy mandando

    agregar_estructura_al_paquete_personalizado(paquete_pcb, proceso, sizeof(pcb)); // Agrego el struct pcb al paquete

    enviar_paquete(paquete_pcb, conexion_kernel_cpu); // Envio el paquete a través del socket

    eliminar_paquete(paquete_pcb); // Libero el paquete
}

// -----------------------------------------------------------------------------------
// ------------- ACÁ TERMINAN LAS FUNCIONES PARA ENVIAR UN PROCESO A CPU -------------
// -----------------------------------------------------------------------------------





// --------------------------------------------------------------------------------------
// ------------- ACÁ COMIENZAN LAS FUNCIONES PARA RECIBIR UN PROCESO DE CPU -------------
// --------------------------------------------------------------------------------------


// -------------------------------------------------------------------------------------
// ------------- ACÁ TERMINAN LAS FUNCIONES PARA RECIBIR UN PROCESO DE CPU -------------
// -------------------------------------------------------------------------------------





// ************* Funcion que sirve para crear un hilo por el proceso enviado, y segun el algoritmo recibirlo por FIFO o RR *************
void crear_hilo_proceso(pcb* proceso){

    // Creo un hilo
    pthread_t hilo_recibe_proceso, hilo_interrupcion;
    thread_args_procesos_kernel args_hilo = {proceso};

    // Si el algoritmo de planificacion es FIFO entonces recibo el pcb normalmente desde CPU
    if(strcmp(config_kernel->algoritmo_planificacion, "FIFO") == 0)
    {
        pthread_create(&hilo_recibe_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); 
        pthread_join(hilo_recibe_proceso, NULL);
    }
    else if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0 || strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
    {
        pthread_create(&hilo_recibe_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); 
        pthread_create(&hilo_interrupcion, NULL, (void*)algoritmo_round_robin,(void*)&args_hilo); 
        
        pthread_join(hilo_recibe_proceso, NULL);
        sem_wait(&destruir_hilo_interrupcion);
        pthread_cancel(hilo_interrupcion);

    }else log_error(log_kernel, "Estan mal las configs capo");
    
}

void algoritmo_round_robin (void* arg){

    //aca vamos a armar dos hilos, uno que maneje solo cuando mandarle
    //la interrupcion y otro que siemp este esperando a q cpu le mande el pcb
    thread_args_procesos_kernel*args = (thread_args_procesos_kernel*)arg;
    pcb* proceso_actual = args->proceso;

    usleep(proceso_actual->quantum); //aca usamos el quantum del proceso, asi podemos reutilziar la funcion para VRR
    desalojar_proceso_hilo(args);
    
    return;
}

void accionar_segun_estado(pcb* proceso, int flag){

    //flag = 1, ya ejecuto todo, tengo q pasarlo a exit
    //flag = 0 aun no ejecuto del todo, lo mando a la cola de ready
    //flag = 2, entonces tengo que bloquear el proceso
    //flag = -1 no hago nada, ya lo hicieron 
    
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
            if(proceso->quantum > 0)
            {
                //si aun le queda quantum disponible se ira a la cola de prioridad
                pthread_mutex_lock(&mutex_cola_prioridad_vrr);
                queue_push(cola_prioridad_vrr,proceso);
                pthread_mutex_unlock(&mutex_cola_prioridad_vrr);
                sem_post(&sem_cola_prioridad_vrr);
                log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY", proceso->pid, estado_anterior);


            }else{
                //no le queda quantum, ira a ready normal
                proceso->quantum = config_kernel->quantum; // le vuelvo a asignar todo el quantum
                pthread_mutex_lock(&mutex_cola_de_ready);
                queue_push(cola_de_ready,proceso);
                pthread_mutex_unlock(&mutex_cola_de_ready);
                sem_post(&sem_cola_de_ready);
                log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY", proceso->pid, estado_anterior);

            }
        }else{ // no estoy en vrr, siempre madno a cola de ready normal
            pthread_mutex_lock(&mutex_cola_de_ready);
            queue_push(cola_de_ready,proceso);
            pthread_mutex_unlock(&mutex_cola_de_ready);
            sem_post(&sem_cola_de_ready);
            log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: READY", proceso->pid , estado_anterior);

        };

        // Hago un log obligatorio
    }
    
    return;

}

void pasar_proceso_a_exit(pcb* proceso){

    //En esta funcion faltan liberar los recursos q tenia el proceso
    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = EXIT; 
    pthread_mutex_unlock(&proceso->mutex_pcb);

    //libero los recursos que tenia asignado el proceso
    while(queue_is_empty(proceso->recursos_asignados) == false)
    {
        int recurso_a_liberar = (int)(intptr_t)queue_peek(proceso->recursos_asignados);
        hacer_signal(recurso_a_liberar, proceso);
    }
    
    pthread_mutex_lock(&mutex_cola_general_de_procesos); //bloqueo la cola gral para sacar el proceso q paso a exit

    pcb* primer_pcb_cola_gral = queue_pop(cola_general_de_procesos);

    pcb* aux = primer_pcb_cola_gral;

    while(aux->pid != proceso->pid)
    {
        queue_push(cola_general_de_procesos, aux); //agrego el q saque de la cola gral
        aux = queue_pop(cola_general_de_procesos); //saco el proximo de la cola
    }
    pthread_mutex_unlock(&mutex_cola_general_de_procesos); //desbloqueo la cola gral


    log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: EXIT", proceso->pid, estado_anterior);

    free(proceso->registros);
    free(proceso); 
    if(queue_size(cola_general_de_procesos) < config_kernel->grado_multiprogramacion)
    {
        sem_post(&sem_multiprogramacion);//agrego 1 al grado de multiprogramacion solo si puedo
    }
     
}

void pasar_proceso_a_blocked(pcb* proceso){
    
    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = BLOCKED; 
    pthread_mutex_unlock(&proceso->mutex_pcb);

    log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: BLOCKED", proceso->pid, estado_anterior);
}

// ************* Funcion que sirve para obtener de regreso un proceso a traves de una interrupcion *************
void desalojar_proceso(pcb* proceso){
   
    //Esto capaz se puede poner desde donde se la llama a la funcion
    log_info(log_kernel, "PID: %d - Desalojado por fin de Quantum", proceso->pid);

    // Creo un nuevo paquete
    t_paquete* paquete_a_enviar = crear_paquete_personalizado(INTERRUPCION);
    enviar_paquete(paquete_a_enviar, interrupcion_kernel_cpu);

    eliminar_paquete(paquete_a_enviar);


    return;
}

// ************* Funcion que llama a la de arriba a traves de un hilo *************
void desalojar_proceso_hilo(void* arg){
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;
    pcb* proceso = args->proceso;

    desalojar_proceso(proceso); 
}

// ************* Funcion para recibir el pcb desde CPU si termina su ejecucion sin interrupcion *************
void recibir_pcb(pcb* proceso) {
    
   //esta funcion deberia tener un switch para accionar segun el codop q nos mandan, porque para el manejo de recursos necesitamos hacer algo segun el codop 
    //inicio un reloj, va a contar cuanto tiempo estuvo esperando hasta q llegue el paquete (sirve para vrr)
    clock_t inicio, fin;
    int tiempo_que_tardo_en_recibir;

    inicio = clock(); // en este momento arranco a esperar

    int codigo_operacion = recibir_operacion(conexion_kernel_cpu);

    int flag_estado;
    
    t_buffer* buffer;
    
    switch(codigo_operacion)
    {
        case PCB_CPU_A_KERNEL: 

            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);
            fin = clock();
            flag_estado = 0;

            break;

        case CPU_TERMINA_EJECUCION_PCB:
            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);
            flag_estado = 1;

            break;

        case SIGNAL:
            //NECESITAMOS que nos manden desde CPU que recurso
            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);
            fin = clock();
            //flag_estado = 0;

            break;

        case WAIT:
            buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);
            fin = clock();
            //NECESITAMOS que nos manden desde CPU que recurso
            //flag_estado = 0;
            //flag_estado = hacer_wait(); // Hacemos que si devuelve un 2 entonces se manda a la cola de blocked

            break;
        
        default:
            log_error(log_kernel, "El codigo de operacion no es reconocido :(");
            break;
    }

    // Creo un nuevo paquete
    //t_paquete* paquete = crear_paquete_personalizado(PCB_CPU_A_KERNEL);

    // Recibo el paquete a través del socket y los guardo en una lista

    //paquete->buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);

    //fin = clock(); // en este momento termino de esperar
    tiempo_que_tardo_en_recibir = (int)((double)(fin - inicio) * 1000.0 / CLOCKS_PER_SEC);

    if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0 || strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
    {
        //VER SI FUNCA
        sem_post(&destruir_hilo_interrupcion);
    }

    proceso = recibir_estructura_del_buffer(buffer);

    if(codigo_operacion == WAIT)
    {
        int indice_recurso = recibir_int_del_buffer(buffer);
        flag_estado = hacer_wait(indice_recurso, proceso);

    }else if(codigo_operacion == SIGNAL)
    {
        int indice_recurso = recibir_int_del_buffer(buffer);
        //Del recurso nos tienen q mandar el indice
        flag_estado = hacer_signal(indice_recurso, proceso);
    }
    
    if(strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
    {
        proceso->quantum = proceso->quantum - tiempo_que_tardo_en_recibir; //le pongo el quantum que le queda disponible, solo si estoy en vrr que es cuando me importa
    };

    log_info(log_kernel, "Recibi PID: %d", proceso->pid);
    
    //eliminar_paquete(paquete);
    free(buffer->stream);
    free(buffer);

    //Este paquete es para recibir el flag, el flag esta en 1 si el proceso ta ejecuto todo y en 0 si aun le falta

    // t_paquete* paquete_estado = crear_paquete_personalizado(CPU_TERMINA_EJECUCION_PCB);

    // paquete_estado->buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);

    //flag_estado = recibir_int_del_buffer(paquete_estado->buffer);

    sem_post(&sem_puedo_mandar_a_cpu);//aviso que ya volvio el proceso q estaba en CPU, puedo mandar otro

    
    // Libero el paquete

    //eliminar_paquete(paquete_estado);

    accionar_segun_estado(proceso, flag_estado); //este va a mandar el proceso a ready o exit
    
    return;
}

// ************* Funcion que llama a la de arriba a traves de un hilo *************
void recibir_pcb_hilo(void* arg){
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;
    pcb* proceso = args->proceso;

    recibir_pcb(proceso);
}

int hacer_signal(int indice_recurso, pcb* proceso){
    int flag;


    if(indice_recurso < cantidad_recursos) //chequeo que existe el recurso
    {
        ++ config_kernel->instancias_recursos[indice_recurso];

        if(config_kernel->instancias_recursos[indice_recurso] <= 0){
            flag = 0; //flag 0 porque quiero pasarlo a ready de nuevo

            // Saco de la cola de blocked de ese recurso
            pthread_mutex_lock(mutex_por_recurso[indice_recurso]);
            pcb* proceso_desbloqueado = queue_pop(colas_por_recurso[indice_recurso]);
            pthread_mutex_unlock(mutex_por_recurso[indice_recurso]);

            queue_push(proceso_desbloqueado->recursos_asignados, (void*)(intptr_t)indice_recurso); //agrego a la cola de recursos asignados del proceso el recurso
            
            accionar_segun_estado(proceso_desbloqueado , 0); //mando el proceso q se desbloqueo recien a la cola de ready
            
        }else flag = 0; //debo pasar este proceso de nuevo a ready, el signal no lo bloquea

        //Saco de la cola de recursos asignados del proceso el recurso
        int primer_recurso_asignado = (int)(intptr_t)queue_pop(proceso->recursos_asignados);
        int recurso_aux = primer_recurso_asignado;
        while(recurso_aux != indice_recurso)
        {
            queue_push(proceso->recursos_asignados, (void*)(intptr_t)recurso_aux);
            recurso_aux = (int)(intptr_t)queue_pop(proceso->recursos_asignados);
        }

    } else flag = 1;// no existe,  mando a exit el proceso

    return flag;
}

int hacer_wait(int indice_recurso, pcb* proceso){
    int flag;

    if(indice_recurso < cantidad_recursos){ // Si existe el recurso solicitado
        
        -- config_kernel->instancias_recursos[indice_recurso]; // Quito una instancia de ese recurso

        if(config_kernel->instancias_recursos[indice_recurso] < 0){
            
            flag = -1; // En accionar_segun_estado no hara nada porque ya lo bloqueo aca

            char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
            pthread_mutex_lock(&proceso->mutex_pcb);
            proceso->estado_del_proceso = BLOCKED; 
            pthread_mutex_unlock(&proceso->mutex_pcb);

            log_info(log_kernel, "PID: %d - Estado Anterior: %s - Estado Actual: BLOCKED", proceso->pid, estado_anterior);
            
            
            pthread_mutex_lock(mutex_por_recurso[indice_recurso]);
            queue_push(colas_por_recurso[indice_recurso], proceso); //Mando a la cola de blocked de ese recurso
            pthread_mutex_unlock(mutex_por_recurso[indice_recurso]);
            
        }else{
            flag = 0; //flag 0 para que vuelva a la cola de ready
            queue_push(proceso->recursos_asignados, (void*)(intptr_t)indice_recurso); //agrego el recurso a los recursos asignados
        } 
         
    }else flag = 1; // Entonces no existe el recurso solicitado
    
    return flag;
}

//Esta funcion devuelve un char al pasarle un enum de estados
char* obtener_char_de_estado(estados estado_a_convertir)
{
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
