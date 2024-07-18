#include "paginacion.h"

//******************************************************************
//*************************** PAGINACIÓN ***************************
//************************** FUNCIONES GN **************************

void ocupar_marco(int posicion){
    pthread_mutex_lock(&mutex_bitmap_marcos);
    bitarray_set_bit(bitmap_marcos, posicion);
    pthread_mutex_unlock(&mutex_bitmap_marcos);	
}

void liberar_marco(int posicion){
    pthread_mutex_lock(&mutex_bitmap_marcos);
    bitarray_clean_bit(bitmap_marcos, posicion);
    pthread_mutex_unlock(&mutex_bitmap_marcos);	
}

t_pagina* crear_pagina(t_frame* un_frame){
    t_pagina* pagina = malloc(sizeof(t_pagina));

	pagina->frame = un_frame->id;

    return pagina;
}

void agregar_pag_a_tabla (t_proceso* proceso, t_pagina* pagina){

    list_add(proceso->tabla_paginas, pagina);
    
    log_info(log_memoria, "Agregue una pagina a la tabla: %d", list_size(proceso->tabla_paginas));

}

bool hay_lugar_en_memoria(int paginasNecesarias){ 
    int cant_marcos_libres = 0; 
    int desplazamiento = 0;
    
    while(desplazamiento < cant_marcos){
			
		pthread_mutex_lock(&mutex_bitmap_marcos);

		if(bitarray_test_bit(bitmap_marcos, desplazamiento) == 0){
			cant_marcos_libres++; 
        }
		pthread_mutex_unlock(&mutex_bitmap_marcos);

		desplazamiento++;
	}

    if(cant_marcos_libres >= paginasNecesarias){
        return 1; // Hay espacio en memoria 
    }else{
        return 0; // No hay espacio en memoria
    }
}

t_list* buscar_marcos_libres(){
    t_list* marcosLibres = list_create();
    int base = 0;

    pthread_mutex_lock(&mutex_bitmap_marcos);
    while(base < cant_marcos){
        if(bitarray_test_bit(bitmap_marcos, base) == 0){ 
            // Reviso si los marcos estan en 0 en el bitmap -> estan LIBRES
			t_frame* unFrame = malloc(sizeof(t_frame)); 
            unFrame->id = base;
            list_add(marcosLibres, unFrame);
        }
        base++;
    }   
	pthread_mutex_unlock(&mutex_bitmap_marcos);	
    
    return marcosLibres; 
}

void eliminar_lista(t_list* lista){
	list_destroy_and_destroy_elements(lista, (void*)eliminar_algo);
}

void eliminar_algo(void* algo){
	free(algo);
}


//******************************************************************
//****************** ACCESO A ESPACIO USUARIO **********************
//*********************** LECTURA EN PAG ***************************

// para reutilizarlo voy a hacer que me devuelva un void*
// void* lectura_en_memoria(int pid, int numero_pagina, int desplazamiento, int tamaño){
//     // La direccion fisica es -> [número_pagina | desplazamiento]
//     // int numero_pagina = floor(direccion_logica / tamanio_pagina);
//     // int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

//     // 1ero busco el proceso segun el pid que recibo
//     t_proceso* proceso_por_leer = obtener_proceso_por_id(int pid);
//     bool estaDentroDeSuEspacio = verificar_proceso_dentro_de_su_espacio(proceso_por_leer, numero_pagina);
//     if(estaDentroDeSuEspacio == 1){
//         // Esta dentro de su espacio 
//         // hago lectura
//         void* destino;
//         pthread_mutex_lock(&mutex_espacio_usuario;
//         memcpy(data, buffer->stream + sizeof(int), size_data);
// 		pthread_mutex_unlock(&mutex_espacio_usuario;
        
//     } else {
//         log_info(log_memoria, "El proceso %d quiere acceder a un espacio de memoria que no tiene asignado", pid);
//         exit(1);   
//     }
// }

// Dirección física
// Tamaño
// Valor
// Ante un pedido de lectura, devolver el valor que se encuentra a partir de la dirección física pedida.



// uint32_t registro_ecx = 6489;
//     void* registro_ecx_puntero = &registro_ecx;

//     uint32_t registro_ecx_reconstruido = 0;
//     void* registro_ecx_reconstruido_puntero = &registro_ecx_reconstruido;
    
//     memcpy(espacio_usuario, registro_ecx_puntero, 2);
//     memcpy(espacio_usuario + 28, registro_ecx_puntero + 2, 2);
    
//     printf("El valor de ECX completo es: %d \n", registro_ecx);
//     printf("El valor de ECX antes de reconstruirlo: %d \n", registro_ecx_reconstruido);
    
