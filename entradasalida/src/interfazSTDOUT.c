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

// void recibir_operacion_stdout_de_kernel(Interfaz* configuracion_stdout, op_code codigo) 
// {
//     // Verificar si la operación es para una interfaz stdout
//     if (codigo == IO_STDOUT_WRITE) 
//     {
//         // Lee los parámetros de la solicitud

//         t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_kernel);

//         char* interfaz = recibir_string_del_buffer(buffer);
//         char* registro_direccion = recibir_string_del_buffer(buffer);
//         char* registro_tamanio = recibir_string_del_buffer(buffer);
//         int valor_registro = recibir_int_del_buffer(buffer);
//         int direccion_logica = recibir_int_del_buffer(buffer);

//         // Convierte los registros a enteros
//         // int direccion_logica = obtener_valor_registro(registro_direccion);

//         log_info(log_io, "Operacion recibida: IO_STDOUT_WRITE. Interfaz: %s, Dirección Lógica: %d, Tamaño: %s", interfaz, direccion_logica, registro_tamanio);

//         // Ejecutar la instrucción
//         ejecutar_instruccion_stdout(configuracion_stdout, direccion_logica, atoi (registro_tamanio));

//         // Libera memoria de los registros
//         free(interfaz);
//         free(buffer);
//         free(registro_direccion);
//         free(registro_tamanio);
//     } 
//     else if (codigo == -1)
//     {
//         log_error(log_io, "KERNEL se desconecto. Terminando servidor");
//         exit(1);
//     } 
//     else
//     {
//         log_warning(log_io, "Operacion recibida no es para una interfaz STDOUT.\n");
//     }
// }

void ejecutar_instruccion_stdout(t_list* direccion_fisica, int tamanio, int pid) 
{
    //no pido traduccion, ya me mandan la fisica.
    //int direccion_fisica = solicitar_traduccion_direccion(direccion_logica);

    // Envio la solicitud de lectura a memoria
    // t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);
    // agregar_estructura_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_list));
    // agregar_int_al_paquete_personalizado(paquete, tamanio);
    // enviar_paquete(paquete, conexion_io_memoria);   
}