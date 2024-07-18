#include <direccion.h>

TLB_Entrada* buscar(int numero_pagina) 
{
    //log_info(log_cpu, "Entre a buscar");
    for(int i = 0; i < cantidad_entradas_tlb ; i++) 
    {
        //log_info(log_cpu, "FOR estoy en la entrada %d", tlb->entradas[i].numero_pagina);
        
        if(tlb->entradas[i].numero_pagina == numero_pagina && tlb->entradas[i].pid == pcb_recibido->pid){
            return &tlb->entradas[i];
        }
    }
    //log_info(log_cpu, "No encontre, lo mando vacio");

    TLB_Entrada *entrada_vacia = malloc(sizeof(TLB_Entrada));

    
    entrada_vacia->pid = -1;
    entrada_vacia->numero_pagina = -1;
    entrada_vacia->numero_marco = -1;

    return entrada_vacia;
}

void actualizar_tlb(TLB_Entrada* nueva_entrada) 
{
    // cantidad_entradas_tlb = config_cpu->cantidad_entradas_tlb;
    // algoritmo_tlb = config_cpu->algoritmo_tlb;

    log_info(log_cpu, "Actualizando TLB...");

    if(tlb->cantidad_entradas_libres > 0)  {
        // Si hay espacio en la TLB, simplemente agregamos la entrada al final
   
        tlb->entradas[cantidad_entradas_tlb-tlb->cantidad_entradas_libres] = *nueva_entrada;
        
        actualizar_tlb_libre(nueva_entrada->pid, nueva_entrada->numero_pagina);

        log_info(log_cpu, "No tuve que hacer reemplazos en TLB -> habia lugar");
        log_info(log_cpu, "TLB -> [Proceso: %d - Pag: %d - Marco: %d] agregada \n \n \n", tlb->entradas[cantidad_entradas_tlb-tlb->cantidad_entradas_libres].pid, tlb->entradas[cantidad_entradas_tlb-tlb->cantidad_entradas_libres].numero_pagina, tlb->entradas[cantidad_entradas_tlb-tlb->cantidad_entradas_libres].numero_marco);
        
        tlb->cantidad_entradas_libres--;
    } 
    else 
    {
        if (strcmp(algoritmo_tlb, "FIFO") == 0) 
        {
            // Implementación del algoritmo FIFO
            log_info(log_cpu, "TLB -> [Proceso: %d - Pag: %d - Marco: %d] borrada", tlb->entradas[0].pid, tlb->entradas[0].numero_pagina, tlb->entradas[0].numero_marco);

            for (int i = 0; i < cantidad_entradas_tlb - 1; i++) 
            {
                tlb->entradas[i] = tlb->entradas[i + 1];
            }
            tlb->entradas[cantidad_entradas_tlb - 1] = *nueva_entrada;     
            log_info(log_cpu, "TLB -> [Proceso: %d - Pag: %d - Marco: %d] agregada \n \n \n", tlb->entradas[cantidad_entradas_tlb - 1].pid, tlb->entradas[cantidad_entradas_tlb - 1].numero_pagina, tlb->entradas[cantidad_entradas_tlb - 1].numero_marco);
   
        } 
        else if (strcmp(algoritmo_tlb, "LRU") == 0) 
        {
            // Implementación del algoritmo LRU
            int lru_index = 0; // Indice menos usado

            for (int i = 1; i < cantidad_entradas_tlb; i++) 
            {
                if (tlb->uso_lru[i] > tlb->uso_lru[lru_index]) 
                {
                    lru_index = i;
                }
            } 
            
            // Registrar la entrada que se va a reemplazar
            log_info(log_cpu, "TLB -> [Proceso: %d - Pag: %d - Marco: %d] borrada", 
            tlb->entradas[lru_index].pid, 
            tlb->entradas[lru_index].numero_pagina, 
            tlb->entradas[lru_index].numero_marco);
    
            tlb->entradas[lru_index] = *nueva_entrada;

            log_info(log_cpu, "TLB -> [Proceso: %d - Pag: %d - Marco: %d] agregada", 
            tlb->entradas[lru_index].pid, 
            tlb->entradas[lru_index].numero_pagina, 
            tlb->entradas[lru_index].numero_marco);

            for (int i = 0; i < cantidad_entradas_tlb; i++) 
            {
                if (i != lru_index) 
                {
                    tlb->uso_lru[i]++;
                }
            }
            tlb->uso_lru[lru_index] = 0; // La nueva entrada es la más reciente
        } 
        else 
        {
            log_error(log_cpu, "Algoritmo de reemplazo de TLB no soportado: %d", algoritmo_tlb);
        }
    }
    //log_info(log_cpu, "Pude actualizar");
    
    

}

void actualizar_tlb_HIT(int pid, int numero_pagina){

    if (strcmp(algoritmo_tlb, "LRU") == 0) {
        // TENGO QUE ACTUALIZAR EL USO LRU

        log_info(log_cpu, "Actualizando usos de TLB...");
    
        for (int i = 0; i < cantidad_entradas_tlb; i++) {
            
            if (pid == tlb->entradas[i].pid && numero_pagina == tlb->entradas[i].numero_pagina) {
                tlb->uso_lru[i] = 0; // La nueva entrada es la más reciente
            }

            else {
                tlb->uso_lru[i]++;
            }
        }
    } 
}


void actualizar_tlb_libre(int pid, int numero_pagina){

    if (strcmp(algoritmo_tlb, "LRU") == 0) {
        // TENGO QUE ACTUALIZAR EL USO LRU

        log_info(log_cpu, "Actualizando usos de TLB...");
    
        for (int i = 0; i < cantidad_entradas_tlb; i++) {
            
            if (pid == tlb->entradas[i].pid && numero_pagina == tlb->entradas[i].numero_pagina) {
                
                tlb->uso_lru[i] = 0; // La nueva entrada es la más reciente
            }

            else {
                if(tlb->entradas[i].pid != -1){
                    // significa que existe esa entrada, la actualizo
                    tlb->uso_lru[i]++;
                }
            }
        }
    } 
}

void inicializar_tlb() 
{
    cantidad_entradas_tlb = config_cpu->cantidad_entradas_tlb;
    algoritmo_tlb = config_cpu->algoritmo_tlb;

    tlb = malloc(sizeof(TLB));

    // Inicializa la TLB y otros parámetros
    if (tlb == NULL) 
    {
        fprintf(stderr, "No se pudo asignar memoria para la TLB\n");
        exit(EXIT_FAILURE);
    }

    tlb->cantidad_entradas_libres = config_cpu->cantidad_entradas_tlb;
    tlb->entradas = malloc(config_cpu->cantidad_entradas_tlb * sizeof(TLB_Entrada));

    for(int i = 0 ; i < tlb->cantidad_entradas_libres ; i++){
        tlb->entradas[i].pid = -1;
        tlb->entradas[i].numero_pagina = -1;
        tlb->entradas[i].numero_marco = -1;
        
    }


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

// Despues de enviar la dire, kernel recibe la dirección física y envía IO_STDOUT_WRITE a la interfaz I/O.
void enviar_direccion_fisica_a_kernel(int socket_cliente_kernel, int direccion_fisica) // Ver sockets
{
    t_paquete *paquete = crear_paquete_personalizado(DIRECCION_FISICA);
    agregar_int_al_paquete_personalizado(paquete, direccion_fisica);
    enviar_paquete(paquete, socket_cliente_kernel);
    eliminar_paquete(paquete);
}