#include "pcb.h"

// pcb* proceso;

// void recibir_pcb(t_list *lista, pcb *proceso)
// {
//     // void memcpy(destino, elemento que obtenemos de la lista, size);
    
//     // De la lista, obtengo el pid
//     int i = 0;
// 	memcpy(&(proceso->pid), list_get(lista, i++), sizeof(int));

//     // Obtenemos las instrucciones
// 	int cantidad_instrucciones;
// 	memcpy(&(cantidad_instrucciones), list_get(lista, i++), sizeof(int));

//     // Si proceso->instruccion es == NULL, destruye la lista y sus elementos (del pcb) 
// 	if(proceso->instrucciones) 
//     {
// 		list_destroy_and_destroy_elements(proceso->instrucciones, free);
// 	}

// 	proceso->instrucciones = list_slice_and_remove(lista, i, cantidad_instrucciones);

//     // Obtenemos el program counter
// 	memcpy(&(proceso->program_counter), list_get(lista, i++), sizeof(int));
	
//     // Obtenemos los registros
//     memcpy(proceso->registros.ax, list_get(lista, i++), 8);
// 	memcpy(proceso->registros.bx, list_get(lista, i++), 8);
// 	memcpy(proceso->registros.cx, list_get(lista, i++), 8);
// 	memcpy(proceso->registros.dx, list_get(lista, i++), 8);
// 	memcpy(proceso->registros.eax, list_get(lista, i++), 32);
// 	memcpy(proceso->registros.ebx, list_get(lista, i++), 32);
// 	memcpy(proceso->registros.ecx, list_get(lista, i++), 32);
// 	memcpy(proceso->registros.edx, list_get(lista, i++), 32);
// 	memcpy(proceso->registros.si, list_get(lista, i++), 32);
// 	memcpy(proceso->registros.di, list_get(lista, i++), 32);
// }