#include "interfazSTDIN.h"

// Trunca el texto al tamaño deseado, y luego lo envía a memoria para ser guardado en las direcciones físicas especificadas.

void leer_consola(t_list* direccion_fisica, int tamanio, int pid) 
{
    log_info(log_io, "PID: %d - Operacion: STDIN", pid);

    char *leido;
    char *texto;
    
    // Lee toda la entrada de la consola
    leido = readline("Ingrese el texto: > ");

    // Trunca el texto al tamaño deseado
    strncpy(texto, leido, tamanio);
    texto[tamanio] = '\0'; // Asegura que el texto esté nulo terminado

    log_info(log_io, "Lo que me importa del texto ingresado es:  %s", texto);

    free(leido);

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_ESCRITURA_MEMORIA); // Queremos que memoria lo guarde

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    agregar_string_al_paquete_personalizado(paquete, texto);
    agregar_lista_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_direccion_fisica));
    

    enviar_paquete(paquete, conexion_io_memoria);

    eliminar_paquete(paquete);

    sem_wait(&sem_ok_escritura_memoria);

    avisar_fin_io_a_kernel(); //mando FIN_OP_IO a kernel :P
}
