#include "interfazDIALFS.h"

void leer_configuracion_dialfs(Interfaz *configuracion)
{
    iniciar_config_dialfs(configuracion);

    // Loggeamos el valor de config
    log_info(log_io, "Lei el TIPO_INTERFAZ %s, el TIEMPO_UNIDAD_TRABAJO %d, el IP_KERNEL %s, el PUERTO_KERNEL %d, el IP_MEMORIA %s, el PUERTO_MEMORIA %d, el PATH_BASE_DIALFS %s, el BLOCK_SIZE %d, el BLOCK_COUNT %d y el RETRASO_COMPACTACION %d.", 
             configuracion->archivo->tipo_interfaz, 
             configuracion->archivo->tiempo_unidad_trabajo,
             configuracion->archivo->ip_kernel, 
             configuracion->archivo->puerto_kernel, 
             configuracion->archivo->ip_memoria,
             configuracion->archivo->puerto_memoria,
             configuracion->archivo->path_base_dialfs,
             configuracion->archivo->block_size,
             configuracion->archivo->block_count,
             configuracion->archivo->retraso_compactacion);
}

void iniciar_config_dialfs(Interfaz *configuracion)
{   
    if (configuracion == NULL) 
    {
        printf("El puntero de configuración es NULL\n");
        exit(2);
    }

    // Asignar memoria para configuracion->archivo
    configuracion->archivo = malloc(sizeof(io_config));
    if (configuracion->archivo == NULL) 
    {
        printf("No se puede crear la config archivo\n");
        exit(2);
    }

    // Inicializa la estructura del archivo de configuración desde el archivo de configuración
    t_config* config = config_create("./io.config");
    if (config == NULL) 
    {
        printf("No se puede leer el archivo de config\n");
        free(configuracion->archivo);
        exit(2);
    }
    
    configuracion->archivo->tipo_interfaz = strdup(config_get_string_value(config, "TIPO_INTERFAZ"));
    configuracion->archivo->tiempo_unidad_trabajo = config_get_int_value(config, "TIEMPO_UNIDAD_TRABAJO");
    configuracion->archivo->ip_kernel = strdup(config_get_string_value(config, "IP_KERNEL"));
    configuracion->archivo->puerto_kernel = config_get_int_value(config, "PUERTO_KERNEL");
    configuracion->archivo->ip_memoria = strdup(config_get_string_value(config, "IP_MEMORIA"));
    configuracion->archivo->puerto_memoria = config_get_int_value(config, "PUERTO_MEMORIA");
    configuracion->archivo->path_base_dialfs = strdup(config_get_string_value(config, "PATH_BASE_DIALFS"));
    configuracion->archivo->block_size = config_get_int_value(config, "BLOCK_SIZE");
    configuracion->archivo->block_count = config_get_int_value(config, "BLOCK_COUNT");
    configuracion->archivo->retraso_compactacion = config_get_int_value(config, "RETRASO_COMPACTACION");

    // Liberar el t_config
    config_destroy(config);
}

void liberar_configuracion_dialfs(Interfaz* configuracion)
{
    if(configuracion) 
    {
        free(configuracion->archivo->tipo_interfaz);
        // free(configuracion->archivo->tiempo_unidad_trabajo);
        free(configuracion->archivo->ip_kernel);
        // free(configuracion->archivo->puerto_kernel);
        free(configuracion->archivo->ip_memoria);
        // free(configuracion->archivo->puerto_memoria);
        free(configuracion->archivo->path_base_dialfs);
        // free(configuracion->archivo->block_size);
        // free(configuracion->archivo->block_count);
        // free(configuracion->archivo->retraso_compactacion);
        free(configuracion->archivo);
    }
}

void recibir_operacion_dialfs_de_kernel(Interfaz* configuracion_fs, op_code codigo) 
{
    switch (codigo) 
    {
        t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_kernel);
        char* nombre_archivo = recibir_string_del_buffer(buffer);

        case IO_FS_CREATE:
            manejar_creacion_archivo(nombre_archivo, configuracion_fs);
            break;
        case IO_FS_DELETE:
            manejar_eliminacion_archivo(nombre_archivo, configuracion_fs);
            break;
        case IO_FS_TRUNCATE:
            int nuevo_tamanio = recibir_int_del_buffer(buffer);
            manejar_truncado_archivo(nombre_archivo, nuevo_tamanio, configuracion_fs);
            break;
        case IO_FS_WRITE:
            int direccion = recibir_int_del_buffer(buffer);
            int tamanio = recibir_int_del_buffer(buffer);
            int puntero_archivo = recibir_int_del_buffer(buffer);
            manejar_escritura_archivo(nombre_archivo, direccion, tamanio, puntero_archivo, configuracion_fs);
            break;
        case IO_FS_READ:
            direccion = recibir_int_del_buffer(buffer);
            tamanio = recibir_int_del_buffer(buffer);
            puntero_archivo = recibir_int_del_buffer(buffer);
            manejar_lectura_archivo(nombre_archivo, direccion, tamanio, puntero_archivo, configuracion_fs);
            break;
        default:
            log_warning(log_io, "Operacion desconocida recibida desde Kernel.\n");
            break;

        free(buffer);
        free(nombre_archivo);
    }
}

