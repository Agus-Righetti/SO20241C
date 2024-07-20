#include "interfazSTDIN.h"

// Trunca el texto al tamaño deseado, y luego lo envía a memoria para ser guardado en las direcciones físicas especificadas.

void leer_consola(t_list* direcciones_fisicas, int tamanio, int pid) 
{
    log_info(log_io, "PID: %d - Operacion: STDIN", pid);
    log_info(log_io, "El tamano de la lista q voy a mandar a memoria es:  %d", list_size(direcciones_fisicas));

    char *leido;
    char *texto = malloc(sizeof(char)* (tamanio + 1));
    
    // Lee toda la entrada de la consola
    leido = readline("Ingrese el texto: > ");

    // Trunca el texto al tamaño deseado
    strncpy(texto, leido, tamanio);
    texto[tamanio] = '\0'; // Asegura que el texto esté nulo terminado

    log_info(log_io, "Lo que me importa del texto ingresado es:  %s", texto);

    log_info(log_io, "El tamano de la lista q voy a mandar a memoria es:  %d", list_size(direcciones_fisicas));

    t_direccion_fisica* dir_fisica;
    for(int i = 0; i<list_size(direcciones_fisicas); i++)
    {
        dir_fisica = list_get(direcciones_fisicas, i);
        log_info(log_io, "el maeco nro %d antes de mandar a memoria es : %d", i,dir_fisica->nro_marco);
    }

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_ESCRITURA_MEMORIA); // Queremos que memoria lo guarde

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    agregar_string_al_paquete_personalizado(paquete, texto);
    agregar_lista_al_paquete_personalizado(paquete, direcciones_fisicas, sizeof(t_direccion_fisica));
    
    

    

    enviar_paquete(paquete, conexion_io_memoria);

    for(int i = 0; i<list_size(direcciones_fisicas); i++)
    {
        dir_fisica = list_get(direcciones_fisicas, i);
        log_info(log_io, "el maeco nro %d despues de mandar a memoria es : %d", i,dir_fisica->nro_marco);
        //free(dir_fisica);
    }
    free(leido);

    free(texto);

    eliminar_paquete(paquete);

    list_destroy(direcciones_fisicas);

    sem_wait(&sem_ok_escritura_memoria);

    avisar_fin_io_a_kernel(); //mando FIN_OP_IO a kernel :P
}
