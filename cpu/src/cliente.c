#include "cliente.h"

// Conexion a memoria ----------------------------------------------------------------------------------------------------------------

int conexion_a_memoria()
{    
    int conexion_cpu_memoria = crear_conexion(config_cpu->ip_memoria, config_cpu->puerto_memoria);
    if(conexion_cpu_memoria == -1)
    {
        log_info(log_cpu, "Error: No se pudo crear conexion CPU --> Memoria");
        exit(1);
    }
    
    log_info(log_cpu , "Conexion con el servidor Memoria creada");
    enviar_mensaje("Hola Memoria soy CPU",conexion_cpu_memoria);

    t_list* lista;

    // Solicito las instrucciones 
    
    int cod_op = solicitar_instrucciones_a_memoria(conexion_cpu_memoria);
    
    switch (cod_op) 
    {
        case MENSAJE:
            recibir_mensaje(conexion_cpu_memoria, log_cpu);
            break;
        case PAQUETE:
            lista = recibir_paquete(conexion_cpu_memoria);
            log_info(log_cpu, "Me llegaron los siguientes valores:\n");
            list_iterate(lista, (void*) iterator);
            list_destroy_and_destroy_elements(lista, free);
            break;
        case OK:
			proceso->program_counter++; // Avanza a la siguiente instrucción
			interpretar_instrucciones();
			list_destroy_and_destroy_elements(lista, free);
			break;
        case EXIT:
            error_exit(EXIT);
            list_destroy_and_destroy_elements(lista, free); 
            break;
        case -1:
            log_error(log_cpu, "El cliente se desconecto. Terminando servidor");
            exit(1);
        default:
            log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
            break;
    };

    return conexion_cpu_memoria;
}

// void liberar_cliente(int conexion_cpu_memoria)
// {

// }