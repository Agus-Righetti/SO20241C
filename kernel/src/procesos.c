#include "procesos.h"
typedef enum {
	READY,
	BLOCKED,
	EXECUTE,
	NEW,
	EXIT,
}estados;

// Entiendo que los algoritmos de planificacion me tienen que mandar el proceso seleccionado por referencia asi lo puedo modificar

// ****************** ACA ESTA HECHO PARA RECIBIR UN PROCESO COMO TAL, UN PCB ******************

// void enviar_proceso_a_cpu(pcb* proceso_seleccionado, int socket_cliente){
//     if (proceso_seleccionado->estado_del_proceso == READY) //Verifico que el estado del proceso sea ready
//     {
//         proceso_seleccionado->estado_del_proceso = EXEC; // Cambio el estado del proceso a Execute
//         log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso_seleccionado->pid);

//         enviar_pcb(socket_cliente, proceso_seleccionado); // Tengo que obtener el socket cliente de la conexion de dispatch, y envio el pcb a cpu
//     }else
//     {
//         error_show("El estado seleccionado no se encuentra en estado 'READY'");
//     }
// }

// ****************** ACA ESTA HECHO PARA RECIBIR UN HILO, creo que hay que usarlo asi ******************

// Entiendo que de donde se llame a la funcion tengo que ir creando los hilos que representan a cada proceso y ahi mando sus argumentos

void* enviar_proceso_a_cpu(void* args) {

    thread_args_procesos_kernel* proceso_hilo_args = (thread_args_procesos_kernel*)args; // Tomo los args del hilo-proceso

    // Creo las variables que quiero usar a partir de los argumentos del hilo
    pcb* proceso = proceso_hilo_args->proceso;
    int socket_cliente = proceso_hilo_args->socket_cliente;
    t_log* log_kernel = proceso_hilo_args->log_kernel;

    if (proceso->estado_del_proceso == READY) { // Verifico el estado del proceso actual

        proceso->estado_del_proceso = EXEC; // Si se encuentra en ready lo paso a execute
        log_info(log_kernel, "PID: %d - Estado Anterior: READY - Estado Actual: EXECUTE", proceso->pid);

        enviar_pcb(socket_cliente, proceso); // Envio el proceso a cpu
        recibir_pcb(socket_cliente, proceso); // Espero que cpu me devuelva el proceso con valores actualizados
        log_info(log_kernel, "PID: %d - Estado Anterior: EXECUTE - Estado Actual: %s", proceso->pid, proceso->estado_del_proceso);
    } 
    else {
        error_show("El estado seleccionado no se encuentra en estado 'READY'"); // Marco error si el proceso que ingresa no se encuentra en estado "READY"
    }

    pthread_exit(NULL); // Finalizo el hilo
}

void enviar_pcb(int socket_cliente, pcb* proceso) {
    // Creo un paquete
    t_paquete* paquete = crear_paquete();

    // Agrego el struct pcb al paquete
    agregar_a_paquete(paquete, proceso, sizeof(pcb));

    // Envio el paquete a través del socket, tengo que ver como conseguir el socket del cliente en esta instancia, esta en el main pero creo que a estas funciones se acceden desde las de Maria
    
    if (enviar_paquete(paquete, socket_cliente) == -1) { // Verifico que no haya problemas en el envío del paquete
        error_show("Error al enviar el Proceso al CPU.");
    }

    // Liberar el paquete
    eliminar_paquete(paquete);
}

void recibir_pcb_(int socket_cliente, pcb* proceso) {
    // Crear un nuevo paquete
    t_paquete* paquete = crear_paquete();

    // Recibir el paquete a través del socket
    if (recibir_paquete(paquete, socket_cliente) == -1) { // Verificar si hay problemas al recibir el paquete
        error_show("Error al recibir el paquete del CPU.");
        eliminar_paquete(paquete);
        return;
    }

    // Extraer el struct pcb del paquete
    memcpy(proceso, paquete->buffer->stream, sizeof(pcb));

    // Liberar el paquete
    eliminar_paquete(paquete);
}



