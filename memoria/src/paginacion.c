#include "paginacion.h"

//******************************************************************
//*************************** PAGINACIÓN ***************************
//************************** FUNCIONES GN **************************

void ocupar_marco(int posicion)
{
    pthread_mutex_lock(&mutex_bitmap_marcos);
    bitarray_set_bit(bitmap_marcos, posicion);
    pthread_mutex_unlock(&mutex_bitmap_marcos);
}

void liberar_marco(int posicion)
{
    pthread_mutex_lock(&mutex_bitmap_marcos);
    bitarray_clean_bit(bitmap_marcos, posicion);
    pthread_mutex_unlock(&mutex_bitmap_marcos);
}

t_pagina *crear_pagina(t_frame *un_frame)
{
    t_pagina *pagina = malloc(sizeof(t_pagina));

    pagina->frame = un_frame->id;

    return pagina;
}

void agregar_pag_a_tabla(t_proceso *proceso, t_pagina *pagina)
{

    list_add(proceso->tabla_paginas, pagina);

    log_info(log_memoria, "Agregue una pagina a la tabla: %d", list_size(proceso->tabla_paginas));
}

bool hay_lugar_en_memoria(int paginasNecesarias)
{
    int cant_marcos_libres = 0;
    int desplazamiento = 0;

    while (desplazamiento < cant_marcos)
    {

        pthread_mutex_lock(&mutex_bitmap_marcos);

        if (bitarray_test_bit(bitmap_marcos, desplazamiento) == 0)
        {
            cant_marcos_libres++;
        }
        pthread_mutex_unlock(&mutex_bitmap_marcos);

        desplazamiento++;
    }

    if (cant_marcos_libres >= paginasNecesarias)
    {
        return 1; // Hay espacio en memoria
    }
    else
    {
        return 0; // No hay espacio en memoria
    }
}

t_list *buscar_marcos_libres()
{
    t_list *marcosLibres = list_create();
    int base = 0;

    pthread_mutex_lock(&mutex_bitmap_marcos);
    while (base < cant_marcos)
    {
        if (bitarray_test_bit(bitmap_marcos, base) == 0)
        {
            // Reviso si los marcos estan en 0 en el bitmap -> estan LIBRES
            log_info(log_memoria, "Frame %d -> libre", base);
            t_frame *unFrame = malloc(sizeof(t_frame));
            unFrame->id = base;
            list_add(marcosLibres, unFrame);
        }
        log_info(log_memoria, "Frame %d -> ocupado", base);

        base++;
    }
    pthread_mutex_unlock(&mutex_bitmap_marcos);

    return marcosLibres;
}

void eliminar_lista(t_list *lista)
{
    list_destroy_and_destroy_elements(lista, (void *)eliminar_algo);
}

void eliminar_algo(void *algo)
{
    free(algo);
}

//******************************************************************
//****************** ACCESO A ESPACIO USUARIO **********************
//*********************** LECTURA EN PAG ***************************
void leer_uint8_en_memoria(int pid, t_list *direcciones_fisicas)
{
    uint8_t valor_leido = 0;
    // como funciona lo dejo asi, pero podria no hacer el void*
    void *valor_leido_puntero = &valor_leido;

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    // log_info(log_memoria, "Cantidad de marcos: %d", cantidad_marcos_por_leer);

    if (cantidad_marcos_por_leer == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    // En realidad solo voy a leer un marco -> lo minimo que puedo leer en memoria es 1B
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, 0);

    // Calculo la posicion en el espacio de usuario
    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(valor_leido_puntero, espacio_usuario + despl_esp_usuario, 1);
    pthread_mutex_unlock(&mutex_espacio_usuario);

    log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
    log_info(log_memoria, "Valor leido: %d ", valor_leido);

    enviar_lectura_1B_a_cpu(pid, dir_actual, valor_leido);
}

