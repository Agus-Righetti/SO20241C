#include "interfazDIALFS.h"

// IO_FS_CREATE -> Para crear un archivo

void manejar_creacion_archivo(char* nombre_archivo, int pid) 
{
    // Tenemos que armar el metadata del archivo y cambiar los bloques libres del bitarray y bloquesdat

    FILE* bitmap_file = fopen("bitarray.dat", "rb"); // Abro el "bitarray.dat"

    char *bitmap_buffer = (char*)malloc(bitarray_size);

    fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
    
    fclose(bitmap_file);

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, LSB_FIRST);

    int index_primer_bloque_libre = bitarray_find_first_clear_bit(bitmap);

    if(index_primer_bloque_libre == -1)
    {
        log_error(log_io, "No hay bloques libres para la creacion de un nuevo archivo");
    }
    
    bitarray_set_bit(bitmap, index_primer_bloque_libre);

    bitmap_file = fopen("bitarray.dat", "wb");

    fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);
    
    bitarray_destroy(bitmap);
    free(bitmap_buffer);
    
    // Armamos la metadata del archivo-------------------------------------

    strcat(nombre_archivo, ".config");

    char *config_file_path = nombre_archivo;

    // Crear la configuración desde el archivo
    t_config *config = config_create(config_file_path);
    if (config == NULL) {
        fprintf(stderr, "Error: No se pudo cargar el archivo de configuración %s\n", config_file_path);
        return;
    }

    char* bloque_inicial = pasar_a_string(index_primer_bloque_libre);
    
    config_set_value(config, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config, "TAMANIO_ARCHIVO", "0");

    // Guardar los cambios en el archivo
    config_save(config);
     
    queue_push(cola_archivos_en_fs, nombre_archivo);

    // Liberar memoria utilizada por la configuración
    config_destroy(config);

    avisar_fin_io_a_kernel();
    
    return;
}

// IO_FS_DELETE -> Para eliminar archivo
void manejar_eliminacion_archivo(char* nombre_archivo, int pid) 
{
    strcat(nombre_archivo, ".config");

    char *config_file_path = nombre_archivo;


    t_config* config_aux = config_create(config_file_path);
    
    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
    int tamanio_en_bytes = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

    config_destroy(config_aux);

    remove(nombre_archivo); // Elimino el archivo de metadata
    
    // Creamos un bitmap a partir del archivo y lo editamos

    int bloques_ocupados = calcular_bloques_que_ocupa(tamanio_en_bytes);

    char *bitmap_buffer = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, LSB_FIRST);

    for(bloque_inicial; bloque_inicial< bloques_ocupados; bloque_inicial ++) 
    {
        bitarray_clean_bit(bitmap, bloque_inicial); //pongo en cero los bloques ocupados
    }
    
    free(bitmap_buffer);
    
    escribir_archivo_con_bitmap(bitmap); // Actualizamos el archivo de bitmap

    bitarray_destroy(bitmap);

    // no borramos los bloques del bloques.dat porque al marcar como libres los bloques en el bitmap
    // se va a sobreescribir en esos bloques y listo

    avisar_fin_io_a_kernel();

    return;
}

