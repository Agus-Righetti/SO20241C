#include "atenderCPU-IO.h"

// ********************************************* CPU **********************************************
//******************************************************************
//******************* PIDE UNA INSTRUCCIÓN *************************
//******************************************************************
void cpu_pide_instruccion(t_buffer* un_buffer){        //[PID, IP]
	
	int pid = recibir_int_del_buffer(un_buffer);
	uint32_t ip = recibir_uint32_del_buffer(un_buffer);

	sem_wait(&sem_primero);
	sem_wait(&sem_lista_procesos);

    //tengo que obtener proceso buscando con los PID
    t_proceso* un_proceso = obtener_proceso_por_id(pid);
	sem_wait(&un_proceso->sem_instrucciones);

	//Obtener Instruccion especifica
 	char* instruccion = obtener_instruccion_por_indice(un_proceso->instrucciones, ip);
    
	//Enviar_instruccion a CPU
	enviar_una_instruccion_a_cpu(instruccion);
	sem_post(&un_proceso->sem_instrucciones);
	log_info(log_memoria, "Proceso: <<%d>> - Instruccion: <<%s>> enviada a CPU con exito", pid, instruccion);
	sem_post(&sem_lista_procesos);
	sem_post(&sem_primero);

}

t_proceso* obtener_proceso_por_id(int pid){
	int i = 0;
	pthread_mutex_lock(&mutex_lista_procesos);

	while(i < list_size(lista_procesos_recibidos)){

		if(comparar_pid(pid, list_get(lista_procesos_recibidos, i)) == 1){

			t_proceso* proceso = list_get(lista_procesos_recibidos, i);
			pthread_mutex_unlock(&mutex_lista_procesos);
			return proceso;
		}

		i ++;
	}

	pthread_mutex_unlock(&mutex_lista_procesos);

	log_error(log_memoria, "ERROR: Proceso %d no encontrado", pid);

	return NULL;
}

bool comparar_pid(int pid, t_proceso* proceso){
	
	if(proceso->pid == pid){
		return 1;
	}
	
	return 0;
}


char* obtener_instruccion_por_indice(t_list* instrucciones, uint32_t indice_instruccion){
	char* instruccion_actual;

	if(indice_instruccion >= 0 && indice_instruccion < list_size(instrucciones)){
		instruccion_actual = list_get(instrucciones, indice_instruccion);
		return instruccion_actual;
	}
	else{
		log_error(log_memoria, "Nro de Instruccion <%d> NO VALIDA", indice_instruccion);
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
		log_info(log_memoria, "PID: <%d> - Pagina: <%d> - Marco: <%d>", pid, nro_pag, marco);
		enviar_marco_consultado_a_cpu(marco);
	}

}