//     memcpy(registro_ecx_reconstruido_puntero, espacio_usuario, 2);
//     memcpy(registro_ecx_reconstruido_puntero + 2, espacio_usuario + 28, 2);
    
//     printf("El valor de ECX despues de reconstruirlo: %d \n", registro_ecx_reconstruido);

void leer_uint32_en_memoria (int pid, t_list* direcciones_fisicas){

    uint32_t valor_leido = 0;
    void* valor_leido_puntero = &valor_leido;//void* ? revisar

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_leer == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;
    
    // Si solo hay una DF -> solo leo un marco
    if(cantidad_marcos_por_leer == 1){

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(valor_leido_puntero, espacio_usuario + despl_esp_usuario, 4);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);
        
        enviar_lectura_ult_4B_a_cpu(pid, dir_actual, valor_leido, valor_leido);

    } else {
        int bytes_ya_operados = 0;
        uint32_t valor_leido ;
        uint32_t valor_leido_reconstruido;

        // Voy a leer el dato de a poco
        while(cantidad_marcos_por_leer - 1 > indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido_puntero + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((uint8_t*)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((uint8_t*)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_lectura_4B_a_cpu(pid, dir_actual, valor_leido);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;

        } 
        if(cantidad_marcos_por_leer - 1 == indice_dir){
            // es la ultima lectura
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido_puntero + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((uint8_t*)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((uint8_t*)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_lectura_ult_4B_a_cpu(pid, dir_actual, valor_leido, valor_leido_reconstruido);

        }

    }

}

void enviar_lectura_4B_a_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_4B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);

    log_info(log_memoria, "Valor leido: %u", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void enviar_lectura_ult_4B_a_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor, uint32_t valor_leido_reconstruido){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_U_4B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);
    agregar_uint32_al_paquete_personalizado(paquete, valor_leido_reconstruido);

    log_info(log_memoria, "Valor leido: %u", valor);
    log_info(log_memoria, "Valor leido reconstruido: %u", valor_leido_reconstruido);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void leer_uint8_en_memoria (int pid, t_list* direcciones_fisicas){

    log_info(log_memoria, "Entre a leer_uint8_en_memoria");
    
    uint8_t valor_leido = 0;
    void* valor_leido_puntero = &valor_leido;

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    log_info(log_memoria, "Cantidad de marcos: %d", cantidad_marcos_por_leer);

    if (cantidad_marcos_por_leer == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }


    int indice_dir = 0;

    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido_puntero, espacio_usuario + despl_esp_usuario, 1);
    pthread_mutex_unlock(&mutex_espacio_usuario);

    log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);
    log_info(log_memoria, "Valor leido: %d ", valor_leido);

    enviar_lectura_1B_a_cpu(pid, dir_actual, valor_leido);

}