void manejar_creacion_archivo(char* nombre_archivo, Interfaz* configuracion_fs) 
{
    FILE* archivoFS;

    // Verificar si el archivo ya existe
    if (access(nombre_archivo, F_OK) != -1) 
    {
        log_error(log_io, "PID: %d - El archivo %s ya existe.", proceso->pid, nombre_archivo);
        archivoFS = fopen(nombre_archivo, "rb+"); // Abrir en modo lectura/escritura binaria
    }
    else
    {   
        log_error(log_io, "PID: %d - Crear archivo: %s.", proceso->pid, nombre_archivo);
        archivoFS = fopen(nombre_archivo, "wb+"); // Crear archivo en modo escritura/lectura binaria
    }

    if (archivoFS == NULL)
    {
        log_error(log_io, "PID: %d - Error al abrir/crear el archivo %s.", proceso->pid, nombre_archivo);
    }

    int block_size = configuracion_fs->archivo->block_size;
    int block_count = configuracion_fs->archivo->block_count;

    if (strcmp(nombre_archivo, "bloques.dat") == 0)
    {
        // Calcular el tamaño total del archivo bloques.dat
        int bloques_file_size = block_size * block_count;
        
        // Asignar memoria para el archivo de bloques (simulando bloques vacíos)
        char* buffer = malloc(bloques_file_size);

        if (buffer == NULL) {
            fprintf(stderr, "Error: No se pudo asignar memoria para el archivo de bloques.\n");
        }

        memset(buffer, 0, bloques_file_size); // Inicializar todos los bloques a 0 (libres)

        // Escribir el contenido inicial en el archivo (simulación de bloques)
        if (fwrite(buffer, 1, bloques_file_size, archivoFS) != bloques_file_size)
        {
            log_error(log_io, "PID: %d - Error al escribir en el archivo %s.", proceso->pid, nombre_archivo);
        }
        
        free(buffer);
    }
    else if (strcmp(nombre_archivo, "bitmap.dat") == 0)
    {
        // Calcular el tamaño necesario para el bitmap en bytes
        int bitmap_size = (block_count + 7) / 8; // 8 bits por byte

        // Crear un bitmap inicial con todos los bloques libres (0 indica libre, 1 indica ocupado)
        unsigned char *bitmap_data = malloc(bitmap_size);

        if (bitmap_data == NULL) 
        {
            fprintf(stderr, "Error: No se pudo asignar memoria para el bitmap.\n");
        }

        memset(bitmap_data, 0, bitmap_size); // Inicializar todo el bitmap a 0 (todos los bloques libres)

        // Buscar el primer bloque libre
        int bloque_libre = obtener_primer_bloque_libre(bitmap_data, block_count);

        if (bloque_libre == -1) 
        {
            log_error(log_io, "No hay bloques libres disponibles");
            return;
        }

        // Actualizar el bitmap en memoria
        bitmap_data[bloque_libre / 8] |= (1 << (bloque_libre % 8)); // Marcar el bloque como ocupado

        // Actualizar el bitmap
        fseek(archivoFS, 0, SEEK_SET);
        
        // Escribir el contenido inicial en el archivo
        if (fwrite(bitmap_data, 1, bitmap_size, archivoFS) != bitmap_size)
        {
            log_error(log_io, "PID: %d - Error al escribir en el archivo %s.", proceso->pid, nombre_archivo);
        }

        free(bitmap_data);
    }
    else
    {
        // Lógica para otros archivos de metadatos

        armar_config_metadatos();
        config_set_value(config_metadatos, config_metadatos->bloque_inicial, 0);

        // char path_metadatos[256];
        char path_metadatos = config_metadatos->max_path;

        char* path = configuracion_fs->archivo->path_base_dialfs;
        snprintf(path_metadatos, sizeof(path_metadatos), "%s/%s.metadata", path, nombre_archivo);

        FILE* metadata_file = fopen(path_metadatos, "wb+");
        if (metadata_file == NULL)
        {
            log_error(log_io, "PID: %d - No se pudo abrir el archivo de metadatos para %s.", proceso->pid, nombre_archivo);
        }

        // Escribir los metadatos en el archivo de metadatos
        fprintf(archivoFS, "BLOQUE_INICIAL=%d\n", config_metadatos->bloque_inicial);
        fprintf(archivoFS, "TAMANIO_ARCHIVO=%d\n", config_metadatos->tamanio_archivo);
    }
    fclose(archivoFS);
}

