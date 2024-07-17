#include "conexion.h"

pthread_t escuchar_kernel()
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)atender_kernel, NULL);
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
    t_list* registro_direccion;
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

                registro_direccion = recibir_estructura_del_buffer(buffer);
                registro_tamanio = recibir_int_del_buffer(buffer);
                leer_consola(registro_direccion, registro_tamanio, pid);
                break;

            case IO_STDOUT_WRITE: // (Interfaz, Registro Dirección, Registro Tamaño)

                registro_direccion = recibir_estructura_del_buffer(buffer);
                registro_tamanio = recibir_int_del_buffer(buffer);
                ejecutar_instruccion_stdout(registro_direccion, registro_tamanio, pid);
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

                nombre_archivo = recibir_string_del_buffer(buffer); 
                registro_tamanio = recibir_int_del_buffer(buffer);
                manejar_truncado_archivo(nombre_archivo, registro_tamanio, pid);
                break;

            case IO_FS_WRITE: // (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)

                nombre_archivo = recibir_string_del_buffer(buffer);
                registro_direccion = recibir_estructura_del_buffer(buffer);
                registro_tamanio = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);
                manejar_escritura_archivo(nombre_archivo,registro_direccion, registro_tamanio, registro_puntero_archivo, pid);
                break;

            case IO_FS_READ: // (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
            
                nombre_archivo = recibir_string_del_buffer(buffer);
                registro_direccion = recibir_estructura_del_buffer(buffer);
                registro_tamanio = recibir_int_del_buffer(buffer);
                registro_puntero_archivo = recibir_int_del_buffer(buffer);
                break;
            
            case -1:
                log_error(log_io, "Se desconecto kernel, me voy");
                exit(1);
            
            default:
                log_error(log_io, "El codigo de operacion no es reconocido :(");
                break;
        }
    }
}

pthread_t escuchar_memoria()
{
    pthread_t thread;
    pthread_create(&thread, NULL, (void*)atender_a_memoria, NULL);
    return thread;
}

void atender_a_memoria()
{
    t_buffer* buffer;
    int cod_op_io;
    while(1)
    {
        cod_op_io = recibir_operacion(conexion_io_memoria);
        buffer = recibiendo_paquete_personalizado(conexion_io_memoria); 
        
        switch(cod_op_io) // Segun el codigo de operación actúo 
        {   
            case IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA: // [char*]

                char* valor_a_mostrar = recibir_string_del_buffer(buffer);
            
                if(strcmp(config_io->tipo_interfaz, "STDOUT") == 0)
                {
                    printf("Valor leído en memoria: %s", valor_a_mostrar);
                }

                avisar_fin_io_a_kernel();
                break;
            case IO_RECIBE_RESPUESTA_DE_ESCRITURA_DE_MEMORIA: 
                
                char* valor_recibido = recibir_string_del_buffer(buffer); 

                if(strcmp(config_io->tipo_interfaz, "DIALFS") == 0)
                {

                }

                avisar_fin_io_a_kernel();
                break;
            case -1:
                log_error(log_io, "MEMORIA se desconecto. Terminando servidor.");
                exit(1);
            default:
                log_error(log_io, "El codigo de operacion no es reconocido.");
                break;
        }
    }
    
    free(buffer->stream);
    free(buffer);
}