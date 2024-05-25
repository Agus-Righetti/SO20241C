#include "procesos.h"

// ************* Funcion que crea un hilo para mantener la consola siempre abierta *************
pthread_t hilo_consola (){ 

    // Creo el hilo de consola
    pthread_t thread_consola;
            
    // ********* CREO EL HILO SERVER PARA RECIBIR A CPU *********
    pthread_create(&thread_consola, NULL, (void*)leer_consola, NULL);

    return thread_consola;
}

pthread_t hilo_enviar_procesos_cpu (){ 

    // Creo el hilo de consola
    pthread_t thread_enviar_procesos_cpu;
            
    // ********* CREO EL HILO SERVER PARA RECIBIR A CPU *********
    pthread_create(&thread_enviar_procesos_cpu, NULL, (void*)enviar_proceso_a_cpu, NULL);

    return thread_enviar_procesos_cpu;
}

// ************* Funcion que utiliza un hilo para mantener la consola siempre abierta *************
void leer_consola(void* arg){
    
     while(1) {
        // Pido que ingresen un comando
        char* lectura = readline("Ingrese comando: ");

        // partes me separa segun los espacios lo que hay, en partes[0] esta INICIAR_PROCESO
        // en partes[1] va a estar el path
        char **partes = string_split(lectura, " "); 

        // Si el comando es "INCIAR_PROCESO entonces llamo a la funcion correspondiente"
        if (strcmp(partes[0], "INICIAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
            iniciar_proceso(partes[1]);// Esta es la funcion a la que llamo

        } else if (strcmp(lectura, "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");

        } else {
            
            printf("Opción no válida\n");
        }

        // Libera la memoria asignada por readline
        free(lectura);
    }
}

// ************* Funcion que sirve para iniciar un proceso considerando la multiprogramacion *************
void iniciar_proceso(char* path ){    

    // Creo una estructura de pcb e inicializo todos los campos
    pcb* nuevo_pcb = malloc(sizeof(pcb)); //HAY QUE LIBERAR EN EXIT
    pid_contador += 1;
    nuevo_pcb->estado_del_proceso = NEW;
    nuevo_pcb->program_counter = 0;
    nuevo_pcb->direccion_instrucciones = path; 
    nuevo_pcb->pid = pid_contador;
    nuevo_pcb->quantum = 0;
    nuevo_pcb->registros = malloc(sizeof(registros_cpu)); //HAY QUE LIBERARLO
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

    //Log obligatorio
    log_info(log_kernel, "Se crea el proceso %d en NEW", nuevo_pcb->pid);
    t_paquete* paquete_path = crear_paquete_personalizado(CREACION_PROCESO_KERNEL_A_MEMORIA);
    agregar_string_al_paquete_personalizado(paquete_path, path);
    agregar_int_al_paquete_personalizado(paquete_path, nuevo_pcb->pid);
    enviar_paquete(paquete_path, conexion_kernel_memoria);
    eliminar_paquete(paquete_path);

    // Si el grado de multiprogramacion me lo permite, modifico el estado a READY
    int multiprogramacion_actual;
    sem_getvalue(&sem_multiprogramacion, &multiprogramacion_actual);
    if (multiprogramacion_actual > 0)
    {   
        //Chequeo si tengo lugar para aceptar otro proceso en base al grado de multiprogramacion actual q tengo
        
        // Modifico el estado del proceso - Uso semaforo porque es una variable que tocan muchos hilos
        pthread_mutex_lock(&nuevo_pcb->mutex_pcb);
        nuevo_pcb->estado_del_proceso = READY; 
        pthread_mutex_unlock(&nuevo_pcb->mutex_pcb);

        // Ingreso el proceso a la cola de READY - Tambien semaforo porque es seccion critica 
        pthread_mutex_lock(&mutex_cola_de_ready);
        queue_push(cola_de_ready, nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_ready);
        sem_post(&sem_cola_de_ready); //agrego 1 al semaforo contador
        
        //bajo el grado de programacion actual, ya que agregue un proceso mas a ready
        sem_wait(&sem_multiprogramacion); //resto 1 al grado de multiprogramacion

    }else {
        // Si no hay espacio para un nuevo proceso en ready lo sumo a la cola de NEW - Semaforo SC
        pthread_mutex_lock(&mutex_cola_de_new);
        queue_push(cola_de_new, nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_new);
        sem_post(&sem_cola_de_new); //agrego 1 al semaforo contador
    }
}


// ************* Funcion para enviar un proceso a cpu *************
// Es llamada por un hilo especifico para esto
void enviar_proceso_a_cpu(){

    while(1){

        sem_wait(&sem_puedo_mandar_a_cpu);//espero si ya hay otro proceso ejecutando en CPU
        sem_wait(&sem_cola_de_ready); //hago que haya algo dentro de la cola de ready
        // Saco el proceso siguiente de la cola de READY
        pthread_mutex_lock(&mutex_cola_de_ready);
        pcb* proceso_seleccionado = queue_pop(cola_de_ready);
        pthread_mutex_unlock(&mutex_cola_de_ready);

        //Verifico que el estado del proceso sea READY
        if (proceso_seleccionado->estado_del_proceso == READY) 
        {
            // Cambio el estado del proceso sacado de la cola de READY
            pthread_mutex_lock(&proceso_seleccionado->mutex_pcb);
            proceso_seleccionado->estado_del_proceso = EXECUTE; 
            pthread_mutex_unlock(&proceso_seleccionado->mutex_pcb);

            // Hago un log obligatorio
            log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso_seleccionado->pid); 

            // Envio el pcb a CPU
            enviar_pcb(proceso_seleccionado); 

            // Inicio un hilo que maneje la ejecucion del proceso
            crear_hilo_proceso (proceso_seleccionado); 

        }else
        {
            // Si el proceso seleccionado no se encuentra en READY muestro un error
            error_show("El estado seleccionado no se encuentra en estado 'READY'");
        }
    }
}

