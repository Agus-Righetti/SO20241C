#include "procesos.h"
#include "configuracion.h"

extern t_log* log_kernel;
extern kernel_config* config_kernel;
extern t_queue* cola_de_new;
extern t_queue* cola_de_ready;
extern int grado_multiprogramacion_actual;
extern int pid_contador; 
extern int conexion_kernel_cpu;
extern int interrupcion_kernel_cpu;
extern int conexion_kernel_memoria;
extern pthread_mutex_t mutex_cola_de_ready;
extern pthread_mutex_t mutex_cola_de_new;
extern pthread_mutex_t mutex_grado_programacion;

//Las extern son variables de otro archivo q quiero usar en este
//Atencion con las variables cola de new, cola de ready y grado de multiprogramacion actual, habria que implementar semaforo


pthread_t hilo_consola (){ 

    // Creo lel hilo de consola
    pthread_t thread_consola;
            
    // ********* CREO EL HILO SERVER PARA RECIBIR A CPU *********
    pthread_create(&thread_consola, NULL, (void*)leer_consola, NULL); // Si saco lo de arriba el ultimo parametro == NULL

    return thread_consola;
}

void leer_consola(void* arg){
    
     while(1) {
        char* lectura = readline("Ingrese comando: ");

        char **partes = string_split(lectura, " "); 
        // partes me separa segun los espacios lo que hay, en partes[0] esta INICIAR_PROCESO
        // en partes[1] va a estar el path

        if (strcmp(partes[0], "INICIAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción INICIAR_PROCESO\n");
            iniciar_proceso(partes[1]);

        } else if (strcmp(lectura, "FINALIZAR_PROCESO") == 0) {

            printf("Ha seleccionado la opción FINALIZAR_PROCESO\n");

        } else {
            
            printf("Opción no válida\n");
        }

        // Libera la memoria asignada por readline
        free(lectura);
    }
}

void iniciar_proceso(char* path )
{    
    
    //le mando a memoria el path ingresado en consola
    enviar_mensaje(path , conexion_kernel_memoria); 
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

    //log obligatorio
    log_info(log_kernel, "Se crea el proceso %d en NEW", nuevo_pcb->pid);

    
    if (config_kernel->grado_multiprogramacion > grado_multiprogramacion_actual)
    {   
        //chequeo si tengo lugar para aceptar otro proceso en base al grado de multiprogramacion actual q tengo
        
        pthread_mutex_lock(&nuevo_pcb->mutex_pcb);
        nuevo_pcb->estado_del_proceso = READY; 
        pthread_mutex_unlock(&nuevo_pcb->mutex_pcb);

        pthread_mutex_lock(&mutex_cola_de_ready);
        queue_push(cola_de_ready, nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_ready);
        

        pthread_mutex_lock(&mutex_grado_programacion);
        grado_multiprogramacion_actual += 1; //aumento el grado de programacion actual, ya que agregue un proceso mas a ready
        pthread_mutex_unlock(&mutex_grado_programacion);

    }else {
        pthread_mutex_lock(&mutex_cola_de_new);
        queue_push(cola_de_new, nuevo_pcb);
        pthread_mutex_unlock(&mutex_cola_de_new);
    } // no tengo espacio para un nuevo proceso en ready, lo mando a la cola de new

    

}

// ****************** ACA ESTA HECHO PARA RECIBIR UN PROCESO COMO TAL, UN PCB ******************

// Funcion para enviar un proceso a cpu
void enviar_proceso_a_cpu(){

    pthread_mutex_lock(&mutex_cola_de_ready);
    pcb* proceso_seleccionado = queue_pop(cola_de_ready);
    pthread_mutex_unlock(&mutex_cola_de_ready);

    if (proceso_seleccionado->estado_del_proceso == READY) //Verifico que el estado del proceso sea ready
    {
        // ACA HAY QUE USAR SEMAFORO PARA CAMBIAR LOS ESTADOS DE UN PROCESO
        pthread_mutex_lock(&proceso_seleccionado->mutex_pcb);
        proceso_seleccionado->estado_del_proceso = EXECUTE; // Cambio el estado del proceso a Execute
        pthread_mutex_unlock(&proceso_seleccionado->mutex_pcb);
        log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso_seleccionado->pid); // Log obligatorio

        enviar_pcb(proceso_seleccionado); // Envio el pcb a CPU

        crear_hilo_proceso (proceso_seleccionado); //inicio un hilo que maneje la ejecucion del proceso

    }else
    {
        error_show("El estado seleccionado no se encuentra en estado 'READY'");
    }
}


void crear_hilo_proceso(pcb* proceso){

    //chequeamos el algoritmo a usar (VRR, RR, FIFO)
    //segun el algoritmo armamos un hilo

    pthread_t hilo_proceso;
    thread_args_procesos_kernel args_hilo = {proceso};
    if(config_kernel->algoritmo_planificacion == "FIFO")
    {
        pthread_create(&hilo_proceso, NULL, (void*)recibir_pcb_hilo,(void*)&args_hilo); 

    }else if(config_kernel->algoritmo_planificacion == "RR")
    {
        pthread_create(&hilo_proceso, NULL, (void*)algoritmo_round_robin,(void*)&args_hilo); 
    }
}

void algoritmo_round_robin (void* arg){

    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;
    usleep(config_kernel->quantum);
    desalojar_proceso_hilo(args);

    
    accionar_segun_estado(args->proceso); //si esta en exit elimina las estructuras, si vuelve en ready lo pone en la cola
    
    //-------------------falta destruir el hilo----------------
    
    return;
}