void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, int pid)
{
    strcat(nombre_archivo, ".config");

    char *config_file_path = nombre_archivo;

    FILE* config_archivo = fopen(config_file_path, "r+");

    t_config* config_aux = config_create(config_file_path);

    t_metadata* metadata = malloc(sizeof(t_metadata));

    char* buffer_bitmap = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, LSB_FIRST);

    char* buffer;

    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
    int tamanio_original = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

    fclose(config_archivo);

    if(tamanio_original < nuevo_tamanio) // Vamos a tener que bsucar que haya bloques libres para agrandar el tamanio
    {
        int bytes_a_agregar = nuevo_tamanio - tamanio_original;

        int resto = tamanio_original % config_io->block_size;

        if(resto > 0) // si los bloques ocupados no estan ocupados en su totalidad
        {
            bytes_a_agregar = bytes_a_agregar - (config_io->block_size - resto);
        }
        
        int bloques_necesarios = calcular_bloques_que_ocupa(bytes_a_agregar);
        
        //verificamos que haya la cantidad de bloques libres en total q necesitamos

        int cantidad_bloques_libres = contar_bloques_libres(bitmap);
        
        if(cantidad_bloques_libres < bloques_necesarios)
        {
            log_error(log_io, "No se puede realizar la opeacion TRUNCATE, no se posee el espacio necesario");
            return;
        }

        int bloque_inicial_nuevo = buscar_bloques_contiguos_desde_cierto_bloque(bloque_inicial, bloques_necesarios, bitmap);

        if(bloque_inicial_nuevo == -1) // Devuelve -1, no hay libres desde ese bloque
        {
            bloques_necesarios = calcular_bloques_que_ocupa(nuevo_tamanio); 
            bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);
            
            if(bloque_inicial_nuevo == -1) //No hay bloques contiguos, necesito compactar si entra
            {
                //Primero tenemos q sacar los bloques ocupados y ppner la info en un buffer
                //dsp compacto y pongo lo q habia sacado actuizando el bitmap
                
                int bloques_que_ocupa_original = calcular_bloques_que_ocupa(tamanio_original);
                
                buffer = leer_bloques(bloque_inicial, bloques_que_ocupa_original); // Este buffer tiene lo que ya venía en el archivo

                //ahora marco en el bitmap esos bloques como libres
                
                for(int i = bloque_inicial; i < bloques_que_ocupa_original; i++)
                {
                    bitarray_clean_bit(bitmap, i);
                }

                compactar(bitmap); // compactar actualiza el bloques.dat y el bitmap
                
                // tenemos q agregar al final lo q esta en el buffer

                bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);

                agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

                //tenemos q actualizar el config del archivo con el tamano y bloque inicial
                metadata->nombre_archivo = nombre_archivo;
                metadata->bloque_inicial = bloque_inicial_nuevo;
                metadata->tamanio_archivo = nuevo_tamanio;

                actualizar_metadata(metadata);
                

            }else{//hay bloques contiguos, tengo q ponerlo en esos bloques

                int bloques_del_tamanio_original = calcular_bloques_que_ocupa(tamanio_original);

                buffer = leer_bloques(bloque_inicial, bloques_del_tamanio_original);

                for(int i = bloque_inicial; i < bloques_del_tamanio_original; i++)
                {
                    bitarray_clean_bit(bitmap, i);
                }
                
                agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

                //marco los nuevos bits como ocupados
                for(int i = bloque_inicial_nuevo; i < bloques_necesarios; i++)
                {
                    bitarray_set_bit(bitmap, i);
                }
                
                //guardo la metadata actualidad
                metadata->tamanio_archivo = nuevo_tamanio;
                metadata->nombre_archivo = nombre_archivo;
                metadata->bloque_inicial = bloque_inicial_nuevo;

                //actualizo los dos archivos
                escribir_archivo_con_bitmap(bitmap);
                actualizar_metadata(metadata);
            }
        }else{

            //tenemos en bloque_inicial el bloque inicial
            metadata->tamanio_archivo = nuevo_tamanio;
            metadata->nombre_archivo = nombre_archivo;
            metadata->bloque_inicial = bloque_inicial;

            //falta actualizar el bitmap

            bloques_necesarios = calcular_bloques_que_ocupa(nuevo_tamanio);

            for(int i = bloque_inicial; i < bloques_necesarios; i++)
            {
                bitarray_set_bit(bitmap,i);
            }

            escribir_archivo_con_bitmap(bitmap);
            actualizar_metadata(metadata);
        }

    }else if(tamanio_original > nuevo_tamanio) //solo tengo que truncar el archivo, marco el bitmap como libre
    { 
        int bloques_a_liberar = calcular_bloques_que_ocupa(tamanio_original) - calcular_bloques_que_ocupa(nuevo_tamanio);
        

        for(int i = bloque_inicial; i < bloques_a_liberar ; i++ )
        {
            bitarray_clean_bit(bitmap, i);
        }

        metadata->tamanio_archivo = nuevo_tamanio;
        metadata->nombre_archivo = nombre_archivo;
        metadata->bloque_inicial = bloque_inicial;

        escribir_archivo_con_bitmap(bitmap);
        actualizar_metadata(metadata);
    }

    config_destroy(config_aux);
    free(buffer_bitmap);
    bitarray_destroy(bitmap);

    avisar_fin_io_a_kernel();

    return;
}   