void enviar_lectura_1B_a_cpu(int pid, t_direccion_fisica* dir_actual, uint8_t valor){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_1B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint8_al_paquete_personalizado(paquete, valor);

    log_info(log_memoria, "Valor leido: %u", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}



//******************************************************************
//********************** ESCRITURA EN PAG **************************
//******************************************************************
// Ante un pedido de escritura, escribir lo indicado a partir de la dirección física pedida. 
// En caso satisfactorio se responderá un mensaje de OK. 

// Tengo que guardar algo en memoria
// me llega [PID, DF, TAMAÑO, VALOR]
//           int, lista, int, void

void guardar_uint32_en_memoria (int pid, t_list* direcciones_fisicas, uint32_t valor){
    // podria revisar que este dentro de su espacio -> ver

    log_info(log_memoria, "entre a guardar uint32 en memoria");

    // Creo un puntero al dato por guardar, para poder operar
    void* valor_puntero = &valor;

    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0; //lo cambie a cero
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;
    
    // Si solo hay una DF -> solo guardo el dato entero en un marco
    if(cantidad_marcos_por_guardar == 1){

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, valor_puntero, 4);  
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

        // faltaria hacer el caso en el que hay un error y no se manda
        enviar_ult_ok_4B_escritura_cpu(pid, dir_actual, valor, valor);


    } else {
        int bytes_ya_operados = 0;
        uint32_t valor_escrito ;

        // Tengo que particionar el dato y guardarlo
       while (cantidad_marcos_por_guardar - 1 > indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor_puntero + bytes_ya_operados, dir_actual->bytes_a_operar); 
            memcpy(espacio_usuario + despl_esp_usuario, (uint8_t*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_ok_4B_escritura_cpu(pid, dir_actual, valor_escrito);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;

        } 
        if (cantidad_marcos_por_guardar - 1 == indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            // es la ultima escritura
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor_puntero + bytes_ya_operados, dir_actual->bytes_a_operar); 
            memcpy(espacio_usuario + despl_esp_usuario, (uint8_t*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_ult_ok_4B_escritura_cpu(pid, dir_actual, valor_escrito, valor);

        }

    }
}

void enviar_ok_4B_escritura_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_OK_4B_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);
    
    log_info(log_memoria, "Valor escrito: %u", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void enviar_ult_ok_4B_escritura_cpu(int pid, t_direccion_fisica* dir_actual, uint32_t valor, uint32_t valor_completo){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_ULT_OK_4B_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);
    agregar_uint32_al_paquete_personalizado(paquete, valor_completo);

    log_info(log_memoria, "Valor escrito: %u", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void guardar_uint8_en_memoria (int pid, t_list* direcciones_fisicas, uint8_t valor){
    // podria revisar que este dentro de su espacio -> ver

    log_info(log_memoria, "Entre a guardar_uint8_en_memoria");

    // Creo un puntero al dato por guardar, para poder operar
    void* valor_puntero = &valor;

    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(espacio_usuario + despl_esp_usuario, valor_puntero, 1);  
    pthread_mutex_unlock(&mutex_espacio_usuario);

    log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);
    log_info(log_memoria, "VAlor leido: %u", valor);
    // faltaria hacer el caso en el que hay un error y no se manda
    enviar_ok_1B_escritura_cpu(pid, dir_actual, valor);

}


void enviar_ok_1B_escritura_cpu(int pid, t_direccion_fisica* dir_actual, uint8_t valor){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_OK_1B_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint8_al_paquete_personalizado(paquete, valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}


void guardar_string_en_memoria (int pid, t_list* direcciones_fisicas, char* valor, int tamanio){
    
    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;
    
    // Si solo hay una DF -> solo guardo en un marco
    if(cantidad_marcos_por_guardar == 1){

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, valor, tamanio);  
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

        enviar_ult_ok_string_escritura_cpu(pid, dir_actual, valor, valor);

    } else {
        int bytes_ya_operados = 0;
        char* valor_escrito ;

        // Tengo que particionar el dato y guardarlo
        while (cantidad_marcos_por_guardar - 1 > indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor + bytes_ya_operados, dir_actual->bytes_a_operar); 
            memcpy(espacio_usuario + despl_esp_usuario, (char*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_ok_string_escritura_cpu(pid, dir_actual, valor_escrito);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;
    

        } 
        if (cantidad_marcos_por_guardar - 1 == indice_dir){
            // es la ultima escritura
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor + bytes_ya_operados, dir_actual->bytes_a_operar); 
            memcpy(espacio_usuario + despl_esp_usuario, (char*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_ult_ok_string_escritura_cpu(pid, dir_actual, valor_escrito, valor);

        }

    }
}

void enviar_ok_string_escritura_cpu(int pid, t_direccion_fisica* dir_actual, char* valor){
    
    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_OK_STRING_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_string_al_paquete_personalizado(paquete, valor);
    
    log_info(log_memoria, "Valor escrito: %s", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void enviar_ult_ok_string_escritura_cpu(int pid, t_direccion_fisica* dir_actual, char* valor, char* valor_completo){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_ULT_OK_STRING_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_string_al_paquete_personalizado(paquete, valor);
    agregar_string_al_paquete_personalizado(paquete, valor_completo);

    log_info(log_memoria, "Valor escrito: %s", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}



void leer_string_en_memoria (int pid, t_list* direcciones_fisicas, int tamanio){

    char* valor_leido = NULL;

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_leer == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;
    
    // Si solo hay una DF -> solo leo un marco
    if(cantidad_marcos_por_leer == 1){

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(valor_leido, espacio_usuario + despl_esp_usuario, tamanio);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);
        
        enviar_lectura_ult_string_a_cpu(pid, dir_actual, valor_leido, valor_leido);

    } else {
        int bytes_ya_operados = 0;
        char* valor_leido ;
        char* valor_leido_reconstruido;

        // Voy a leer el dato de a poco
        while(cantidad_marcos_por_leer - 1 > indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char*)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char*)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_lectura_string_a_cpu(pid, dir_actual, valor_leido);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;
            

        } 
        if(cantidad_marcos_por_leer - 1 == indice_dir){
            // es la ultima lectura

            dir_actual = list_get(direcciones_fisicas, indice_dir);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_lectura_ult_string_a_cpu(pid, dir_actual, valor_leido, valor_leido_reconstruido);

        }

    }

}

void enviar_lectura_string_a_cpu(int pid, t_direccion_fisica* dir_actual, char* valor){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_STRING);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_string_al_paquete_personalizado(paquete, valor);

    log_info(log_memoria, "Valor leido: %s", valor);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}

void enviar_lectura_ult_string_a_cpu(int pid, t_direccion_fisica* dir_actual, char* valor, char* valor_leido_reconstruido){

    t_paquete* paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_U_STRING);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_lista_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));

    agregar_string_al_paquete_personalizado(paquete, valor);
    agregar_string_al_paquete_personalizado(paquete, valor_leido_reconstruido);

    log_info(log_memoria, "Valor leido: %s", valor);
    log_info(log_memoria, "Valor leido reconstruido: %s", valor_leido_reconstruido);

	enviar_paquete(paquete, socket_cliente_cpu);
	eliminar_paquete(paquete);
}