int obtener_primer_bloque_libre(unsigned char* bitmap, int block_count) 
{
    for (int i = 0; i < block_count; i++) 
    {
        if ((bitmap[i / 8] & (1 << (i % 8))) == 0) 
        { // Si el bit está en 0, el bloque está libre
            return i;
        }
    }
    
    return -1; // No hay bloques libres
}

void manejar_eliminacion_archivo(char* nombre_archivo, Interfaz* configuracion_fs) 
{
    // Lógica para otros archivos según sea necesario
    char path_archivo[256];
    snprintf(path_archivo, sizeof(path_archivo), "%s/%s", configuracion_fs->archivo->path_base_dialfs, nombre_archivo);
    
    // Verificar si el otro archivo existe
    if (access(path_archivo, F_OK) != -1)
    {
        // Eliminar el otro archivo
        if (remove(path_archivo) != 0)
        {
            log_info(log_io, "PID: %d - Error al eliminar el archivo %s.\n", proceso->pid, nombre_archivo);
        }
        else
        {
            log_info(log_io, "PID: %d - Eliminar archivo: %s.\n", proceso->pid, nombre_archivo);
        }
    }
    else
    {
        log_info(log_io, "PID: %d - El archivo %s no existe o no se puede acceder.\n", proceso->pid, nombre_archivo);
    }
}

void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, Interfaz* configuracion_fs)
{
    // Verificar parámetros de entrada
    if (nombre_archivo == NULL || nuevo_tamanio < 0 || configuracion_fs == NULL) 
    {
        printf("Error: Parámetros inválidos para truncar el archivo.\n");
    }

    // Construir la ruta completa al archivo
    char path_archivo[256];
    snprintf(path_archivo, sizeof(path_archivo), "%s/%s", configuracion_fs->archivo->path_base_dialfs, nombre_archivo);

    // Abrir el archivo en modo lectura/escritura binaria
    FILE* archivoFS = fopen(path_archivo, "rb+");
    if (archivoFS == NULL) 
    {
        log_info(log_io, "PID: %d - Error al abrir el archivo %s.\n", proceso->pid, nombre_archivo);
    }
    
    log_info(log_io, "PID: %d - Truncar Archivo: %s - Tamaño: %d\n", proceso->pid, nombre_archivo, nuevo_tamanio);
    
    // Truncar el archivo al nuevo tamaño
    // fileno convierte un puntero FILE* (devuelto por fopen) en un descriptor de archivo (int)
    // ftruncate es una función de C que se utiliza para truncar un archivo a una longitud específica.
    // Si (ftruncate(fileno(archivoFS), nuevo_tamanio) != 0) se cumple, cambia el tamaño, sino, error
    if (ftruncate(fileno(archivoFS), nuevo_tamanio) != 0) 
    {
        log_info(log_io, "PID: %d - Error al abrir el archivo %s.\n", proceso->pid, nombre_archivo);
    }

    fclose(archivoFS);
}   

