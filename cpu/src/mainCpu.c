#include <include/mainCpu.h>

t_log* log_cpu;
cpu_config* config_cpu;

int main(int argc, char* argv[]) 
{
    decir_hola("CPU");
    
	// ************* LOG Y CONFIG DE USO GENERAL *************
    log_cpu = log_create("cpu.log", "CPU", 1, LOG_LEVEL_DEBUG);
    config_cpu = armar_config(log_cpu);

    // ************* CONEXION CPU --> MEMORIA *************
    int conexion_cpu_memoria = conexion_a_memoria(log_cpu, config_cpu);
	
	// ********* SERVER PARA RECIBIR A KERNEL *********
	server_para_kernel(config_cpu, log_cpu);

	//Se puede hacer funcion y liberar todas las memorias
    log_destroy(log_cpu);
	free(config_cpu);

	return EXIT_SUCCESS;
}
