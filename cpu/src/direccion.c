int traducir_direccion_logica_a_fisica(int direccion_logica)
{
    int tamanio_pagina = config_get_int_value(config_cpu, "TAM_MAX_PAGINACION");
    int numero_pagina = floor(direccion_logica / tamanio_pagina);
    int desplazamiento = direccion_logica - numero_pagina * tamanio_pagina;
    log_info(log_cpu, "Nro pagina: %d | desplazamiento: %d ", numero_pagina, desplazamiento);

    tlb* respuesta = buscar(numero_pagina)

    if(respuesta != NULL)
    {
        log_info(log_cpu, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numero_pagina);
        return (respuesta->marco * tamanio_pagina) + desplazamiento;
    } 
    else 
    {
        log_info(log_cpu, "PID: %d - TLB MISS - Pagina: %d", proceso->pid, numero_pagina);

        // Solicitar marco
        t_paquete* paquete= crear_paquete(MARCO);
        agregar_a_paquete(paquete, &numero_pagina, sizeof(int));
        agregar_a_paquete(paquete, &pid, sizeof(int));
        enviar_paquete(paquete, socket_cliente_cpu);
        
        int cod_op = recibir_operacion(socket_cliente_cpu);
        if(cod_op == MARCO) // Agregar a struct
        {
            int marco;
            // Recibir marco
            // Agregarlo a la tlb

            log_info(log_cpu, "PID: %d - OBTENER MARCO - Página: %d - Marco: %d", proceso->pid, numero_pagina, marco);
            return (marco * tamanio_pagina) + desplazamiento;
        }

        // RECIBIR MARCO
        char *mensaje = recibir_valor(socket_memoria);
        nro_marco = atoi(mensaje);
        free(mensaje);

        if (nro_marco >= 0){
            log_info(logger, "PID: %d -OBTUVE MARCO - Página:< %d  > - Marco: < %d >", pid, numero_pagina, nro_marco);
            direccion_fisica = (nro_marco * tamanio_pagina) + desplazamiento;
        }

    }
}

tlb* buscar(int numero_pagina) 
{
    for(int i = 0; i < list_size(listaTLB); i++) // No entiendo que es la listaTLB
    {
        tlb* numero = list_get(listaTLB, i);
        if (numero->pagina == numero_pagina) 
        {
            return numero;
        }
    }
    return NULL;
}

//----------------------------------------------------------------

#include <stdio.h>
#include <stdint.h>

#define TAMANO_PAGINA 4096 // Tamaño de página en bytes

// Estructura para representar una entrada en la tabla de páginas
typedef struct {
    uint32_t numero_marco;
    // Otros campos necesarios...
} Entrada_Tabla_Paginas;

// Tabla de páginas para un proceso específico
Entrada_Tabla_Paginas tabla_paginas[256]; // Supongamos que hay 256 entradas en la tabla de páginas

// Función para buscar en la tabla de páginas
uint32_t buscar_Tabla_Paginas(uint32_t pid, uint32_t pagina) {
    // Supongamos que aquí se realiza una búsqueda en la memoria principal para encontrar el número de marco correspondiente al número de página
    // Por simplicidad, asumiremos que el número de marco es igual al número de página (es decir, paginación de un solo nivel)
    // En un sistema real, la búsqueda involucraría acceder a la tabla de páginas del proceso pid en la memoria principal
    return pagina;
}

// Función para buscar en la TLB
int buscar_TLB(uint32_t pid, uint32_t pagina) {
    // Supongamos que aquí se realiza una búsqueda en la TLB del proceso pid
    // Esta función debería devolver el índice de la entrada de la TLB si se encuentra, o -1 si no se encuentra (TLB miss)
    // Por simplicidad, asumiremos que siempre es un TLB miss
    return -1;
}

// Función para actualizar la TLB después de un TLB miss
void actualizar_TLB(uint32_t pid, uint32_t pagina, uint32_t marco) {
    // Supongamos que aquí se actualiza la TLB con la nueva entrada
}

// Función para traducir una dirección lógica en una dirección física utilizando la TLB y, si es necesario, consultando la memoria principal
uint32_t traducir_con_TLB(uint32_t pid, uint32_t direccion_logica) {
    uint32_t pagina = direccion_logica / TAMANO_PAGINA;
    int indice = buscar_TLB(pid, pagina);
    if (indice != -1) {
        printf("TLB Hit\n");
        return indice * TAMANO_PAGINA + direccion_logica % TAMANO_PAGINA;
    } else {
        printf("TLB Miss\n");
        // Consulta a la tabla de páginas (memoria principal)
        uint32_t marco = buscar_Tabla_Paginas(pid, pagina);
        actualizar_TLB(pid, pagina, marco);
        return marco * TAMANO_PAGINA + direccion_logica % TAMANO_PAGINA;
    }
}

int main() {
    // Ejemplo de uso de la función traducir_con_TLB
    uint32_t pid = 123; // ID del proceso
    uint32_t direccion_logica = 8192; // Ejemplo de dirección lógica
    uint32_t direccion_fisica = traducir_con_TLB(pid, direccion_logica);
    printf("Dirección física: %u\n", direccion_fisica);
    return 0;
}

//----------------------------------------------------------------

int MMU(int dirLogica) {


    objetoTLB* retornoTLB = buscarEnTLB(numeroPagina);  // Buscar en la TLB
    if (retornoTLB != NULL) {
        // TLB HIT
        log_info(logger, "PID: %d - TLB HIT - Pagina: %d", proceso->pid, numeroPagina);
        return (retornoTLB->numMarco * tamanio_pagina) + desplazamiento;
    } else {
        // TLB MISS
        log_info(logger, "PID: %d - TLB MISS - Pagina: %d", proceso->pid, numeroPagina);
        
        // Mandar solicitud de marco correspondiente a la memoria
        mandar_numero_actualizado(conexion_memoria, numeroPagina, BUSCAR_MARCO);
        
        op_code codOp = recibir_operacion(conexion_memoria);
        if (codOp == NUMERO) {
            int marco = recibir_numero(conexion_memoria);
            
            // Agregar la nueva entrada a la TLB
            if (list_size(listaTLB) < config.cantidad_entradas_tlb) {
                agregar_a_TLB(proceso->pid, numeroPagina, marco);
            } else {
                // Ejecutar el algoritmo de sustitución si la TLB está llena
                algoritmoSustitucion(proceso->pid, numeroPagina, marco);
            }
            
            return (marco * tamanio_pagina) + desplazamiento;
        } else {
            // Manejar el caso de error en la recepción de marco
            log_error(logger, "Error al recibir el marco de memoria");
            return -1;  // o algún valor de error adecuado
        }
    }
}