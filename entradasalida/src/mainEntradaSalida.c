#include <mainEntradaSalida.h>

int main(int argc, char* argv[]) 
{
    decir_hola("IO");
   
    // Log y config de uso general ----------------------------------------------------------------------------------------------------

    log_io = log_create("io.log", "IO", 1, LOG_LEVEL_DEBUG);
    config_io = armar_config(log_io);

    // Interfaz* configuracion_fs;

    // // Inicializar la configuración (ejemplo)
    // // configuracion_fs->archivo->block_size = 64;
    // // configuracion_fs->archivo->block_count = 1024;
    // //strcpy(configuracion_fs->archivo->path_base_dialfs, "/home/utnso/dialfs");

    // manejar_creacion_archivo("bloques.dat", configuracion_fs);
    // manejar_creacion_archivo("bitmap.dat", configuracion_fs);
    // manejar_creacion_archivo("notas.txt", configuracion_fs);

    // Conexion IO --> Memoria --------------------------------------------------------------------------------------------------------

    escuchar_memoria(); 

    // Conexion IO --> Kernel ---------------------------------------------------------------------------------------------------------
    
    escuchar_kernel();
  
    // terminar_programa(log_io, config_io);

    return 0;
}

void terminar_programa(t_log* log_io, t_config* config_io)
{
    if (log_io != NULL) 
    {
		log_destroy(log_io);
    }
    if (config_io != NULL) 
    {
		config_destroy(config_io);
    }
}