int obtener_marco_segun_pagina (int pid, int nro_pag){
	// Busco el proceso en mi lista de procesos
	t_proceso* un_proceso = obtener_proceso_por_id(pid);
	// Si sigue, es porque encontró el proceso
	// Tengo que buscar en la tabla de páginas del proceso, si la página está

	// 1ero controlo si esta vacia
	if (list_is_empty(un_proceso->tabla_paginas)){
		log_error(log_memoria, "La tabla de paginas esta vacia");
		log_error(log_memoria, "Se intento acceder a un espacio de memoria antes de asignarle un tamaño.");
	}
	
	
	// 2ndo controlo que el nro de pag sea valido
	if(nro_pag > list_size(un_proceso->tabla_paginas)){
		log_error(log_memoria, "NRO DE PÁGINA <%d> DEL PROCESO <%d> NO VALIDO", nro_pag, pid);
		return -1;
	}


	t_pagina* una_pagina = list_get(un_proceso->tabla_paginas, nro_pag);


	// NO PUEDE HABER PAGE FAULT NO HAY MEMORIA VIRTUAL
	
	return una_pagina->frame;
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
	int tamanio_nuevo = recibir_int_del_buffer(un_buffer);

	t_proceso* mi_proceso = obtener_proceso_por_id(pid);

	// Primero veo si tengo que AMPLIAR o REDUCIR
	if (tamanio_nuevo < mi_proceso->tamanio){
		// REDUCIR
		int tamanio_a_reducir = mi_proceso->tamanio - tamanio_nuevo;

		// LOG OBLIGATORIO PARA REDUCCION 
		log_info(log_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a Reducir: <%d>", pid, mi_proceso->tamanio, tamanio_a_reducir);

		// Hay dos casos:
		// CASO 1 -> no tengo que eliminar paginas porque el tamaño usado en la ultima pagina es mayor al que quiero reducir 
		if(mi_proceso->tam_usado_ult_pag > tamanio_a_reducir){
			
			mi_proceso->tamanio = tamanio_nuevo;
			// Actualizo el tamaño usado de la ultima pagina 
			mi_proceso->tam_usado_ult_pag = mi_proceso->tam_usado_ult_pag - tamanio_a_reducir;
			
		} else {
			
			// CASO 2 -> tengo que eliminar paginar
			int cantidad_pag_a_reducir = ceil ((double)tamanio_a_reducir / (double)config_memoria->tam_pagina);
			log_info(log_memoria, "Cantidad de paginas a eliminar: %d", cantidad_pag_a_reducir);

			int tamanioc = list_size(mi_proceso->tabla_paginas) - 1;
			while (cantidad_pag_a_reducir > 0){
				// Empiezo eliminando ultima pag

				t_pagina* pag_a_eliminar = list_remove(mi_proceso->tabla_paginas, tamanioc);


				liberar_marco(pag_a_eliminar->frame);

				cantidad_pag_a_reducir --;
				tamanioc -- ;
			}
			

			mi_proceso->tamanio = tamanio_nuevo;
			mi_proceso->tam_usado_ult_pag = config_memoria->tam_pagina - ((list_size(mi_proceso->tabla_paginas)*config_memoria->tam_pagina) - mi_proceso->tamanio);
			//                                     tam pag             -      ( cant pag * tam pag                                           - tam del proceso)
		}

		enviar_ok_del_resize_a_cpu();   

	} else {

		// AMPLIAR
		int tamanio_a_aumentar = tamanio_nuevo - mi_proceso->tamanio;

		// LOG OBLIGATORIO PARA AMPLIACION
		log_info(log_memoria, "PID: <%d> - Tamaño Actual: <%d> - Tamaño a aumentar: <%d>", pid, mi_proceso->tamanio, tamanio_a_aumentar);

		// Hay dos casos:
		// CASO 1 -> quiero ampliar pero no llego a agregar una pagina
		
		if((config_memoria->tam_pagina - mi_proceso->tam_usado_ult_pag) >= tamanio_a_aumentar){	

			if(list_is_empty(mi_proceso->tabla_paginas)){
				t_list* marcos_libres = buscar_marcos_libres();
				t_frame* marco_por_usar = list_get(marcos_libres, 0); // AGARRO EL PRIMERO QUE ENCUENTRO EN LA LISTA DE LIBRES

				ocupar_marco(marco_por_usar->id);
		
				t_pagina* pagina = crear_pagina(marco_por_usar);
				// Creo la pagina
				agregar_pag_a_tabla(mi_proceso, pagina);

			}
			//                 espacio libre de ult pag                 <= lo que quiero agregar
			// NO TENGO QUE AGREGAR UNA PAGINA -> HAY LUGAR EN LA ULTIMA

			mi_proceso->tamanio = tamanio_nuevo;
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
				log_info(log_memoria, "Proceso PID: < %d > - NO PUDO COMPLETAR EL RESIZE POR OUT OF MEMORY", pid);
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
					
					t_pagina* pagina = crear_pagina(marco_por_usar);


					// Creo la pagina
				   	agregar_pag_a_tabla(mi_proceso, pagina);

					cantidad_pag_a_aumentar --;
					posicion ++;
					}

				eliminar_lista(marcos_libres);
				mi_proceso->tamanio = tamanio_nuevo;
				mi_proceso->tam_usado_ult_pag = (list_size(mi_proceso->tabla_paginas)*config_memoria->tam_pagina) - mi_proceso->tamanio;
				enviar_ok_del_resize_a_cpu();
			}
		}
	}


	return;
}

void enviar_out_of_memory_a_cpu(){ 
	t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_OUT_OF_MEMORY_DE_MEMORIA);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
	return;
}

void enviar_ok_del_resize_a_cpu(){ 

	t_paquete* paquete_enviar_ok = crear_paquete_personalizado((int)CPU_RECIBE_OK_DEL_RESIZE);
	log_info(log_memoria, "Enviando OK del resize a CPU");

	enviar_paquete(paquete_enviar_ok, socket_cliente_cpu);
	eliminar_paquete(paquete_enviar_ok);

	return;
}

// void cpu_pide_guardar_1B(t_buffer* un_buffer){    // [PID, DFs, VALOR] -> [Int, lista, uint8]
// 	int pid = recibir_int_del_buffer(un_buffer);
// 	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
// 	//int tamanio = recibir_int_del_buffer(un_buffer);
// 	uint8_t valor = recibir_uint8_del_buffer(un_buffer);

// 	guardar_uint8_en_memoria (pid, direcciones_fisicas, valor);

// }

// void cpu_pide_guardar_4B(t_buffer* un_buffer){    // [PID, DFs, VALOR] -> [Int, lista, uint32]
// 	int pid = recibir_int_del_buffer(un_buffer);
// 	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
// 	//int tamanio = recibir_int_del_buffer(un_buffer);
// 	uint32_t valor = recibir_uint8_del_buffer(un_buffer);
	
// 	guardar_uint32_en_memoria (pid, direcciones_fisicas, valor);

// }

// void cpu_pide_leer_1B(int pid, t_list* direcciones_fisicas){    // [PID, DFs] -> [Int, lista]
	
// 	log_info(log_memoria, "entre a cpu pode leer 1B");
// 	// int pid = recibir_int_del_buffer(un_buffer);
// 	// t_list* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);