void leer_string_io_en_memoria(int pid, t_list* direcciones_fisicas, int tamanio){

    char* valor_leido = NULL;

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_leer == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;
    
    // Si solo hay una DF -> solo leo un marco
    if(cantidad_marcos_por_leer == 1){

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(valor_leido, espacio_usuario + despl_esp_usuario, tamanio);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);
        
        enviar_lectura_ult_string_a_cpu(pid, dir_actual, valor_leido, valor_leido);

    } else {
        int bytes_ya_operados = 0;
        char* valor_leido ;
        char* valor_leido_reconstruido;

        // Voy a leer el dato de a poco
        while(cantidad_marcos_por_leer - 1 > indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char*)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char*)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: <%d> - Accion: <LEER> - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            // enviar_lectura_string_a_io(pid, dir_actual, valor_leido);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;
        } 
        if(cantidad_marcos_por_leer - 1 == indice_dir){
            // es la ultima lectura

            dir_actual = list_get(direcciones_fisicas, indice_dir);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            memcpy((char)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_lectura_ult_string_a_io(valor_leido_reconstruido);

        }

    }

}

// void enviar_lectura_string_a_io(int pid, t_direccion_fisica* dir_actual, char* valor){

//     // t_paquete* paquete = crear_paquete_personalizado(IO_RECIBE_LECTURA_STRING);

//     // agregar_int_al_paquete_personalizado(paquete, pid);
//     // agregar_lista_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
//     // agregar_string_al_paquete_personalizado(paquete, valor);

//     log_info(log_memoria, "Valor leido: %s", valor);

// 	// enviar_paquete(paquete, socket_cliente_io);
// 	// eliminar_paquete(paquete);
// }


void enviar_lectura_ult_string_a_io(char* valor_leido_reconstruido){

    t_paquete* paquete = crear_paquete_personalizado(IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA);

    agregar_string_al_paquete_personalizado(paquete, valor_leido_reconstruido);

    log_info(log_memoria, "Valor leido reconstruido: %s", valor_leido_reconstruido);

	enviar_paquete(paquete, socket_cliente_io);

	eliminar_paquete(paquete);
}

void guardar_string_io_en_memoria (int pid, t_list* direcciones_fisicas, char* valor, int tamanio){
    
    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0) {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica* dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset ;
    
    // Si solo hay una DF -> solo guardo en un marco
    if(cantidad_marcos_por_guardar == 1){

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, valor, tamanio);  
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

        enviar_ult_ok_string_escritura_cpu(pid, dir_actual, valor, valor);

    } else {
        int bytes_ya_operados = 0;
        char* valor_escrito ;

        // Tengo que particionar el dato y guardarlo
        while (cantidad_marcos_por_guardar - 1 > indice_dir){
            dir_actual = list_get(direcciones_fisicas, indice_dir);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor + bytes_ya_operados, dir_actual->bytes_a_operar); 
            memcpy(espacio_usuario + despl_esp_usuario, (char*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            //enviar_ok_string_escritura_io(pid, dir_actual, valor_escrito);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;
    

        } 
        if (cantidad_marcos_por_guardar - 1 == indice_dir){
            // es la ultima escritura
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor + bytes_ya_operados, dir_actual->bytes_a_operar); 
            memcpy(espacio_usuario + despl_esp_usuario, (char*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar); 
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_ult_ok_string_escritura_io();
        }
    }
}


void enviar_ult_ok_string_escritura_io(){

    t_paquete* paquete = crear_paquete_personalizado(IO_RECIBE_RESPUESTA_DE_ESCRITURA_DE_MEMORIA);
    // falta aca agregar el valor completo

	enviar_paquete(paquete, socket_cliente_io);
	eliminar_paquete(paquete);
}