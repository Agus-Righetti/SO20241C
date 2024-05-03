#include "atenderCPU.h"
//***** ESTO LO DEJO ARMADO PARA IMPLEMENTAR DESPUES
//usleep(config_memoria->retardo_respuesta *1000); //Espero el retardo de respuesta -> hago el pasaje de milisegundos a microsegundos
	

//********** LEE EL ARCHIVO Y CARGA LAS INSTRUCCIONES EN UNA LISTA (LAS INSTRUCCIONES LAS DEJA DEL TIPO "t_instruccion_codigo")
t_list* leer_archivo_y_cargar_instrucciones(const char* archivo_pseudocodigo) {
    //Abro el archivo en modo lectura
    FILE* archivo = fopen(archivo_pseudocodigo, "r");

    t_list* instrucciones = list_create(); //Creo una lista para almacenar todas las instrucciones
    
    char* instruccion_formateada = NULL;
    int i = 0;
    if (archivo == NULL) {
        log_error(log_memoria, "No se puede abrir el archivo_pseudocodigo para leer instrucciones.");  
        return instrucciones;
    }

    char* linea_instruccion = malloc(256 * sizeof(int));
    while (fgets(linea_instruccion, 256, archivo)) {
    	int size_linea_actual = strlen(linea_instruccion);
    	if(size_linea_actual > 2){
    		if(linea_instruccion[size_linea_actual - 1] == '\n'){
				char* linea_limpia = string_new();
				string_n_append(&linea_limpia, linea_instruccion, size_linea_actual - 1);
				free(linea_instruccion);
				linea_instruccion = malloc(256 * sizeof(int));
				strcpy(linea_instruccion,linea_limpia);
    		}
    	}
    	//-----------------------------------------------
        char** l_instrucciones = string_split(linea_instruccion, " ");
        log_info(log_memoria, "Instruccion: [%s]", linea_instruccion);
        while (l_instrucciones[i]) {
        	i++;
        }

        t_instruccion_codigo* pseudo_cod = malloc(sizeof(t_instruccion_codigo));
        pseudo_cod->mnemonico = strdup(l_instrucciones[0]);
        //log_info(log_memoria, "%s", pseudo_cod->mnemonico);

        pseudo_cod->primero_parametro = (i > 1) ? strdup(l_instrucciones[1]) : NULL;
        //if(i > 1) log_info(log_memoria, "%s", pseudo_cod->primero_parametro);
        pseudo_cod->segundo_parametro = (i > 2) ? strdup(l_instrucciones[2]) : NULL;
        //if(i > 2) log_info(log_memoria, "%s", pseudo_cod->segundo_parametro);
        pseudo_cod->tercero_parametro = (i > 3) ? strdup(l_instrucciones[3]) : NULL;
        //if(i > 3) log_info(log_memoria, "%s", pseudo_cod->tercero_parametro);
        pseudo_cod->cuarto_parametro = (i > 4) ? strdup(l_instrucciones[4]) : NULL;
        //if(i > 4) log_info(log_memoria, "%s", pseudo_cod->cuarto_parametro);
        pseudo_cod->quinto_parametro = (i > 5) ? strdup(l_instrucciones[5]) : NULL;
        //if(i > 5) log_info(log_memoria, "%s", pseudo_cod->quinto_parametro);


        if (i == 5) {
            instruccion_formateada = string_from_format("%s %s %s %s %s", pseudo_cod->mnemonico, pseudo_cod->primero_parametro, pseudo_cod->segundo_parametro, pseudo_cod->tercero_parametro, pseudo_cod->cuarto_parametro);
        } else if(i == 4){
            instruccion_formateada = string_from_format("%s %s %s %s", pseudo_cod->mnemonico, pseudo_cod->primero_parametro, pseudo_cod->segundo_parametro, pseudo_cod->tercero_parametro);
        } else if(i == 3){
            instruccion_formateada = string_from_format("%s %s %s", pseudo_cod->mnemonico, pseudo_cod->primero_parametro, pseudo_cod->segundo_parametro);
        } else if (i == 2) {
            instruccion_formateada = string_from_format("%s %s", pseudo_cod->mnemonico, pseudo_cod->primero_parametro);
        } else {
            instruccion_formateada = strdup(pseudo_cod->mnemonico);
        }

        list_add(instrucciones, instruccion_formateada);
        for (int j = 0; j < i; j++) {
            free(l_instrucciones[j]);
        }

        free(l_instrucciones);
        free(pseudo_cod->mnemonico);
		if(pseudo_cod->primero_parametro) free(pseudo_cod->primero_parametro);
		if(pseudo_cod->segundo_parametro) free(pseudo_cod->segundo_parametro);
        if(pseudo_cod->tercero_parametro) free(pseudo_cod->tercero_parametro);
        if(pseudo_cod->cuarto_parametro) free(pseudo_cod->cuarto_parametro);
        if(pseudo_cod->quinto_parametro) free(pseudo_cod->quinto_parametro);

		free(pseudo_cod);
        i = 0; // Restablece la cuenta para la próxima iteración
    }

    fclose(archivo);
    free(linea_instruccion);
    return instrucciones;
}

void cpu_pide_instruccion(t_list* instrucciones){ //en realidad, lo va a recibir de un: t_buffer* un_buffer
	//Tenemos que implementar identificador del procesos
    int ip = 2; 

	//Obtener Instruccion especifica
	char* instruccion = obtener_instruccion_por_indice(instrucciones, ip);

	log_info(log_memoria, "<IP:%d> <%s>", ip, instruccion);
    enviar_mensaje(instruccion, socket_cliente_cpu);
    log_info(log_memoria, "Instruccion enviada a CPU");
	//Enviar_instruccion a CPU
	//enviar_una_instruccion_a_cpu(instruccion);
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

// void enviar_una_instruccion_a_cpu(char* instruccion){
// 	usleep(config_memoria->retardo_respuesta *1000); //Espero el retardo de respuesta -> hago el pasaje de milisegundos a microsegundos
	
//     t_paquete* paquete = crear_paquete_personalizado(INSTRUCCION); 
//     cargar_string_a_paquete_personalizado(paquete, instruccion);
// 	enviar_paquete(paquete, socket_cliente_cpu);
// 	eliminar_paquete(paquete);
// }