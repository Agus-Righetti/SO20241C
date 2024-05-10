#include "conexion.h"

void escuchar_memoria()
{
    int conexion_io_memoria = conexion_a_memoria(log_io, config_io); 
}

void escuchar_kernel()
{
    int conexion_io_kernel = conexion_a_kernel(log_io, config_io);
    int cod_op_io;

    // Si bien nosotros somos clientes de kernel, en este momento se podria decir que estamos actuando como servidores al recibir la operacion
    cod_op_io = recibir_operacion(socket_servidor_kernel);

    while(1)
    {
        switch(cod_op_io)
        {
            case GENERICA:
                leer_configuracion();
                recibir_operacion_generica_de_kernel(cod_op_io);
                break;
            case STDIN:
                break;
            case STDOUT:
                break;
            case DIALFS:
                break;
            case -1:
                log_error(log_io, "KERNEL se desconecto. Terminando servidor");
                free(socket_servidor_kernel);
                exit(1);
                return;
            default:
                log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}