// ************* Funcion que sirve para enviar el pcb como tal al CPU *************
void enviar_pcb(pcb* proceso) {

    // Creo un paquete
    t_paquete* paquete_pcb = crear_paquete_personalizado(PCB_KERNEL_A_CPU);

    // Agrego el struct pcb al paquete
    agregar_estructura_al_paquete_personalizado(paquete_pcb, proceso, sizeof(pcb));

    // Envio el paquete a través del socket
    enviar_paquete(paquete_pcb, conexion_kernel_cpu);

    // Libero el paquete
    eliminar_paquete(paquete_pcb);
}

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
    // Si el algoritmo es RR, lo recibo por una interrupcion contra CPU
    else if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0)
    {
        pthread_create(&hilo_recibe_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); 
        pthread_create(&hilo_interrupcion, NULL, (void*)algoritmo_round_robin,(void*)&args_hilo); 
        
        pthread_join(hilo_recibe_proceso, NULL);
        sem_wait(&destruir_hilo_interrupcion);
        pthread_cancel(hilo_interrupcion);

    }
}

void algoritmo_round_robin (void* arg){

    //aca vamos a armar dos hilos, uno que maneje solo cuando mandarle
    //la interrupcion y otro que siemp este esperando a q cpu le mande el pcb
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;

    usleep(*config_kernel->quantum);
    desalojar_proceso_hilo(args);
    
    return;
}

void accionar_segun_estado(pcb* proceso, int flag)
{
    //flag = 1, ya ejecuto todo, tengo q pasarlo a exit
    //flag = 0 aun no ejecuto del todo, lo mando a la cola de ready
    if(flag == 1){
        pasar_proceso_a_exit(proceso);
    }else if (flag ==0)
    {
        //lo mando a la cola de ready
        pthread_mutex_lock(&proceso->mutex_pcb);
        proceso->estado_del_proceso = READY; 
        pthread_mutex_unlock(&proceso->mutex_pcb);
        
        pthread_mutex_lock(&mutex_cola_de_ready);
        queue_push(cola_de_ready,proceso);
        pthread_mutex_unlock(&mutex_cola_de_ready);
        sem_post(&sem_cola_de_ready);

        // Hago un log obligatorio
        log_info(log_kernel, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: READY", proceso->pid);
    }
    
    return;

}

void pasar_proceso_a_exit(pcb* proceso)
{
    pthread_mutex_lock(&proceso->mutex_pcb);
    proceso->estado_del_proceso = EXIT; 
    pthread_mutex_unlock(&proceso->mutex_pcb);

    log_info(log_kernel, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: EXIT", proceso->pid);
    //aca lo tengo q cargar a la cola de exit
    pthread_mutex_lock(&cola_de_exit);
    queue_push(proceso, cola_de_exit);
    pthread_mutex_unlock(&cola_de_exit);

    //estos dos free tendrian q hacerse cuando se termina el programa (todo el programa)
    //free(proceso->registros);
    //free(proceso); 
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
    // Falta destruir el hilo
}

// ************* Funcion para recibir el pcb desde CPU si termina su ejecucion sin interrupcion *************
void recibir_pcb(pcb* proceso) {
    
    // Creo un nuevo paquete

    t_paquete* paquete = crear_paquete_personalizado(PCB_CPU_A_KERNEL);

    // Recibo el paquete a través del socket y los guardo en una lista

    paquete->buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);

    if(strcmp(config_kernel->algoritmo_planificacion, "RR") == 0)
    {
        //VER SI FUNCA
        sem_post(&destruir_hilo_interrupcion);
    }

    proceso = recibir_estructura_del_buffer(paquete->buffer);
    
    log_info(log_kernel, "Recibi PID: %d", proceso->pid);
    
    eliminar_paquete(paquete);

    //Este paquete es para recibir el flag, el flag esta en 1 si el proceso ta ejecuto todo y en 0 si aun le falta


    t_paquete* paquete_estado = crear_paquete_personalizado(CPU_TERMINA_EJECUCION_PCB);

    paquete_estado->buffer = recibiendo_paquete_personalizado(conexion_kernel_cpu);

    int flag_estado = recibir_int_del_buffer(paquete_estado->buffer);

    sem_post(&sem_puedo_mandar_a_cpu);//aviso que ya volvio el proceso q estaba en CPU, puedo mandar otro

    
    // Libero el paquete

    eliminar_paquete(paquete_estado);

    accionar_segun_estado(proceso, flag_estado); //este va a mandar el proceso a ready o exit


    return;
}

// ************* Funcion que llama a la de arriba a traves de un hilo *************
void recibir_pcb_hilo(void* arg){
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;
    pcb* proceso = args->proceso;

    recibir_pcb(proceso);
    //-------------------------falta destruir el hilo-----------
}
