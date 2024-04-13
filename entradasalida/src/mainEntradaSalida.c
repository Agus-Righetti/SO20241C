#include <mainEntradaSalida.h>

t_log* log_io;
io_config* config_io;

void iterator(char* value) {
	log_info(log_io, value);
}


io_config* armar_config(void) {
    t_config* config_aux;
    io_config* aux_io_config = malloc(sizeof(io_config));  

    config_aux = config_create("io.config");
    //aux_io_config->tipo_interfaz = strdup(config_get_string_value(config_aux, "TIPO_INTERFAZ"));
    aux_io_config->tiempo_unidad_trabajo = strdup(config_get_string_value(config_aux, "TIEMPO_UNIDAD_TRABAJO"));
    aux_io_config->ip_kernel = strdup(config_get_string_value(config_aux, "IP_KERNEL"));
    aux_io_config->puerto_kernel = strdup(config_get_string_value(config_aux, "PUERTO_KERNEL"));
    aux_io_config->ip_memoria= strdup(config_get_string_value(config_aux, "IP_MEMORIA"));
    aux_io_config->puerto_memoria = strdup(config_get_string_value(config_aux, "PUERTO_MEMORIA"));
    aux_io_config->path_base_dialfs = strdup(config_get_string_value(config_aux, "PATH_BASE_DIALFS"));
    aux_io_config->block_size = strdup(config_get_string_value(config_aux, "BLOCK_SIZE"));
    aux_io_config->block_count = strdup(config_get_string_value(config_aux, "BLOCK_COUNT"));

    log_info(log_io, "Se creo el struct config_io con exito");

    config_destroy(config_aux);

    return aux_io_config;
}




int main(int argc, char* argv[]) 
{
    decir_hola("una Interfaz de Entrada/Salida");
   
    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config();

    // ************* Esto es para funcionar como cliente con Memoria *************
    int conexion_io_memoria = crear_conexion(config_io->ip_memoria, config_io->puerto_memoria);
    log_info(log_io , "Conexion con el servidor memoria creada");
    enviar_mensaje("Hola Memoria, soy IO",conexion_io_memoria);

    // ************* Esto es para funcionar como cliente con Kernel *************
    int conexion_io_kernel= crear_conexion(config_io->ip_kernel, config_io->puerto_kernel); 
    log_info(log_io , "Conexion con el servidor kernel creada");
    enviar_mensaje("Hola Kernel, soy IO",conexion_io_kernel);

    return 0;
}