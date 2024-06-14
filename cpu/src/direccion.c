#include <direccion.h>

int traducir_direccion_logica_a_fisica(int direccion_logica)
{
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
    log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

    TLB_Entrada respuesta = buscar(numero_pagina); 

    if(respuesta.pid != -1)
    {
        log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numero_pagina);
        return (respuesta.numero_marco * tamanio_pagina) + desplazamiento;
    } 
    else 
    {
        // NO esta en TLB -> tiene que buscar en memoria
        log_info(log_cpu, "PID: %d - TLB MISS - Pagina: %d", proceso->pid, numero_pagina);

        // Tengo el numero de pag -> hago una consulta a memoria por el marco
    
        t_paquete *paquete = crear_paquete_personalizado(MARCO);      // [PID, NUMERO DE PAGINA]
        agregar_int_al_paquete_personalizado(paquete, proceso->pid);
        agregar_int_al_paquete_personalizado(paquete, numero_pagina);
        enviar_paquete(paquete, socket_cliente_cpu);

        // Recibir marco de memoria
        //////////////////////////////////////////
        int cod_op_memoria = recibir_operacion(socket_cliente_cpu);
        while(1) {
            switch (cod_op_memoria) {
                
                case CPU_RECIBE_NUMERO_DE_MARCO_DE_MEMORIA:
                    t_buffer* buffer = recibiendo_paquete_personalizado(socket_cliente_cpu);
                    int marco = recibir_int_del_buffer(buffer);
                    log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", proceso->pid, numero_pagina, marco);
                    // Agregarlo a la tlb
                    TLB_Entrada nueva_entrada;
                    nueva_entrada.pid = proceso->pid;
                    nueva_entrada.numero_pagina = numero_pagina;
                    nueva_entrada.numero_marco = marco;
                    actualizar_tlb(&nueva_entrada); 
                    free(buffer);
                    break; 
    
                case EXIT:
                    error_exit(EXIT);
                    list_destroy_and_destroy_elements(lista, free); 
                    break;
                case -1:
                    log_error(log_cpu, "MEMORIA se desconecto. Terminando servidor");
                    free(socket_cliente_cpu);
                    exit(1);
                    return;
                default:
                    log_warning(log_cpu,"Operacion desconocida. No quieras meter la pata");
                    break;
            }
        }
        return (marco * tamanio_pagina) + desplazamiento;
    }
}
        

// int recibir_marco(int socket_cliente, int* marco) 
// {
//     // Intenta recibir el número de marco desde el socket
//     int bytes_recibidos = recv(socket_cliente, marco, sizeof(int), MSG_WAITALL);
    
//     // Verifica si la recepción fue exitosa
//     if (bytes_recibidos == sizeof(int)) 
//     {
//         return 0; // Recepción exitosa
//     } 
//     else if (bytes_recibidos == -1) 
//     {
//         // Si recv devuelve -1, indica un error en la recepción
//         perror("Error al recibir el número de marco");
//         close(socket_cliente);
//         return -1; // Error
//     } 
//     else 
//     {
//         // Si recv devuelve otro valor, indica que se recibió una cantidad inesperada de bytes
//         fprintf(stderr, "Error: Se recibieron %d bytes en lugar de %lu\n", bytes_recibidos, sizeof(int));
//         close(socket_cliente);
//         return -1; // Error
//     }
// }

TLB_Entrada buscar(int numero_pagina) 
{
    for(int i = 0; i < tlb->cantidad_entradas; i++) 
    {
        if(tlb->entradas[i].numero_pagina == numero_pagina)
        {
            return tlb->entradas[i];
        }
    }

    TLB_Entrada entrada_vacia;
    entrada_vacia.pid = -1;
    entrada_vacia.numero_pagina = -1;
    entrada_vacia.numero_marco = -1;
    return entrada_vacia;
}

void actualizar_tlb(TLB_Entrada* nueva_entrada) 
{
    cantidad_entradas_tlb = config_get_int_value(config_cpu, "CANTIDAD_ENTRADAS_TLB");
    algoritmo_tlb = strdup(config_get_string_value(config_cpu, "ALGORITMO_TLB"));

    if(tlb->cantidad_entradas < cantidad_entradas_tlb) 
    {
        // Si hay espacio en la TLB, simplemente agregamos la entrada al final
        tlb->entradas[tlb->cantidad_entradas] = *nueva_entrada;
        tlb->uso_lru[tlb->cantidad_entradas] = tlb->cantidad_entradas; // Actualiza el uso LRU
        tlb->cantidad_entradas++;
    } 
    else 
    {
        if (strcmp(algoritmo_tlb, "FIFO") == 0) 
        {
            // Implementación del algoritmo FIFO
            for (int i = 0; i < cantidad_entradas_tlb - 1; i++) 
            {
                tlb->entradas[i] = tlb->entradas[i + 1];
            }
            tlb->entradas[cantidad_entradas_tlb - 1] = *nueva_entrada;        
        } 
        else if (strcmp(algoritmo_tlb, "LRU") == 0) 
        {
            // Implementación del algoritmo LRU
            int lru_index = 0;
            for (int i = 1; i < cantidad_entradas_tlb; i++) 
            {
                if (tlb->uso_lru[i] < tlb->uso_lru[lru_index]) 
                {
                    lru_index = i;
                }
            }
            tlb->entradas[lru_index] = *nueva_entrada;
            // Actualiza los usos para reflejar el nuevo uso más reciente
            for (int i = 0; i < cantidad_entradas_tlb; i++) 
            {
                if (tlb->uso_lru[i] >= 0) 
                {
                    tlb->uso_lru[i]++;
                }
            }
            tlb->uso_lru[lru_index] = 0;
        } 
        else 
        {
            log_error(log_cpu, "Algoritmo de reemplazo de TLB no soportado: %s", algoritmo_tlb);
        }
    }
}

void inicializar_tlb() 
{
    // Inicializa la TLB y otros parámetros
    tlb = malloc(sizeof(TLB));

    if (tlb == NULL) 
    {
        fprintf(stderr, "No se pudo asignar memoria para la TLB\n");
        exit(EXIT_FAILURE);
    }

    tlb->cantidad_entradas = 0;
    tlb->entradas = malloc(cantidad_entradas_tlb * sizeof(TLB_Entrada));

    if (tlb->entradas == NULL) 
    {
        fprintf(stderr, "No se pudo asignar memoria para las entradas de la TLB\n");
        exit(EXIT_FAILURE);
    }

    tlb->uso_lru = malloc(cantidad_entradas_tlb * sizeof(int)); // Solo necesario para LRU

    if (tlb->uso_lru == NULL) 
    {
        fprintf(stderr, "No se pudo asignar memoria para el seguimiento de uso LRU\n");
        exit(EXIT_FAILURE);
    }

    // Inicializa el array de uso LRU
    for (int i = 0; i < cantidad_entradas_tlb; i++) 
    {
        tlb->uso_lru[i] = -1;
    }
}