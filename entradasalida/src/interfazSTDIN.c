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

    //PEDIR A MEMORIA - VICKY ACÁ SOS VOS
    
    //ahora le tenemos q decir a memoria q lo guarde en la direccion del registro direccion
    //tiene q guardar lo q esta en la variable texto en la pos "direccion_fisica", va a tener el tamano "tamanio"

}

// void recibir_operacion_stdin_de_kernel(Interfaz* interfaz, op_code codigo)
// {
//     // Verificar si la operación es para una interfaz stdin
//     if (codigo == IO_STDIN_READ) 
//     {
//         log_info(log_io, "Operacion recibida: IO_STDIN_READ.");
//         leer_consola(); // Esta función se encuentra en el módulo de I/O y debe estar definida allí
//     } 
//     else if (codigo == -1) 
//     {
//         log_error(log_io, "KERNEL se desconecto. Terminando servidor");
//         exit(1);
//     } 
//     else 
//     {
//         log_warning(log_io, "Operacion recibida no es para una interfaz STDIN.\n");
//     }
// }

// void liberar_configuracion_stdin(Interfaz* configuracion)
// {
//     if(configuracion) 
//     {
//         free(configuracion->archivo->tipo_interfaz);
//         free(configuracion->archivo->ip_kernel);
//         // free(configuracion->archivo->puerto_kernel);
//         free(configuracion->archivo->ip_memoria);
//         // free(configuracion->archivo->puerto_memoria);
//         free(configuracion->archivo);
//     }
// }