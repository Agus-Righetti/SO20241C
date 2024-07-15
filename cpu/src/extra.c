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
    log_info(log_cpu, "entre a traduccir dl a df antes del list create");
    t_list* direcciones_fisicas = list_create();
    log_info(log_cpu, "entre a traduccir dl a df dsp del list create");


    // Calculo mi primera pagina y su desplazamiento
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

    log_info(log_cpu, "BUSCANDO...");
    log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

    // Traduzco la DL -> busco el numero de marco
    int bytes_operar_pag = minimo(bytes_a_operar, tamanio_pagina - desplazamiento);

    t_direccion_fisica* direccion_fisica_traducida = traducir_una_dl_a_df(numero_pagina, desplazamiento, bytes_operar_pag);
    log_info(log_cpu, "volvi de traducir dl a df");
    
    list_add(direcciones_fisicas, direccion_fisica_traducida);

    log_info(log_cpu, "VICKY : )");

    int bytes_ya_evaluado = tamanio_pagina - desplazamiento;

    // Voy a preguntar si me alcanza lo leido con lo que quiero operar -> si no tengo que leer otra pag
    while (bytes_a_operar > bytes_ya_evaluado){
        
        numero_pagina = numero_pagina + 1;
        desplazamiento = 0;
        bytes_operar_pag = minimo(bytes_a_operar, tamanio_pagina - desplazamiento);

        direccion_fisica_traducida = traducir_una_dl_a_df(numero_pagina, desplazamiento, bytes_operar_pag);
        list_add(direcciones_fisicas, direccion_fisica_traducida);
        bytes_ya_evaluado = bytes_ya_evaluado + bytes_operar_pag;
    }
    log_info(log_cpu, "lo  : )");
    
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
    log_info(log_cpu, "HOLA AMIGOS");
    
    // 1ero busco en TLB
    TLB_Entrada* respuesta = buscar(numero_pagina); 

    if(respuesta->pid != -1) {
        // Encontro la pagina en la TLB, no hace falta que busque en memoria
        
        // LOG OBLIGATORIO - TLB HIT
        log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d", pcb_recibido->pid, numero_pagina);

        dir_traducida->nro_marco = respuesta->numero_marco;
        dir_traducida->offset = desplazamiento;
        dir_traducida->bytes_a_operar = bytes_operar_pag;

        

    } else {
        // No esta en TLB -> tiene que buscar en memoria
        // LOG OBLIGATORIO - TLB MISS
        log_info(log_cpu, "PID: %d - TLB MISS - Pagina: %d", pcb_recibido->pid, numero_pagina);

        // Tengo el numero de pag -> hago una consulta a memoria por el marco
        t_paquete *paquete = crear_paquete_personalizado(CPU_PIDE_MARCO_A_MEMORIA); // [PID, NUMERO DE PAGINA]
        log_info(log_cpu, "Le pedi un marco a memoria");
        
        agregar_int_al_paquete_personalizado(paquete, pcb_recibido->pid);
        agregar_int_al_paquete_personalizado(paquete, numero_pagina);
        enviar_paquete(paquete, socket_cliente_cpu);
        log_info(log_cpu, "Ya envie el paquete con el pid y el numero de pagina");
        
        // LOG OBLIGATORIO - OBTENER MARCO
        log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", pcb_recibido->pid, numero_pagina, marco);

        
        sem_wait(&sem_tengo_el_marco);

       

        log_info(log_cpu, "Ya paso el semaforo"); 
        log_info(log_cpu, "Me llego un marco de memoria: %d", marco);
        // Agregarlo a la tlb
        TLB_Entrada* nueva_entrada = malloc(sizeof(TLB_Entrada));
        
        // ************ FALLA ACA ************

        log_info("El pcb_recibido->pid es %d", pcb_recibido->pid);
        

        
        nueva_entrada->pid = pcb_recibido->pid;
        nueva_entrada->numero_pagina = numero_pagina;
        log_info(log_cpu, "marco global es %d", marco);
        
        nueva_entrada->numero_marco = marco;
         log_info(log_cpu, "nueva_entrada->pid es %d", nueva_entrada->pid);

        log_info(log_cpu, "estoy antes de entrar a actualizar tlb");
        actualizar_tlb(nueva_entrada); 
        
        log_info(log_cpu, "Volvi de actualizar, estoy por poner valores en dir_traducida");

        dir_traducida->nro_marco = nueva_entrada->numero_marco;
        dir_traducida->offset = desplazamiento;
        dir_traducida->bytes_a_operar = bytes_operar_pag;

        log_info(log_cpu, "pude cargar la direccion traducida");
        
        log_info(log_cpu, "dir_traducida->nro_marco: %d", dir_traducida->nro_marco );
        log_info(log_cpu, "dir_traducida->offset: %d", dir_traducida->offset );
        log_info(log_cpu, "dir_traducida->bytes_a_operar: %d", dir_traducida->bytes_a_operar );

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

    log_info(log_cpu, "estoy dentro de peticion lectura a memoria");
    t_paquete* paquete = crear_paquete_personalizado(code_op);

    //log_info(log_cpu, "Direccion fisica = %d", direcciones_fisicas);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, direcciones_fisicas, sizeof(t_direccion_fisica));
    log_info(log_cpu, "estoy por enviar la peticion de lectura");

	enviar_paquete(paquete, socket_cliente_cpu);
    log_info(log_cpu, "ya mande la peticion a memoria");
	eliminar_paquete(paquete);

    return;
}

