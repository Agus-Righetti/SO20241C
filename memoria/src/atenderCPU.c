#include "atenderCPU.h"

//******************************************************************
//******************* PIDE UNA INSTRUCCIÓN *************************
//******************************************************************
void cpu_pide_instruccion(t_buffer* un_buffer){        //[PID, IP]
	int pid = recibir_int_del_buffer(un_buffer);
	int ip = recibir_int_del_buffer(un_buffer);

    //tengo que obtener proceso buscando con los PID
    t_proceso* un_proceso = obtener_proceso_por_id(pid);

	//Obtener Instruccion especifica
	char* instruccion = obtener_instruccion_por_indice(un_proceso->instrucciones, ip);
    
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

	agregar_string_al_paquete_personalizado(paquete, instruccion);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

//******************************************************************
//****************** ACCESO A ESPACIO USUARIO **********************
//******************************************************************


//******************************************************************
//****************** ACCESO A TABLA DE PÁGINA **********************
//******************************************************************
int consulta_marco_segun_pagina (int pid, int nro_pag){
	// Busco el proceso en mi lista de procesos
	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	// Si sigue, es porque encontró el proceso
	// Tengo que buscar en la tabla de páginas del proceso, si la página está
	// Para eso controlo bit de presencia
	t_fila_tabla_paginas* linea_actual = list_get(un_proceso->tabla_paginas, nro_pag);

	if(linea_actual == NULL){
		log_error(log_memoria, "NRO DE PÁGINA <%d> DEL PROCESO <%d> NO VALIDO", nro_pag, pid);
		return -1;
	}

	if(linea_actual->presencia == 0){
		log_error(log_memoria, "PAGE FAULT: NRO DE PÁGINA <%d> - PROCESO <%d>", nro_pag, pid);
		return -1;
	}
	return linea_actual->frame;
}