// 	t_direccion_fisica* xd = list_get(direcciones_fisicas, 0);

//     log_info(log_memoria, "el marco q saque es: %d", xd->nro_marco);
//     log_info(log_memoria, "bytes a operar q saque es: %d", xd->bytes_a_operar);
// 	log_info(log_memoria, "offset q saque es: %d", xd->offset);

// 	log_info(log_memoria, "saque la direccion fisica del buffer");
// 	//int tamanio = recibir_int_del_buffer(un_buffer);

// 	leer_uint8_en_memoria (pid, direcciones_fisicas);
	
// }

// void cpu_pide_leer_4B(int pid, ){    // [PID, DFs] -> [Int, lista]

// 	int pid = recibir_int_del_buffer(un_buffer);
// 	void* direcciones_fisicas = recibir_estructura_del_buffer(un_buffer);
	
// 	leer_uint32_en_memoria (pid, direcciones_fisicas);
// }

void cpu_pide_leer_string(t_buffer* un_buffer){
	int pid = recibir_int_del_buffer(un_buffer);
	t_list* direcciones_fisicas = recibir_lista_del_buffer(un_buffer, sizeof(t_direccion_fisica));

	if(direcciones_fisicas == NULL){
        log_error(log_memoria, "Direcciones_fisicas es null");
    }

	int tamanio = recibir_int_del_buffer(un_buffer);

	leer_string_en_memoria(pid, direcciones_fisicas, tamanio);
}

void cpu_pide_guardar_string(t_buffer* un_buffer){    
	
	int pid = recibir_int_del_buffer(un_buffer);
	char* valor = recibir_string_del_buffer(un_buffer);
	
	t_list* direcciones_fisicas = recibir_lista_del_buffer(un_buffer, sizeof(t_direccion_fisica));

	if(direcciones_fisicas == NULL){
        log_error(log_memoria, "Direcciones_fisicas es null");
    }

	guardar_string_en_memoria(pid, direcciones_fisicas, valor);
}

// ********************************************* IO **********************************************
// IO_STDIN_READ (Interfaz, Registro Dirección, Registro Tamaño)
// Esta instrucción solicita al Kernel que mediante la interfaz ingresada se lea desde el STDIN (Teclado) un valor cuyo tamaño está delimitado por el valor del Registro Tamaño y el mismo se guarde a partir de la Dirección Lógica almacenada en el Registro Dirección.
// necesita -> guardar string en memoria

// IO_STDOUT_WRITE (Interfaz, Registro Dirección, Registro Tamaño)
// Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde la posición de memoria indicada por la Dirección Lógica almacenada en el Registro Dirección, un tamaño indicado por el Registro Tamaño y se imprima por pantalla.
// necesita -> leer string en memoria

// IO_FS_WRITE (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
// Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde Memoria la cantidad de bytes indicadas por el Registro Tamaño a partir de la dirección lógica que se encuentra en el Registro Dirección y se escriban en el archivo a partir del valor del Registro Puntero Archivo.
// necesita -> leer string en memoria

// IO_FS_READ (Interfaz, Nombre Archivo, Registro Dirección, Registro Tamaño, Registro Puntero Archivo)
// Esta instrucción solicita al Kernel que mediante la interfaz seleccionada, se lea desde el archivo a partir del valor del Registro Puntero Archivo la cantidad de bytes indicada por Registro Tamaño y se escriban en la Memoria a partir de la dirección lógica indicada en el Registro Dirección.
// necesita -> guardar string en memoria

void io_pide_lectura(int socket, int pid, int tamanio, t_list* direcciones_fisicas){

	// pid = recibir_int_del_buffer(un_buffer);
	// direcciones_fisicas = recibir_lista_del_buffer(un_buffer, sizeof(t_direccion_fisica));
	// tamanio = recibir_int_del_buffer(un_buffer);
	
	// t_direccion_fisica* dir_fisica;
    // for(int i = 0; i<list_size(direcciones_fisicas); i++)
    // {
    //     dir_fisica = list_get(direcciones_fisicas, i);
    //     //log_info(log_memoria, "el maeco nro %d es : %d", i,dir_fisica->nro_marco);
    // }

	if(direcciones_fisicas == NULL){
        log_error(log_memoria, "Direcciones_fisicas es null");
    }

	leer_string_io_en_memoria(pid , direcciones_fisicas, tamanio, socket);

}

void io_pide_escritura(int socket, int pid, int tamanio, char* valor, t_list* direcciones_fisicas){

	// t_direccion_fisica* dir_fisica;
    // for(int i = 0; i<list_size(direcciones_fisicas); i++)
    // {
    //     dir_fisica = list_get(direcciones_fisicas, i);
    //     //log_info(log_memoria, "el maeco nro %d es : %d", i,dir_fisica->nro_marco);
    // }

	if(direcciones_fisicas == NULL){
        log_error(log_memoria, "Direcciones_fisicas es null");
    }

	guardar_string_io_en_memoria(pid, direcciones_fisicas, valor, tamanio, socket);

	return;

}

void destroy_element(void *element) {
	free(element);
}
