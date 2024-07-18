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

pthread_t hilo_io(){ 

    pthread_t thread_io; 
            
    pthread_create(&thread_io, NULL, (void*)server_para_io, NULL);

    return thread_io;
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
        
        char* lectura = readline("Ingrese comando: \n"); // Pido que se ingrese un comando

        // Partes me separa segun los espacios lo que hay, en partes[0] esta INICIAR_PROCESO
        // En partes[1] va a estar el path
        char **partes = string_split(lectura, " "); 

        // Dependiendo el comando llamo a la función correspondiente
        if (strcmp(partes[0], "INICIAR_PROCESO") == 0) { 

            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
            //en partes[1] esta el path
            iniciar_proceso(partes[1]); // Esta es la funcion a la que llamo

        } else if (strcmp(partes[0], "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");
            finalizar_proceso(partes[1]); // Acá mando el PID del proceso que quiero finalizar

        }else if (strcmp(lectura, "PROCESO_ESTADO") == 0){
            printf("Ha seleccionado la opción PROCESO_ESTADO\n");
            listar_procesos_por_estado();
        
        }else if (strcmp(partes[0], "MULTIPROGRAMACION") == 0){
            printf("Ha seleccionado la opción MULTIPROGRAMACION\n");
            cambiar_grado_multiprogramacion(partes[1]);
        }else if(strcmp(partes[0], "EJECUTAR_SCRIPT") == 0){
            printf("Ha seleccionado la opción EJECUTAR_SCRIPT\n");
            ejecutar_script(partes[1]);
        }else if(strcmp(partes[0], "INICIAR_PLANIFICACION") == 0){
            printf("Ha seleccionaldo la opción INICIAR_PLANIFICACION\n");
            iniciar_planificacion();
        }else if(strcmp(partes[0], "DETENER_PLANIFICACION") == 0){
            printf("Ha seleccionaldo la opción DETENER_PLANIFICACION\n");
            detener_planificacion();
        }
        else {
            
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


            log_info(log_kernel, "PID: <%d> - Estado Anterior: <READY> - Estado Actual: <EXECUTE>", proceso_seleccionado->pid); // Hago un log obligatorio 

            enviar_pcb(proceso_seleccionado); // Envio el pcb a CPU

            crear_hilo_proceso(proceso_seleccionado); // Inicio un hilo que maneje el algoritmo del proceso

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
    char* estado_anterior;

    while(1)
    {
        sem_wait(&sem_cola_de_new); // Uso semaforos sobre la cola y el grado de multiprogramacion porque son cosas compartidas
        sem_wait(&sem_multiprogramacion); // Tanto para ver que haya procesos en New como para ver de no pasarme del grado de multiprogramacion

        sem_wait(&sem_planificacion_activa); // Para bloquear todo si la planificación está detenida
        sem_post(&sem_planificacion_activa); // Para que, si sí está activo, a la próxima vuelta no se bloquee

        pthread_mutex_lock(&mutex_cola_de_new);
        proceso_a_mandar_a_ready = queue_pop(cola_de_new); // Saco un proceso de la cola de New para despues pasarlo a Ready
        pthread_mutex_unlock(&mutex_cola_de_new);

        estado_anterior = obtener_char_de_estado(proceso_a_mandar_a_ready->estado_del_proceso);
        
        //log_obligatorio
        log_info(log_kernel, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <READY>", proceso_a_mandar_a_ready->pid, estado_anterior);
        
        pthread_mutex_lock(&proceso_a_mandar_a_ready->mutex_pcb);
        proceso_a_mandar_a_ready->estado_del_proceso = READY; // Le asigno el nuevo estado "Ready"
        pthread_mutex_unlock(&proceso_a_mandar_a_ready->mutex_pcb);

        //armo el log_obligatorio

        pthread_mutex_lock(&mutex_cola_de_ready); // Tmbn semaforo porque es seccion critica
        queue_push(cola_de_ready,proceso_a_mandar_a_ready); // Ingreso el proceso a la cola de Ready. 
        pthread_mutex_unlock(&mutex_cola_de_ready);
        
        hacer_el_log_obligatorio_de_ingreso_a_ready(proceso_a_mandar_a_ready);

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
    if (nuevo_pcb->registros == NULL) {
        log_error(log_kernel, "Error al asignar memoria para registros\n");
    }
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

    //log obligatorio
    log_info(log_kernel, "Se crea el proceso <%d> en <NEW>", nuevo_pcb->pid); 
    
    //Le mandamos el path de las instrucciones a memoria
    t_paquete* paquete_path = crear_paquete_personalizado(CREACION_PROCESO_KERNEL_A_MEMORIA);
    agregar_string_al_paquete_personalizado(paquete_path, path);
    agregar_int_al_paquete_personalizado(paquete_path, nuevo_pcb->pid);
    enviar_paquete(paquete_path, conexion_kernel_memoria);
    eliminar_paquete(paquete_path);

    // Si el grado de multiprogramacion me lo permite, modifico el estado a READY
    // int multiprogramacion_actual;
    // sem_getvalue(&sem_multiprogramacion, &multiprogramacion_actual); // Obtengo valor del grado de multiprogramacion
    
    // if (multiprogramacion_actual > 0 )
    // {   
    //     //Chequeo si tengo lugar para aceptar otro proceso en base al grado de multiprogramacion actual q tengo
        
    //     // Modifico el estado del proceso - Uso semaforo porque es una variable que tocan muchos hilos
    //     pthread_mutex_lock(&nuevo_pcb->mutex_pcb);
    //     nuevo_pcb->estado_del_proceso = READY; 
    //     pthread_mutex_unlock(&nuevo_pcb->mutex_pcb);

    //     // Ingreso el proceso a la cola de READY - Tambien semaforo porque es seccion critica 
    //     pthread_mutex_lock(&mutex_cola_de_ready);
    //     queue_push(cola_de_ready,nuevo_pcb);
    //     pthread_mutex_unlock(&mutex_cola_de_ready);
    //     sem_post(&sem_cola_de_ready); //Agrego 1 al semaforo contador
        
    //     //Bajo el grado de programacion actual, ya que agregue un proceso mas a ready
    //     sem_wait(&sem_multiprogramacion); //Resto 1 al grado de multiprogramacion

    // }else {
        // Si no hay espacio para un nuevo proceso en ready lo sumo a la cola de NEW - Semaforo SC
        pthread_mutex_lock(&mutex_cola_de_new);
        queue_push(cola_de_new,nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_new);
        sem_post(&sem_cola_de_new);
    //}
}

// ************* MUESTRA EN FORMA DE TABLA EN QUÉ ESTADO SE ENCUENTRA CADA PROCESO ACTUALMENTE ************* 
void listar_procesos_por_estado(){
    
    // Armo colas auxiliares de cada estado para mostrar por pantalla, asi pueden seguir las colas normales sin problema
    t_queue* cola_aux_ready = queue_create();
    t_queue* cola_aux_execute = queue_create();
    t_queue* cola_aux_blocked = queue_create();
    t_queue* cola_aux_exit = queue_create();
    t_queue* cola_aux_new = queue_create();

    bool ya_recorri_todo = false;

    pthread_mutex_lock(&mutex_cola_general_de_procesos); // Bloqueo la cola general

    if (queue_is_empty(cola_general_de_procesos)== false)
    {

        pcb* primer_pcb_cola_gral = queue_pop(cola_general_de_procesos); // Saco el 1er PCB de la cola gral

        pcb* aux = primer_pcb_cola_gral; // Esta variable tendrá los procesos de la cola
        
        // Este while recorre la cola gral, agregando cada proceso a la cola aux q corresponda

        while(ya_recorri_todo == false)
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
            queue_push(cola_general_de_procesos, aux);
            aux = queue_pop(cola_general_de_procesos);
            if (aux == primer_pcb_cola_gral)
            {
                queue_push(cola_general_de_procesos, aux);
                ya_recorri_todo = true;
            }
        }

        pthread_mutex_unlock(&mutex_cola_general_de_procesos); // Desbloqueo la cola general

        // Recorro cada cola y hago los logs de cada una

        log_info(log_kernel, "Procesos en estado <NEW>: \n");
        while(queue_is_empty(cola_aux_new)!= true)
        {
            aux = queue_pop(cola_aux_new);
            log_info(log_kernel, "PID: %d\n" , aux->pid );
        }

        log_info(log_kernel, "Procesos en estado <READY>: \n");
        while(queue_is_empty(cola_aux_ready)!= true)
        {
            aux = queue_pop(cola_aux_ready);
            log_info(log_kernel, "PID: %d\n" , aux->pid );
        }

        log_info(log_kernel, "Procesos en estado <BLOCKED>: \n");
        while(queue_is_empty(cola_aux_blocked)!= true)
        {
            aux = queue_pop(cola_aux_blocked);
            log_info(log_kernel, "PID: %d\n" , aux->pid );
        }

        log_info(log_kernel, "Procesos en estado <EXIT>: \n");
        while(queue_is_empty(cola_aux_exit)!= true)
        {
            aux = queue_pop(cola_aux_exit);
            log_info(log_kernel, "PID: %d\n" , aux->pid );
        }

        log_info(log_kernel, "Procesos en estado <EXECUTE>: \n");
        while(queue_is_empty(cola_aux_execute)!= true)
        {
            aux = queue_pop(cola_aux_execute);
            log_info(log_kernel, "PID: %d\n" , aux->pid );
        }
    }else log_info(log_kernel, "No hay ningun proceso en el sistema" );
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
// Terminar
void finalizar_proceso(char* pid_formato_char){

    int pid = atoi(pid_formato_char); // Paso a int el pid mandado por consola
    bool no_esta_en_el_sistema = false; // Hago un flag para saber si el proceso se encuentra en el sistema o no

    pthread_mutex_lock(&mutex_cola_general_de_procesos); // Bloqueo la cola general

    pcb* aux = queue_pop(cola_general_de_procesos); 
    
    int primer_pid = aux->pid;

    while(pid != aux->pid) // Después de recorrer toda la cola en aux nos queda el proceso correspondiente a ese pid
    {
        queue_push(cola_general_de_procesos, aux);
        aux = queue_pop(cola_general_de_procesos);
        if (aux->pid == primer_pid)
        {
            no_esta_en_el_sistema = true; // Si no encuentro el pid en toda la cola general de procesos, entonces marco que no lo encontré
            break;
        }
    }

    queue_push(cola_general_de_procesos, aux);
    pthread_mutex_unlock(&mutex_cola_general_de_procesos); // Desbloqueo la cola general
 
    if (no_esta_en_el_sistema == false){ // Si sí está en el sistema, entonces lo voy a sacar de su respectiva cola, y posteriormente será enviado a exit
        switch(aux->estado_del_proceso)
        {
            case READY:
                sacar_de_cola_de_ready(pid);
                break;
            case EXECUTE:
                sacar_de_execute(pid);
                break;
            case BLOCKED:
                sacar_de_blocked(pid);
                break;
            case NEW:
                sacar_de_cola_de_new(pid);
                break;
        }
    }else log_error(log_kernel, "El pid seleccionado no existe en el sistema"); // Si no está en el sistema, simplemente digo que no lo encontré
}

// ************* EJECUTA UN SCRIPT DE COMANDOS ************* 
// Creo que ya están todos los ifs pero no toy seguro
void ejecutar_script(char* script_path){
    
    FILE *archivo; // Declaro un archivo
    char linea[100]; // Declaro un tamaño de la línea de 100 caracteres

    char* filepath = string_new();

    string_append(&filepath, "../kernel");

    string_append(&filepath, script_path);

    

    archivo = fopen(filepath, "r") ;// Abro el archivo en modo lectura
    
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

        } else if (strcmp(partes[0], "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");
            finalizar_proceso(partes[1]); // Acá mando el PID del proceso que quiero finalizar

        }else if (strcmp(partes[0], "PROCESO_ESTADO") == 0){
            printf("Ha seleccionado la opción PROCESO_ESTADO\n");
            listar_procesos_por_estado();
        
        }else if (strcmp(partes[0], "MULTIPROGRAMACION") == 0){
            printf("Ha seleccionado la opción MULTIPROGRAMACION\n");
            cambiar_grado_multiprogramacion(partes[1]);
        }else if(strcmp(partes[0], "EJECUTAR_SCRIPT") == 0){
            printf("Ha seleccionado la opción EJECUTAR_SCRIPT\n");
            ejecutar_script(partes[1]);
        }
        else if(strcmp(partes[0], "INICIAR_PLANIFICACION") == 0){
            printf("Ha seleccionaldo la opción INICIAR_PLANIFICACION\n");
            iniciar_planificacion();
        }else if(strcmp(partes[0], "DETENER_PLANIFICACION") == 0){
            printf("Ha seleccionaldo la opción DETENER_PLANIFICACION\n");
            detener_planificacion();
        }else {
            
            printf("Opción no válida\n");
        }

        liberar_array_strings(partes); // Libero la memoria asignada por string_split después de usarla

    }

    fclose(archivo); // Cierro el archivo

    free(filepath);

    return;
    
}

// ************* DETIENE LA PLANIFICACIÓN DE LOS PROCESOS ************* 
void detener_planificacion() {
    pthread_mutex_lock(&mutex_planificacion_activa);
    if (planificacion_activa) {
        planificacion_activa = 0;
        sem_wait(&sem_planificacion_activa);
        log_info(log_kernel, "Planificación detenida \n");
    } else {
        log_info(log_kernel, "La planificación ya se encuentra detenida \n");
    }
    pthread_mutex_unlock(&mutex_planificacion_activa);
}

// ************* REANUDA LA PLANIFICACIÓN DE LOS PROCESOS ************* 
void iniciar_planificacion() {
    pthread_mutex_lock(&mutex_planificacion_activa);
    if (!planificacion_activa) {
        planificacion_activa = 1;
        sem_post(&sem_planificacion_activa);
        log_info(log_kernel, "Planificación reanudada \n");
    } else {
        log_info(log_kernel, "La planificación ya se encuentra iniciada \n");
    }
    pthread_mutex_unlock(&mutex_planificacion_activa);
}
    
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
    agregar_estructura_al_paquete_personalizado(paquete_pcb, proceso->registros, sizeof(registros_cpu));

    enviar_paquete(paquete_pcb, conexion_kernel_cpu); // Envio el paquete a través del socket
    
    tiempo_de_quantum = temporal_create(); //comienza a contar el quantum

    eliminar_paquete(paquete_pcb); // Libero el paquete

    return;
}

