#include "paginacion.h"

//******************************************************************
//******************** PAGINACIÓN SIMPLE ***************************
//******************************************************************

// Me llega algo para guardar, primero reviso si hay lugar en memoria
// SI hay lugar, guardo elemento
// SI no Out Of Memory

// bool hayLugarEnMemoria(int paginasNecesarias){
    
//     int cant_marcos_libres = 0; 
//     int desplazamiento = 0;
    
//     while(desplazamiento < cant_marcos){
			
// 		pthread_mutex_lock(&mutex_bitmap_marcos);

// 		if(bitarray_test_bit(bitmap_marcos, desplazamiento) == 0){
// 			cant_marcos_libres++; 
//         }
// 		pthread_mutex_unlock(&mutex_bitmap_marcos);

// 		desplazamiento++;
// 	}

//     if(cant_marcos_libres >= paginasNecesarias){
//         return 1;
//     }else{
//         return 0;
//     }
// }

// t_list* guardarElemento(void* elemento_a_guardar, int tamanio){ 
    
//     t_list* paginasQueOcupe = list_create();

//     // ver si esta bien la lista, o es mas conveniente buscar aca con el bitmap
//     // REVISAR
    
//     if(tamanio <= config_memoria->tam_pagina){   
//         // LO QUE TENGO QUE GUARDAR ENTRA EN UNA SOLA PAGINA
        
//         t_frame* frame = list_get(marcos_libres, 0); // ELIJO EL PRIMER FRAME DE LA LISTA DE LIBRES
        
//         guardarEnMemoriaPaginacion(elemento_a_guardar, tamanio, frame);
        
//         // ver para que sirve crear pagina
//         t_fila_tabla_paginas* pagina = crearPagina(frame, (config_memoria->tam_pagina - tamanio)); 
//         // Faltaria agregar la pagina a la tabla correspondiente 
//         agregar_pag_a_tabla (t_proceso* proceso, pagina);

//     } else {  // NECESITO MÁS PÁGINAS, NO ENTRA TODO EN UNA
//         //analizo cuantas pag necesito

//         int sobrante = tamanio - config_memoria->tam_pagina;        // lo que no me entra en la primer pagina
//         t_list* pagina = guardarElemento(elemento_a_guardar, config_memoria->tam_pagina);   //GUARDO LO PRIMERO
//         t_list* restoPaginas = guardarElemento(elemento_a_guardar + config_memoria->tam_pagina, sobrante);
        
//         list_add_all(paginasQueOcupe,pagina);
// 	    list_add_all(paginasQueOcupe,restoPaginas);
//         list_destroy(pagina);
//         list_destroy(restoPaginas);
//     }
    
//     return paginasQueOcupe;
// }

// void guardarEnMemoriaPaginacion(void* algo, int tamanio, t_frame* unFrame){

//     ocuparFrame(unFrame);
//     ocuparMemoriaPaginacion(algo, tamanio, unFrame->id, 0); //LE PASO 0 PORQUE ES UNA PAGINA VACIA
    
// }

// void ocuparMemoriaPaginacion(void* algo, int tamanio, int idFrame, int desplazamiento){
//     // Pongo desplazamiento por si el marco tenia un espacio libre
//     //BASE DEL FRAME = ID*TAMANIO_PAGINA + DESPLAZAMIENTO(dentro de la pagina) -> EL ID INDICA EL NRO DE FRAME

//     int base = ((idFrame * config_valores.tamanio_pagina) + desplazamiento);
    
//     pthread_mutex_lock(&mutex_espacio_usuario);
//     memcpy(espacio_usuario + base, algo, tamanio);
//     pthread_mutex_unlock(&mutex_espacio_usuario);
// }

// // Pongo el marco del bitmap ocupado y saco de la lista de marcos libres al que ocupe
// void ocuparFrame(t_frame* unFrame){

//     pthread_mutex_lock(&mutex_bitmap_marcos);

//     bitarray_set_bit(bitmap_marcos,(unFrame->id));
//     list_remove(marcos_libres, 0);

//     pthread_mutex_unlock(&mutex_bitmap_marcos);	
    
// }

// t_fila_tabla_paginas* crearPagina(t_frame* frame, int espacioDisp){

//     t_fila_tabla_paginas* pagina = malloc(sizeof(t_fila_tabla_paginas));

// 	pagina->frame = frame->id;
// 	pagina->presencia = 1; // esta en MP
// 	pagina->modificado = 1;// Lo pongo en 1 asi se carga en disco
//     pagina->tam_disponible = espacioDisp;

//     return pagina;
// }

// void agregar_pag_a_tabla (t_proceso* proceso, t_instruccion_codigo* pagina){
//     list_add(proceso->tabla_paginas, pagina);
// }