void manejar_escritura_archivo(char* nombre_archivo, t_list* direccion_fisica, int tamanio, int puntero_archivo, int pid)
{
    
    
    return;
   
}

void manejar_lectura_archivo(char* nombre_archivo, t_list* direccion_fisica, int tamanio, int puntero_archivo, Interfaz* configuracion_fs)
{
    return;
}

void crear_archivos_gestion_fs()
{
    // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bloques.dat

    FILE* bloques_dat = fopen("bloques.dat", "wb");

    if (bloques_dat == NULL) {
        perror("Error al abrir el archivo");
        return;
    }

    int tamanio = config_io->block_size * config_io->block_count;

    // Mueve el puntero al final del archivo para definir su tamaño
    if (fseek(bloques_dat, tamanio - 1, SEEK_SET) != 0) {
        perror("Error al establecer el tamaño del archivo");
        fclose(bloques_dat);
        return;
    }

    // Escribe un byte en la última posición para establecer el tamaño
    fputc('\0', bloques_dat);

    fclose(bloques_dat);
   

    // CREO EL ARCHIVO CON EL BITMAP--------------------------------------------------------

    FILE *bitmap_file = fopen("bitmap.dat", "wb");  // Abre el archivo en modo lectura y escritura binaria
    if (bitmap_file == NULL) 
    {
        perror("Error al abrir bitmap.dat");
        return;
    }

    // 1. Calcular el tamaño del bitmap en bytes
    size_t bitmap_size_bytes = (config_io->block_count + 7) / 8;  // 1 bit por bloque

    // 2. Reservar memoria para el bitmap en un buffer
    char *bitmap_buffer = (char*)malloc(bitmap_size_bytes);
    memset(bitmap_buffer, 0, bitmap_size_bytes);
    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, LSB_FIRST);

    if (bitmap_buffer == NULL) {
        perror("Error al reservar memoria para el bitmap");
        fclose(bitmap_file);
        return;
    }

    // 3. Inicializar el bitmap en 0 (todos los bloques libres)

    // 4. Escribir el bitmap inicializado de vuelta al archivo
    fseek(bitmap_file, 0, SEEK_SET);  // Mueve el puntero al inicio del archivo
    fwrite(bitmap_buffer, sizeof(char), bitmap_size_bytes, bitmap_file);

    // 5. Crear el bitarray utilizando la función proporcionada
    if (bitmap == NULL) {
        fprintf(stderr, "Error al crear el bitmap\n");
        free(bitmap_buffer);
        fclose(bitmap_file);
        return;
    }

    free(bitmap_buffer);
    fclose(bitmap_file);
    bitarray_destroy(bitmap);
    
    // bitarray_set_bit(bitmap, 0);  // Por ejemplo, establece el primer bloque como ocupado
    // bitarray_set_bit(bitmap, 1);  // Y el segundo bloque como ocupado

    // bitarray_clean_bit(bitmap, 0) // Pone el bit número 0 libre
}

char* pasar_a_string(int valor)
{
    static char buffer[20]; // Asegúrate de que el tamaño sea suficiente
    snprintf(buffer, sizeof(buffer), "%d", valor);
    return buffer;
}

int calcular_bloques_que_ocupa(int bytes)
{
    int bloques_que_ocupa = (bytes + config_io->block_size - 1) / config_io->block_size;
    
    return bloques_que_ocupa;
}

char* obtener_bitmap(){

    FILE* bitmap_file = fopen("bitarray.dat", "rb"); 

    //bitarray_size = (config_io->block_count + 7) / 8;

    char *bitmap_buffer = (char *)malloc(bitarray_size);

    fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
    
    fclose(bitmap_file);

    return bitmap_buffer;
}