// ----------------------------------------------------------------------
// ------------- FIN FUNCIONES PARA ENVIAR UN PROCESO A CPU -------------
// ----------------------------------------------------------------------





// ---------------------------------------------------------------------------
// ------------- INICIO FUNCIONES PARA RECIBIR UN PROCESO DE CPU -------------
// ---------------------------------------------------------------------------


// ************* CREA HILOS SEGUN EL ALGORITMO DE PLANIFICACION Y LLAMA FUNCIONES PARA MANEJAR LA RECEPCION DEL PCB *************
void crear_hilo_proceso(pcb* proceso){

    // Funcion llamada por enviar_proceso_a_cpu

    pthread_t hilo_recibe_proceso, hilo_interrupcion; // Creo un hilo
    thread_args_procesos_kernel args_hilo = {proceso}; // En sus args le cargo el proceso

    if(strcmp(config_kernel->algoritmo_planificacion, "FIFO") == 0)
    {
        // Si el algoritmo de planificacion es FIFO entonces recibo el pcb normalmente desde CPU

        pthread_create(&hilo_recibe_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); // Recibir unpcb normalmente, sin interrupciones
        pthread_join(hilo_recibe_proceso, NULL); // No sigo hasta que no haya terminado de recibirlo, y ahílibero el hilo
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


// ************* RECIBE EL PCB NORMALMENTE (SIN INTERRUPCION) SEGUN ALGORITMO *************
void recibir_pcb(pcb* proceso) {

    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);

    //clock_t inicio, fin; //ahora uso temporal
    int tiempo_que_tardo_en_recibir;

    //inicio = clock(); // En este momento comienzo a esperar

    int codigo_operacion = recibir_operacion(conexion_kernel_cpu); // Recibo el codigo de operacion para ver como actúo según eso

    int flag_estado; // Declaro un flag que me va a servir para manejar el estado del proceso posteriormente
    char* recurso;
    char* nombre_interfaz;
    int unidades_de_trabajo;
    t_list* direcciones_fisicas;
    int registro_tamano;
    int registro_puntero_archivo;
    char* nombre_archivo;
    t_buffer* buffer; // Buffer para recibir el paquete
    int motivo = -1;
    pcb* pcb_recibido;

      switch(codigo_operacion) // Segun el codigo de operacion actuo 
        {
            case -1:
                log_info(log_kernel, "Se desconecto CPU");
                break;
            case FIN_DE_QUANTUM: 
                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                flag_estado = 0; // El proceso todavia no termino
                pcb_recibido  = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                
                actualizar_pcb(proceso, pcb_recibido);

                //log obligatorio
                log_info(log_kernel, "Fin de Quantum: “PID: <%d> - Desalojado por fin de Quantum”", proceso->pid);
                
                break;
            case INTERRUPTED_BY_USER: 

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                flag_estado = 1; // mando el proceso a exit
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                motivo = INTERRUPTED_BY_USER;

                actualizar_pcb(proceso, pcb_recibido);

                break;
            

            case CPU_TERMINA_EJECUCION_PCB:

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empecé a esperar la recepción
                flag_estado = 1; // El proceso ya finalizo, no quedan rafagas por ejecutar
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                motivo = SUCCESS;

                actualizar_pcb(proceso, pcb_recibido);

                break;

            case SIGNAL:

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                recurso = recibir_string_del_buffer(buffer); // Obtengo el indice del recurso que se usa para manejarlo
                
                actualizar_pcb(proceso, pcb_recibido);
                
                flag_estado = hacer_signal(recurso, proceso); // Asigno un flag que sirve para manejar el estado del proceso
        
                break;

            case WAIT:

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                recurso = recibir_string_del_buffer(buffer); // Obtengo el nombre del recurso que se usa para manejarlo

                actualizar_pcb(proceso, pcb_recibido);
                
                flag_estado = hacer_wait(recurso, proceso);

                break;

            case IO_GEN_SLEEP: //(Interfaz, Unidades de trabajo)

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                unidades_de_trabajo = recibir_int_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_gen_sleep(nombre_interfaz, unidades_de_trabajo, proceso);
                
                break;

            //le tengo q decir a CPU q me los mande como ints a los valores del registro
            case IO_STDIN_READ: //(Interfaz, Registro Dirección, Registro Tamaño)
                
                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_stdin_read(nombre_interfaz, direcciones_fisicas, registro_tamano, proceso);

                break;

            case IO_STDOUT_WRITE: //(Interfaz, Registro Dirección, Registro Tamaño)

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_stdout_write(nombre_interfaz, direcciones_fisicas, registro_tamano, proceso);

                break;
            
            case IO_FS_CREATE: // (Interfaz, Nombre Archivo)

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                nombre_archivo = recibir_string_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_fs_create(nombre_interfaz, nombre_archivo, proceso);

                break;

            case IO_FS_DELETE: //(Interfaz, Nombre Archivo)

                temporal_stop(tiempo_de_quantum);
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                nombre_archivo = recibir_string_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_fs_delete(nombre_interfaz, nombre_archivo, proceso);

                break;

            case IO_FS_TRUNCATE: //(Interfaz, Nombre Archivo, Registro Tamaño)

                temporal_stop(tiempo_de_quantum);
                
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                nombre_archivo = recibir_string_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_fs_truncate(nombre_interfaz, nombre_archivo, registro_tamano, proceso);

                break;

            case IO_FS_WRITE: //(Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)

                temporal_stop(tiempo_de_quantum);
                
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                nombre_archivo = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_fs_write(nombre_interfaz, nombre_archivo, direcciones_fisicas, registro_tamano, registro_puntero_archivo, proceso);

                break;

            case IO_FS_READ: //(Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
            
                temporal_stop(tiempo_de_quantum);
                
                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);
                nombre_interfaz = recibir_string_del_buffer(buffer);
                nombre_archivo = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = io_fs_read(nombre_interfaz, nombre_archivo, direcciones_fisicas, registro_tamano, registro_puntero_archivo, proceso);

                break;
            case OUT_OF_MEMORY:
                
                temporal_stop(tiempo_de_quantum);

                buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu); // Recibo el PCB normalmente
                //fin = clock(); // Termino el tiempo desde que empece a esperar la recepcion 
                pcb_recibido = recibir_estructura_del_buffer(buffer);
                pcb_recibido->registros = recibir_estructura_del_buffer(buffer);

                actualizar_pcb(proceso, pcb_recibido);

                flag_estado = 1; //paso el proceso a exit
                motivo = OUT_OF_MEMORY;
                break;
            default:            
                log_error(log_kernel, "El codigo de operacion no es reconocido :(");
                break;
        }

        //tiempo_que_tardo_en_recibir = (int)((double)((fin - inicio) * 1000.0 / CLOCKS_PER_SEC)); // Calculo el tiempo que me tarde en recibir el PCB
        tiempo_que_tardo_en_recibir = (int)temporal_gettime(tiempo_de_quantum);

        temporal_destroy(tiempo_de_quantum);//lo destruyo porq lo creo cada vez q mando un proceso

        //log_info(log_kernel, "el tiempo q tardo en recibir es: %d", tiempo_que_tardo_en_recibir);

        if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0 || strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0)
        {
            //VER SI FUNCA
            sem_post(&destruir_hilo_interrupcion);
        }

        if(strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0) // Si estoy recibiendo a traves del algoritmo VRR
        {
            proceso->quantum = proceso->quantum - tiempo_que_tardo_en_recibir; // Le asigno el quantum que le queda disponible
            
            //log_info(log_kernel,"el quantum que le queda al proceso es de: %d", proceso->quantum);
        };  
    
    free(buffer->stream); // Libero directamente el buffer, no arme paquete asi que no hace falta
    free(buffer);

    sem_wait(&sem_planificacion_activa); // Si la planificación está activa, dejo que se manden procesos a execute
    sem_post(&sem_puedo_mandar_a_cpu); // Aviso que ya volvio el proceso que estaba en CPU, puedo mandar otro
    sem_post(&sem_planificacion_activa); // Está activa, entonces aumento para que el próximo chequeo no lo bloquee

    accionar_segun_estado(proceso, flag_estado, motivo); // Mando el proceso a Ready o Exit segun corresponda

    return;
}


