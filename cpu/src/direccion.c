#include <direccion.h>

#define MAX_ENTRADAS_TLB 100 // No se como buscar este valor

int traducir_direccion_logica_a_fisica(int direccion_logica)
{
    int tamanio_pagina = config_get_int_value(config_cpu, "TAM_MAX_PAGINACION");
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
    log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

    TLB_Entrada respuesta = buscar(numero_pagina); // No es un puntero

    if(respuesta.pid != -1)
    {
        log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numero_pagina);
        return (respuesta.numero_marco * tamanio_pagina) + desplazamiento;
    } 
    else 
    {
        log_info(log_cpu, "PID: %d - TLB MISS - Pagina: %d", proceso->pid, numero_pagina);

        // Solicitar marco
        t_paquete *paquete = crear_paquete_personalizado(MARCO);
        agregar_estructura_al_paquete_personalizado(paquete, &numero_pagina, sizeof(int));
        agregar_estructura_al_paquete_personalizado(paquete, &proceso->pid, sizeof(int));
        enviar_paquete(paquete, socket_cliente_cpu);

        // Recibir respuesta de memoria
        int cod_op = recibir_operacion(socket_cliente_cpu);
        if(cod_op == MARCO) // Agregar a struct
        {
            int marco;
            recibir_marco(socket_cliente_cpu, &marco); // Suponiendo que recibimos el número de marco correctamente
            log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", proceso->pid, numero_pagina, marco);
            
            // Agregarlo a la tlb
            TLB_Entrada nueva_entrada;
            nueva_entrada.pid = proceso->pid;
            nueva_entrada.numero_pagina = numero_pagina;
            nueva_entrada.numero_marco = marco;
            actualizar_tlb(&nueva_entrada); // Suponiendo que tienes una función para actualizar la TLB

            return (marco * tamanio_pagina) + desplazamiento;
        }
        else
        {
            log_error(log_cpu, "Error al recibir el marco de la memoria");
            return -1; 
        }

        // RECIBIR MARCO
        // char *mensaje = recibir_valor(socket_memoria);
        // nro_marco = atoi(mensaje);
        // free(mensaje);

        // if (nro_marco >= 0){
        //     log_info(logger, "PID: %d -OBTUVE MARCO - Página:< %d  > - Marco: < %d >", pid, numero_pagina, nro_marco);
        //     direccion_fisica = (nro_marco * tamanio_pagina) + desplazamiento;
        // }

    }
}

int recibir_marco(int socket_cliente, int* marco) 
{
    // Intenta recibir el número de marco desde el socket
    int bytes_recibidos = recv(socket_cliente, marco, sizeof(int), MSG_WAITALL);
    
    // Verifica si la recepción fue exitosa
    if (bytes_recibidos == sizeof(int)) 
    {
        return 0; // Recepción exitosa
    } 
    else if (bytes_recibidos == -1) 
    {
        // Si recv devuelve -1, indica un error en la recepción
        perror("Error al recibir el número de marco");
        close(socket_cliente);
        return -1; // Error
    } 
    else 
    {
        // Si recv devuelve otro valor, indica que se recibió una cantidad inesperada de bytes
        fprintf(stderr, "Error: Se recibieron %d bytes en lugar de %lu\n", bytes_recibidos, sizeof(int));
        close(socket_cliente);
        return -1; // Error
    }
}

TLB_Entrada buscar(int numero_pagina) 
{
    for(int i = 0; i < tlb->cantidad_entradas; i++) // No entiendo que es la listaTLB
    {
        // tlb* numero = list_get(listaTLB, i);
        // if (numero->pagina == numero_pagina) 
        if(tlb->entradas[i].numero_pagina == numero_pagina)
        {
            // return numero;
            return tlb->entradas[i];
        }
    }
    // return NULL;
    TLB_Entrada entrada_vacia;
    entrada_vacia.pid = -1;
    entrada_vacia.numero_pagina = -1;
    entrada_vacia.numero_marco = -1;
    return entrada_vacia;
}

void actualizar_tlb(TLB_Entrada* nueva_entrada) 
{
    // Insertar la nueva entrada en la TLB
    // Por ejemplo, aquí podrías implementar un algoritmo de reemplazo si la TLB está llena
    // O simplemente agregar la entrada al final si hay espacio
    // Aquí asumo que tienes una estructura TLB definida y un arreglo de entradas en ella
    // Supongamos que tienes una variable global tlb de tipo TLB*
    if (tlb->cantidad_entradas < MAX_ENTRADAS_TLB) {
        // Si hay espacio en la TLB, simplemente agregamos la entrada al final
        tlb->entradas[tlb->cantidad_entradas] = *nueva_entrada;
        tlb->cantidad_entradas++;
    } else {
        // Si la TLB está llena, puedes implementar un algoritmo de reemplazo (FIFO, LRU, etc.)
        // Por simplicidad, aquí asumiremos que simplemente sobrescribimos la primera entrada (FIFO)
        // O una entrada aleatoria (LRU)
        // Implementación del algoritmo de reemplazo
        // Por ejemplo, reemplazamos la primera entrada (FIFO)
        tlb->entradas[0] = *nueva_entrada;
    }
}

//----------------------------------------------------------------

// int MMU(int dirLogica) {


//     objetoTLB* retornoTLB = buscarEnTLB(numeroPagina);  // Buscar en la TLB
//     if (retornoTLB != NULL) {
//         // TLB HIT
//         log_info(logger, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numeroPagina);
//         return (retornoTLB->numMarco * tamanio_pagina) + desplazamiento;
//     } else {
//         // TLB MISS
//         log_info(logger, "PID: %d - TLB MISS - Pagina: %d", proceso->pid, numeroPagina);
        
//         // Mandar solicitud de marco correspondiente a la memoria
//         mandar_numero_actualizado(conexion_memoria, numeroPagina, BUSCAR_MARCO);
        
//         op_code codOp = recibir_operacion(conexion_memoria);
//         if (codOp == NUMERO) {
//             int marco = recibir_numero(conexion_memoria);
            
//             // Agregar la nueva entrada a la TLB
//             if (list_size(listaTLB) < config.cantidad_entradas_tlb) {
//                 agregar_a_TLB(proceso->pid, numeroPagina, marco);
//             } else {
//                 // Ejecutar el algoritmo de sustitución si la TLB está llena
//                 algoritmoSustitucion(proceso->pid, numeroPagina, marco);
//             }
            
//             return (marco * tamanio_pagina) + desplazamiento;
//         } else {
//             // Manejar el caso de error en la recepción de marco
//             log_error(logger, "Error al recibir el marco de memoria");
//             return -1;  // o algún valor de error adecuado
//         }
//     }
// }