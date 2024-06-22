#include "interfazSTDOUT.h"

void leer_configuracion_stdout(Interfaz *configuracion)
{
    iniciar_config_stdout(configuracion);

    // Loggeamos el valor de config
    log_info(log_io, "Lei el TIPO_INTERFAZ %s, el TIEMPO_UNIDAD_TRABAJO %d, el IP_KERNEL %s, el PUERTO_KERNEL %d, el IP_MEMORIA %s y el PUERTO_MEMORIA %d.", 
             configuracion->archivo->tipo_interfaz, 
             configuracion->archivo->tiempo_unidad_trabajo,
             configuracion->archivo->ip_kernel, 
             configuracion->archivo->puerto_kernel, 
             configuracion->archivo->ip_memoria,
             configuracion->archivo->puerto_memoria);
}

void iniciar_config_stdout(Interfaz *configuracion)
{   
    if (configuracion == NULL) 
    {
        printf("El puntero de configuración es NULL\n");
        exit(2);
    }

    // Asignar memoria para configuracion->archivo
    configuracion->archivo = malloc(sizeof(io_config));
    if (configuracion->archivo == NULL) 
    {
        printf("No se puede crear la config archivo\n");
        exit(2);
    }

    // Inicializa la estructura del archivo de configuración desde el archivo de configuración
    t_config* config = config_create("./io.config");
    if (config == NULL) 
    {
        printf("No se puede leer el archivo de config\n");
        free(configuracion->archivo);
        exit(2);
    }
    
    configuracion->archivo->tipo_interfaz = strdup(config_get_string_value(config, "TIPO_INTERFAZ"));
    configuracion->archivo->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    configuracion->archivo->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
    configuracion->archivo->puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    configuracion->archivo->ip_memoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
    configuracion->archivo->puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");

    // Liberar el t_config
    config_destroy(config);
}

void liberar_configuracion_stdout(Interfaz* configuracion)
{
    if(configuracion) 
    {
        free(configuracion->archivo->tipo_interfaz);
        // free(configuracion->archivo->tiempo_unidad_trabajo);
        free(configuracion->archivo->ip_kernel);
        // free(configuracion->archivo->puerto_kernel);
        free(configuracion->archivo->ip_memoria);
        // free(configuracion->archivo->puerto_memoria);
        free(configuracion->archivo);
    }
}

void recibir_operacion_stdout_de_kernel(Interfaz* configuracion_stdout, op_code codigo) 
{
    // Verificar si la operación es para una interfaz stdout
    if (codigo == IO_STDOUT_WRITE) 
    {
        // Lee los parámetros de la solicitud

        t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_kernel);

        char* interfaz = recibir_string_del_buffer(buffer);
        char* registro_direccion = recibir_string_del_buffer(buffer);
        char* registro_tamanio = recibir_string_del_buffer(buffer);
        int valor_registro = recibir_int_del_buffer(buffer);
        int direccion_logica = recibir_int_del_buffer(buffer);

        // Convierte los registros a enteros
        // int direccion_logica = obtener_valor_registro(registro_direccion);

        log_info(log_io, "Operacion recibida: IO_STDOUT_WRITE. Interfaz: %s, Dirección Lógica: %d, Tamaño: %s", interfaz, direccion_logica, registro_tamanio);

        // Ejecutar la instrucción
        ejecutar_instruccion_stdout(configuracion_stdout, direccion_logica, atoi (registro_tamanio));

        // Libera memoria de los registros
        free(interfaz);
        free(buffer);
        free(registro_direccion);
        free(registro_tamanio);
    } 
    else if (codigo == -1)
    {
        log_error(log_io, "KERNEL se desconecto. Terminando servidor");
        exit(1);
    } 
    else
    {
        log_warning(log_io, "Operacion recibida no es para una interfaz STDOUT.\n");
    }
}

void ejecutar_instruccion_stdout(Interfaz* configuracion_stdout, int direccion_logica, int tamanio) 
{
    int direccion_fisica = solicitar_traduccion_direccion(direccion_logica);

    // Envio la solicitud de lectura a memoria
    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);
    agregar_int_al_paquete_personalizado(paquete, direccion_fisica);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    enviar_paquete(paquete, conexion_io_memoria);

    // Recibo respuesta de memoria    
    int cod_op_memoria = recibir_operacion(conexion_io_memoria);
    while(1) 
    {
        switch (cod_op_memoria) 
        {
            case IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA:
                t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_memoria);
                char* valor_leido = recibir_string_del_buffer(buffer);
                log_info(log_io, "Valor leído desde la dirección física %d: %s", direccion_fisica, valor_leido);
                // Muestro el valor por pantalla
                printf("Valor leído desde memoria: %s\n", valor_leido);
                free(valor_leido);
                free(buffer);
                eliminar_paquete(paquete);
                close(conexion_io_memoria);
                break; 
            case -1:
                log_error(log_io, "MEMORIA se desconecto. Terminando servidor");
                free(conexion_io_memoria);
                return;
            default:
                log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
    eliminar_paquete(paquete);
}