void algoritmo_round_robin (void* arg){

    // Aca vamos a armar dos hilos, uno que maneje solo cuando mandarle
    // La interrupcion y otro que siemp este esperando a q cpu le mande el pcb
    thread_args_procesos_kernel*args = (thread_args_procesos_kernel*)arg;
    pcb* proceso_actual = args->proceso;

    log_info(log_kernel, "El quantum que le voy a dar es de: %d", proceso_actual->quantum);

    usleep((proceso_actual->quantum)*1000); // Acá usamos el quantum del proceso, asi podemos reutilziar la funcion para VRR
    if(!interrupcion_por_fin_de_proceso){ // Si no tengo que finalizar el proceso por pedido de usuario

        desalojar_proceso_hilo(args, FIN_DE_QUANTUM); // Entonces lo desalojo por el algoritmo de planificación
        
    }else sem_post(&destruir_hilo_interrupcion); // Sino se maneja el desalojo en otro lado
    
    return;
}

// ************* LLAMA A DESALOJAR_PROCESO (NOTIFICAR INTERRUPCION) A TRAVES DE UN HILO *************
void desalojar_proceso_hilo(void* arg, int operacion){
    
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg; // Creo un hilo
    pcb* proceso = args->proceso; // Le cargo el proceso en los args

    desalojar_proceso(proceso, operacion); 
}

