#include "interfazSTDIN.h"

void leer_consola()
{
	char* leido;

	leido = readline("Ingrese el texto: > ");
	log_info(log_io, ">>%s", leido);

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while(strcmp(leido, "") != 0)
    {
		free(leido);
		leido = readline("> ");
		log_info(log_io, ">> %s", leido);
	}
	
    free(leido);
}

void leer_configuracion_stdin(Interfaz *configuracion)
{
    iniciar_config_stdin(configuracion);

    // Loggeamos el valor de config
    log_info(log_io, "Lei el TIPO_INTERFAZ %s, el IP_KERNEL %s, el PUERTO_KERNEL %s, el IP_MEMORIA %s y el PUERTO_MEMORIA %s.", 
             configuracion->archivo->tipo_interfaz, 
             configuracion->archivo->ip_kernel, 
             configuracion->archivo->puerto_kernel, 
             configuracion->archivo->ip_memoria,
             configuracion->archivo->puerto_memoria);
}

void iniciar_config_stdin(Interfaz *configuracion)
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
    configuracion->archivo->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
    configuracion->archivo->puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    configuracion->archivo->ip_memoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
    configuracion->archivo->puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");

    // Liberar el t_config
    config_destroy(config);
}

void recibir_operacion_stdin_de_kernel(Interfaz* interfaz, op_code codigo)
{
    // Verificar si la operación es para una interfaz stdin
    if (codigo == IO_STDIN_READ) 
    {
        log_info(log_io, "Operacion recibida: IO_STDIN_READ.");
        leer_consola(); // Esta función se encuentra en el módulo de I/O y debe estar definida allí
    } 
    else if (codigo == -1) 
    {
        log_error(log_io, "KERNEL se desconecto. Terminando servidor");
        exit(1);
    } 
    else 
    {
        log_warning(log_io, "Operacion recibida no es para una interfaz STDIN.\n");
    }
}

void liberar_configuracion_stdin(Interfaz* configuracion)
{
    if(configuracion) 
    {
        free(configuracion->archivo->tipo_interfaz);
        free(configuracion->archivo->ip_kernel);
        free(configuracion->archivo->puerto_kernel);
        free(configuracion->archivo->ip_memoria);
        free(configuracion->archivo->puerto_memoria);
        free(configuracion->archivo);
    }
}