#include "cliente.h"

// Conexion a Memoria ----------------------------------------------------------------------------------------------------------------

int conexion_a_memoria()
{ 
    int conexion_cpu_memoria = crear_conexion(config_cpu->ip_memoria, config_cpu->puerto_memoria);
    
    if(conexion_cpu_memoria == -1)
    {
        log_info(log_cpu, "Error: No se pudo crear conexion CPU --> Memoria");
        exit(1);
    }
    
    log_info(log_cpu , "Conexion con el servidor MEMORIA creada");
    enviar_mensaje("Hola MEMORIA soy CPU", conexion_cpu_memoria);

    return conexion_cpu_memoria;
}

// void liberar_cliente(int conexion_cpu_memoria)
// {

// }