#include "extra.h"

// los registros solo van a tener dos tamaños distintos
// typedef struct{
// 	uint32_t pc; // Program Counter
// 	uint8_t ax; // Acumulador
// 	uint8_t bx; // Base
// 	uint8_t cx; // Counter
// 	uint8_t dx; // Data
// 	uint32_t eax;
// 	uint32_t ebx;
// 	uint32_t ecx;
// 	uint32_t edx;
// 	uint32_t si; // Contiene la direccion logica de memoria de origen desde donde se va a copiar un string
// 	uint32_t di; // Contiene la direccion logica de memoria de destino a donde se va a copiar un string
// }registros_cpu;

// INT 8 O 32 bits
//     1 o 4 bytes

// ARMO UNA LISTA POR SI TENGO QUE LEER/ESCRIBIR DE MAS DE UNA PAGINA

t_list* buscar_todas_las_paginas(int direccion_logica, int tamaño_a_operar){
    // Creo una lista en donde voy a guardar todas las direcciones fisicas que necesite para operar desde la direccion logica solicitada
    t_list* direcciones_fisicas = list_create();

    // Calculo mi primera pagina y su desplazamiento
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;

    // La traduzco y la agrego a la lista
    //t_direccion_fisica* direccion_fisica_traducida = traducir_direccion_logica_a_fisica(numero_pagina, desplazamiento);
    //list_add(direcciones_fisicas, direccion_fisica_traducida);

    int tamaño_ya_evaluado = tamanio_pagina - desplazamiento;
    
    // Voy a preguntar si me alcanza lo leido con lo que quiero operar -> si no tengo que leer otra pag
    while (tamaño_a_operar > tamaño_ya_evaluado){
        numero_pagina = numero_pagina + 1;
        desplazamiento = 0;
        //direccion_fisica_traducida = traducir_direccion_logica_a_fisica(numero_pagina, desplazamiento);
        //list_add(direcciones_fisicas, direccion_fisica_traducida);
        tamaño_ya_evaluado = tamaño_ya_evaluado + tamanio_pagina;
    }
    
    return direcciones_fisicas;
}


// t_direccion_fisica* traducir_direccion_logica_a_fisica(int numero_pagina, int desplazamiento) {
//     t_direccion_fisica* direccion_fisica_traducida;

//     log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

//     // 1ero voy a verificar si esta en TLB -> si esta en TLB me ahorro un acceso a memoria
//     TLB_Entrada respuesta = buscar(numero_pagina); 

//     if(respuesta.pid != -1) {
//         // Esta en TLB
//         log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numero_pagina);
//         direccion_fisica_traducida->nro_marco = respuesta.numero_marco;
//         direccion_fisica_traducida->offset = desplazamiento;
//         return direccion_fisica_traducida;

//     } else {
//         // No esta en TLB -> tiene que buscar en memoria
//         log_info(log_cpu, "PID: %d - TLB MISS - Pagina: %d", proceso->pid, numero_pagina);

//         // Tengo el numero de pag -> hago una consulta a memoria por el marco
//         t_paquete *paquete = crear_paquete_personalizado(CPU_PIDE_MARCO_A_MEMORIA); // [PID, NUMERO DE PAGINA]
//         agregar_int_al_paquete_personalizado(paquete, proceso->pid);
//         agregar_int_al_paquete_personalizado(paquete, numero_pagina);
//         enviar_paquete(paquete, socket_cliente_cpu);

//         // Recibo marco de memoria
//         int cod_op_memoria = recibir_operacion(socket_cliente_cpu);
//         while(1) {
//             switch (cod_op_memoria) {
//                 case CPU_RECIBE_NUMERO_DE_MARCO_DE_MEMORIA:
//                     t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                     int marco = recibir_int_del_buffer(buffer);
//                     log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", proceso->pid, numero_pagina, marco);
//                     direccion_fisica_traducida->nro_marco = marco;
//                     direccion_fisica_traducida->offset = desplazamiento;

//                     // Agregarlo a la tlb
//                     TLB_Entrada nueva_entrada;
//                     nueva_entrada.pid = proceso->pid;
//                     nueva_entrada.numero_pagina = numero_pagina;
//                     nueva_entrada.numero_marco = marco;
//                     actualizar_tlb(&nueva_entrada); 
//                     free(buffer);
//                     return direccion_fisica_traducida;
//                     break; 
//                 case EXIT:
//                     error_exit(EXIT);
//                     free(direccion_fisica_traducida);
//                     break;
//                 case -1:
//                     log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//                     free(direccion_fisica_traducida);
//                     free(socket_cliente_cpu);
//                     exit(1);
//                     return;
//                 default:
//                     log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//                     free(direccion_fisica_traducida);
//                     break;
//             }
//         }
//         eliminar_paquete(paquete);
//     }
// }

// // voy a implementar lectura de esas direcciones fisicas

//  leer_de_memoria (t_list* direcciones_fisicas){
//     void* valor_leido;
//     void* valor_leido_reconstruido;
//     int posicion = 0;
//     t_direccion_fisica una_direccion;

//     while (posicion < list_size(direcciones_fisicas)){
//         una_direccion = list_get(direcciones_fisicas, posicion);
//         valor_leido = pedirle_a_memoria_lectura(una_direccion, int tamaño_que_quiero_leer);
//         valor_leido_reconstruido = valor_leido_reconstruido 
//     }

//     return valor_leido;
// }

// void pedirle_a_memoria_lectura(t_direccion_fisica una_direccion, int tamaño_que_quiero_leer){
//     t_paquete *paquete = crear_paquete_personalizado(CPU_PIDE_LECTURA_MEMORIA); // [PID, DIRECCION FISICA, TAMAÑO]
//     agregar_int_al_paquete_personalizado(paquete, proceso->pid);
//     agregar_void_al_paquete_personalizado(paquete, una_direccion);
//     agregar_int_al_paquete_personalizado(paquete, tamaño_que_quiero_leer);

//     enviar_paquete(paquete, socket_cliente_cpu);

//     // Recibo valor leido de memoria
//         int cod_op_memoria = recibir_operacion(socket_cliente_cpu);
//         while(1) {
//             switch (cod_op_memoria) {
//                 case CPU_RECIBE_LECTURA_DE_MEMORIA:
//                     t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                     int direccion = recibir_int_del_buffer(buffer);
//                     void* leido = recibir_estructura_del_buffer(buffer);
                    
//                     log_info(log_cpu, "PID: %d - Acción: LEER - Dirección Física: %d - Valor: %p", proceso->pid, direccion, leido);
        
//                     free(buffer);
//                     return leido;

//                     break; 
//                 case EXIT:
//                     error_exit(EXIT);
//                     break;
//                 case -1:
//                     log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
//                     free(socket_cliente_cpu);
//                     exit(1);
//                     return;
//                 default:
//                     log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
//                     break;
//             }
//         }
//         eliminar_paquete(paquete);
// }