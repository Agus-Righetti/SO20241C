#include "interfazSTDOUT.h"

// void liberar_configuracion_stdout(Interfaz* configuracion)
// {
//     if(configuracion) 
//     {
//         free(configuracion->archivo->tipo_interfaz);
//         // free(configuracion->archivo->tiempo_unidad_trabajo);
//         free(configuracion->archivo->ip_kernel);
//         // free(configuracion->archivo->puerto_kernel);
//         free(configuracion->archivo->ip_memoria);
//         // free(configuracion->archivo->puerto_memoria);
//         free(configuracion->archivo);
//     }
// }

void ejecutar_instruccion_stdout(t_list* direccion_fisica, int tamanio, int pid) 
{
    // No pido traduccion, ya me mandan la fisica.
    
    log_info(log_io, "PID: %d - Ejecutando instrucción IO_STDOUT_WRITE", pid);

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);
    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_list));
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    enviar_paquete(paquete, conexion_io_memoria);
}