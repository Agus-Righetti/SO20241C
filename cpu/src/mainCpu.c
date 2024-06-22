#include <mainCpu.h>

int main(int argc, char* argv[]) 
{
    decir_hola("CPU");
    
	// Log y config de uso general ----------------------------------------------------------------------------------------------------

	log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
	config_cpu = armar_config(log_cpu);
	inicializar_tlb();

    // Conexion CPU --> Memoria -------------------------------------------------------------------------------------------------------

	int socket_cliente_cpu = conexion_a_memoria();

	// Server para recibir a Kernel ---------------------------------------------------------------------------------------------------
	
	escuchar_kernel();

    // Verificar los valores fuera de la función
    // printf("Verificación fuera de la función:\n");
    // printf("El PID es: %d\n", (*pcb_recibido)->pid);
    // printf("El PC es: %d\n", (*pcb_recibido)->program_counter);
	
	// Instrucciones con memoria
	
	escuchar_memoria();
   
	terminar_programa();
	liberar_conexion(socket_cliente_kernel);
	return EXIT_SUCCESS;
}

void terminar_programa()
{
	if (log_cpu != NULL) 
    {
		log_destroy(log_cpu);
	}
	if (config_cpu != NULL) 
    {
		config_destroy(config_cpu);
	}
}