void enviar_lectura_1B_a_cpu(int pid, t_direccion_fisica *dir_actual, uint8_t valor)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_1B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint8_al_paquete_personalizado(paquete, valor);

    log_info(log_memoria, "Valor leido: %u", valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

//***********************************************************************************************************************
void leer_uint32_en_memoria(int pid, t_list *direcciones_fisicas)
{
    // Hago una variable en donde voy a guardar el valor leido final
    uint32_t valor_leido;

    // void* valor_leido_puntero = &valor_leido;

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_leer == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para leer.");
        return;
    }

    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, 0);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    // Si solo hay una DF -> solo leo un marco
    if (cantidad_marcos_por_leer == 1)
    {

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(&valor_leido, espacio_usuario + despl_esp_usuario, 4);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

        enviar_lectura_ult_4B_a_cpu(pid, dir_actual, valor_leido, valor_leido);
    }
    else if (cantidad_marcos_por_leer == 2)
    {

        uint16_t parte1, parte2;

        // Voy a leer el primer marco
        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(&parte1, espacio_usuario + despl_esp_usuario, 2);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "DEsplazamiento usuario 1 %d ", despl_esp_usuario);
        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
        log_info(log_memoria, "VAlor leido parte 1 %u ", parte1);

        enviar_lectura_4B_a_cpu(pid, dir_actual, (uint32_t)parte1);

        dir_actual = list_get(direcciones_fisicas, 1);

        despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;
        log_info(log_memoria, "DEsplazamiento usuario 2 %d ", despl_esp_usuario);

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(&parte2, espacio_usuario + despl_esp_usuario, sizeof(uint16_t));
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
        log_info(log_memoria, "VAlor leido parte 2 %u ", parte2);

        // Reconstruir el uint32_t combinando las dos partes
        uint32_t valor_reconstruido = ((uint32_t)parte1 << 16) | (uint32_t)parte2;

        log_info(log_memoria, "VAlor REALLLL %u ", valor_reconstruido);

        enviar_lectura_ult_4B_a_cpu(pid, dir_actual, (uint32_t)parte2, valor_reconstruido);
    }
}

void enviar_lectura_4B_a_cpu(int pid, t_direccion_fisica *dir_actual, uint32_t valor)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_4B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

void enviar_lectura_ult_4B_a_cpu(int pid, t_direccion_fisica *dir_actual, uint32_t valor, uint32_t valor_leido_reconstruido)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_U_4B);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);
    agregar_uint32_al_paquete_personalizado(paquete, valor_leido_reconstruido);

    log_info(log_memoria, "Valor leido: %u", valor);
    log_info(log_memoria, "Valor leido reconstruido: %u", valor_leido_reconstruido);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

//******************************************************************
//********************** ESCRITURA EN PAG **************************
//******************************************************************
// Ante un pedido de escritura, escribir lo indicado a partir de la dirección física pedida.
// En caso satisfactorio se responderá un mensaje de OK.

// Tengo que guardar algo en memoria
// me llega [PID, DF, TAMAÑO, VALOR]
//           int, lista, int, void

