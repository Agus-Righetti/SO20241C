#include "conexion.h"

pthread_t escuchar_memoria()
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)atender_a_memoria, NULL);
    return thread;
}

void atender_kernel()
{
    log_info(log_io, "estoy en atender kernel");
    int cod_op_io;

    // Recibimos de kernel una operacion de ES

    t_buffer* buffer; 
    int pid;
    int unidades_de_trabajo;
    t_list* direcciones_fisicas;
    int registro_tamanio;
    int registro_puntero_archivo;
    char* nombre_archivo;

    while(1)
    {
        cod_op_io = recibir_operacion(conexion_io_kernel);
        buffer = recibiendo_paquete_personalizado(conexion_io_kernel); // Recibo el PCB normalmente
        pid = recibir_int_del_buffer(buffer);

        switch(cod_op_io) // Segun el codigo de operación actúo 
        {
            case IO_GEN_SLEEP: // (Interfaz, Unidades de trabajo)

                unidades_de_trabajo = recibir_int_del_buffer(buffer);
                realizar_sleep(unidades_de_trabajo, pid);
                break;

            case IO_STDIN_READ: // (Interfaz, Registro Dirección, Registro Tamaño)

                log_info(log_io, "Estoy en el case IO_STDIN_READ atendiendo a kernel");

                registro_tamanio = recibir_int_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));

                t_direccion_fisica* dir_fisica;
                for(int i = 0; i<list_size(direcciones_fisicas); i++)
                {
                    dir_fisica = list_get(direcciones_fisicas, i);
                    log_info(log_io, "el maeco nro %d es : %d", i,dir_fisica->nro_marco);
                }
                
                leer_consola(direcciones_fisicas, registro_tamanio, pid);
                break;

            case IO_STDOUT_WRITE: // (Interfaz, Registro Dirección, Registro Tamaño)

                registro_tamanio = recibir_int_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));
                ejecutar_instruccion_stdout(direcciones_fisicas, registro_tamanio, pid);
                break;
            
            case IO_FS_CREATE: // (Interfaz, Nombre Archivo)
                
                nombre_archivo = recibir_string_del_buffer(buffer);
                manejar_creacion_archivo(nombre_archivo, pid);          
                break;
                
            case IO_FS_DELETE: // (Interfaz, Nombre Archivo)

                nombre_archivo = recibir_string_del_buffer(buffer);
                manejar_eliminacion_archivo(nombre_archivo, pid);
                break;

            case IO_FS_TRUNCATE: // (Interfaz, Nombre Archivo, Registro Tamaño)

                log_info(log_io, "Estoy dentro del case de IO_FS_TRUNCATE");
                
                nombre_archivo = recibir_string_del_buffer(buffer); 
                registro_tamanio = recibir_int_del_buffer(buffer);
                manejar_truncado_archivo(nombre_archivo, registro_tamanio, pid);
                break;

            case IO_FS_WRITE: // (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)

                nombre_archivo = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));
                registro_tamanio = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);
                manejar_escritura_archivo(nombre_archivo, direcciones_fisicas, registro_tamanio, registro_puntero_archivo, pid);
                break;

            case IO_FS_READ: // (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
            
                nombre_archivo = recibir_string_del_buffer(buffer);
                direcciones_fisicas = recibir_lista_del_buffer(buffer, sizeof(t_direccion_fisica));
                registro_tamanio = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);
                break;
            
            case -1:
                log_error(log_io, "Se desconecto kernel, me voy");
                exit(1);
            
            default:
                log_error(log_io, "El codigo de operacion no es reconocido chauchau :(");
                exit(1);
                break;
        }

        free(buffer);
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

void atender_a_memoria()
{
    t_buffer* buffer;
    int cod_op_io;
    
    while(1)
    {
        cod_op_io = recibir_operacion(conexion_io_memoria);
        
        
        switch(cod_op_io) // Segun el codigo de operación actúo 
        {   
            case IO_RECIBE_RESPUESTA_DE_ESCRITURA_DE_MEMORIA: 
                
                buffer = recibiendo_paquete_personalizado(conexion_io_memoria); 
                sem_post(&sem_ok_escritura_memoria);
                free(buffer);
                
                break;
            
            case IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA: // [char*]

                buffer = recibiendo_paquete_personalizado(conexion_io_memoria);
                valor_a_mostrar = recibir_string_del_buffer(buffer);
                sem_post(&sem_ok_lectura_memoria);
                free(buffer->stream);
                free(buffer);
                break;
            case -1:
                log_error(log_io, "Memoria se desconecto bye :P");
                exit(1);
                break;
            default:
                log_error(log_io, "El codigo de operacion no es reconocido :(");
                break;
            
        }
    }

    
}