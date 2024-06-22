#include "conexion.h"

void escuchar_memoria()
{
    conexion_io_memoria = conexion_a_memoria(log_io, config_io); 
}

void atender_kernel()
{
    // Si bien nosotros somos clientes de kernel, en este momento se podria decir que estamos actuando como servidores al recibir la operacion
    int cod_op_io = recibir_operacion(conexion_io_kernel);

    while(1)
    {
        Interfaz configuracion;

        switch(cod_op_io)
        {
            case GENERICA:
                leer_configuracion_generica(&configuracion);
                recibir_operacion_generica_de_kernel(GENERICA, cod_op_io);
                liberar_configuracion_generica(&configuracion);
                break;
            case STDIN:
                leer_configuracion_stdin(&configuracion);
                recibir_operacion_stdin_de_kernel(STDIN, cod_op_io);
                liberar_configuracion_stdin(&configuracion);
                break;
            case STDOUT:
                leer_configuracion_stdout(&configuracion);
                recibir_operacion_stdout_de_kernel(STDOUT, cod_op_io);
                liberar_configuracion_stdout(&configuracion);
                break;
            case DIALFS:
                leer_configuracion_dialfs(&configuracion);
                recibir_operacion_dialfs_de_kernel(DIALFS, cod_op_io);
                liberar_configuracion_dialfs(&configuracion);
                break;
            case -1:
                log_error(log_io, "KERNEL se desconecto. Terminando servidor");
                // free(conexion_io_kernel);
                exit(1);
                return;
            default:
                log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
}

void escuchar_kernel()
{
    pthread_t thread;
    conexion_io_kernel = conexion_a_kernel(log_io, config_io);
    pthread_create(&thread, NULL, (void*)atender_kernel, NULL);
    pthread_join(thread, NULL);
}

// Envia la solicitud de traduccion a Kernel y recibe la direccion fisica
int solicitar_traduccion_direccion(int direccion_logica) 
{
    int direccion_fisica = -1; // Inicializamos la dirección física como -1 por defecto

    t_paquete* paquete = crear_paquete_personalizado(SOLICITAR_TRADUCCION);
    agregar_int_al_paquete_personalizado(paquete, direccion_logica);
    enviar_paquete(paquete, conexion_io_kernel);

    while(1)
    { 
        int cod_op_kernel = recibir_operacion(conexion_io_kernel);
        switch (cod_op_kernel) 
        {
            case IO_RECIBE_TRADUCCION_DE_KERNEL:
                t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_kernel);
                direccion_fisica = recibir_int_del_buffer(buffer);
                free(buffer);
                break;
            case -1:
                log_error(log_io, "KERNEL se desconecto. Terminando servidor");
                return EXIT_FAILURE;
            default:
                log_warning(log_io, "Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
    eliminar_paquete(paquete);
    return direccion_fisica;
}