void guardar_uint32_en_memoria(int pid, t_list *direcciones_fisicas, uint32_t valor)
{
    // Creo un puntero al dato por guardar, para poder operar
    void *valor_puntero = &valor;

    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0; // lo cambie a cero
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    // Si solo hay una DF -> solo guardo el dato entero en un marco
    if (cantidad_marcos_por_guardar == 1)
    {

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, &valor, 4);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

        // faltaria hacer el caso en el que hay un error y no se manda
        enviar_ult_ok_4B_escritura_cpu(pid, dir_actual, valor, valor);
    }
    else if (cantidad_marcos_por_guardar == 2)
    {
        // Significa que tengo que leer de dos marcos distintos
        log_info(log_memoria, "REGISTRO: %u", valor);

        uint16_t parte1 = (uint16_t)(valor >> 16); // Primeros 16 bits
        log_info(log_memoria, "PARTE 1 %u ", parte1);

        uint16_t parte2 = (uint16_t)(valor & 0xFFFF); // Últimos 16 bits
        log_info(log_memoria, "PARTE 2 %u ", parte2);

        // uint8_t *byte_puntero = (uint8_t*)&valor;

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, &parte1, 2);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "Desplazamiento usuario 1 %d ", despl_esp_usuario);
        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
        log_info(log_memoria, "EScribi en la 1 %u ", parte1);

        enviar_ok_4B_escritura_cpu(pid, dir_actual, valor);
        int bytes_ya_operados = dir_actual->bytes_a_operar;

        dir_actual = list_get(direcciones_fisicas, 1);
        despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;
        log_info(log_memoria, "Desplazamiento usuario 2 %d ", despl_esp_usuario);

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, &parte2, 2);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
        log_info(log_memoria, "EScribi en la 2 %u ", parte2);

        enviar_ult_ok_4B_escritura_cpu(pid, dir_actual, valor, valor);
    }
    else
    {
        log_info(log_memoria, "VICKY NO CONTEMPLO ESTE CASO ");
        log_info(log_memoria, "MATAME ");

        //                  ---------------------------------------   ANTES ------------------------------------------------
        //     int bytes_ya_operados = 0;
        //     //uint32_t valor_escrito ;

        //     // Tengo que particionar el dato y guardarlo
        //    while (cantidad_marcos_por_guardar - 1 > indice_dir){
        //         dir_actual = list_get(direcciones_fisicas, indice_dir);

        //         pthread_mutex_lock(&mutex_espacio_usuario);
        //         memcpy(espacio_usuario + despl_esp_usuario, valor_puntero + bytes_ya_operados, dir_actual->bytes_a_operar);
        //         //memcpy(espacio_usuario + despl_esp_usuario, (uint32_t*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar);
        //         pthread_mutex_unlock(&mutex_espacio_usuario);
        //         log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

        //         log_info(log_memoria, "VAlor escrito: %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

        //         enviar_ok_4B_escritura_cpu(pid, dir_actual, (uint32_t)(valor_puntero + bytes_ya_operados));

        //         bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
        //         indice_dir += 1;

        //     }
        //     if (cantidad_marcos_por_guardar - 1 == indice_dir){
        //         dir_actual = list_get(direcciones_fisicas, indice_dir);

        //         // es la ultima escritura
        //         pthread_mutex_lock(&mutex_espacio_usuario);
        //         memcpy(espacio_usuario + despl_esp_usuario, valor_puntero + bytes_ya_operados, dir_actual->bytes_a_operar);
        //         //memcpy(espacio_usuario + despl_esp_usuario, (uint32_t*)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar);
        //         pthread_mutex_unlock(&mutex_espacio_usuario);

        //         log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco ,dir_actual->offset, dir_actual->bytes_a_operar);

        //         enviar_ult_ok_4B_escritura_cpu(pid, dir_actual,  (uint32_t)(valor_puntero + bytes_ya_operados), valor);

        //     }
    }
}

void enviar_ok_4B_escritura_cpu(int pid, t_direccion_fisica *dir_actual, uint32_t valor)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_OK_4B_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);

    // log_info(log_memoria, "Valor escrito: %u", valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

void enviar_ult_ok_4B_escritura_cpu(int pid, t_direccion_fisica *dir_actual, uint32_t valor, uint32_t valor_completo)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_ULT_OK_4B_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint32_al_paquete_personalizado(paquete, valor);
    agregar_uint32_al_paquete_personalizado(paquete, valor_completo);

    // log_info(log_memoria, "Valor escrito: %u", valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

void guardar_uint8_en_memoria(int pid, t_list *direcciones_fisicas, uint8_t valor)
{
    // podria revisar que este dentro de su espacio -> ver

    log_info(log_memoria, "Entre a guardar_uint8_en_memoria");

    // Creo un puntero al dato por guardar, para poder operar
    void *valor_puntero = &valor;

    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    pthread_mutex_lock(&mutex_espacio_usuario);
    memcpy(espacio_usuario + despl_esp_usuario, valor_puntero, 1);
    pthread_mutex_unlock(&mutex_espacio_usuario);

    log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
    log_info(log_memoria, "VAlor escrito: %u", valor);
    // faltaria hacer el caso en el que hay un error y no se manda
    enviar_ok_1B_escritura_cpu(pid, dir_actual, valor);
}

