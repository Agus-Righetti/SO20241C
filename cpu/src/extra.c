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

// t_list* traducir_todas_las_fisicas_1byte (int direccion_logica){
//     t_list
// }
// t_list* traducir_direccion_logica_a_fisica(int direccion_logica) {
//     int numero_pagina = floor(direccion_logica / tamanio_pagina);
//     int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
//     log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

//     TLB_Entrada respuesta = buscar(numero_pagina); 

//     if(respuesta.pid != -1)
//     {
//         log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numero_pagina);
//         return (respuesta.numero_marco * tamanio_pagina) + desplazamiento;
//     } 
//     else 
//     {
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
//             switch (cod_op_memoria)
//             {
//                 case CPU_RECIBE_NUMERO_DE_MARCO_DE_MEMORIA:
//                     t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
//                     int marco = recibir_int_del_buffer(buffer);
//                     log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", proceso->pid, numero_pagina, marco);
                    
//                     // Agregarlo a la tlb
//                     TLB_Entrada nueva_entrada;
//                     nueva_entrada.pid = proceso->pid;
//                     nueva_entrada.numero_pagina = numero_pagina;
//                     nueva_entrada.numero_marco = marco;
//                     actualizar_tlb(&nueva_entrada); 
//                     free(buffer);
//                     return (marco * tamanio_pagina) + desplazamiento;
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
//     }
// }