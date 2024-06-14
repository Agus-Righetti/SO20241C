#include "cliente.h"

// Conexion a Memoria ----------------------------------------------------------------------------------------------------------------

int conexion_a_memoria()
{ 
    // socket_cliente_cpu -> conexion_cpu_memoria
    int socket_cliente_cpu = crear_conexion(config_cpu->ip_memoria, config_cpu->puerto_memoria);
    
    if(socket_cliente_cpu == -1)
    {
        log_info(log_cpu, "Error: No se pudo crear conexion CPU --> Memoria");
        exit(1);
    }
    
    log_info(log_cpu , "Conexion con el servidor MEMORIA creada");
    enviar_mensaje("Hola MEMORIA soy CPU", socket_cliente_cpu);

    return socket_cliente_cpu;
}