#include <mainEntradaSalida.h>

t_log* log_io;
io_config* config_io;

void iterator(char* value) 
{
	log_info(log_io, value);
}


int main(int argc, char* argv[]) 
{
    decir_hola("una Interfaz de Entrada/Salida");

    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config();
    int conexion_io_kernel;
    conexion_io_kernel = crear_conexion(config_io->ip_kernel, config_io->puerto_kernel);
    log_info(log_io , "Conexion con el servidor IO creada");
    enviar_mensaje("Hola Kernel soy IO",conexion_io_kernel);

    return 0;
}

io_config* armar_config(void)
{
    t_config* config_aux;
    io_config* aux_io_config = malloc(sizeof(io_config));  

    config_aux = config_create("io.config");
    aux_io_config->tipo_interfaz = strdup(config_get_string_value(config_aux, "TIPO_INTERFAZ"));
    aux_io_config->rfaz = strdup(config_get_string_value(config_aux, "TIPO_INTERFAZ"));


   aux_kernel_config->puerto_escucha = strdup(config_get_string_value(config_aux, "PUERTO_ESCUCHA"));
    aux_kernel_config->ip_memoria = strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
    aux_kernel_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
    aux_kernel_config->ip_cpu = strdup(config_get_string_value(config_aux, "IP_CPU"));
    aux_kernel_config->puerto_cpu_dispatch = strdup(config_get_string_value(config_aux, "PUERTO_CPU_DISPATCH"));
    aux_kernel_config->puerto_cpu_interrupt = strdup(config_get_string_value(config_aux, "PUERTO_CPU_INTERRUPT"));
    aux_kernel_config->algoritmo_planificacion = strdup(config_get_string_value(config_aux, "ALGORITMO_PLANIFICACION"));
    aux_kernel_config->quantum = strdup(config_get_string_value(config_aux, "QUANTUM"));
    aux_kernel_config->recursos = strdup(config_get_string_value(config_aux, "RECURSOS"));
    aux_kernel_config->instancias_recursos = strdup(config_get_string_value(config_aux, "INSTANCIAS_RECURSOS"));
    aux_kernel_config->grado_multiprogramacion = config_get_int_value(config_aux, "GRADO_MULTIPROGRAMACION");   

    log_info(log_kernel, "Se creo el struct config_kernel con exito");

    config_destroy(config_aux);

    return aux_kernel_config;
}