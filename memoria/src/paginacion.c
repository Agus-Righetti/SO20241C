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
// void* lectura_en_memoria(int direccion_fisica, int tamaño, void* valor){

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
