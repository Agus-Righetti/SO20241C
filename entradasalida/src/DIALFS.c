// #include "DIALFS.h"

// t_bitarray* bitmap;
// char* bitmap_buffer;

// void crear_archivos_gestion_fs() 
// {
//     // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bloques.dat
    
//     FILE* bloques_dat = fopen("bloques.dat", "rb+");
    
//     if (bloques_dat == NULL) 
//     {
//         // Si el archivo no existe, lo creamos y lo inicializamos
//         bloques_dat = fopen("bloques.dat", "wb+");

//         if (bloques_dat == NULL) 
//         {
//             perror("Error al crear bloques.dat");
//             return;
//         }

//         int tamanio = config_io->block_size * config_io->block_count;

//         // Mueve el puntero al final del archivo para definir su tamaño
//         if (fseek(bloques_dat, tamanio - 1, SEEK_SET) != 0) 
//         {
//             perror("Error al establecer el tamaño del archivo bloques.dat");
//             fclose(bloques_dat);
//             return;
//         }

//         // Escribe un byte en la última posición para establecer el tamaño
//         fputc('\0', bloques_dat);
//     }

//     fclose(bloques_dat);

//     // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bitmap.dat

//     FILE *bitmap_file = fopen("bitmap.dat", "rb+");  // Abre el archivo en modo lectura y escritura binaria
    
//     if (bitmap_file == NULL) 
//     {
//         // Si el archivo no existe, lo creamos y lo inicializamos
//         bitmap_file = fopen("bitmap.dat", "wb+");
        
//         if (bitmap_file == NULL) 
//         {
//             perror("Error al crear bitmap.dat");
//             return;
//         }

//         // 1. Calcular el tamaño del bitmap en bytes
//         size_t bitmap_size_bytes = (config_io->block_count + 7) / 8;  // 1 bit por bloque

//         // 2. Reservar memoria para el bitmap en un buffer
//         bitmap_buffer = (char*)malloc(bitmap_size_bytes);
        
//         if (bitmap_buffer == NULL) 
//         {
//             perror("Error al reservar memoria para el bitmap");
//             fclose(bitmap_file);
//             return;
//         }

//         // 3. Inicializar el bitmap en 0 (todos los bloques libres)
//         memset(bitmap_buffer, 0, bitmap_size_bytes);
//         bitmap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, LSB_FIRST);

//         // 4. Escribir el bitmap inicializado de vuelta al archivo
//         fseek(bitmap_file, 0, SEEK_SET);  // Mueve el puntero al inicio del archivo
//         fwrite(bitmap_buffer, sizeof(char), bitmap_size_bytes, bitmap_file);

//         // 5. Destruir el bitmap y liberar la memoria
//         bitarray_destroy(bitmap);
//         free(bitmap_buffer);
//     }
//     fclose(bitmap_file);
// }

// // IO_FS_CREATE -> Para crear un archivo ------------------------------------------------------------------------------------
// void manejar_creacion_archivo(char* nombre_archivo, int pid) 
// {
//     // Para armar el metadata del archivo debo verificar si hay espacio en el bitmap

//     FILE* bitmap_file = fopen("bitmap.dat", "rb");
//     bitmap_buffer = (char*)malloc(bitarray_size);
//     fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
//     fclose(bitmap_file);

//     bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, LSB_FIRST);
//     int index_primer_bloque_libre = bitarray_find_first_clear_bit(bitmap);

//     if (index_primer_bloque_libre == -1) 
//     {
//         log_error(log_io, "No hay bloques libres para la creacion de un nuevo archivo");
//         bitarray_destroy(bitmap);
//         free(bitmap_buffer);
//         return;
//     }

//     bitarray_set_bit(bitmap, index_primer_bloque_libre);

//     bitmap_file = fopen("bitmap.dat", "wb");

//     if (bitmap_file == NULL) 
//     {
//         log_error(log_io, "Error al abrir el archivo bitmap.dat para escritura");
//         bitarray_destroy(bitmap);
//         free(bitmap_buffer);
//         return;
//     }

//     fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);
//     fclose(bitmap_file);

//     bitarray_destroy(bitmap);
//     free(bitmap_buffer);

//     // Armamos la metadata del archivo-----------------------------------------------

//     strcat(nombre_archivo, ".config");
//     char* config_file_path = nombre_archivo;

//     // Crear la configuración desde el archivo
//     t_config* config = config_create(config_file_path);
//     if (config == NULL) 
//     {
//         fprintf(stderr, "Error: No se pudo cargar el archivo de configuración %s\n", config_file_path);
//         return;
//     }

//     if(index_primer_bloque_libre == -1)
//     {
//         log_error(log_io, "Error: No hay bloques libres");
//         return;
//     }   

//     char* bloque_inicial = pasar_a_string(index_primer_bloque_libre);

