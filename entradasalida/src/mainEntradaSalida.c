#include "mainEntradaSalida.h"

int main(int argc, char* argv[]) 
{
    
   
    // Log y config de uso general ----------------------------------------------------------------------------------------------------

    char* nombre_log = string_new();
    char** parte = string_split(argv[1], ".");

    nombre_log = parte[0];
    
    string_append(&nombre_log,".log");

    log_io = log_create(nombre_log, parte[0], 1, LOG_LEVEL_DEBUG);

    config_io = armar_config(log_io, argv[1]); // En argv[1] esta el nombre de config pasado por parametro

    if(strcmp(config_io->tipo_interfaz , "DIALFS") == 0){
      crear_archivos_gestion_fs();
    }

    sem_init(&sem_ok_escritura_memoria, 0, 0);
    sem_init(&sem_ok_lectura_memoria, 0, 0);

    // Interfaz* configuracion_fs;

    //make start CONFIG_FILE=nombre_archivo.config (para arrancar) 

    // // Inicializar la configuración (ejemplo)
    // // configuracion_fs->archivo->block_size = 64;
    // // configuracion_fs->archivo->block_count = 1024;
    // //strcpy(configuracion_fs->archivo->path_base_dialfs, "/home/utnso/dialfs");

    // manejar_creacion_archivo("bloques.dat", configuracion_fs);
    // manejar_creacion_archivo("bitmap.dat", configuracion_fs);
    // manejar_creacion_archivo("notas.txt", configuracion_fs);

    // Las conexiones de sockets acá

    conexion_io_kernel = conexion_a_kernel();

    pthread_t hilo_escuchar_memoria;

    if(strcmp(config_io->tipo_interfaz, "GENERICA")!=0) //si no es la generica se conecta con memoria
    {
      conexion_io_memoria = conexion_a_memoria(); 
      hilo_escuchar_memoria = escuchar_memoria();
    }

    // Conexion IO --> Kernel ---------------------------------------------------------------------------------------------------------
    
    pthread_t hilo_escuchar_kernel = escuchar_kernel();
    if(strcmp(config_io->tipo_interfaz, "GENERICA")!=0)
    {
      pthread_detach(hilo_escuchar_memoria);
    }
    
    pthread_join(hilo_escuchar_kernel, NULL);

    return 0;
}

void terminar_programa(t_log* log_io, t_config* config_io, char* nombre_log)
{
  free(nombre_log);
  sem_destroy(&sem_ok_escritura_memoria);
  sem_destroy(&sem_ok_lectura_memoria);
    if (log_io != NULL) 
    {
		log_destroy(log_io);
    }
    if (config_io != NULL) 
    {
		config_destroy(config_io);
    }
}