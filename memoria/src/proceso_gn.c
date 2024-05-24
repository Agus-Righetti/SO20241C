#include "proceso_gn.h"

// t_proceso* obtener_proceso_por_id(int pid){
// 	bool _buscar_el_pid(t_proceso* proceso){
// 		return proceso->pid == pid;
// 	}
// 	t_proceso* un_proceso = list_find(lista_procesos_recibidos, (void*)_buscar_el_pid);
// 	if(un_proceso == NULL){
// 		log_error(log_memoria, "PID<%d> No encontrado en la lista de procesos", pid);
// 		// exit(EXIT_FAILURE);
// 	}
// 	return un_proceso;
// }

// t_instruccion_codigo* obtener_instruccion_por_indice(t_list* instrucciones, int indice_instruccion){
// 	t_instruccion_codigo* instruccion_actual;
// 	if(indice_instruccion >= 0 && indice_instruccion < list_size(instrucciones)){
// 		instruccion_actual = list_get(instrucciones, indice_instruccion);
// 		return instruccion_actual;
// 	}
// 	else{
// 		log_error(log_memoria, "Nro de Instruccion <%d> NO VALIDA", indice_instruccion);
// 		//exit(EXIT_FAILURE);
// 		return NULL;
// 	}
// }
