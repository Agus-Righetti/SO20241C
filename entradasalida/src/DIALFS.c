t_bitarray* bitmap;
char* bitmap_buffer;

void crear_archivos_gestion_fs() 
{
    // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bloques.dat
    
    FILE* bloques_dat = fopen("bloques.dat", "rb+");
    
    if (bloques_dat == NULL) 
    {
        // Si el archivo no existe, lo creamos y lo inicializamos
        bloques_dat = fopen("bloques.dat", "wb+");

        if (bloques_dat == NULL) 
        {
            perror("Error al crear bloques.dat");
            return;
        }

        int tamanio = config_io->block_size * config_io->block_count;

        // Mueve el puntero al final del archivo para definir su tamaño
        if (fseek(bloques_dat, tamanio - 1, SEEK_SET) != 0) 
        {
            perror("Error al establecer el tamaño del archivo bloques.dat");
            fclose(bloques_dat);
            return;
        }

        // Escribe un byte en la última posición para establecer el tamaño
        fputc('\0', bloques_dat);
    }

    fclose(bloques_dat);

    // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bitmap.dat

    FILE *bitmap_file = fopen("bitmap.dat", "rb+");  // Abre el archivo en modo lectura y escritura binaria
    
    if (bitmap_file == NULL) 
    {
        // Si el archivo no existe, lo creamos y lo inicializamos
        bitmap_file = fopen("bitmap.dat", "wb+");
        
        if (bitmap_file == NULL) 
        {
            perror("Error al crear bitmap.dat");
            return;
        }

        // 1. Calcular el tamaño del bitmap en bytes
        size_t bitmap_size_bytes = (config_io->block_count + 7) / 8;  // 1 bit por bloque

        // 2. Reservar memoria para el bitmap en un buffer
        char* bitmap_buffer = (char*)malloc(bitmap_size_bytes);
        
        if (bitmap_buffer == NULL) 
        {
            perror("Error al reservar memoria para el bitmap");
            fclose(bitmap_file);
            return;
        }

        // 3. Inicializar el bitmap en 0 (todos los bloques libres)
        memset(bitmap_buffer, 0, bitmap_size_bytes);
        t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, LSB_FIRST);

        // 4. Escribir el bitmap inicializado de vuelta al archivo
        fseek(bitmap_file, 0, SEEK_SET);  // Mueve el puntero al inicio del archivo
        fwrite(bitmap_buffer, sizeof(char), bitmap_size_bytes, bitmap_file);

        // 5. Destruir el bitmap y liberar la memoria
        bitarray_destroy(bitmap);
        free(bitmap_buffer);
    }
    fclose(bitmap_file);
}

void manejar_creacion_archivo(char* nombre_archivo, int pid) 
{
    // Abro el "bitmap.dat"
    FILE* bitmap_file = fopen("bitmap.dat", "rb+");
    if (bitmap_file == NULL) {
        perror("Error al abrir bitmap.dat");
        return;
    }

    size_t bitmap_size_bytes = (config_io->block_count + 7) / 8;
    char* bitmap_buffer = (char*)malloc(bitmap_size_bytes);
    if (bitmap_buffer == NULL) {
        perror("Error al reservar memoria para el bitmap");
        fclose(bitmap_file);
        return;
    }

    fread(bitmap_buffer, 1, bitmap_size_bytes, bitmap_file);
    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, LSB_FIRST);
    int index_primer_bloque_libre = bitarray_find_first_clear_bit(bitmap);

    if (index_primer_bloque_libre == -1) 
    {
        log_error(log_io, "No hay bloques libres para la creacion de un nuevo archivo");
        bitarray_destroy(bitmap);
        free(bitmap_buffer);
        fclose(bitmap_file);
        return;
    }

    bitarray_set_bit(bitmap, index_primer_bloque_libre);
    fseek(bitmap_file, 0, SEEK_SET);
    fwrite(bitmap_buffer, 1, bitmap_size_bytes, bitmap_file);

    bitarray_destroy(bitmap);
    free(bitmap_buffer);
    fclose(bitmap_file);
}

void manejar_creacion_archivo(char* nombre_archivo, int pid) 
{
    // Para armar el metadata del archivo debo verificar si hay espacio en el bitmap

    FILE* bitmap_file = fopen("bitmap.dat", "rb");
    char *bitmap_buffer = (char*)malloc(bitarray_size);
    fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, LSB_FIRST);
    int index_primer_bloque_libre = bitarray_find_first_clear_bit(bitmap);

    if (index_primer_bloque_libre == -1) 
    {
        log_error(log_io, "No hay bloques libres para la creacion de un nuevo archivo");
        bitarray_destroy(bitmap);
        free(bitmap_buffer);
        return;
    }

    bitarray_set_bit(bitmap, index_primer_bloque_libre);

    bitmap_file = fopen("bitmap.dat", "wb");

    if (bitmap_file == NULL) 
    {
        log_error(log_io, "Error al abrir el archivo bitmap.dat para escritura");
        bitarray_destroy(bitmap);
        free(bitmap_buffer);
        return;
    }

    fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);

    bitarray_destroy(bitmap);
    free(bitmap_buffer);

    // Armamos la metadata del archivo-----------------------------------------------

    strcat(nombre_archivo, ".config");
    char *config_file_path = nombre_archivo;

    // Crear la configuración desde el archivo
    t_config *config = config_create(config_file_path);
    if (config == NULL) 
    {
        fprintf(stderr, "Error: No se pudo cargar el archivo de configuración %s\n", config_file_path);
        return;
    }

    if(index_primer_bloque_libre == -1)
    {
        log_error(log_io, "Error: No hay bloques libres");
        return;
    }   

    char* bloque_inicial = pasar_a_string(index_primer_bloque_libre);

    config_set_value(config, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config, "TAMANIO_ARCHIVO", "0");

    // Guardar los cambios en el archivo
    config_save(config);
    queue_push(cola_archivos_en_fs, strdup(nombre_archivo));
    
    // Liberar memoria utilizada por la configuración
    config_destroy(config);

    avisar_fin_io_a_kernel();
    return;
}

char* pasar_a_string(int valor)
{
    static char buffer[20]; // Asegúrate de que el tamaño sea suficiente
    snprintf(buffer, sizeof(buffer), "%d", valor);
    return buffer;
}

int bitarray_find_first_clear_bit(t_bitarray* bitmap)
{
    for (int i = 0; i < config_io->block_count; i++)
    {
        // bitarray_test_bit(bitmap, i) --> Si retorna 0, quiere decir que el bloque esta libre
        // !bitarray_test_bit(bitmap, i) --> 0 en C es FALSO, !0 es VERDADERO, entonces queremos que entre en el if para que 
        // retorne el bloque que esta libre

        if (!bitarray_test_bit(bitmap, i)) // Si está vacío el bitmap, entonces devuelvo el índice en el que me encuentro
        {
            return i;
        }
    }
    return -1;
}
