#include "cliente.h"

// Conexion a memoria -----------------------------------------------------------------------------------------------------------------

int conexion_a_memoria(t_log* log_io, io_config* config_io)
{    
    conexion_io_memoria = crear_conexion(config_io->ip_memoria,config_io->puerto_memoria);
    if(conexion_io_memoria == -1)
    {
        log_info(log_io, "Error: No se pudo crear conexion IO --> MEMORIA");
        exit(1);
    }
    
    log_info(log_io, "Conexion con el servidor MEMORIA creada");
    enviar_mensaje("Hola MEMORIA soy IO",conexion_io_memoria);

    return conexion_io_memoria;
}

// Conexion a Kernel ------------------------------------------------------------------------------------------------------------------

int conexion_a_kernel(t_log* log_io, io_config* config_io)
{    
    conexion_io_kernel = crear_conexion(config_io->ip_kernel,config_io->puerto_kernel);
    if(conexion_io_kernel == -1)
    {
        log_info(log_io, "Error: No se pudo crear conexion IO --> KERNEL");
        exit(1);
    }
    
    log_info(log_io, "Conexion con el servidor KERNEL creada");
    
    // le mandamos al kernel que tipo de interfaz somos y como nos llamamos :)
    
    t_paquete* paquete = crear_paquete_personalizado(NUEVA_INTERFAZ);
    op_code tipo_interfaz = obtener_tipo_interfaz(config_io->tipo_interfaz);
    agregar_int_al_paquete_personalizado(paquete, tipo_interfaz);
    agregar_string_al_paquete_personalizado(paquete, config_io->nombre);
    enviar_paquete(paquete, conexion_io_kernel);

    eliminar_paquete(paquete);
    
    // enviar_mensaje("Hola KERNEL soy IO",conexion_io_kernel);

    return conexion_io_kernel;
}

op_code obtener_tipo_interfaz(char* char_interfaz)
{
    if(strcmp(char_interfaz, "GENERICA") == 0)
    {
        return GENERICA;
    }
    else if(strcmp(char_interfaz, "STDIN") == 0){
        
        return STDIN;

    }
    else if(strcmp(char_interfaz, "STDOUT") == 0){
        
        return STDOUT;
        
    }
    else if(strcmp(char_interfaz, "DIALFS") == 0){
        
        return DIALFS;

    }
    else{
        return -1;
    }
}

void avisar_fin_io_a_kernel()
{
    //le aviso a kernel q ya ejecute lo q tenia q hacer
    t_paquete* paquete = crear_paquete_personalizado(FIN_OP_IO);
    
    enviar_paquete(paquete, conexion_io_kernel);
    
    eliminar_paquete(paquete);

    return;
}