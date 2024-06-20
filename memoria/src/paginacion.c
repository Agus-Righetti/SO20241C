#include "paginacion.h"

//******************************************************************
//******************** PAGINACIÓN SIMPLE ***************************
//******************************************************************

// Funciones generales utiles
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
	//pagina->presencia = 1; // esta en MP -> puede ser innecesario

    // pagina->tam_disponible = espacioDisp;

    return pagina;
}

void agregar_pag_a_tabla (t_proceso* proceso, t_pagina* pagina){
    list_add(proceso->tabla_paginas, pagina);
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
//******************************************************************

//******************************************************************
//*********************** LECTURA EN PAG ***************************
//******************************************************************
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

//******************************************************************
//********************** ESCRITURA EN PAG **************************
//******************************************************************
// Tengo que guardar algo en memoria
// me llega [PID, DF, TAMAÑO, VALOR]

// Dirección física
// Tamaño
// Valor

// Ante un pedido de escritura, escribir lo indicado a partir de la dirección física pedida. 
//En caso satisfactorio se responderá un mensaje de ‘OK’.

// AUXILIARES PARA PROBAR Y ENTENDER GUARDAR
// Función para guardar un dato en un espacio de memoria predefinido
// Función para guardar un dato en una posición específica dentro de un bloque de memoria
void guardar_en_memoria(void *dato, size_t tamano, size_t offset) {
    if (dato != NULL) {
        memcpy((char*)espacio_usuario + offset, dato, tamano);  // Copiamos el dato a la posición especificada
    }
}

// Función para leer un dato desde una posición específica dentro de un bloque de memoria
// Función para leer un dato desde una posición específica dentro de un bloque de memoria
void* leer_desde_memoria(size_t tamano, size_t offset) {

    void* destino = malloc(tamano);  // Asignamos memoria para el dato leído
    if (destino != NULL) {
        memcpy(destino, (char*)espacio_usuario + offset, tamano);  // Copiamos el dato desde la posición especificada
        return destino;  // Devolvemos el puntero al dato leído
    }

    return NULL;  // Devolvemos NULL si hay algún problema
}