// ************* NOTIFICA A CPU QUE HAY UNA INTERRUPCION Y DEBE MANDAR INMEDIATAMENTE EL PCB DE REGRESO *************
void desalojar_proceso(pcb* proceso, int operacion){
   
    t_paquete* paquete_a_enviar = crear_paquete_personalizado(operacion); // Creo un paquete con el codop especifico
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
void accionar_segun_estado(pcb* proceso, int flag, int motivo){

    sem_wait(&sem_planificacion_activa); // Si la planificación está activa entonces sigo (paso de 1 a 0 el sem)
    sem_post(&sem_planificacion_activa); // Estoy acá si la planificación sigue activa, entonces aumento para que no se bloquee a la próxima

    //flag =  1, ya ejecutó todo, tengo q pasarlo a exit
    //flag =  0, aun no ejecutó del todo, lo mando a la cola de ready
    //flag =  2, tengo que bloquear el proceso
    //flag = -1, no hago nada, ya lo hicieron 
    
    if(flag == 2){
        pasar_proceso_a_blocked(proceso);
    }
    else if(flag == 1){
        pasar_proceso_a_exit(proceso, motivo);
    }else if (flag == 0)
    { 

        char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);

        log_info(log_kernel, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <READY>", proceso->pid, estado_anterior);
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

                hacer_el_log_obligatorio_de_ingreso_a_ready_prioridad(proceso);
                
                sem_post(&sem_cola_prioridad_vrr);

            }else{ // Si no le queda quantum

                proceso->quantum = config_kernel->quantum; // Le reinicio el quantum
                pthread_mutex_lock(&mutex_cola_de_ready);
                queue_push(cola_de_ready,proceso); // Y la sumo a la cola de Ready normal
                pthread_mutex_unlock(&mutex_cola_de_ready);
                
                hacer_el_log_obligatorio_de_ingreso_a_ready(proceso);
                
                sem_post(&sem_cola_de_ready);

            }
        } else{ // No estoy en vrr, siempre madno a cola de Ready normal
            
            pthread_mutex_lock(&mutex_cola_de_ready);
            queue_push(cola_de_ready,proceso); // Meto a la cola de Ready
            pthread_mutex_unlock(&mutex_cola_de_ready);
            
            hacer_el_log_obligatorio_de_ingreso_a_ready(proceso);
            
            sem_post(&sem_cola_de_ready);
        };
    }
    return;
}