void escribir_archivo_con_bitmap(t_bitarray* bitmap){

    FILE* bitmap_file = fopen("bitarray.dat", "wb"); 
    
    fwrite(bitmap, 1, bitarray_size, bitmap_file);

    fclose(bitmap_file);
    //bitarray_destroy(bitmap);

}

int buscar_bloques_contiguos_desde_cierto_bloque(int bloque_inicial, int bloques_necesarios, t_bitarray* bitmap) {
    
    int cont_bloques_libres_consecutivos = 0;

    for (int i = bloque_inicial; i < config_io->block_count; i++) {

        if (!bitarray_test_bit(bitmap, i)) { // Si el bloque i está libre (bit es 0)

            if (cont_bloques_libres_consecutivos == 0) { // Veo si es el primer bloque 

                if (i != bloque_inicial) {
                    // Si hemos encontrado bloques libres consecutivos, pero no empiezan desde bloque_inicial, reiniciamos
                    return -1;
                }
            }

            cont_bloques_libres_consecutivos++; // Incrementa el cont de bloques libres consecutivos

            if (cont_bloques_libres_consecutivos == bloques_necesarios) {

                return bloque_inicial; // Retorna bloque_inicial si se encontraron suficientes bloques libres consecutivos
            }
        } else {
            cont_bloques_libres_consecutivos = 0; // Reinicia el cont si se encuentra un bloque ocupado
        }
    }

    return -1; // Retorna -1 si no se encuentran suficientes bloques libres contiguos
}

int buscar_bloques_contiguos(int bloques_necesarios, t_bitarray* bitmap){
    
    int contador_bloques_contiguos = 0;
    int bloque_inicio = -1; // Inicializa bloque_inicio

    for (int i = 0; i < config_io->block_count; i++) {
        
        if (!bitarray_test_bit(bitmap, i)) { // Bloque libre
        
            if (contador_bloques_contiguos == 0) {
                bloque_inicio = i;
            }

            contador_bloques_contiguos++;
            if (contador_bloques_contiguos == bloques_necesarios){
            
                return bloque_inicio;
            }
        } else {
            contador_bloques_contiguos = 0;
            bloque_inicio = -1;
        }
    }

    return -1; // No hay suficientes bloques contiguos disponibles 
}

int contar_bloques_libres(t_bitarray* bitmap){
    
    int bloques_libres = 0;

    for (int i = 0; i < config_io->block_count; i++)
    {
        if (!bitarray_test_bit(bitmap, i)) { // Si el bit es 0, el bloque está libre,
            bloques_libres++;
        }
    }
    
    return bloques_libres;
}

char* leer_bloques(int bloque_inicial, int num_bloques) {
    
    FILE* archivo = fopen ("bloques.dat", "r+");
    
    // Calcular el offset al inicio del primer bloque
    int offset = bloque_inicial * config_io->block_size;

    // Mover el puntero al inicio del primer bloque
    fseek(archivo, offset, SEEK_SET);

    // Leer el contenido de los bloques
    char *buffer = malloc(num_bloques * config_io->block_size);

    fread(buffer, config_io->block_size, num_bloques, archivo);
    fclose(archivo);

    return buffer;
}

char *agregar_al_final(char *buffer, const char *informacion) {
    if (buffer == NULL) {
        // Si el buffer es NULL, asigna memoria suficiente para la información
        buffer = malloc(strlen(informacion) + 1);  // +1 para el terminador nulo
        
        strcpy(buffer, informacion);
    } else {
        // Si el buffer ya contiene datos, realloca memoria para incluir la nueva información
        size_t tam_buffer = strlen(buffer);
        size_t tam_informacion = strlen(informacion);
        buffer = realloc(buffer, tam_buffer + tam_informacion + 1);  // +1 para el terminador nulo
        if (buffer == NULL) {
            perror("Error al realocar memoria");
            return NULL;
        }
        strcat(buffer, informacion);
    }
    return buffer;
}

