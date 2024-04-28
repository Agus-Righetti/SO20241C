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