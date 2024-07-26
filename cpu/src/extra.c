#include "extra.h"
// Los registros solo van a tener dos tamaños distintos
//  uint32_t -> 4 bytes  
//                      pc - eax - ebx - ecx - edx - si - di

//  uint8_t -> 1 bytes 
//                      ax - bx - cx - dx

// ARMO UNA LISTA POR SI TENGO QUE LEER/ESCRIBIR DE MAS DE UNA PAGINA

// ESTA ES TRADUCIR DIRECCIÓN GRANDE
t_list* traducir_dl_a_df_completa(int direccion_logica, int bytes_a_operar) {

    // Creo una lista en donde voy a guardar todas las direcciones fisicas que necesite para operar desde la direccion logica solicitada
    
    t_list* direcciones_fisicas = list_create();
    
    // Calculo mi primera pagina y su desplazamiento
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

    log_info(log_cpu, "BUSCANDO...");
    log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

    // Traduzco la DL -> busco el numero de marco
    int bytes_operar_pag = minimo(bytes_a_operar, tamanio_pagina - desplazamiento);

    t_direccion_fisica* direccion_fisica_traducida = traducir_una_dl_a_df(numero_pagina, desplazamiento, bytes_operar_pag);
    
    list_add(direcciones_fisicas, direccion_fisica_traducida);


    int bytes_ya_evaluado = tamanio_pagina - desplazamiento;

    // Voy a preguntar si me alcanza lo leido con lo que quiero operar -> si no tengo que leer otra pag
    while (bytes_a_operar > bytes_ya_evaluado){
        
        numero_pagina = numero_pagina + 1;
        desplazamiento = 0;
        bytes_operar_pag = minimo(bytes_a_operar - bytes_ya_evaluado, tamanio_pagina - desplazamiento);
        direccion_fisica_traducida = traducir_una_dl_a_df(numero_pagina, desplazamiento, bytes_operar_pag);
        list_add(direcciones_fisicas, direccion_fisica_traducida);
        bytes_ya_evaluado = bytes_ya_evaluado + bytes_operar_pag;
    }

    
    return direcciones_fisicas;

}

int minimo(int a, int b) {
    if (a < b) {
        return a;
    } else {
        return b;
    }
}

