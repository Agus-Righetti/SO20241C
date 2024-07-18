#include "interfazSTDOUT.h"

void ejecutar_instruccion_stdout(t_list* direccion_fisica, int tamanio, int pid) 
{
    // No pido traduccion, ya me mandan la fisica.
    
    log_info(log_io, "PID: %d - Ejecutando instrucción IO_STDOUT_WRITE", pid);

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_list));
    agregar_int_al_paquete_personalizado(paquete, tamanio);

    enviar_paquete(paquete, conexion_io_memoria);

    eliminar_paquete(paquete);

    sem_wait(&sem_ok_lectura_memoria);
    
    //muestro por pantalla lo que lei de memoria
    log_info(log_io, valor_a_mostrar);

    avisar_fin_io_a_kernel();
}