#include "interfazGenerica.h"

void recibir_operacion_generica_de_kernel(const char* configuracion, op_code codigo)
{
    int unidades_trabajo;

    // Verificar si la operación es para una interfaz genérica
    if (codigo != IO_GEN_SLEEP) 
    {
        log_warning(log_io, "Operacion recibida no es para una interfaz generica.\n");
    }

    // Recibir la cantidad de unidades de trabajo 
    // unidades_trabajo = recibir_unidades_trabajo();

    // Simular el procesamiento de las unidades de trabajo
    // usleep(unidades_trabajo * configuracion->tiempo_unidad_trabajo);

    // Registrar en el log la operación y la cantidad de unidades de trabajo
    log_info(log_io, "Operacion generica recibida: IO_GEN_SLEEP. Unidades de trabajo: %d", unidades_trabajo);
}

Interfaz* iniciar_config(void)
{
	Interfaz* nuevo_config = config_create("./io.config");
	if(nuevo_config == NULL)
    {
		printf("No se puede crear la config\n");
		exit(2);
	}
	return nuevo_config;
}

void leer_configuracion(Interfaz *configuracion)
{   
    configuracion = iniciar_config();

	// Usando el config creado previamente, leemos los valores del config y los dejamos en las variables 'ip', 'puerto' y 'valor'
	char* tipo_interfaz = config_get_string_value(configuracion, "TIPO_INTERFAZ");
	int tiempo_unidad_trabajo = config_get_int_value(configuracion, "TIEMPO_UNIDAD_TRABAJO");
    char* ip_kernel = config_get_string_value(configuracion, "IP_KERNEL");
    int puerto_kernel = config_get_int_value(configuracion, "PUERTO_KERNEL");

	// Loggeamos el valor de config
	log_info(log_io, "Lei el TIPO_INTERFAZ %s, el TIEMPO_UNIDAD_TRABAJO %d, el IP_KERNEL %s y el PUERTO_KERNEL %d.", tipo_interfaz, tiempo_unidad_trabajo, ip_kernel, puerto_kernel);
}
