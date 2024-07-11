#include "atenderCPU.h"

//******************************************************************
//******************* PIDE UNA INSTRUCCIÓN *************************
//******************************************************************
void cpu_pide_instruccion(t_buffer* un_buffer){        //[PID, IP]
	int pid = recibir_int_del_buffer(un_buffer);
	int ip = recibir_int_del_buffer(un_buffer);

	log_info(log_memoria, "estoy antes del semaforo");
	sem_wait(&sem_lista_procesos);
    //tengo que obtener proceso buscando con los PID
    t_proceso* un_proceso = obtener_proceso_por_id(pid);

	//Obtener Instruccion especifica
	char* instruccion = obtener_instruccion_por_indice(un_proceso->instrucciones, ip);
    
	//Enviar_instruccion a CPU
	enviar_una_instruccion_a_cpu(instruccion);
	sem_post(&sem_lista_procesos);
	
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

	t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_INSTRUCCION_DE_MEMORIA);

	agregar_string_al_paquete_personalizado(paquete, instruccion);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

//******************************************************************
//****************** ACCESO A ESPACIO USUARIO **********************
//******************************************************************
// Dirección física
// Tamaño
// Valor
// Ante un pedido de lectura, devolver el valor que se encuentra a partir de la dirección física pedida.
// Ante un pedido de escritura, escribir lo indicado a partir de la dirección física pedida. En caso satisfactorio se responderá un mensaje de OK.

// FALTA IMPLEMENTAR

//******************************************************************
//****************** ACCESO A TABLA DE PÁGINA **********************
//******************************************************************
void cpu_pide_numero_de_marco(t_buffer* un_buffer){        // [PID, NUMERO DE PAGINA]
	int pid = recibir_int_del_buffer(un_buffer);
	int nro_pag = recibir_int_del_buffer(un_buffer);

	// Busco el numero de marco
	int marco = obtener_marco_segun_pagina (pid, nro_pag);
	if(marco != -1){
		log_info(log_memoria, "PID: <%d> - Pagina: <%d> - Marco: <%d>\n", pid, nro_pag, marco);
		enviar_marco_consultado_a_cpu(marco);
	}

}

int obtener_marco_segun_pagina (int pid, int nro_pag){
	// Busco el proceso en mi lista de procesos
	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	// Si sigue, es porque encontró el proceso
	// Tengo que buscar en la tabla de páginas del proceso, si la página está
	
	t_pagina* pagina_actual = list_get(un_proceso->tabla_paginas, nro_pag);

	if(pagina_actual == NULL){
		log_error(log_memoria, "NRO DE PÁGINA <%d> DEL PROCESO <%d> NO VALIDO", nro_pag, pid);
		return -1;
	}
	
	// NO PUEDE HABER PAGE FAULT NO HAY MEMORIA VIRTUAL
	
	return pagina_actual->frame;
}

void enviar_marco_consultado_a_cpu(int marco_consultado){ 
	t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_NUMERO_DE_MARCO_DE_MEMORIA);

	agregar_int_al_paquete_personalizado(paquete, marco_consultado);
	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

