#include "procesos.h"
#include "configuracion.h"

extern t_log* log_kernel;
extern kernel_config* config_kernel;
extern t_list* cola_de_new;
extern t_list* cola_de_ready;
extern int grado_multiprogramacion_actual;
extern int pid_contador; 
extern int conexion_kernel_cpu;

//Las extern son variables de otro archivo q quiero usar en este
//Atencion con las variables cola de new, cola de ready y grado de multiprogramacion actual, habria que implementar semaforo


pthread_t hilo_consola (t_log* log_kernel, kernel_config* config_kernel){ 
    // Creo lel hilo de consola
    pthread_t thread_consola;

    //Defino la estrcutura del argumento
    thread_args_kernel args = {config_kernel, log_kernel};
        
    // ********* CREO EL HILO SERVER PARA RECIBIR A CPU *********
    pthread_create(&thread_consola, NULL, (void*)leer_consola, (void*) &args);

    return thread_consola;
}

void leer_consola(){
    
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

    
    //aca creo el pcb solamente, ver si hay que madnarlo a memoria entero o que partes

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

    //log obligatorio
    log_info(log_kernel, "Se crea el proceso %d en NEW", nuevo_pcb->pid);

    
    if (config_kernel->grado_multiprogramacion > grado_multiprogramacion_actual)
    {   
        //chequeo si tengo lugar para aceptar otro proceso en base al grado de multiprogramacion actual q tengo
        
        nuevo_pcb->estado_del_proceso = READY; 

        list_add(cola_de_ready, nuevo_pcb);
        
        grado_multiprogramacion_actual += 1; //aumento el grado de programacion actual, ya que agregue un proceso mas a ready
        

    }else list_add(cola_de_new, nuevo_pcb); // no tengo espacio para un nuevo proceso en ready, lo mando a la cola de new

}


// Entiendo que los algoritmos de planificacion me tienen que mandar el proceso seleccionado por referencia asi lo puedo modificar

// ****************** ACA ESTA HECHO PARA RECIBIR UN PROCESO COMO TAL, UN PCB ******************

void enviar_proceso_a_cpu(pcb* proceso_seleccionado, int socket_cliente){
    if (proceso_seleccionado->estado_del_proceso == READY) //Verifico que el estado del proceso sea ready
    {
        proceso_seleccionado->estado_del_proceso = EXECUTE; // Cambio el estado del proceso a Execute
        log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso_seleccionado->pid);

        enviar_pcb(socket_cliente, proceso_seleccionado); // Tengo que obtener el socket cliente de la conexion de dispatch, y envio el pcb a cpu
    }else
    {
        error_show("El estado seleccionado no se encuentra en estado 'READY'");
    }
}

// ****************** ACA ESTA HECHO PARA RECIBIR UN HILO, creo que hay que usarlo asi ******************

// Entiendo que de donde se llame a la funcion tengo que ir creando los hilos que representan a cada proceso y ahi mando sus argumentos

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

void enviar_pcb(int socket_cliente, pcb* proceso) {
    // Creo un paquete
    t_paquete* paquete = crear_paquete();

    // Agrego el struct pcb al paquete
    agregar_a_paquete(paquete, proceso, sizeof(pcb));

    // Envio el paquete a través del socket, tengo que ver como conseguir el socket del cliente en esta instancia, esta en el main pero creo que a estas funciones se acceden desde las de Maria
    enviar_paquete(paquete, socket_cliente);

    //if (enviar_paquete(paquete, socket_cliente) == -1) { // Verifico que no haya problemas en el envío del paquete
      //  error_show("Error al enviar el Proceso al CPU.");
    //}

    // Liberar el paquete
    eliminar_paquete(paquete);
}

void recibir_pcb_(int socket_cliente, pcb* proceso) {
    // Crear un nuevo paquete
    t_paquete* paquete = crear_paquete();

    // Recibir el paquete a través del socket
    recibir_paquete(socket_cliente);
    
    // if (recibir_paquete(paquete, socket_cliente) == -1) { // Verificar si hay problemas al recibir el paquete
    //     error_show("Error al recibir el paquete del CPU.");
    //     eliminar_paquete(paquete);
        
    // }

    // Extraer el struct pcb del paquete
    memcpy(proceso, paquete->buffer->stream, sizeof(pcb));

    // Liberar el paquete
    eliminar_paquete(paquete);
    return;
}