t_direccion_fisica* traducir_una_dl_a_df(int numero_pagina, int desplazamiento, int bytes_operar_pag){
   
    t_direccion_fisica* dir_traducida = malloc(sizeof(t_direccion_fisica));
    
    if(config_cpu->cantidad_entradas_tlb > 0) // 1ero busco en TLB, solo si la tlb tiene entradas
    {
        TLB_Entrada* respuesta = buscar(numero_pagina); 

        if(respuesta->pid != -1) {
            // Encontro la pagina en la TLB, no hace falta que busque en memoria
            
            // LOG OBLIGATORIO - TLB HIT
            log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d ", pcb_recibido->pid, numero_pagina);
            
            actualizar_tlb_HIT(pcb_recibido->pid, numero_pagina);

            dir_traducida->nro_marco = respuesta->numero_marco;
            dir_traducida->offset = desplazamiento;
            dir_traducida->bytes_a_operar = bytes_operar_pag;

        } else {
            // No esta en TLB -> tiene que buscar en memoria
            // LOG OBLIGATORIO - TLB MISS
            log_info(log_cpu, "PID: %d - TLB MISS - Pagina: %d", pcb_recibido->pid, numero_pagina);

            // Tengo el numero de pag -> hago una consulta a memoria por el marco
            t_paquete *paquete = crear_paquete_personalizado(CPU_PIDE_MARCO_A_MEMORIA); // [PID, NUMERO DE PAGINA]
            
            agregar_int_al_paquete_personalizado(paquete, pcb_recibido->pid);
            agregar_int_al_paquete_personalizado(paquete, numero_pagina);
            enviar_paquete(paquete, socket_cliente_cpu);

            log_info(log_cpu, "Esperando marco de memoria...");
            sem_wait(&sem_tengo_el_marco);

            // LOG OBLIGATORIO - OBTENER MARCO
            log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pcb_recibido->pid, numero_pagina, marco);
        

            // Agregarlo a la tlb
            TLB_Entrada* nueva_entrada = malloc(sizeof(TLB_Entrada));
            
            

            

            
            nueva_entrada->pid = pcb_recibido->pid;
            nueva_entrada->numero_pagina = numero_pagina;
            
            nueva_entrada->numero_marco = marco;


            actualizar_tlb(nueva_entrada); 
            

            dir_traducida->nro_marco = marco;
            dir_traducida->offset = desplazamiento;
            dir_traducida->bytes_a_operar = bytes_operar_pag;



        } 

    }else { //no tengo tlb le pido a memoria directo

        t_paquete *paquete = crear_paquete_personalizado(CPU_PIDE_MARCO_A_MEMORIA); // [PID, NUMERO DE PAGINA]
            
        agregar_int_al_paquete_personalizado(paquete, pcb_recibido->pid);
        agregar_int_al_paquete_personalizado(paquete, numero_pagina);
        enviar_paquete(paquete, socket_cliente_cpu);

        log_info(log_cpu, "Esperando marco de memoria...");
        sem_wait(&sem_tengo_el_marco);

        // LOG OBLIGATORIO - OBTENER MARCO
        log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pcb_recibido->pid, numero_pagina, marco);

        dir_traducida->nro_marco = marco;
        dir_traducida->offset = desplazamiento;
        dir_traducida->bytes_a_operar = bytes_operar_pag;


    }

   
    
    
    return dir_traducida;
}


bool es_Registro_de_1B(const char* registro) {

    // Verifica si el registro es igual a "AX", "BX", "CX" o "DX"
    if (strcmp(registro, "AX") == 0 || strcmp(registro, "BX") == 0 || strcmp(registro, "CX") == 0 || strcmp(registro, "DX") == 0) {
        return true;
    } else {
        return false; 
    }
}

void peticion_lectura_a_memoria(op_code code_op, int pid, t_list* direcciones_fisicas){

    
    t_paquete* paquete = crear_paquete_personalizado(code_op);


    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, direcciones_fisicas, sizeof(t_direccion_fisica));
  

	enviar_paquete(paquete, socket_cliente_cpu);

	eliminar_paquete(paquete);

    return;
}
    

void peticion_lectura_string_a_memoria(int pid, t_list* df_origen, int tamanio){

    t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_LEER_STRING);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, df_origen, sizeof(t_direccion_fisica));
    agregar_int_al_paquete_personalizado(paquete, tamanio);
   
	enviar_paquete(paquete, socket_cliente_cpu);

	eliminar_paquete(paquete);

    return;
}

void peticion_escritura_1B_a_memoria(int pid, t_list* direcciones_fisicas, uint8_t valor_por_escribir){

    t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_GUARDAR_REGISTRO_1B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, direcciones_fisicas, sizeof(t_direccion_fisica));
    agregar_uint8_al_paquete_personalizado(paquete, valor_por_escribir);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void peticion_escritura_4B_a_memoria(int pid, t_list* direcciones_fisicas, uint32_t valor_por_escribir){

    t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_GUARDAR_REGISTRO_4B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, direcciones_fisicas, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor_por_escribir);
    
	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void peticion_escritura_string_a_memoria(int pid, t_list* direcciones_fisicas, char* string_por_escribir){

    t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_GUARDAR_STRING);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_string_al_paquete_personalizado(paquete, string_por_escribir);
   
    
    agregar_lista_al_paquete_personalizado(paquete, direcciones_fisicas, sizeof(t_direccion_fisica));

	enviar_paquete(paquete, socket_cliente_cpu);
   
	eliminar_paquete(paquete);
}