//******************************************************************
//***************************** RESIZE *****************************
//******************************************************************
void cpu_pide_resize(t_buffer* un_buffer){          // [PID, TAMAÑO NUEVO] -> [Int, Int]
	int pid = recibir_int_del_buffer(un_buffer);
	int tamaño_nuevo = recibir_int_del_buffer(un_buffer);

	t_proceso* mi_proceso = obtener_proceso_por_id(pid);

	// Primero veo si tengo que AMPLIAR o REDUCIR
	if (tamaño_nuevo < mi_proceso->tamaño){
		// REDUCIR
		int tamanio_a_reducir = mi_proceso->tamaño - tamaño_nuevo;

		// LOG OBLIGATORIO PARA REDUCCION 
		log_info(log_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>\n", pid, mi_proceso->tamaño, tamanio_a_reducir);

		// Hay dos casos:
		// CASO 1 -> no tengo que eliminar paginas porque el tamaño usado en la ultima pagina es mayor al que quiero reducir 
		if(mi_proceso->tam_usado_ult_pag > tamanio_a_reducir){
			mi_proceso->tamaño = tamaño_nuevo;
			// Actualizo el tamaño usado de la ultima pagina 
			mi_proceso->tam_usado_ult_pag = mi_proceso->tam_usado_ult_pag - tamanio_a_reducir;
			
		} else {
			
			// CASO 2 -> tengo que eliminar paginar
			int cantidad_pag_a_reducir = ceil ((double)tamanio_a_reducir / (double)config_memoria->tam_pagina);
			
			while (cantidad_pag_a_reducir > 0){
				// Empiezo eliminando ultima pag
				t_pagina* pag_a_eliminar = list_remove(mi_proceso->tabla_paginas, list_size(mi_proceso->tabla_paginas));
				liberar_marco(pag_a_eliminar->frame);

				cantidad_pag_a_reducir --;
			}


			mi_proceso->tamaño = tamaño_nuevo;
			mi_proceso->tam_usado_ult_pag = config_memoria->tam_pagina - ((list_size(mi_proceso->tabla_paginas)*config_memoria->tam_pagina) - mi_proceso->tamaño);
			//                                     tam pag             -      ( cant pag * tam pag                                           - tam del proceso)
		}

		enviar_ok_del_resize_a_cpu();   

	} else {

		// AMPLIAR
		int tamanio_a_aumentar = tamaño_nuevo - mi_proceso->tamaño;

		// LOG OBLIGATORIO PARA AMPLIACION
		log_info(log_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Ampliar: <%d>\n", pid, mi_proceso->tamaño, tamanio_a_aumentar);

		// Hay dos casos:
		// CASO 1 -> quiero ampliar pero no llego a agregar una pagina
		if((config_memoria->tam_pagina - mi_proceso->tam_usado_ult_pag) <= tamanio_a_aumentar){
			//                 espacio libre de ult pag                 <= lo que quiero agregar

			mi_proceso->tamaño = tamaño_nuevo;
			// Actualizo el tamaño usado de la ultima pagina 
			mi_proceso->tam_usado_ult_pag = mi_proceso->tam_usado_ult_pag + tamanio_a_aumentar;
			enviar_ok_del_resize_a_cpu();

		} else {
			
			// CASO 2 -> tengo que agregar paginas
			int cantidad_pag_a_aumentar = ceil ((double)tamanio_a_aumentar / (double)config_memoria->tam_pagina);
			
			// Primero verifico que haya lugar en memoria
			bool lugar = hay_lugar_en_memoria(cantidad_pag_a_aumentar);
			if (lugar == 0){
				// NO HAY ESPACIO -> no puedo guardar el proceso
				log_info(log_memoria, "OUT OF MEMORY \n Proceso PID: <%d> - NO PUDO COMPLETAR EL RESIZE", pid);
				enviar_out_of_memory_a_cpu();

			} else{
				// HAY LUGAR
				// busco los marcos libres
				t_list* marcos_libres = buscar_marcos_libres();
				int posicion = 0;
				while (cantidad_pag_a_aumentar > 0){
					// Elijo el marco en el que voy a guardar
					t_frame* marco_por_usar = list_get(marcos_libres, posicion); // AGARRO EL PRIMERO QUE ENCUENTRO EN LA LISTA DE LIBRES
					ocupar_marco(marco_por_usar->id);

					// Creo la pagina
					t_pagina* pag_nueva = crear_pagina(marco_por_usar);

			    	agregar_pag_a_tabla (mi_proceso, pag_nueva);

					cantidad_pag_a_aumentar --;
					posicion ++;
					}

				eliminar_lista(marcos_libres);
				mi_proceso->tamaño = tamaño_nuevo;
				mi_proceso->tam_usado_ult_pag = (list_size(mi_proceso->tabla_paginas)*config_memoria->tam_pagina) - mi_proceso->tamaño;
				enviar_ok_del_resize_a_cpu();
			}
		}
	}
}

void enviar_out_of_memory_a_cpu(){ 
	t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_OUT_OF_MEMORY_DE_MEMORIA);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void enviar_ok_del_resize_a_cpu(){ 
	t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_OK_DEL_RESIZE);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void cpu_pide_guardar_1B(t_buffer* un_buffer){    // [PID, DFs, VALOR] -> [Int, lista, uint8]
	int pid = recibir_int_del_buffer(un_buffer);
	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	//int tamanio = recibir_int_del_buffer(un_buffer);
	uint8_t valor = recibir_uint8_del_buffer(un_buffer);

	guardar_uint8_en_memoria (pid, direcciones_fisicas, valor);

}

void cpu_pide_guardar_4B(t_buffer* un_buffer){    // [PID, DFs, VALOR] -> [Int, lista, uint32]
	int pid = recibir_int_del_buffer(un_buffer);
	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	//int tamanio = recibir_int_del_buffer(un_buffer);
	uint32_t valor = recibir_uint8_del_buffer(un_buffer);
	
	guardar_uint32_en_memoria (pid, direcciones_fisicas, valor);

}

void cpu_pide_leer_1B(t_buffer* un_buffer){    // [PID, DFs] -> [Int, lista]
	int pid = recibir_int_del_buffer(un_buffer);
	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	//int tamanio = recibir_int_del_buffer(un_buffer);
	
	leer_uint8_en_memoria (pid, direcciones_fisicas);
	
}


void cpu_pide_leer_4B(t_buffer* un_buffer){    // [PID, DFs] -> [Int, lista]
	int pid = recibir_int_del_buffer(un_buffer);
	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	
	leer_uint32_en_memoria (pid, direcciones_fisicas);
}


void cpu_pide_leer_string(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	int tamanio = recibir_int_del_buffer(un_buffer);
	
	//leer_string_en_memoria (pid, direcciones_fisicas, tamanio);
}


void cpu_pide_guardar_string(t_buffer* un_buffer){    
	int pid = recibir_int_del_buffer(un_buffer);
	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	char* valor = recibir_estructura_del_buffer(un_buffer);
	int tamanio = recibir_int_del_buffer(un_buffer);

	//guardar_string_en_memoria (pid, direcciones_fisicas, valor, tamanio);

}