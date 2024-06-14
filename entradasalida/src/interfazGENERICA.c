#include "interfazGENERICA.h"

void recibir_operacion_generica_de_kernel(InterfazGenerica* interfaz_generica, op_code codigo)
{
    // Verificar si la operación es para una interfaz genérica
    if (codigo == IO_GEN_SLEEP) 
    {
        int unidades_trabajo = recibir_unidades_trabajo(conexion_io_kernel);
        realizar_sleep(unidades_trabajo * interfaz_generica->tiempo_unidad_trabajo);
        log_info(log_io, "Operacion recibida: IO_GEN_SLEEP. Unidades de trabajo: %d", unidades_trabajo);
    } 
    else if (codigo == -1) 
    {
        log_error(log_io, "KERNEL se desconecto. Terminando servidor");
        exit(1);
    } 
    else 
    {
        log_warning(log_io, "La operacion recibida no es para una interfaz GENERICA.\n");
    }
}

void iniciar_config_generica(Interfaz *configuracion)
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

    // Liberar el t_config
    config_destroy(config);
}

void leer_configuracion_generica(Interfaz *configuracion)
{
    iniciar_config_generica(configuracion);

    // Loggeamos el valor de config
    log_info(log_io, "Lei el TIPO_INTERFAZ %s, el TIEMPO_UNIDAD_TRABAJO %d, el IP_KERNEL %s y el PUERTO_KERNEL %s.", 
             configuracion->archivo->tipo_interfaz, 
             configuracion->archivo->tiempo_unidad_trabajo, 
             configuracion->archivo->ip_kernel, 
             configuracion->archivo->puerto_kernel);
}

int recibir_unidades_trabajo(int socket)
{
    // Implementa la lógica para recibir la cantidad de unidades de trabajo desde el Kernel
    int unidades_trabajo;
    recv(socket, &unidades_trabajo, sizeof(unidades_trabajo), 0);
    return unidades_trabajo;
}

void realizar_sleep(int tiempo) 
{
    // Implementa la lógica para dormir la cantidad de tiempo especificado
    log_info(log_io, "PID: %d - Operacion: Sleep", proceso->pid);
    usleep(tiempo * 1000); // Convertir de milisegundos a microsegundos
}

void liberar_configuracion_generica(Interfaz* configuracion)
{
    if(configuracion) {
        free(configuracion->archivo->tipo_interfaz);
        free(configuracion->archivo->ip_kernel);
        free(configuracion->archivo->puerto_kernel);
        free(configuracion->archivo);
    }
}