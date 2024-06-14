#include "interfazSTDOUT.h"

void leer_configuracion_stdout(Interfaz *configuracion)
{
    iniciar_config_stdout(configuracion);

    // Loggeamos el valor de config
    log_info(log_io, "Lei el TIPO_INTERFAZ %s, el TIEMPO_UNIDAD_TRABAJO %s, el IP_KERNEL %s, el PUERTO_KERNEL %s, el IP_MEMORIA %s y el PUERTO_MEMORIA %s.", 
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
        free(configuracion->archivo->tiempo_unidad_trabajo);
        free(configuracion->archivo->ip_kernel);
        free(configuracion->archivo->puerto_kernel);
        free(configuracion->archivo->ip_memoria);
        free(configuracion->archivo->puerto_memoria);
        free(configuracion->archivo);
    }
}

void recibir_operacion_stdout_de_kernel(Interfaz* configuracion_stdout, op_code codigo) 
{
    // Verificar si la operación es para una interfaz stdout
    if (codigo == IO_STDOUT_WRITE) 
    {
        // Lee los parámetros de la solicitud
        char* interfaz = recibir_string_del_buffer(conexion_io_kernel);
        char* registro_direccion = recibir_string_del_buffer(conexion_io_kernel);
        char* registro_tamano = recibir_string_del_buffer(conexion_io_kernel);

        // Convierte los registros a enteros
        int direccion_logica = obtener_valor_registro(registro_direccion);
        int tamano = obtener_valor_registro(registro_tamano);

        log_info(log_io, "Operacion recibida: IO_STDOUT_WRITE. Interfaz: %s, Dirección Lógica: %d, Tamaño: %d", interfaz, direccion_logica, tamano);

        // Ejecutar la instrucción
        ejecutar_instruccion_stdout(configuracion_stdout, direccion_logica, tamano);

        // Libera memoria de los registros
        free(interfaz);
        free(registro_direccion);
        free(registro_tamano);
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

void ejecutar_instruccion_stdout(Interfaz* configuracion_stdout, int direccion_logica, int tamano) 
{
    int direccion_fisica = solicitar_traduccion_direccion(configuracion_stdout, direccion_logica);

    // Envio la solicitud de lectura a memoria
    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);
    agregar_int_al_paquete_personalizado(paquete, direccion_fisica);
    agregar_int_al_paquete_personalizado(paquete, tamano);
    enviar_paquete(paquete, conexion_io_memoria);

    // Recibo respuesta de memoria    
    int cod_op_memoria = recibir_operacion(conexion_io_memoria);
    while(1) 
    {
        switch (cod_op_memoria) 
        {
            case IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEM:
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
            case EXIT:
                error_exit(EXIT);
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

// Envia la solicitud de traduccion a Kernel y recibe la dire fisica
int solicitar_traduccion_direccion(Interfaz* configuracion_stdout, int direccion_logica) 
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
            case EXIT:
                error_exit(EXIT);
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

int obtener_valor_registro(char* registro) // Pedirlo a CPU
{
    return (int)dictionary_get(registros, registro);
}