// Función para compactar bloques
void compactar(t_bitarray* bitmap) 
{
    
    char* buffer;
    int contador_ocupados = 0;
    t_metadata* metadata = malloc(sizeof(t_metadata)); //metadata->nombre_archivo, ->BLOQUE_INICIAL, ->TAMANIO_ARCHIVO
    int bloques_ocupados;
    //i recorre cada bloque del bitmap i == un numero de bloque

    for (int i = 0; i < config_io->block_count; i++) {
        
        if(bitarray_test_bit(bitmap, i))
        {
            metadata = buscar_archivo_que_inicia_en_bloque(i);
            bloques_ocupados = calcular_bloques_que_ocupa(metadata->tamanio_archivo);
            buffer = agregar_al_final(buffer , leer_bloques(i , bloques_ocupados));
            metadata->bloque_inicial = contador_ocupados;
            actualizar_metadata(metadata);
            bitarray_set_bit(bitmap,contador_ocupados);
            bitarray_clean_bit(bitmap, i);
            contador_ocupados ++;
            i += bloques_ocupados - 1;
        }
        
    }
    
    //falta copiar la info en el archivo bloques.dat

    actualizar_archivo_bloques(buffer);

    free(metadata);

}

void actualizar_archivo_bloques(char* buffer)
{
    FILE * archivo_bloques = fopen("bloques.dat", "rb+");

    fwrite(buffer, config_io->block_count * config_io->block_size, 1, archivo_bloques);

    fclose(archivo_bloques);

    free(buffer);
}

t_metadata* buscar_archivo_que_inicia_en_bloque(int nro_bloque)
{
    t_metadata* meta_aux = malloc(sizeof(t_metadata));
    meta_aux->nombre_archivo = queue_pop(cola_archivos_en_fs);

    t_config* config_aux = config_create(meta_aux->nombre_archivo);

    meta_aux->bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");

    meta_aux->tamanio_archivo = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

    while(meta_aux->bloque_inicial != nro_bloque)
    {
        //borro todo lo anterior para poner lo nuevo
        queue_push(cola_archivos_en_fs, meta_aux->nombre_archivo);
        config_destroy(config_aux);
        //vuelvo a buscar en el siguiente archivo
        meta_aux->nombre_archivo = queue_pop(cola_archivos_en_fs);


        config_aux = config_create(meta_aux->nombre_archivo);

        meta_aux->bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");

        meta_aux->tamanio_archivo = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");


    }

    log_info(log_io, "Encontre la metadata que queria :)");

    queue_push(cola_archivos_en_fs, meta_aux->nombre_archivo); //vuelvo a agregar el archivo a la cola




    return meta_aux;
}

void actualizar_metadata(t_metadata* metadata)
{
    //FILE* archivo = fopen(metadata->nombre_archivo ,"r+");
    t_config* config_archivo = config_create(metadata->nombre_archivo);

    char* bloque_inicial = pasar_a_string(metadata->bloque_inicial);
    char* tamanio_bloque = pasar_a_string(metadata->tamanio_archivo);

    config_set_value(config_archivo, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config_archivo, "TAMANIO_BLOQUE", tamanio_bloque);

    config_save(config_archivo);

    //fclose(archivo);

    free(metadata);

}

int agregar_info_en_cierto_bloque(int bloque_inicial_nuevo, int cant_bloques , char* buffer)
{
    FILE * archivo_bloques = fopen("bloques.dat", "rb+");

    int offset = bloque_inicial_nuevo * config_io->block_size;

    fseek(archivo_bloques, offset, SEEK_SET);

    fwrite(buffer, config_io->block_size, cant_bloques, archivo_bloques);

    fclose(archivo_bloques);

    free(buffer);

    return 1;
}

int bitarray_find_first_clear_bit(t_bitarray* bitmap){

    for (int i = 0; i < config_io->block_count; i++){

        if (!bitarray_test_bit(bitmap, i)) // Si está vacío el bitmap, entonces devuelvo el índice en el que me encuentro
        {
            return i;
        }
    }
    
    return -1;
}