void enviar_ok_1B_escritura_cpu(int pid, t_direccion_fisica *dir_actual, uint8_t valor)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_OK_1B_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_uint8_al_paquete_personalizado(paquete, valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

//***********************************************************************************************************************
void guardar_string_en_memoria(int pid, t_list *direcciones_fisicas, char *valor, int tamanio)
{

    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    // Si solo hay una DF -> solo guardo en un marco
    if (cantidad_marcos_por_guardar == 1)
    {

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, &valor, tamanio);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

        enviar_ult_ok_string_escritura_cpu(pid, dir_actual, valor); // valor);
    }
    // else
    // {
    //     // Voy a tener que hacer mas de una escritura
    //     int bytes_ya_operados = 0;
    //     char *valor_escrito;

    //     // Tengo que particionar el dato y guardarlo
    //     while (cantidad_marcos_por_guardar - 1 > indice_dir)
    //     {
    //         dir_actual = list_get(direcciones_fisicas, indice_dir);

    //         pthread_mutex_lock(&mutex_espacio_usuario);
    //         memcpy(espacio_usuario + despl_esp_usuario, &valor + bytes_ya_operados, dir_actual->bytes_a_operar);
    //         memcpy(valor_escrito, &valor + bytes_ya_operados, dir_actual->bytes_a_operar);
    //         pthread_mutex_unlock(&mutex_espacio_usuario);

    //         log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
    //         log_info(log_memoria, "VALOR ESCRITO [%s] ", valor_escrito);

    //         enviar_ok_string_escritura_cpu(pid, dir_actual, valor_escrito);

    //         bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
    //         indice_dir += 1;
    //     }
    //     if (cantidad_marcos_por_guardar - 1 == indice_dir)
    //     {
    //         // es la ultima escritura
    //         dir_actual = list_get(direcciones_fisicas, indice_dir);

    //         pthread_mutex_lock(&mutex_espacio_usuario);
    //         memcpy(espacio_usuario + despl_esp_usuario, &valor + bytes_ya_operados, dir_actual->bytes_a_operar);
    //         memcpy(valor_escrito, &valor + bytes_ya_operados, dir_actual->bytes_a_operar);
    //         pthread_mutex_unlock(&mutex_espacio_usuario);

    //         log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
    //         log_info(log_memoria, "VALOR ESCRITO [%s] ", valor_escrito);

    //         enviar_ult_ok_string_escritura_cpu(pid, dir_actual, valor_escrito); // valor);
    //     }
    // }
}

void enviar_ok_string_escritura_cpu(int pid, t_direccion_fisica *dir_actual, char *valor)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_OK_STRING_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_string_al_paquete_personalizado(paquete, valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

void enviar_ult_ok_string_escritura_cpu(int pid, t_direccion_fisica *dir_actual, char *valor) //char *valor_completo)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_ULT_OK_STRING_DE_ESCRITURA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_string_al_paquete_personalizado(paquete, valor);
    //agregar_string_al_paquete_personalizado(paquete, valor_completo);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

void leer_string_en_memoria(int pid, t_list *direcciones_fisicas, int tamanio) {

    char *valor_leido = malloc(tamanio + 1);

    if (valor_leido == NULL){
        log_error(log_memoria, "ERROR: en el malloc.");
        return;
    }

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);
    // segmentation fault aca
    log_info(log_memoria, "Tengo que leer  %d ", cantidad_marcos_por_leer);

    if (cantidad_marcos_por_leer == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        free(valor_leido);
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    // Si solo hay una DF -> solo leo un marco
    if (cantidad_marcos_por_leer == 1)
    {
        log_info(log_memoria, "El desplazamiento de usuario es %s ", despl_esp_usuario);
        memset(valor_leido, 0, tamanio + 1); // Inicializar la memoria asignada a 0

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(valor_leido, espacio_usuario + despl_esp_usuario, tamanio);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);
        log_info(log_memoria, "VALOR LEIDO %s ", valor_leido);

        enviar_lectura_ult_string_a_cpu(pid, dir_actual, valor_leido);
        free(valor_leido);
    }
    // else
    // {
    //     int bytes_ya_operados = 0;
    //     char *valor_leido;
    //     char *valor_leido_reconstruido;

    //     // Voy a leer el dato de a poco
    //     while (cantidad_marcos_por_leer - 1 > indice_dir)
    //     {
    //         dir_actual = list_get(direcciones_fisicas, indice_dir);

    //         pthread_mutex_lock(&mutex_espacio_usuario);
    //         memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
    //         memcpy((char *)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
    //         memcpy((char *)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
    //         pthread_mutex_unlock(&mutex_espacio_usuario);

    //         log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

    //         enviar_lectura_string_a_cpu(pid, dir_actual, valor_leido);

    //         bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
    //         indice_dir += 1;
    //     }
    //     if (cantidad_marcos_por_leer - 1 == indice_dir)
    //     {
    //         // es la ultima lectura

    //         dir_actual = list_get(direcciones_fisicas, indice_dir);
    //         pthread_mutex_lock(&mutex_espacio_usuario);
    //         memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
    //         memcpy((char)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
    //         memcpy((char)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
    //         pthread_mutex_unlock(&mutex_espacio_usuario);

    //         log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

    //         enviar_lectura_ult_string_a_cpu(pid, dir_actual, valor_leido);
    //     }
}