// ACÁ ESTÁ EL PROBLEMA DE LA ESCUCHA 
// uint8_t espero_rta_lectura_1B_de_memoria(){ //  NO SE DEBERIA USAR

//     uint8_t valor_leido;

//     int cod_op_memoria_aux = recibir_operacion(socket_cliente_cpu);
//     switch (cod_op_memoria_aux) {
//         case CPU_RECIBE_LECTURA_1B:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_8]

//             // Esta va a ser una sola
//             t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//             int pid = recibir_int_del_buffer(buffer);
//             t_direccion_fisica* dir_fisica = recibir_estructura_del_buffer(buffer);
//             valor_leido = recibir_uint8_del_buffer(buffer);

//             log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_leido);
//             log_info(log_cpu, "ACCIÓN COMPLETADA: LEER %u EN MEMORIA", valor_leido);
//             free(buffer);
//             break; 

//         case -1:
//             log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//             exit(1);

//         default:
//             log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//             break;
//         }

//     return valor_leido;
// }

// uint32_t espero_rta_lectura_4B_de_memoria(){
//     uint32_t valor_reconstruido;
//     int control = 0;
//     t_buffer* buffer;
//     int pid;
//     t_direccion_fisica* dir_fisica;
//     uint32_t valor_leido;

//     int cod_op_memoria_aux = recibir_operacion(socket_cliente_cpu);

//     while(control == 0){
//         switch (cod_op_memoria_aux) {

//             case CPU_RECIBE_LECTURA_4B:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_32]
//                 // Pueden ser mas de una
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 valor_leido = recibir_uint32_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_leido);
//                 free(buffer);
//                 break; 

//             case CPU_RECIBE_LECTURA_U_4B:   // [PID, DF, VALOR, VALOR FINAL] -> [Int, Direccion_fisica, uint_32, uint_32]

//                 // Esta es la ultima
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 valor_leido = recibir_uint32_del_buffer(buffer);
//                 valor_reconstruido = recibir_uint32_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_leido);
//                 log_info(log_cpu, "ACCIÓN COMPLETADA: LEER %u EN MEMORIA", valor_reconstruido);
//                 control = 1;
//                 free(buffer);
//                 break; 

//             case -1:
//                 log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//                 exit(1);

//             default:
//                 log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//                 break;
//             }

//     }

//     free(buffer);
//     return valor_reconstruido;
// }


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

// void espero_rta_escritura_1B_de_memoria(){
//     uint8_t valor_escrito;

//     int cod_op_memoria_aux = recibir_operacion(socket_cliente_cpu);
//     switch (cod_op_memoria_aux) {
//         case CPU_RECIBE_OK_1B_DE_ESCRITURA:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_8]

//             // Esta va a ser una sola
//             t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//             int pid = recibir_int_del_buffer(buffer);
//             t_direccion_fisica* dir_fisica = recibir_estructura_del_buffer(buffer);
//             valor_escrito = recibir_uint8_del_buffer(buffer);

//             log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_escrito);
//             log_info(log_cpu, "ACCIÓN COMPLETADA: ESCRIBIR %u EN MEMORIA", valor_escrito);
//             free(buffer);
//             break; 

//         case -1:
//             log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//             exit(1);

//         default:
//             log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//             break;
//     }


// }


// void espero_rta_escritura_4B_de_memoria(){
//     uint32_t valor_completo;
//     int control = 0;
//     t_buffer* buffer;
//     int pid;
//     t_direccion_fisica* dir_fisica;
//     uint32_t valor_escrito;

//     int cod_op_memoria_aux = recibir_operacion(socket_cliente_cpu);
//     while(control == 0){
//         switch (cod_op_memoria_aux) {

//             case CPU_RECIBE_OK_4B_DE_ESCRITURA:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_32]
//                 // Pueden ser mas de una
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 valor_escrito = recibir_uint32_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_escrito);
//                 free(buffer);
//                 break; 

//             case CPU_RECIBE_ULT_OK_4B_DE_ESCRITURA:   // [PID, DF, VALOR, VALOR FINAL] -> [Int, Direccion_fisica, uint_32, uint_32]

