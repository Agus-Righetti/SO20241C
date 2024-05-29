#include "conexion.h"

void escuchar_memoria()
{
    conexion_io_memoria = conexion_a_memoria(log_io, config_io); 
}

void atender_kernel()
{
    // conexion_io_kernel = conexion_a_kernel(log_io, config_io);
    // int cod_op_io;

    // Si bien nosotros somos clientes de kernel, en este momento se podria decir que estamos actuando como servidores al recibir la operacion
    int cod_op_io = recibir_operacion(conexion_io_kernel);

    while(1)
    {
        Interfaz configuracion;

        switch(cod_op_io)
        {
            // case GENERICA:
            //     leer_configuracion_generica(&configuracion);
            //     recibir_operacion_generica_de_kernel(GENERICA, cod_op_io);
            //     liberar_configuracion(&configuracion);
            //     break;
            // case STDIN:
            //     leer_consola(log_io);
            //     leer_configuracion_stdin(configuracion);
            //     break;
            // case STDOUT:
            //     leer_configuracion_stdout(configuracion);
            //     break;
            // case DIALFS:
            //     leer_configuracion_dialfs(configuracion);
            //     break;
            case -1:
                log_error(log_io, "KERNEL se desconecto. Terminando servidor");
                // free(socket_servidor_kernel);
                exit(1);
                return;
            default:
                log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

void escuchar_kernel(){
    pthread_t thread;
    conexion_io_kernel = conexion_a_kernel(log_io, config_io);
    pthread_create(&thread, NULL, (void*)atender_kernel, NULL);
    pthread_join(thread, NULL);
}