void leer_string_io_en_memoria(int pid, t_list *direcciones_fisicas, int tamanio, int socket)
{

    char *valor_leido = malloc(sizeof(char) * tamanio); // agregue esto

    int cantidad_marcos_por_leer = list_size(direcciones_fisicas);

    log_info(log_memoria, "tengo q leer %d marcos", cantidad_marcos_por_leer);

    if (cantidad_marcos_por_leer == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    // Si solo hay una DF -> solo leo un marco
    if (cantidad_marcos_por_leer == 1)
    {

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(valor_leido, espacio_usuario + despl_esp_usuario, tamanio);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

        enviar_lectura_ult_string_a_io(valor_leido, socket);
    }
    else
    {
        int bytes_ya_operados = 0;
        // char* valor_leido ; //lo comento porq ya esta inicializado arriba
        char *valor_leido_reconstruido;

        // Voy a leer el dato de a poco
        while (cantidad_marcos_por_leer - 1 > indice_dir)
        {
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            pthread_mutex_lock(&mutex_espacio_usuario);

            memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
            memcpy((char *)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
            memcpy((char *)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: <%d> - Accion: <LEER> - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

            // enviar_lectura_string_a_io(pid, dir_actual, valor_leido);

            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;
        }
        if (cantidad_marcos_por_leer - 1 == indice_dir)
        {
            // es la ultima lectura

            dir_actual = list_get(direcciones_fisicas, indice_dir);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(valor_leido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
            memcpy((char)&valor_leido, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
            memcpy((char)&valor_leido_reconstruido + bytes_ya_operados, espacio_usuario + despl_esp_usuario, dir_actual->bytes_a_operar);
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: LEER - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_lectura_ult_string_a_io(valor_leido_reconstruido, socket);
        }
    }
}

void enviar_lectura_string_a_cpu(int pid, t_direccion_fisica *dir_actual, char *valor)
{

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_STRING);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
    agregar_string_al_paquete_personalizado(paquete, valor);

    log_info(log_memoria, "Valor leido: %s", valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

void enviar_lectura_ult_string_a_cpu(int pid, t_direccion_fisica *dir_actual, char *valor){

    t_paquete *paquete = crear_paquete_personalizado(CPU_RECIBE_LECTURA_U_STRING);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_estructura_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));

    agregar_string_al_paquete_personalizado(paquete, valor);

    enviar_paquete(paquete, socket_cliente_cpu);
    eliminar_paquete(paquete);
}

// void enviar_lectura_string_a_io(int pid, t_direccion_fisica* dir_actual, char* valor){

//     // t_paquete* paquete = crear_paquete_personalizado(IO_RECIBE_LECTURA_STRING);

//     // agregar_int_al_paquete_personalizado(paquete, pid);
//     // agregar_lista_al_paquete_personalizado(paquete, dir_actual, sizeof(t_direccion_fisica));
//     // agregar_string_al_paquete_personalizado(paquete, valor);

//     log_info(log_memoria, "Valor leido: %s", valor);

// 	// enviar_paquete(paquete, socket_cliente_io);
// 	// eliminar_paquete(paquete);
// }

void enviar_lectura_ult_string_a_io(char *valor_leido_reconstruido, int socket)
{

    t_paquete *paquete = crear_paquete_personalizado(IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA);

    agregar_string_al_paquete_personalizado(paquete, valor_leido_reconstruido);

    log_info(log_memoria, "Valor leido reconstruido: %s", valor_leido_reconstruido);

    enviar_paquete(paquete, socket);

    eliminar_paquete(paquete);
}

void guardar_string_io_en_memoria(int pid, t_list *direcciones_fisicas, char *valor, int tamanio, int socket)
{

    int cantidad_marcos_por_guardar = list_size(direcciones_fisicas);

    if (cantidad_marcos_por_guardar == 0)
    {
        log_error(log_memoria, "ERROR: no hay direcciones físicas para escribir.");
        return;
    }

    int indice_dir = 0;
    t_direccion_fisica *dir_actual = list_get(direcciones_fisicas, indice_dir);

    int despl_esp_usuario = dir_actual->nro_marco * config_memoria->tam_pagina + dir_actual->offset;

    // Si solo hay una DF -> solo guardo en un marco
    if (cantidad_marcos_por_guardar == 1)
    {

        pthread_mutex_lock(&mutex_espacio_usuario);
        memcpy(espacio_usuario + despl_esp_usuario, valor, tamanio);
        pthread_mutex_unlock(&mutex_espacio_usuario);

        log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

        enviar_ult_ok_string_escritura_io(socket); // esto va aunque estemos en un pedido de io?
    }
    else
    {
        int bytes_ya_operados = 0;
        char *valor_escrito;

        // Tengo que particionar el dato y guardarlo
        while (cantidad_marcos_por_guardar - 1 > indice_dir)
        {
            dir_actual = list_get(direcciones_fisicas, indice_dir);

            log_info(log_memoria, "esto es nro marco de dir_actual: %d", dir_actual->nro_marco);
            log_info(log_memoria, "esto es bytes a operar de dir_actual: %d", dir_actual->bytes_a_operar);
            pthread_mutex_lock(&mutex_espacio_usuario);
            memcpy(espacio_usuario + despl_esp_usuario, valor + bytes_ya_operados, dir_actual->bytes_a_operar);
            memcpy(espacio_usuario + despl_esp_usuario, (char *)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar);
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

            // enviar_ok_string_escritura_io(pid, dir_actual, valor_escrito);
            enviar_ult_ok_string_escritura_io(socket);
            bytes_ya_operados = bytes_ya_operados + dir_actual->bytes_a_operar;
            indice_dir += 1;
        }
        if (cantidad_marcos_por_guardar - 1 == indice_dir)
        {
            // es la ultima escritura
            dir_actual = list_get(direcciones_fisicas, indice_dir);
            log_info(log_memoria, "esto es nro marco de dir_actual: %d", dir_actual->nro_marco);
            log_info(log_memoria, "esto es bytes a operar de dir_actual: %d", dir_actual->bytes_a_operar);

            pthread_mutex_lock(&mutex_espacio_usuario);
            // Segm fault aca: POSIBLE RAZON bytes_a_esperar es muy grande y se pasa de espacio usuario?
            memcpy(espacio_usuario + despl_esp_usuario, valor + bytes_ya_operados, dir_actual->bytes_a_operar);
            memcpy(espacio_usuario + despl_esp_usuario, (char *)&valor_escrito + bytes_ya_operados, dir_actual->bytes_a_operar);
            pthread_mutex_unlock(&mutex_espacio_usuario);

            log_info(log_memoria, "PID: %d - Accion: ESCRIBIR - Direccion fisica: [%d - %d] - Tamaño %d ", pid, dir_actual->nro_marco, dir_actual->offset, dir_actual->bytes_a_operar);

            enviar_ult_ok_string_escritura_io(socket);
        }
    }
}

void enviar_ult_ok_string_escritura_io(int socket)
{

    t_paquete *paquete = crear_paquete_personalizado(IO_RECIBE_RESPUESTA_DE_ESCRITURA_DE_MEMORIA);
    // falta aca agregar el valor completo

    enviar_paquete(paquete, socket);
    eliminar_paquete(paquete);
}