void manejar_escritura_archivo(Interfaz* configuracion_fs, char* nombre_archivo, int direccion_logica, int tamanio, int puntero_archivo) 
{
    // Leer el archivo de metadata para obtener el bloque inicial y el tamaño
    char metadata_path[256];
    sprintf(metadata_path, "%s/%s.metadata", configuracion_fs->archivo->path_base_dialfs, nombre_archivo);
    
    FILE* metadata_file = fopen(metadata_path, "r+");
    if (metadata_file == NULL) 
    {
        log_error(log_io, "No se puede abrir el archivo de metadata para %s.", nombre_archivo);
        return;
    }

    int bloque_inicial;
    int tamanio_actual;
    
    if (fscanf(metadata_file, "BLOQUE_INICIAL=%d\nTAMANIO_ARCHIVO=%d\n", &bloque_inicial, &tamanio_actual) != 2) 
    {
        log_error(log_io, "Error al leer los metadatos del archivo %s.", nombre_archivo);
        fclose(metadata_file);
        return;
    }

    // Cerrar el archivo de metadata
    fclose(metadata_file);

    // Leer datos desde la memoria
    int direccion_fisica = solicitar_traduccion_direccion(direccion_logica);
    
    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_ESCRITURA_MEMORIA);
    agregar_int_al_paquete_personalizado(paquete, direccion_fisica);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    enviar_paquete(paquete, conexion_io_memoria);

    char* datos_a_escribir = NULL;

    // Recibo respuesta de memoria    
    int cod_op_memoria = recibir_operacion(conexion_io_memoria);
    while(1) 
    {
        switch (cod_op_memoria) 
        {
            case IO_RECIBE_RESPUESTA_DE_ESCRITURA_DE_MEMORIA:
                t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_memoria);
                datos_a_escribir = recibir_string_del_buffer(buffer);
                free(buffer);
                eliminar_paquete(paquete);
                close(conexion_io_memoria);
                break; 
            case -1:
                log_error(log_io, "Error al leer datos de memoria para escribir en %s.", nombre_archivo);
                free(conexion_io_memoria);
                return;
            default:
                log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }
    eliminar_paquete(paquete);

    // Escribir datos en el archivo
    FILE* archivoFS = fopen(metadata_file, "rb+");
    if (archivoFS == NULL) 
    {
        log_error(log_io, "Error al abrir el archivo %s para escritura.", nombre_archivo);
        free(datos_a_escribir);
        eliminar_paquete(paquete);
        close(conexion_io_memoria);
        return;
    }

    fseek(archivoFS, bloque_inicial* configuracion_fs->archivo->block_size + puntero_archivo, SEEK_SET);
    if (fwrite(datos_a_escribir, 1, tamanio, archivoFS) != tamanio) 
    {    
        log_error(log_io, "Error al escribir en el archivo %s desde la dirección %d.", nombre_archivo, direccion_logica);
        fclose(archivoFS);
        free(datos_a_escribir);
        eliminar_paquete(paquete);
        close(conexion_io_memoria);
        return;
    }

    // Actualizar el tamaño del archivo si es necesario
    if (puntero_archivo + tamanio > tamanio_actual) 
    {
        fseek(metadata_file, 0, SEEK_SET);
        fprintf(metadata_file, "BLOQUE_INICIAL=%d\n", bloque_inicial);
        fprintf(metadata_file, "TAMANIO_ARCHIVO=%d\n", puntero_archivo + tamanio);
    }

    fclose(metadata_file);
    fclose(archivoFS);
    free(datos_a_escribir);
    eliminar_paquete(paquete);
    close(conexion_io_memoria);

    log_info(log_io, "Datos escritos exitosamente en el archivo %s.", nombre_archivo);
}

void manejar_lectura_archivo(char* nombre_archivo, int direccion, int tamanio, int puntero_archivo, Interfaz* configuracion_fs)
{
    // Leer el archivo de metadata para obtener el bloque inicial y el tamaño
    char metadata_path[256];
    sprintf(metadata_path, "%s/%s", configuracion_fs->archivo->path_base_dialfs, nombre_archivo);
    FILE* metadata_file = fopen(metadata_path, "r");
    
    if (metadata_file == NULL) 
    {
        log_error(log_io, "No se puede abrir el archivo de metadata.");
        return;
    }

    int bloque_inicial;
    int tamanio_actual;

    fscanf(metadata_file, "BLOQUE_INICIAL=%d\n", &bloque_inicial);
    fscanf(metadata_file, "TAMANIO_ARCHIVO=%d\n", &tamanio_actual);
    fclose(metadata_file);

    // Verificar que el puntero de archivo y el tamaño no excedan los límites del archivo
    if (puntero_archivo + tamanio > tamanio_actual) 
    {
        log_error(log_io, "Intento de lectura fuera de los límites del archivo.");
        return;
    }

    // Leer datos del archivo de bloques
    FILE* bloques_file = fopen(configuracion_fs->archivo->path_base_dialfs, "r");
    if (bloques_file == NULL) 
    {
        log_error(log_io, "No se puede abrir el archivo de bloques.");
        return;
    }

    char* datos_leidos = malloc(tamanio);
    fseek(bloques_file, bloque_inicial * configuracion_fs->archivo->block_size + puntero_archivo, SEEK_SET);
    fread(datos_leidos, 1, tamanio, bloques_file);
    fclose(bloques_file);

    // Escribir los datos leídos en la memoria
    int direccion_fisica = solicitar_traduccion_direccion(direccion);
    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);
    agregar_int_al_paquete_personalizado(paquete, direccion_fisica);
    agregar_string_al_paquete_personalizado(paquete, datos_leidos);
    enviar_paquete(paquete, conexion_io_memoria);

    int cod_op_memoria = recibir_operacion(conexion_io_memoria);
    char* datos_a_leer = NULL;

    while(1) 
    {
        switch (cod_op_memoria) 
        {
            case IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEMORIA:
                t_buffer* buffer = recibiendo_paquete_personalizado(conexion_io_memoria);
                datos_a_leer = recibir_string_del_buffer(buffer);
                free(buffer);
                eliminar_paquete(paquete);
                close(conexion_io_memoria);
                break; 
            case -1:
                log_error(log_io, "Error al leer datos de memoria para leer en %s.", nombre_archivo);
                free(conexion_io_memoria);
                return;
            default:
                log_warning(log_io,"Operacion desconocida. No quieras meter la pata");
                break;
        }
    }

    eliminar_paquete(paquete);
    close(conexion_io_memoria);
    free(datos_leidos);
}