//     config_set_value(config, "BLOQUE_INICIAL", bloque_inicial);
//     config_set_value(config, "TAMANIO_ARCHIVO", "0");

//     // Guardar los cambios en el archivo
//     config_save(config);
//     queue_push(cola_archivos_en_fs, strdup(nombre_archivo));
    
//     // Liberar memoria utilizada por la configuración
//     config_destroy(config);

//     avisar_fin_io_a_kernel();
//     return;
// }

// char* pasar_a_string(int valor)
// {
//     static char buffer[20]; // Asegúrate de que el tamaño sea suficiente
//     snprintf(buffer, sizeof(buffer), "%d", valor);
//     return buffer;
// }

// int bitarray_find_first_clear_bit(t_bitarray* bitmap)
// {
//     for (int i = 0; i < config_io->block_count; i++)
//     {
//         // bitarray_test_bit(bitmap, i) --> Si retorna 0, quiere decir que el bloque esta libre
//         // !bitarray_test_bit(bitmap, i) --> 0 en C es FALSO, !0 es VERDADERO, entonces queremos que entre en el if para que 
//         // retorne el bloque que esta libre

//         if (!bitarray_test_bit(bitmap, i)) // Si está vacío el bitmap, entonces devuelvo el índice en el que me encuentro
//         {
//             return i;
//         }
//     }
//     return -1;
// }

// // IO_FS_DELETE -> Para eliminar archivo ------------------------------------------------------------------
// void manejar_eliminacion_archivo(char* nombre_archivo, int pid) 
// {
//     strcat(nombre_archivo, ".config");
//     char *config_file_path = nombre_archivo;
//     t_config* config_aux = config_create(config_file_path);
    
//     int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
//     int tamanio_en_bytes = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

//     config_destroy(config_aux);
//     remove(nombre_archivo); // Elimino el archivo de metadata
    
//     // Creamos un bitmap a partir del archivo y lo editamos
//     int bloques_ocupados = cantidad_total_bloques_que_ocupa(tamanio_en_bytes);
//     bitmap_buffer = obtener_bitmap();
//     bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, LSB_FIRST);

//     for(int i = 0; i < bloques_ocupados; i++) 
//     {
//         bitarray_clean_bit(bitmap, bloque_inicial + i); // Pongo en cero los bloques ocupados
//     }
    
//     free(bitmap_buffer);
//     escribir_archivo_con_bitmap(bitmap); // Actualizamos el archivo de bitmap
//     bitarray_destroy(bitmap);

//     // No borramos los bloques del bloques.dat porque al marcar como libres los bloques en el bitmap
//     // se va a sobreescribir en esos bloques y listo

//     avisar_fin_io_a_kernel();
//     return;
// }

// int cantidad_total_bloques_que_ocupa(int tamanio_archivo_bytes)
// {
//     int bloques_que_ocupa = (tamanio_archivo_bytes + config_io->block_size - 1) / config_io->block_size;
//     return bloques_que_ocupa; // Cantidad de bloques que ocupa el archivo
// }

// char* obtener_bitmap()
// {
//     FILE* bitmap_file = fopen("bitmap.dat", "rb"); 

//     //bitarray_size = (config_io->block_count + 7) / 8;

//     bitmap_buffer = (char*)malloc(bitarray_size);
//     fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
//     fclose(bitmap_file);
//     return bitmap_buffer;
// }

// void escribir_archivo_con_bitmap(t_bitarray* bitmap)
// {
//     FILE* bitmap_file = fopen("bitmap.dat", "wb"); 
//     fwrite(bitmap, 1, bitarray_size, bitmap_file);
//     fclose(bitmap_file);
//     //bitarray_destroy(bitmap);
// }

// void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, int pid)
// {
//     strcat(nombre_archivo, ".config");
//     char *config_file_path = nombre_archivo;

//     FILE* config_archivo = fopen(config_file_path, "r+");
//     t_config* config_aux = config_create(config_file_path);
//     t_metadata* metadata = malloc(sizeof(t_metadata));

//     buffer_bitmap = obtener_bitmap();
//     bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, LSB_FIRST);

//     char* buffer;
//     int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
//     int tamanio_original = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

//     fclose(config_archivo);

//     if(tamanio_original < nuevo_tamanio) // Vamos a tener que buscar que haya bloques libres para agrandar el tamanio
//     // Puede que haya espacio suficiente pero que no este contiguo, en ese caso hay que hacer compactacion
//     {
//         int bytes_a_agregar = nuevo_tamanio - tamanio_original;
//         int resto = tamanio_original % config_io->block_size;

//         if(resto > 0) // Si los bloques ocupados no estan ocupados en su totalidad
//         {
//             bytes_a_agregar = bytes_a_agregar - (config_io->block_size - resto);
//         }
        
