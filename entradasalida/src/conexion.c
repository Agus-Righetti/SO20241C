#include "conexion.h"

pthread_t escuchar_memoria()
{
    pthread_t thread;

    pthread_create(&thread, NULL, (void*)atender_a_memoria, NULL);

    return thread;
}

void atender_kernel()
{
    // Si bien nosotros somos clientes de kernel, en este momento se podria decir que estamos actuando como servidores al recibir la operacion
    int cod_op_io;
    //recibimos de kernel una operacion de ES

    t_buffer* buffer; 
    int pid;
    int unidades_de_trabajo;
    int registro_direccion;
    int registro_tamano;
    int registro_puntero_archivo;
    char* nombre_archivo;

    while(1)
    {
        cod_op_io = recibir_operacion(conexion_io_kernel);

        buffer = recibiendo_paquete_personalizado(conexion_io_kernel); // Recibo el PCB normalmente
        
        pid = recibir_int_del_buffer(buffer);

        switch(cod_op_io) // Segun el codigo de operación actúo 
        {
            case IO_GEN_SLEEP: //(Interfaz, Unidades de trabajo)

                unidades_de_trabajo = recibir_int_del_buffer(buffer);
                realizar_sleep(unidades_de_trabajo, pid);
                break;

            case IO_STDIN_READ: //(Interfaz, Registro Dirección, Registro Tamaño)

                registro_direccion = recibir_int_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);
                leer_consola(registro_direccion, registro_tamano, pid);

                break;

            case IO_STDOUT_WRITE: //(Interfaz, Registro Dirección, Registro Tamaño)

                registro_direccion = recibir_int_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);
                ejecutar_instruccion_stdout(registro_direccion, registro_tamano, pid);

                break;
            
            case IO_FS_CREATE: // (Interfaz, Nombre Archivo)
                
                nombre_archivo = recibir_string_del_buffer(buffer);
                manejar_creacion_archivo(nombre_archivo, pid);
            
                break;
                
            case IO_FS_DELETE: //(Interfaz, Nombre Archivo)

                nombre_archivo = recibir_string_del_buffer(buffer);

                break;

            case IO_FS_TRUNCATE: //(Interfaz, Nombre Archivo, Registro Tamaño)

                nombre_archivo = recibir_string_del_buffer(buffer); 
                registro_tamano = recibir_int_del_buffer(buffer);

                break;

            case IO_FS_WRITE: //(Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)

                nombre_archivo = recibir_string_del_buffer(buffer);
                registro_direccion = recibir_int_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);

                break;

            case IO_FS_READ: //(Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
            
                nombre_archivo = recibir_string_del_buffer(buffer);
                registro_direccion = recibir_int_del_buffer(buffer);
                registro_tamano = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);

                break;
            
            default:
                log_error(log_io, "El codigo de operacion no es reconocido :(");
                break;
        }

        // ************ 

        // Interfaz configuracion;

        // switch(cod_op_io)
        // {
        //     case GENERICA:
        //         leer_configuracion_generica(&configuracion);
        //         recibir_operacion_generica_de_kernel(GENERICA, cod_op_io);
        //         liberar_configuracion_generica(&configuracion);
        //         break;
        //     case STDIN:
        //         leer_configuracion_stdin(&configuracion);
        //         recibir_operacion_stdin_de_kernel(STDIN, cod_op_io);
        //         liberar_configuracion_stdin(&configuracion);
        //         break;
        //     case STDOUT:
        //         leer_configuracion_stdout(&configuracion);
        //         recibir_operacion_stdout_de_kernel(STDOUT, cod_op_io);
        //         liberar_configuracion_stdout(&configuracion);
        //         break;
        //     case DIALFS:
        //         leer_configuracion_dialfs(&configuracion);
        //         recibir_operacion_dialfs_de_kernel(DIALFS, cod_op_io);
        //         liberar_configuracion_dialfs(&configuracion);
        //         break;
        //     case -1:
        //         log_error(log_io, "KERNEL se desconecto. Terminando servidor");
        //         // free(conexion_io_kernel);
        //         exit(1);
        //         return;
        //     default:
        //         log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
        //         break;
        // }
    }
}

pthread_t escuchar_kernel()
{
    pthread_t thread;

    pthread_create(&thread, NULL, (void*)atender_kernel, NULL);

    return thread;

}

// Envia la solicitud de traduccion a Kernel y recibe la direccion fisica
// int solicitar_traduccion_direccion(int direccion_logica) 
// {
//     int direccion_fisica = -1; // Inicializamos la dirección física como -1 por defecto

//     t_paquete* paquete = crear_paquete_personalizado(SOLICITAR_TRADUCCION);
//     agregar_int_al_paquete_personalizado(paquete, direccion_logica);
//     enviar_paquete(paquete, conexion_io_kernel);

//     while(1)
//     { 
//         int cod_op_kernel = recibir_operacion(conexion_io_kernel);
//         switch (cod_op_kernel) 
//         {
//             case IO_RECIBE_TRADUCCION_DE_KERNEL:
//                 t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_kernel);
//                 direccion_fisica = recibir_int_del_buffer(buffer);
//                 free(buffer);
//                 break;
//             case -1:
//                 log_error(log_io, "KERNEL se desconecto. Terminando servidor");
//                 return EXIT_FAILURE;
//             default:
//                 log_warning(log_io, "Operacion desconocida. No quieras meter la pata");
//                 break;
//         }
//     }
    
//     eliminar_paquete(paquete);
//     return direccion_fisica;
// }

void atender_a_memoria(){

    int cod_op_io;
    t_buffer* buffer;

    while(1){

        cod_op_io = recibir_operacion(conexion_io_memoria);

        buffer = recibiendo_paquete_personalizado(conexion_io_memoria); 
        
        switch(cod_op_io) // Segun el codigo de operación actúo 
        { 
            case IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA:

                char* valor_a_mostrar = recibir_string_del_buffer(buffer);
            
                if(strcmp(config_io->tipo_interfaz, "STDOUT"))
                {
                    printf("Valor leído en memoria: %s", valor_a_mostrar);
                    avisar_fin_io_a_kernel();
                }
                
                break;
            default:
                log_error(log_io, "El codigo de operacion no es reconocido :(");
                break;
        }
    }

    free(buffer->stream);
    free(buffer);

}