void accionar_segun_estado (pcb* proceso)
{
    if(proceso->estado_del_proceso == READY)
    {
        pthread_mutex_lock(&mutex_cola_de_ready);
        queue_push(cola_de_ready, proceso);
        pthread_mutex_unlock(&mutex_cola_de_ready);
    }
    //faltarian las opciones para exit
    return;

}
// Funcion que sirve para enviar el pcb como tal al CPU
void enviar_pcb(pcb* proceso) {
    
    // Creo un paquete
    t_paquete* paquete = crear_paquete();

    // Agrego el struct pcb al paquete
    agregar_a_paquete(paquete, proceso, sizeof(pcb));

    // Envio el paquete a través del socket
    enviar_paquete(paquete, conexion_kernel_cpu);

    // Libero el paquete
    eliminar_paquete(paquete);
}

// Entiendo que por ahora unicamente puede desalojar RR por fin de quantum
// Creo que hay que poner el proceso que se recibe al final de la cola de ready, pero creo que tambien iria en donde se llama a esta funcion
// Esta es para recibir el pcb en caso de que se lo interrumpa (a traves del puerto INTERRUPT)
void desalojar_proceso(pcb* proceso){
   
    //Esto capaz se puede poner desde donde se la llama a la funcion
    log_info(log_kernel, "PID: <PID> - Desalojado por fin de Quantum", proceso_seleccionado->pid);
    
    //en cpu deberiamos hacer que si se llega este mensaje deben devolver el pcb
    enviar_mensaje("FIN DE QUANTUM",interrupcion_a_cpu);
    
    // Creo un nuevo paquete
    t_paquete* paquete = crear_paquete();

    // Recibo el paquete a través del socket y los guardo en una lista
    t_list* valores_recibidos = recibir_paquete(interrupcion_kernel_cpu);

    // Verifico que se hayan recibidos valores
    if (valores_recibidos == NULL) {
        error_show("No se recibio el proceso por parte del CPU");
        return;
    }

    // Antes de copiar los datos, verifico que vino el tamaño que corresponde a un pcb
    if (paquete->buffer->size != sizeof(pcb)) {
        error_show("El tamaño del buffer no coincide con el tamaño esperado del proceso");
        eliminar_paquete(paquete);
        list_destroy_and_destroy_elements(valores_recibidos, free);
        return;
    }

    // Extraigo el struct pcb del paquete
    memcpy(proceso, paquete->buffer->stream, sizeof(pcb));

    // Libero el paquete
    eliminar_paquete(paquete);

    // Libero la lista y los elementos de la misma
    list_destroy_and_destroy_elements(valores_recibidos, free);
    return;
}

// Esto es lo mismo que la de arriba, solo que adaptada para poder llamarla por un hilo
void desalojar_proceso_hilo(void* arg){
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;
    pcb* proceso = args->proceso;

    desalojar_proceso(proceso); 
}

// Funcion para recibir el pcb desde CPU si termina su ejecucion sin interrupcion
void recibir_pcb(pcb* proceso) {
    
    // Creo un nuevo paquete
    t_paquete* paquete = crear_paquete();

    // Recibo el paquete a través del socket y los guardo en una lista
    t_list* valores_recibidos = recibir_paquete(conexion_kernel_cpu);

    // Verifico que se hayan recibidos valores
    if (valores_recibidos == NULL) {
        error_show("No se recibio el proceso por parte del CPU");
        return;
    }

    // Antes de copiar los datos, verifico que vino el tamaño que corresponde a un pcb
    if (paquete->buffer->size != sizeof(pcb)) {
        error_show("El tamaño del buffer no coincide con el tamaño esperado del proceso");
        eliminar_paquete(paquete);
        list_destroy_and_destroy_elements(valores_recibidos, free);
        return;
    }

    // Extraigo el struct pcb del paquete
    memcpy(proceso, paquete->buffer->stream, sizeof(pcb));

    // Libero el paquete
    eliminar_paquete(paquete);

    // Libero la lista y los elementos de la misma
    list_destroy_and_destroy_elements(valores_recibidos, free);
    return;
}

// Adaptado para poder recibir un pcb usando hilos
void recibir_pcb_hilo(void* arg){
    thread_args_procesos_kernel* args = (thread_args_procesos_kernel*)arg;
    pcb* proceso = args->proceso;

    recibir_pcb(proceso);
    //-------------------------falta destruir el hilo-----------
}


// ****************** ACA ESTA HECHO PARA RECIBIR UN HILO, pero no hay que usarlo asi ******************
// No borrar por ahora pero creo que no voy a usar esto

// void* enviar_proceso_a_cpu(void* args) {

//     thread_args_procesos_kernel* proceso_hilo_args = (thread_args_procesos_kernel*)args; // Tomo los args del hilo-proceso

//     // Creo las variables que quiero usar a partir de los argumentos del hilo
//     pcb* proceso = proceso_hilo_args->proceso;
//     int socket_cliente = proceso_hilo_args->socket_cliente;
//     // t_log* log_kernel = proceso_hilo_args->log_kernel;

//     if (proceso->estado_del_proceso == READY) { // Verifico el estado del proceso actual

//         proceso->estado_del_proceso = EXEC; // Si se encuentra en ready lo paso a execute
//         log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);

//         enviar_pcb(socket_cliente, proceso); // Envio el proceso a cpu
//         recibir_pcb(socket_cliente, proceso); // Espero que cpu me devuelva el proceso con valores actualizados
//         log_info(log_kernel, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: %s", proceso->pid, proceso->estado_del_proceso);
//         // Aca hay que seguir trabajando con el pcb segun sus valores actualizados
//     } 
//     else {
//         error_show("El estado seleccionado no se encuentra en estado 'READY'"); // Marco error si el proceso que ingresa no se encuentra en estado "READY"
//     }

//     pthread_exit(NULL); // Finalizo el hilo
// }