#include "servidor.h"

// Server para recibir a kernel ------------------------------------------------------------------------------------------------------- 

void server_para_kernel() // Atiendo al cliente
{
    socket_servidor_dispatch = iniciar_servidor(config_cpu->puerto_escucha_dispatch, log_cpu);
    if (socket_servidor_dispatch == -1)
    {
	    log_info(log_cpu, "ERROR: No se pudo iniciar CPU como servidor para KERNEL");
        exit(1);
    }

	log_info(log_cpu, "Esperando a KERNEL...");

    socket_cliente_kernel = esperar_cliente(socket_servidor_dispatch, log_cpu);
    
    t_paquete* paquete = crear_paquete_personalizado(HAND);
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);
}

// Server para recibir interrupciones de Kernel ---------------------------------------------------------------------------------------

void interrupcion_para_kernel()
{	
    socket_servidor_interrupt = iniciar_servidor(config_cpu->puerto_escucha_interrupt, log_cpu);
    if(socket_servidor_interrupt == -1)
    {
	    log_info(log_cpu, "Error: No se pudo iniciar CPU como servidor para Kernel");
        exit(1);
    }

    socket_interrupt_kernel = esperar_cliente(socket_servidor_interrupt, log_cpu);
}