//         int bloques_necesarios = calcular_bloques_que_ocupa(bytes_a_agregar);
        
//         // Verificamos que haya la cantidad de bloques libres en total q necesitamos
//         int cantidad_bloques_libres = contar_bloques_libres(bitmap);
        
//         if(cantidad_bloques_libres < bloques_necesarios)
//         {
//             log_error(log_io, "No se puede realizar la opeacion TRUNCATE, no se posee el espacio necesario");
//             return;
//         }

//         int bloque_inicial_nuevo = buscar_bloques_contiguos_desde_cierto_bloque(bloque_inicial, bloques_necesarios, bitmap);

//         if(bloque_inicial_nuevo == -1) // Devuelve -1, no hay libres desde ese bloque
//         {
//             bloques_necesarios = calcular_bloques_que_ocupa(nuevo_tamanio); 
//             bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);
            
//             if(bloque_inicial_nuevo == -1) // No hay bloques contiguos, necesito compactar si entra
//             {
//                 // Primero tenemos q sacar los bloques ocupados y ppner la info en un buffer
//                 // dsp compacto y pongo lo q habia sacado actuizando el bitmap
                
//                 int bloques_que_ocupa_original = calcular_bloques_que_ocupa(tamanio_original);
                
//                 buffer = leer_bloques(bloque_inicial, bloques_que_ocupa_original); // Este buffer tiene lo que ya venía en el archivo

//                 // ahora marco en el bitmap esos bloques como libres
                
//                 for(int i = bloque_inicial; i < bloques_que_ocupa_original; i++)
//                 {
//                     bitarray_clean_bit(bitmap, i);
//                 }

//                 compactar(bitmap); // compactar actualiza el bloques.dat y el bitmap
                
//                 // tenemos q agregar al final lo q esta en el buffer

//                 bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);

//                 agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

//                 // tenemos q actualizar el config del archivo con el tamano y bloque inicial
//                 metadata->nombre_archivo = nombre_archivo;
//                 metadata->bloque_inicial = bloque_inicial_nuevo;
//                 metadata->tamanio_archivo = nuevo_tamanio;

//                 actualizar_metadata(metadata);
//             }
//             else
//             {   
//                 // hay bloques contiguos, tengo q ponerlo en esos bloques

//                 int bloques_del_tamanio_original = calcular_bloques_que_ocupa(tamanio_original);

//                 buffer = leer_bloques(bloque_inicial, bloques_del_tamanio_original);

//                 for(int i = bloque_inicial; i < bloques_del_tamanio_original; i++)
//                 {
//                     bitarray_clean_bit(bitmap, i);
//                 }
                
//                 agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

//                 // marco los nuevos bits como ocupados
//                 for(int i = bloque_inicial_nuevo; i < bloques_necesarios; i++)
//                 {
//                     bitarray_set_bit(bitmap, i);
//                 }
                
//                 // guardo la metadata actualidad
//                 metadata->tamanio_archivo = nuevo_tamanio;
//                 metadata->nombre_archivo = nombre_archivo;
//                 metadata->bloque_inicial = bloque_inicial_nuevo;

//                 // actualizo los dos archivos
//                 escribir_archivo_con_bitmap(bitmap);
//                 actualizar_metadata(metadata);
//             }
//         }else{

//             // tenemos en bloque_inicial el bloque inicial
//             metadata->tamanio_archivo = nuevo_tamanio;
//             metadata->nombre_archivo = nombre_archivo;
//             metadata->bloque_inicial = bloque_inicial;

//             // falta actualizar el bitmap

//             bloques_necesarios = calcular_bloques_que_ocupa(nuevo_tamanio);

//             for(int i = bloque_inicial; i < bloques_necesarios; i++)
//             {
//                 bitarray_set_bit(bitmap,i);
//             }

//             escribir_archivo_con_bitmap(bitmap);
//             actualizar_metadata(metadata);
//         }

//     }else if(tamanio_original > nuevo_tamanio) // solo tengo que truncar el archivo, marco el bitmap como libre
//     { 
//         int bloques_a_liberar = calcular_bloques_que_ocupa(tamanio_original) - calcular_bloques_que_ocupa(nuevo_tamanio);
        

//         for(int i = bloque_inicial; i < bloques_a_liberar ; i++ )
//         {
//             bitarray_clean_bit(bitmap, i);
//         }

//         metadata->tamanio_archivo = nuevo_tamanio;
//         metadata->nombre_archivo = nombre_archivo;
//         metadata->bloque_inicial = bloque_inicial;

//         escribir_archivo_con_bitmap(bitmap);
//         actualizar_metadata(metadata);
//     }

//     config_destroy(config_aux);
//     free(buffer_bitmap);
//     bitarray_destroy(bitmap);

//     avisar_fin_io_a_kernel();

//     return;
// }   