//                 // Esta es la ultima
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 valor_escrito = recibir_uint32_del_buffer(buffer);
//                 valor_completo = recibir_uint32_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %u ", pid, dir_fisica->nro_marco ,dir_fisica->offset, valor_escrito);
//                 log_info(log_cpu, "ACCIÓN COMPLETADA: ESCRIBIR %u EN MEMORIA", valor_completo);
//                 control = 1;
//                 free(buffer);
//                 break; 

//             case -1:
//                 log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//                 exit(1);

//             default:
//                 log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//                 break;
//         }

//     }

//     free(buffer);
// }


// void peticion_lectura_string_a_memoria(int pid, t_list* direcciones_fisicas, int tamanio){

//     t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_LEER_STRING);

//     agregar_int_al_paquete_personalizado(paquete, pid);
//     agregar_estructura_al_paquete_personalizado(paquete, &direcciones_fisicas, sizeof(t_list));
//     agregar_int_al_paquete_personalizado(paquete, tamanio);

// 	enviar_paquete(paquete, socket_cliente_cpu);
// 	eliminar_paquete(paquete);
// }
   
// char* espero_rta_lectura_string_de_memoria(){
//     char* string_reconstruido;
//     int control = 0;
//     t_buffer* buffer;
//     int pid;
//     t_direccion_fisica* dir_fisica;
//     char* string_leido;

//     int cod_op_memoria_aux = recibir_operacion(socket_cliente_cpu);

//     while(control == 0){
//         switch (cod_op_memoria_aux) {

//             case CPU_RECIBE_LECTURA_STRING:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_32]
//                 // Pueden ser mas de una
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 string_leido = recibir_uint32_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %s ", pid, dir_fisica->nro_marco ,dir_fisica->offset, string_leido);
//                 free(buffer);
//                 break; 

//             case CPU_RECIBE_LECTURA_U_STRING:   // [PID, DF, VALOR, VALOR FINAL] -> [Int, Direccion_fisica, uint_32, uint_32]

//                 // Esta es la ultima
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 string_leido = recibir_uint32_del_buffer(buffer);
//                 string_reconstruido = recibir_uint32_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Valor: %s ", pid, dir_fisica->nro_marco ,dir_fisica->offset, string_leido);
//                 log_info(log_cpu, "ACCIÓN COMPLETADA: LEER %u EN MEMORIA", string_reconstruido);
//                 control = 1;
//                 free(buffer);
//                 break; 

//             case -1:
//                 log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//                 exit(1);

//             default:
//                 log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//                 break;
//             }

//     }

//     free(buffer);
//     return string_reconstruido;
// }

// void peticion_escritura_string_a_memoria(int pid, t_list* direcciones_fisicas, char* string_por_escribir){

//     t_paquete* paquete = crear_paquete_personalizado(CPU_PIDE_GUARDAR_STRING);

//     agregar_int_al_paquete_personalizado(paquete, pid);
//     agregar_estructura_al_paquete_personalizado(paquete, &direcciones_fisicas, sizeof(t_list));
//     agregar_uint32_al_paquete_personalizado(paquete, string_por_escribir);

// 	enviar_paquete(paquete, socket_cliente_cpu);
// 	eliminar_paquete(paquete);
// }


// void espero_rta_escritura_string_de_memoria(int tamanio){
//     char string_completo[tamanio];
//     int control = 0;
//     t_buffer* buffer;
//     int pid;
//     t_direccion_fisica* dir_fisica;
//     char* string_escrito;

//     int cod_op_memoria_aux = recibir_operacion(socket_cliente_cpu);
//     while(control == 0){
//         switch (cod_op_memoria_aux) {

//             case CPU_RECIBE_OK_STRING_DE_ESCRITURA:   // [PID, DF, VALOR] -> [Int, Direccion_fisica, uint_32]
//                 // Pueden ser mas de una
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 string_escrito = recibir_string_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %s ", pid, dir_fisica->nro_marco ,dir_fisica->offset, string_escrito);
//                 free(buffer);
//                 break; 

//             case CPU_RECIBE_ULT_OK_STRING_DE_ESCRITURA:   // [PID, DF, VALOR, VALOR FINAL] -> [Int, Direccion_fisica, uint_32, uint_32]
//                 // Esta es la ultima
//                 buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                 pid = recibir_int_del_buffer(buffer);
//                 dir_fisica = recibir_estructura_del_buffer(buffer);
//                 string_escrito = recibir_string_del_buffer(buffer);
//                 string_completo = recibir_string_del_buffer(buffer);
//                 log_info(log_cpu, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Valor: %s ", pid, dir_fisica->nro_marco ,dir_fisica->offset, string_escrito);
//                 log_info(log_cpu, "ACCIÓN COMPLETADA: ESCRIBIR %s EN MEMORIA", string_completo);
//                 control = 1;
//                 free(buffer);
//                 break; 

//             case -1:
//                 log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//                 exit(1);

//             default:
//                 log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//                 break;
//             }

//     }

//     free(buffer);
// }