// ************* PASA EL PROCESO SELECCIONADO A EXIT *************
void pasar_proceso_a_exit(pcb* proceso, int motivo){

    log_info(log_kernel, "entre a pasar proceso a exit por pid: %d", proceso->pid);
    //En esta funcion faltan liberar los recursos q tenia el proceso
    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso); // Devuelvo el estado como string

    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = EXIT; // Asigno el estado del proceso como Exit
    pthread_mutex_unlock(&proceso->mutex_pcb);

    switch(motivo)
    {
        case SUCCESS:
            log_info(log_kernel, "Finaliza el proceso <%d> - Motivo: <SUCCESS>", proceso->pid );
            break;
        case INVALID_INTERFACE:
            log_info(log_kernel, "Finaliza el proceso <%d> - Motivo: <INVALID_INTERFACE>", proceso->pid );
            break;
        case INVALID_RESOURCE:
            log_info(log_kernel, "Finaliza el proceso <%d> - Motivo: <INVALID_RESOURCE>", proceso->pid );
            break;
        case OUT_OF_MEMORY:
            log_info(log_kernel, "Finaliza el proceso <%d> - Motivo: <OUT_OF_MEMORY>", proceso->pid );
            break;
        case INTERRUPTED_BY_USER:
            log_info(log_kernel, "Finaliza el proceso <%d> - Motivo: <INTERRUPTED_BY_USER>", proceso->pid );
            break;
        default:
            log_info(log_kernel, "Finaliza el proceso <%d> - Motivo: <NI IDEA MACHOOO>", proceso->pid );
            break;
    }

    int indice_recurso_a_liberar;
    char* recurso_a_liberar;

    while(queue_is_empty(proceso->recursos_asignados) != true) // Mientras la cola de recursos asignados no esta vacia
    {
        indice_recurso_a_liberar = (int)(intptr_t)queue_peek(proceso->recursos_asignados); // Entonces libero los recursos correspondientes 

        recurso_a_liberar = config_kernel->recursos[indice_recurso_a_liberar]; //lo paso a char*

        log_info(log_kernel, "Estoy en pasar a exit el recurso a liberar es: %s", recurso_a_liberar);

        hacer_signal(recurso_a_liberar, proceso); //tengo q pasarle a signal el nombre del recurso

        log_info(log_kernel, "hice signal del recurso: %s", recurso_a_liberar);
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

    log_info(log_kernel, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <EXIT>", proceso->pid, estado_anterior);

    free(proceso->registros);
    queue_destroy(proceso->recursos_asignados);
    pthread_mutex_destroy(&proceso->mutex_pcb);
    free(proceso); 

    if(queue_size(cola_general_de_procesos) < config_kernel->grado_multiprogramacion)
    {
        sem_post(&sem_multiprogramacion); //Agrego 1 al grado de multiprogramacion solo si puedo
    }
    return;
}

// ************* PASA EL PROCESO SELECCIONADO A BLOCKED *************
void pasar_proceso_a_blocked(pcb* proceso){
    
    char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
    
    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = BLOCKED; 
    pthread_mutex_unlock(&proceso->mutex_pcb);

    log_info(log_kernel, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <BLOCKED>", proceso->pid, estado_anterior);
}

// -----------------------------------------------------------------
// ------------- FIN FUNCIONES PARA MANEJAR UN PROCESO -------------
// -----------------------------------------------------------------





// -------------------------------------------------------------------
// ------------- INICIO FUNCIONES AUXILIARES DE COMANDOS -------------
// -------------------------------------------------------------------

// ************* AUXILIAR DE FINALIZAR_PROCESO, SACA UN PROCESO DE LA COLA DE READY Y LO FINALIZA ************* 
void sacar_de_cola_de_ready(int pid){
    // Tenemos q recorrer la cola de ready buscando el pid y sacar ese, luego pasarlo a exit

    pcb* aux;
    bool encontrado = false;

    if(strcmp(config_kernel->algoritmo_planificacion, "VRR") == 0){

        pthread_mutex_lock(&mutex_cola_prioridad_vrr);

        if(queue_is_empty(cola_prioridad_vrr) == false){ // Si la cola de prioridad no está vacía

            aux  = queue_pop(cola_prioridad_vrr);
            int primer_pid = aux->pid;

            while(aux -> pid != pid) // Entonces busco en la cola de prioridad
            {

                queue_push(cola_prioridad_vrr, aux);
                aux = queue_pop(cola_prioridad_vrr);
                if(aux->pid == primer_pid) // Si recorrí toda la cola y no encontré el proceso, entonces voy a buscar a Ready normal
                {
                    break;
                }
            }
            encontrado = true; // Si lo encontré en la cola de prioridad, marco que ya se encontró, sino, busco en Ready normal
        }
        pthread_mutex_unlock(&mutex_cola_prioridad_vrr);
        sem_wait(&sem_cola_prioridad_vrr);
    }

    pthread_mutex_lock(&mutex_cola_de_ready);
    if(encontrado == false) // Si no lo encontré al proceso en la cola de prioridad, entonces lo busco en Ready normal
    {
        aux = queue_pop(cola_de_ready);
        while(aux -> pid != pid) // Recorro la cola hasta que lo encuentre
        {
            queue_push(cola_de_ready, aux);
            aux = queue_pop(cola_de_ready);
        }
        encontrado = true; // Encontré el proceso, dejo de buscar
        sem_wait(&sem_cola_de_ready);
    }
    
    pthread_mutex_unlock(&mutex_cola_de_ready);
    
    pasar_proceso_a_exit(aux, INTERRUPTED_BY_USER); // Ahora sí, una vez que saqué el proceso de la cola en la que se encontraba, entonces lo paso a exit
}
// ************* AUXILIAR DE FINALIZAR_PROCESO, SACA UN PROCESO DE LA COLA DE NEW Y LO FINALIZA ************* 
void sacar_de_cola_de_new(int pid){

    pthread_mutex_lock(&mutex_cola_de_new);

    pcb* aux = queue_pop(cola_de_new);

    while(aux -> pid != pid) // Recorro la cola hasta que lo encuentre
    {
        queue_push(cola_de_new, aux);
        aux = queue_pop(cola_de_new);
    }
    
    sem_wait(&sem_cola_de_new);
    pthread_mutex_unlock(&mutex_cola_de_new);
    pasar_proceso_a_exit(aux, INTERRUPTED_BY_USER);
}
// ************* AUXILIAR DE FINALIZAR_PROCESO, SACA UN PROCESO DE EXECUTE Y LO FINALIZA ************* 
void sacar_de_execute(int pid){
    //Si un proceso esta en execute hay que mandar la interrupcion para que cpu devuelva el proceso
    // Busco el proceso para mandarselo a desalojar proceso

    interrupcion_por_fin_de_proceso = true;

    pthread_mutex_lock(&mutex_cola_general_de_procesos);
    pcb* aux  = queue_pop(cola_general_de_procesos);
    while(pid != aux->pid)
    {
        queue_push(cola_general_de_procesos, aux);
        aux  = queue_pop(cola_general_de_procesos);
    }
    queue_push(cola_general_de_procesos, aux);
    
    pthread_mutex_unlock(&mutex_cola_general_de_procesos);

    sem_post(&destruir_hilo_interrupcion);
    
    desalojar_proceso(aux, INTERRUPTED_BY_USER);
    

}

void sacar_de_blocked(int pid){
    //sabemos q el proceso esta bloqueado, no sabemos donde
    //busco en las colas de los recursos

    log_info(log_kernel, "entre a sacar de blocked por pid: %d", pid);

    pcb* aux;
    int primer_pid;
    bool encontrado = false;


    for(int i=0 ; i < cantidad_recursos && encontrado == false ;i++)
    {
        log_info(log_kernel, "estoy buscando en la cola de bloqueados de cada recurso");
        pthread_mutex_lock(&mutex_por_recurso[i]);
        
        if(queue_is_empty(colas_por_recurso[i]) == false)
        {
            aux = queue_pop(colas_por_recurso[i]);
            log_info(log_kernel, "el aux del indice recurso: %d es el pid:%d", i, aux->pid);
            primer_pid = aux->pid;
            while(aux->pid != pid)
            {
                queue_push(colas_por_recurso[i], aux);
                aux = queue_pop(colas_por_recurso[i]);
                if(aux->pid == primer_pid)
                {
                    queue_push(colas_por_recurso[i], aux);
                    break;
                }
            }

            //queue_push(colas_por_recurso[i], aux); //agregue esto
            log_info(log_kernel, "Agregue a la cola del recurso indice %d el proceso pid: %d", i, aux->pid);

            pthread_mutex_unlock(&mutex_por_recurso[i]);
            if(aux->pid == pid) 
            {
                encontrado = true;
                log_info(log_kernel, "encontre el proceso, ya lo saque");
            }
        }
        
    }

    
    interfaz_kernel* interfaz_aux;
    argumentos_para_io* args_aux ;
    while(encontrado == false)//No lo encontre en las colas por recursos tengo q buscarlo en las interfaces IO
    {
        log_info(log_kernel, "ya busque entre los bloqueados por recurso pero no encontre, ahora voy a bsucar en bloqueado x interfaz");
        interfaz_aux = queue_pop(cola_interfaces_conectadas);
        pthread_mutex_lock(&interfaz_aux->mutex_cola);
        args_aux = queue_pop(interfaz_aux->cola_de_espera);
        primer_pid = args_aux->proceso->pid;

        while(args_aux->proceso->pid != pid)
        {
            queue_push(interfaz_aux->cola_de_espera, args_aux);
            args_aux = queue_pop(interfaz_aux->cola_de_espera);
            if(args_aux->proceso->pid == primer_pid)
            {
                queue_push(interfaz_aux->cola_de_espera, args_aux);
                break;
            };

        };
        pthread_mutex_unlock(&interfaz_aux->mutex_cola);
        queue_push(cola_interfaces_conectadas, interfaz_aux);
        if(args_aux->proceso->pid == pid)
        {
            encontrado = true;
        }
    }

    if(encontrado) //si no lo encontre es porq estaba en io
    {
        log_info(log_kernel, "encontre el proceso, es el: %d", aux->pid);
        pasar_proceso_a_exit(aux, INTERRUPTED_BY_USER);

    }else pid_eliminar = pid;

    return;

}
// -------------------------------------------------------------
// ------------- INICIO FUNCIONES DE INSTRUCCIONES -------------
// -------------------------------------------------------------

// ************* REALIZA EL SIGNAL DE UN RECURSO *************
int hacer_signal(char* recurso, pcb* proceso){

    int flag;

    int indice_recurso = buscar_indice_recurso_segun_nombre(recurso); // Devuevle -1 si no lo encuentra

    log_info(log_kernel, "Entre a hacer_signal , la instancia del recurso %s es: %d", recurso, config_kernel->instancias_recursos[indice_recurso]);

    if(indice_recurso != -1) //Si el recurso existe
    {
        ++ config_kernel->instancias_recursos[indice_recurso];

        //if(config_kernel->instancias_recursos[indice_recurso] < 0)
        if(queue_size(colas_por_recurso[indice_recurso]) > 0) //si habia un proceso bloqueado lo desbloqueo
        { // Verifico si tenia un proceso bloqueado
            
             // no le sumo porq le doy esa instancia al q estaba bloqueado
            
            flag = 0; // Esto es porque el proceso que llama al signal tiene que quedar en ready
            
            pthread_mutex_lock(&mutex_por_recurso[indice_recurso]);

            log_info(log_kernel, "El tamaño de la cola bloqueada (antes del pop) del recurso %s es: %d", recurso, queue_size(colas_por_recurso[indice_recurso]));
            log_info(log_kernel, "El indice que estoy por buscar dentro de la cola es: %d", indice_recurso);

            pcb* proceso_desbloqueado = queue_pop(colas_por_recurso[indice_recurso]); // Saco de la cola de blocked de ese recurso
            pthread_mutex_unlock(&mutex_por_recurso[indice_recurso]);

            log_info(log_kernel, "desbloquie el proceso: %d , por hacer signal", proceso_desbloqueado->pid);

            queue_push(proceso_desbloqueado->recursos_asignados, (void*)(intptr_t)indice_recurso); // Agrego el recurso a la cola de recursos asignados del proceso
            
            accionar_segun_estado(proceso_desbloqueado , 0, -1); // Con esto es con lo que desbloqueo lo que estaba bloqueado
            
        }else {
            flag = 0; // Debo pasar este proceso de nuevo a ready, el signal no lo bloquea
        }

        //Saco de la cola de recursos asignados del proceso el recurso
        
        if(queue_is_empty(proceso->recursos_asignados) == false){ // Si la cola de recursos asignados del proceso no está vacía

            int primer_recurso_asignado = (int)(intptr_t)queue_pop(proceso->recursos_asignados); // Entonces busco el recurso asignado 
            int recurso_aux = primer_recurso_asignado; // Y lo quito de la cola
        
            while(recurso_aux != indice_recurso) // Acá se está recorriendo la cola, hasta que saco el recurso que quiero, si no tengo el que quiero lo vuelvo a meter en la cola
            {
                queue_push(proceso->recursos_asignados, (void*)(intptr_t)recurso_aux);
                recurso_aux = (int)(intptr_t)queue_pop(proceso->recursos_asignados);

                if(recurso_aux == primer_recurso_asignado){ // Si di toda la vuelta y no está el recurso, entonces salgo del bucle
                    break;
                }
            }
        }

    } else
    {
        flag = -1;
        pasar_proceso_a_exit(proceso, INVALID_RESOURCE);
    }  // No existe,  mando a exit el proceso

    return flag;
}

// ************* REALIZA EL WAIT DE UN RECURSO *************
int hacer_wait(char* recurso, pcb* proceso){
    
    int flag;

    int indice_recurso = buscar_indice_recurso_segun_nombre(recurso); //devuelve -1 si no lo encuentra

    log_info(log_kernel, "Entre a hacer_wait , la instancia del recurso %s es: %d", recurso, config_kernel->instancias_recursos[indice_recurso]);

    if(indice_recurso != -1){ // Si existe el recurso solicitado
        
        -- config_kernel->instancias_recursos[indice_recurso]; // Quito una instancia de ese recurso

        if(config_kernel->instancias_recursos[indice_recurso] < 0){ // Si no tengo más instancias disponibles, entonces bloqueo el proceso
            
            flag = -1; // En accionar_segun_estado no hara nada porque ya lo bloqueo aca

            char* estado_anterior = obtener_char_de_estado(proceso->estado_del_proceso);
            
            pthread_mutex_lock(&proceso->mutex_pcb);
            proceso->estado_del_proceso = BLOCKED; // Bloqueo el proceso
            pthread_mutex_unlock(&proceso->mutex_pcb);
            
            log_info(log_kernel, "El indice de la cola por recurso que estoy por pushear es: %d", indice_recurso);
            log_info(log_kernel, "El PID del proceso que estoy por pushear es: %d", proceso->pid);

            pthread_mutex_lock(&mutex_por_recurso[indice_recurso]);
            queue_push(colas_por_recurso[indice_recurso], proceso); // Mando a la cola de blocked de ese recurso
            pthread_mutex_unlock(&mutex_por_recurso[indice_recurso]);

            char* estado_actual = obtener_char_de_estado(proceso->estado_del_proceso);

            log_info(log_kernel, "PID: <%d> - Estado Anterior: <%s> - Estado Actual: <%s>", proceso->pid, estado_anterior, estado_actual);
            log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, recurso);

            log_info(log_kernel, "Bloquee el proceso: %d , por hacer wait", proceso->pid);
            log_info(log_kernel, "Y el tamaño de la cola bloqueada del recurso %s es: %d", recurso, queue_size(colas_por_recurso[indice_recurso]));
            
        }else{ // Si no lo tengo que bloquear, entonces lo mando a Ready

            flag = 0; //flag 0 para que vuelva a la cola de ready
            queue_push(proceso->recursos_asignados, (void*)(intptr_t)indice_recurso); // Agrego el recurso a la cola de recursos asignados
        } 
         
    }else
    {
        flag = -1;
        pasar_proceso_a_exit(proceso, INVALID_RESOURCE);

    }  // Entonces no existe el recurso solicitado, y mando a exit al proceso
    
    return flag;
}

// ----------------------------------------------------------
// ------------- FIN FUNCIONES DE INSTRUCCIONES -------------
// ----------------------------------------------------------





// -----------------------------------------------------------------
// ------------- INICIO FUNCIONES PARA LIBERAR MEMORIA -------------
// -----------------------------------------------------------------

// ************* LIBERA LA MEMORIA DE "PARTES" QUE SIRVE PARA DIVIDIR UN TEXTO *************
// A chequear
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

int buscar_indice_recurso_segun_nombre(char* recurso)
{
    //og_info(log_kernel, "El nombre de recruso buscado es: %s", recurso);
    
    for (int i = 0 ; i < cantidad_recursos; i++)
    {
        //log_info(log_kernel, "El nombre del recurso %d es: %s", i, config_kernel->recursos[i]);
        
        if (strcmp(config_kernel->recursos[i], recurso) == 0) //encontre el recurso
        {
            return i;
        }
    }
    
    return -1; //ya recorri todos los recursos y no lo encontre, devuelvo -1
}

char *agregar_al_final(char *buffer, const char *informacion) {
    if (buffer == NULL) {
        // Si el buffer es NULL, asigna memoria suficiente para la información
        buffer = malloc(strlen(informacion) + 1);  // +1 para el terminador nulo
        
        strcpy(buffer, informacion);
    } else {
        // Si el buffer ya contiene datos, realloca memoria para incluir la nueva información
        size_t tam_buffer = strlen(buffer);
        size_t tam_informacion = strlen(informacion);
        buffer = realloc(buffer, tam_buffer + tam_informacion + 1);  // +1 para el terminador nulo
        if (buffer == NULL) {
            perror("Error al realocar memoria");
            return NULL;
        }
        strcat(buffer, informacion);
    }
    return buffer;
}

char* pasar_a_string(int valor)
{
    static char buffer[20]; // Asegúrate de que el tamaño sea suficiente
    snprintf(buffer, sizeof(buffer), "%d", valor);
    return buffer;
}

void hacer_el_log_obligatorio_de_ingreso_a_ready(pcb* proceso_a_mandar_a_ready)
{
    //Cola Ready: [1,2,3,7,4]
    
    char* char_a_mostrar = NULL;
    pthread_mutex_lock(&mutex_cola_de_ready);

    int tam = queue_size(cola_de_ready);
    pcb* aux; 
    char_a_mostrar = agregar_al_final(char_a_mostrar, "[");
    for(int i=0; i<tam; i++)
    {
        aux = queue_pop(cola_de_ready);
        queue_push(cola_de_ready, aux);
        char_a_mostrar = agregar_al_final(char_a_mostrar, pasar_a_string(aux->pid));
        if (i < tam - 1) {
            char_a_mostrar = agregar_al_final(char_a_mostrar, ", ");
        }
        
    }
    char_a_mostrar = agregar_al_final(char_a_mostrar, "]");

    pthread_mutex_unlock(&mutex_cola_de_ready);

    log_info(log_kernel, "Cola Ready: %s", char_a_mostrar);

    free(char_a_mostrar);
}

void hacer_el_log_obligatorio_de_ingreso_a_ready_prioridad(pcb* proceso_a_mandar_a_ready)
{

    char* char_a_mostrar = NULL;
    pthread_mutex_lock(&mutex_cola_prioridad_vrr);

    int tam = queue_size(cola_prioridad_vrr);
    pcb* aux; 
    char_a_mostrar = agregar_al_final(char_a_mostrar, "[");
    for(int i=0; i<tam; i++)
    {
        aux = queue_pop(cola_prioridad_vrr);
        queue_push(cola_prioridad_vrr, aux);
        char_a_mostrar = agregar_al_final(char_a_mostrar, pasar_a_string(aux->pid));
        if (i < tam - 1) {
            char_a_mostrar = agregar_al_final(char_a_mostrar, ", ");
        }
        
    }
    char_a_mostrar = agregar_al_final(char_a_mostrar, "]");

    pthread_mutex_unlock(&mutex_cola_prioridad_vrr);

    log_info(log_kernel, "Cola Prioridad: %s", char_a_mostrar);

    free(char_a_mostrar);        
}

void machear_con_cola_gral(pcb* proceso)
{
    pthread_mutex_lock(&mutex_cola_general_de_procesos);
    pcb* aux = queue_pop(cola_general_de_procesos);
    int primer_pid = aux->pid;
    while(aux->pid != proceso->pid)
    {
        queue_push(cola_general_de_procesos, aux);
        aux = queue_pop(cola_general_de_procesos);
        
        if(aux->pid == primer_pid)
        {
            break;
            log_error(log_kernel, "No pude machear el proceso con el de la cola gral");
        }
    }
    //si sali es porq en aux tengo el q era, ahora pongo el actualizado en la cola en vez de aux
    queue_push(cola_general_de_procesos, proceso);
    pthread_mutex_unlock(&mutex_cola_general_de_procesos);
}

void actualizar_pcb(pcb* proceso_original, pcb* nuevo)
{
    proceso_original->registros->pc = nuevo->registros->pc;
    //proceso_original->estado_del_proceso = nuevo->estado_del_proceso;
    proceso_original->registros->ax = nuevo->registros->ax;
    proceso_original->registros->bx = nuevo->registros->bx;
    proceso_original->registros->cx = nuevo->registros->cx;
    proceso_original->registros->dx = nuevo->registros->dx;
    proceso_original->registros->eax = nuevo->registros->eax;
    proceso_original->registros->ebx = nuevo->registros->ebx;
    proceso_original->registros->ecx = nuevo->registros->ecx;
    proceso_original->registros->edx = nuevo->registros->edx;
    proceso_original->registros->si = nuevo->registros->si;
    proceso_original->registros->di = nuevo->registros->di;

    return;
    
}
// ------------------------------------------------
// ------------- FIN FUNCIONES VARIAS -------------
// ------------------------------------------------

