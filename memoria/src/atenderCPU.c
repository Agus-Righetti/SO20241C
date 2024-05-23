#include "atenderCPU.h"

//************ DESARROLLO DE TODAS LAS FUNCIONES QUE NECESITA CPU **************
void cpu_pide_instruccion(){//t_buffer* un_buffer){
    //ESTE BUFFER VA A SER DE LA FORMA [PID, IP]
    //                                  Int  Int
	//int pid = recibir_int_del_buffer(un_buffer);
	//int ip = recibir_int_del_buffer(un_buffer);
    int pid = 1;
	int ip = 10;

    //tengo que obtener proceso buscando con los PID
    t_proceso* un_proceso = obtener_proceso_por_id(pid);

	//Obtener Instruccion especifica
	char* instruccion = obtener_instruccion_por_indice(un_proceso->instrucciones, ip);

	log_info(log_memoria, "<IP:%d> <%s>", ip, instruccion);
    
	//Enviar_instruccion a CPU
	enviar_una_instruccion_a_cpu(instruccion);

    log_info(log_memoria, "Instruccion enviada a CPU");
}

t_proceso* obtener_proceso_por_id(int pid){
	bool _buscar_el_pid(t_proceso* proceso){
		return proceso->pid == pid;
	}
	t_proceso* un_proceso = list_find(lista_procesos_recibidos, (void*)_buscar_el_pid);
	if(un_proceso == NULL){
		log_error(log_memoria, "PID<%d> No encontrado en la lista de procesos", pid);
		// exit(EXIT_FAILURE);
	}
	return un_proceso;
}

char* obtener_instruccion_por_indice(t_list* instrucciones, int indice_instruccion){
	char* instruccion_actual;
	if(indice_instruccion >= 0 && indice_instruccion < list_size(instrucciones)){
		instruccion_actual = list_get(instrucciones, indice_instruccion);
		return instruccion_actual;
	}
	else{
		log_error(log_memoria, "Nro de Instruccion <%d> NO VALIDA", indice_instruccion);
		//exit(EXIT_FAILURE);
		return NULL;
	}
}

void enviar_una_instruccion_a_cpu(char* instruccion){
	usleep(config_memoria->retardo_respuesta *1000); //Espero el retardo de respuesta -> hago el pasaje de milisegundos a microsegundos
	t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_INSTRUCCION_DE_MEMORIA);

	// EN REALIDAD DEBO ENVIAR UN BUFFER DE LA FORMA [MNEMONICO, PARAMETRO1, PARAMETRO2, PARAMETRO3, PARAMETRO4, PARAMETRO5]
    //                                                  CHAR*       CHAR*       CHAR*       CHAR*       CHAR*       CHAR* 
    // FALTA ARMAR PAQUETE
	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}