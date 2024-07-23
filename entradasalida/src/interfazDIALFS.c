#include "interfazDIALFS.h"

// IO_FS_CREATE -> Para crear un archivo
void manejar_creacion_archivo(char* nombre_archivo, int pid) 
{

    log_info(log_io, "Entré a CREATE");
    //hago un usleep para lo q tarda
    usleep(config_io->tiempo_unidad_trabajo * 1000);

    log_info(log_io, "Estoy antes de crear el bitmap_file");

    FILE* bitmap_file = fopen("bitmap.dat", "rb"); // Abro el "bitmap.dat"

    char *bitmap_buffer = (char*)malloc(bitarray_size);

    fread(bitmap_buffer, 1, bitarray_size, bitmap_file); // Acá almaceno el contenido del bitmap_file dentro del bitmap_buffer

    log_info(log_io, "Pude leer, estoy por cerrar el archivo");

    fclose(bitmap_file); // Porque ya tengo el contenido en el buffer

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, MSB_FIRST);
    
    log_info(log_io, "cree el bitmap (struct)");

    int index_primer_bloque_libre = bitarray_find_first_clear_bit(bitmap);

    log_info(log_io, "El index del primer bloque libre al crear un archivo es: %d", index_primer_bloque_libre);

    if(index_primer_bloque_libre == -1)
    {
        log_error(log_io, "No hay bloques libres para la creacion de un nuevo archivo");
    }
    
    bitarray_set_bit(bitmap, index_primer_bloque_libre);

    log_info(log_io, "Estoy por abrir el archivo para escribir");

    bitmap_file = fopen("bitmap.dat", "wb");

    fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);

    log_info(log_io, "acabo de escrbir el archivo con los datos del bitmap");
    
    fclose(bitmap_file);

    FILE *file = fopen("bitmap.dat", "rb");

    log_info(log_io, "Pude abrir el archivo");

    unsigned char byte;
    while (fread(&byte, sizeof(unsigned char), 1, file) == 1) {
        for (int i = 7; i >= 0; i--) {
            printf("%d", (byte >> i) & 1);
        }
        printf(" "); // Para separar los bytes
    }

    fclose(file);

    bitarray_destroy(bitmap);
    free(bitmap_buffer);
    
    // Armamos la metadata del archivo -------------------------------------
    //hay q sacarle al archivo el .txt, ahora se llama archivo1.txt

    char** parte = string_split(nombre_archivo, ".");
    
    nombre_archivo = parte[0];
    
    string_append(&nombre_archivo,".config");

    char *config_file_path = nombre_archivo;

    log_info(log_io, "el nombre del config q voy a crear para el archivo es: %s", config_file_path);

    // Crear la configuración desde el archivo
    //Tenemos q armar el archivo primero

    FILE* archivo_config = fopen(config_file_path, "w");
    
    fclose(archivo_config);
    
    t_config *config = config_create(config_file_path);
    
    if (config == NULL) 
    {
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

    log_info(log_io, "Ya creé el archivo");
    
    avisar_fin_io_a_kernel();
    
    return;
}

// IO_FS_DELETE -> Para eliminar archivo
void manejar_eliminacion_archivo(char* nombre_archivo, int pid) 
{
    usleep(config_io->tiempo_unidad_trabajo * 1000);

    char** parte = string_split(nombre_archivo, ".");

    nombre_archivo = parte[0];

    string_append(&nombre_archivo,".config");
    
    char *config_file_path = nombre_archivo;

    // FILE* config_archivo = fopen(config_file_path, "r+");
    // strcat(nombre_archivo, ".config");

    t_config* config_aux = config_create(config_file_path);
    
    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
    int tamanio_en_bytes = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

    log_info(log_io, "Bloque inicial: %d", bloque_inicial);
    log_info(log_io, "Tamaño en bytes: %d", tamanio_en_bytes);

    config_destroy(config_aux);

    remove(nombre_archivo); // Elimino el archivo de metadata

    log_info(log_io, "Pude eliminar el archivo de metadata");
    
    // Creamos un bitmap a partir del archivo y lo editamos

    int bloques_ocupados = calcular_bloques_que_ocupa(tamanio_en_bytes);

    log_info(log_io, "Bloques ocupados: %d", bloques_ocupados);

    char *bitmap_buffer = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitarray_size, MSB_FIRST);

    for(bloque_inicial; bloque_inicial< bloques_ocupados; bloque_inicial ++) 
    {
        bitarray_clean_bit(bitmap, bloque_inicial); // pongo en cero los bloques ocupados
    }
    
    //tengo q actualizar el buffer
    
    escribir_archivo_con_bitmap(bitmap_buffer); // Actualizamos el archivo de bitmap

    free(bitmap_buffer);
    
    bitarray_destroy(bitmap);

    // no borramos los bloques del bloques.dat porque al marcar como libres los bloques en el bitmap se va a sobreescribir en esos bloques y listo

    avisar_fin_io_a_kernel();

    return;
}

// IO_TRUNCATE
void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, int pid)
{
    log_info(log_io,"Entre a truncar archivo");

    FILE *file = fopen("bitmap.dat", "rb");

    log_info(log_io, "Abri el bitmap es:");

    unsigned char byte;
    while (fread(&byte, sizeof(unsigned char), 1, file) == 1) {
        for (int i = 7; i >= 0; i--) {
            printf("%d", (byte >> i) & 1);
        }
        printf(" "); // Para separar los bytes
    }

    fclose(file);

    usleep(config_io->tiempo_unidad_trabajo * 1000);

    char** parte = string_split(nombre_archivo, ".");
    
    nombre_archivo = parte[0];
    
    string_append(&nombre_archivo,".config");

    char *config_file_path = nombre_archivo;

    FILE* config_archivo = fopen(config_file_path, "r+");

    t_config* config_aux = config_create(config_file_path);

    log_info(log_io, "Pude crear el config");

    t_metadata* metadata = malloc(sizeof(t_metadata));

    char* buffer_bitmap = obtener_bitmap();

    t_bitarray* bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, MSB_FIRST);

    char* buffer;

    int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
    int tamanio_original = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

    log_info(log_io, "Bloque inicial: <%d>; Tamaño original: <%d>", bloque_inicial, tamanio_original);

    fclose(config_archivo);

    log_info(log_io, "Nuevo tamaño es: %d", nuevo_tamanio);

    if(tamanio_original < nuevo_tamanio) // Vamos a tener que bsucar que haya bloques libres para agrandar el tamanio
    {
        int bytes_a_agregar = nuevo_tamanio - tamanio_original;

        int tamanio_ultimo_bloque = tamanio_original % config_io->block_size;

        int tamanio_disponible_ult_bloque = config_io->block_size - tamanio_ultimo_bloque;

        int bytes_a_agregar_en_bloques_enteros = bytes_a_agregar - tamanio_disponible_ult_bloque;
        
        if(tamanio_disponible_ult_bloque < bytes_a_agregar) // Si puedo meter todo lo que tengo en algun bloque ocupado lo hago ahi directamente
        {

            log_info(log_io, "Tamanio dispo en el ultimo bloque: %d", tamanio_disponible_ult_bloque);

            if(tamanio_disponible_ult_bloque > 0) // si los bloques ocupados no estan ocupados en su totalidad
            {
                bytes_a_agregar = bytes_a_agregar - tamanio_disponible_ult_bloque;
            }
            
            log_info(log_io, "Bytes a agregar: %d", bytes_a_agregar);
            
            int bloques_necesarios = calcular_bloques_que_ocupa(bytes_a_agregar);
            
            log_info(log_io, "Bloques necesarios %d", bloques_necesarios);

            //verificamos que haya la cantidad de bloques libres en total q necesitamos

            int cantidad_bloques_libres = contar_bloques_libres(bitmap);
            
            log_info(log_io, "Cantidad de bloques libres: %d", cantidad_bloques_libres);

            if(cantidad_bloques_libres < bloques_necesarios)
            {
                log_error(log_io, "No se puede realizar la opeacion TRUNCATE, no se posee el espacio necesario");
                return;
            }

            int bloque_inicial_nuevo = buscar_bloques_contiguos_desde_cierto_bloque(bloque_inicial, bloques_necesarios, bitmap);

            log_info(log_io, "El bloque inicial nuevo es: %d", bloque_inicial_nuevo);

            if(bloque_inicial_nuevo == -1) // Devuelve -1, no hay libres desde ese bloque
            {
                bloques_necesarios = calcular_bloques_que_ocupa(nuevo_tamanio); // Calculo de nuevo, porque van a ser todos desde una nueva posicion

                log_info(log_io, "Los bloques necesarios para el nuevo tamaño son: %d", bloques_necesarios);

                bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);

                log_info(log_io, "El bloque inicial nuevo es: %d", bloque_inicial_nuevo);
                
                if(bloque_inicial_nuevo == -1){ // No hay bloques contiguos, necesito compactar si entra
                    
                    log_info(log_io, "Si entra, entonces tengo que compactar");

                    // Primero tenemos q sacar los bloques ocupados y ppner la info en un buffer
                    // dsp compacto y pongo lo q habia sacado actuizando el bitmap
                    
                    int bloques_que_ocupa_original = calcular_bloques_que_ocupa(tamanio_original);
                    
                    buffer = leer_bloques(bloque_inicial, bloques_que_ocupa_original); // Este buffer tiene lo que ya venía en el archivo

                    // ahora marco en el bitmap esos bloques como libres
                    
                    for(int i = bloque_inicial; i < bloques_que_ocupa_original; i++)
                    {
                        bitarray_clean_bit(bitmap, i);
                    }

                    compactar(bitmap); // compactar actualiza el bloques.dat y el bitmap
                    
                    // tenemos q agregar al final lo q esta en el buffer

                    bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);

                    agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

                    // tenemos q actualizar el config del archivo con el tamano y bloque inicial
                    metadata->nombre_archivo = nombre_archivo;
                    metadata->bloque_inicial = bloque_inicial_nuevo;
                    metadata->tamanio_archivo = nuevo_tamanio;

                    actualizar_metadata(metadata);
                }
                else
                {   
                    // hay bloques contiguos, tengo q ponerlo en esos bloques

                    log_info(log_io, "Hay espacio, no tengo que compactar");

                    int bloques_del_tamanio_original = calcular_bloques_que_ocupa(tamanio_original);
                    
                    if (bloques_del_tamanio_original == 0){
                        bloques_del_tamanio_original = 1;
                    }
                    
                    // Deberia dar 1 en el caso del FS_1
                    log_info(log_io, "Bloques del tamaño original: %d", bloques_del_tamanio_original);

                    buffer = leer_bloques(bloque_inicial, bloques_del_tamanio_original);

                    log_info(log_io, "Lo que esta en el buffer es: %s", buffer);
                    log_info(log_io, "Bloque inicial: %d", bloque_inicial);

                    for(int i = bloque_inicial; i < bloque_inicial + bloques_del_tamanio_original; i++)
                    {
                        log_info(log_io, "Voy a limpiar el indice <%d> del bitmap", i);
                        bitarray_clean_bit(bitmap, i);
                    }
                    
                    agregar_info_en_cierto_bloque(bloque_inicial_nuevo, bloques_necesarios, buffer);

                    // marco los nuevos bits como ocupados
                    for(int i = bloque_inicial_nuevo; i < bloque_inicial_nuevo + bloques_necesarios; i++)
                    {
                        log_info(log_io, "Voy a marcar como ocupado el indice <%d> del bitmap", i);
                        bitarray_set_bit(bitmap, i);
                    }
                    
                    // guardo la metadata actualidad
                    metadata->tamanio_archivo = nuevo_tamanio;
                    metadata->nombre_archivo = nombre_archivo;
                    metadata->bloque_inicial = bloque_inicial_nuevo;

                    log_info(log_io, "El bloque de inicio que estoy por mandar a metadata es: %d", bloque_inicial_nuevo);

                    // actualizo los dos archivos
                    //tengo q mandarle el del bitmap_buffer no el bitmap
                    escribir_archivo_con_bitmap(buffer_bitmap);
                    actualizar_metadata(metadata);
                }
            }else{

                // tenemos en bloque_inicial el bloque inicial
                metadata->tamanio_archivo = nuevo_tamanio;
                metadata->nombre_archivo = nombre_archivo;
                metadata->bloque_inicial = bloque_inicial;


                //bloque_inicial + 1 porq el inicial ya esta seteado
                //bloques_necesarios + bloque_inicial para q sean los necesarios desde el inicial

                log_info(log_io, "Los bloques necesarios son: %d", bloques_necesarios);
                log_info(log_io, "El bloque inicial es: %d", bloque_inicial);

                for(int i = bloque_inicial + 1; i < bloques_necesarios + bloque_inicial + 1; i++)
                {
                    //log_info(log_io, "esto es el bloque en el q estoy por hacer set %d", i);
                    bitarray_set_bit(bitmap,i);
                }

                //le mando el buffer
                escribir_archivo_con_bitmap(buffer_bitmap);
                actualizar_metadata(metadata);
            }
        }else{
            
            //actualizo el metadata solamente, el bitmap no porq no le agregue bloques
            metadata->tamanio_archivo = nuevo_tamanio;
            metadata->nombre_archivo = nombre_archivo;
            metadata->bloque_inicial = bloque_inicial;

            actualizar_metadata(metadata);
        }

        

    }else if(tamanio_original > nuevo_tamanio) // solo tengo que truncar el archivo, marco el bitmap como libre
    { 
        int bloques_a_liberar = calcular_bloques_que_ocupa(tamanio_original) - calcular_bloques_que_ocupa(nuevo_tamanio);
        
        for(int i = bloque_inicial; i < bloques_a_liberar ; i++ )
        {
            bitarray_clean_bit(bitmap, i);
        }

        metadata->tamanio_archivo = nuevo_tamanio;
        metadata->nombre_archivo = nombre_archivo;
        metadata->bloque_inicial = bloque_inicial;

        escribir_archivo_con_bitmap(buffer_bitmap);
        actualizar_metadata(metadata);
    }

    config_destroy(config_aux);
    free(buffer_bitmap);
    bitarray_destroy(bitmap);

    FILE *file_2 = fopen("bitmap.dat", "rb");

    log_info(log_io, "Voy a leer el bitmap");

    unsigned char byte_2;
    while (fread(&byte_2, sizeof(unsigned char), 1, file_2) == 1) {
        for (int i = 7; i >= 0; i--) {
            printf("%d", (byte_2 >> i) & 1);
        }
        printf(" "); // Para separar los bytes
    }

    fclose(file_2);
    
    avisar_fin_io_a_kernel();

    return;
}   

// IO_WRITE
void manejar_escritura_archivo(char* nombre_archivo, t_list* direccion_fisica, int tamanio, int puntero_archivo, int pid)
{
    usleep(config_io->tiempo_unidad_trabajo * 1000);

    // Primero Memoria tiene que leer la direccion para obtener el valor (como en el STDOUT WRITE, recien despues de eso puedo escribir el archivo)

    t_paquete* paquete = crear_paquete_personalizado(IO_PIDE_LECTURA_MEMORIA);

    agregar_int_al_paquete_personalizado(paquete, pid);
    agregar_int_al_paquete_personalizado(paquete, tamanio);
    agregar_lista_al_paquete_personalizado(paquete, direccion_fisica, sizeof(t_list));

    enviar_paquete(paquete, conexion_io_memoria);

    eliminar_paquete(paquete);

    log_info(log_io, "Estoy justo antes del semaforo esperando el okay de memoria");
    sem_wait(&sem_ok_lectura_memoria);
    log_info(log_io, "Estoy después de haber recibido el okay de memoria, ahora puedo usar valor_a_mostrar para escribir");

    // Ahora ya tengo los datos que quiero escribir en el archivo en "valor_a_mostrar" (es global)

    log_info(log_io, "El valor_a_mostrar es: %s", valor_a_mostrar);
    log_info(log_io, "Cantidad de caracteres de valor_a_mostrar: %zu", strlen(valor_a_mostrar)); 
    log_info(log_io, "El nombre_archivo es: %s", nombre_archivo);


    FILE* bloques_dat = fopen("bloques.dat", "r+b");
    
    log_info(log_io, "Acabo de abrir el bloques.dat");

    if (bloques_dat == NULL) {
        log_error(log_io, "Error al abrir bloques.dat");
        return;
    }

    // Mueve el puntero del archivo al offset especificado por puntero_archivo
    fseek(bloques_dat, puntero_archivo, SEEK_SET);

    size_t bytes_a_escribir = tamanio;
    size_t bloque_size = config_io->block_size;

    log_info(log_io, "Bytes a escribir: %d", bytes_a_escribir);
    log_info(log_io, "Tamaño de bloque: %d", bloque_size);
    
    int bytes_operados = 0;

    while (bytes_a_escribir > 0) {

        // Calcula la cantidad de datos a escribir en el bloque actual
        size_t cantidad_a_escribir = (bytes_a_escribir > bloque_size) ? bloque_size : bytes_a_escribir;

        log_info(log_io, "Cantidad a escribir: %d", cantidad_a_escribir);
        log_info(log_io, "Valor a escribir: %s", valor_a_mostrar + bytes_operados);
        
        fwrite(valor_a_mostrar + bytes_operados , cantidad_a_escribir, 1, bloques_dat);

        // Actualiza el puntero del archivo y la cantidad de bytes restantes
        puntero_archivo += cantidad_a_escribir;
        bytes_a_escribir -= cantidad_a_escribir;
        bytes_operados += cantidad_a_escribir;

        // Si el archivo es más grande que el tamaño del bloque, mueve el puntero a la siguiente posición
        fseek(bloques_dat, puntero_archivo, SEEK_SET);
    }

    log_info(log_io, "Pude escribir el bloques.dat");

    fclose(bloques_dat);

    avisar_fin_io_a_kernel();

    return;
}

// *************************************************************************++
// *************************************************************************++
// *************************************************************************++



// void manejar_escritura_archivo(char* nombre_archivo, char* datos, int tamanio_datos, int pid) 
// {
//     log_info(log_io, "Entré a ESCRIBIR");

//     // Simula el tiempo de operación
//     usleep(config_io->tiempo_unidad_trabajo * 1000);

//     // Divide el nombre del archivo y agrega .config
//     char** parte = string_split(nombre_archivo, ".");
//     nombre_archivo = parte[0];
//     string_append(&nombre_archivo, ".config");
//     char* config_file_path = nombre_archivo;

//     // Abre el archivo de configuración del archivo
//     t_config* config_aux = config_create(config_file_path);

//     if (config_aux == NULL) {
//         log_error(log_io, "Error al abrir el archivo de configuración %s", config_file_path);
//         return;
//     }

//     // Obtener el bloque inicial y el tamaño original del archivo
//     int bloque_inicial = config_get_int_value(config_aux, "BLOQUE_INICIAL");
//     int tamanio_original = config_get_int_value(config_aux, "TAMANIO_ARCHIVO");

//     log_info(log_io, "Bloque inicial: %d", bloque_inicial);
//     log_info(log_io, "Tamaño original: %d", tamanio_original);

//     // Obtener el bitmap
//     char* buffer_bitmap = obtener_bitmap();
//     t_bitarray* bitmap = bitarray_create_with_mode(buffer_bitmap, bitarray_size, MSB_FIRST);

//     // Calcula los bloques necesarios para almacenar los datos
//     int bloques_necesarios = calcular_bloques_que_ocupa(tamanio_datos);
//     log_info(log_io, "Bloques necesarios para los datos: %d", bloques_necesarios);







    

//     // Verifica si hay suficiente espacio en el bitmap
//     int cantidad_bloques_libres = contar_bloques_libres(bitmap);
//     log_info(log_io, "Cantidad de bloques libres: %d", cantidad_bloques_libres);

//     if (cantidad_bloques_libres < bloques_necesarios) {
//         log_error(log_io, "No hay suficientes bloques libres para escribir los datos");
//         config_destroy(config_aux);
//         free(buffer_bitmap);
//         bitarray_destroy(bitmap);
//         return;
//     }

//     // Encuentra bloques libres contiguos
//     int bloque_inicial_nuevo = buscar_bloques_contiguos(bloques_necesarios, bitmap);
//     if (bloque_inicial_nuevo == -1) {
//         log_error(log_io, "No hay bloques libres contiguos suficientes para escribir los datos");
//         config_destroy(config_aux);
//         free(buffer_bitmap);
//         bitarray_destroy(bitmap);
//         return;
//     }

//     log_info(log_io, "Bloque inicial nuevo para escribir: %d", bloque_inicial_nuevo);

//     // Escribir los datos en los bloques correspondientes
//     FILE* bloques_dat = fopen("bloques.dat", "rb+");
//     if (bloques_dat == NULL) {
//         log_error(log_io, "Error al abrir el archivo bloques.dat");
//         config_destroy(config_aux);
//         free(buffer_bitmap);
//         bitarray_destroy(bitmap);
//         return;
//     }

//     fseek(bloques_dat, bloque_inicial_nuevo * config_io->block_size, SEEK_SET);
//     fwrite(datos, sizeof(char), tamanio_datos, bloques_dat);
//     fclose(bloques_dat);

//     // Actualizar el bitmap marcando los bloques como ocupados
//     for (int i = bloque_inicial_nuevo; i < bloque_inicial_nuevo + bloques_necesarios; i++) {
//         bitarray_set_bit(bitmap, i);
//     }

//     // Guardar el bitmap actualizado
//     escribir_archivo_con_bitmap(buffer_bitmap);

//     // Actualizar la metadata del archivo
//     config_set_value(config_aux, "BLOQUE_INICIAL", pasar_a_string(bloque_inicial_nuevo));
//     config_set_value(config_aux, "TAMANIO_ARCHIVO", pasar_a_string(tamanio_datos));
//     config_save(config_aux);
//     config_destroy(config_aux);

//     // Liberar memoria
//     free(buffer_bitmap);
//     bitarray_destroy(bitmap);

//     log_info(log_io, "Datos escritos correctamente en el archivo");

//     avisar_fin_io_a_kernel();
// }




// *************************************************************************++
// *************************************************************************++
// *************************************************************************++


// IO_READ 
void manejar_lectura_archivo(char* nombre_archivo, t_list* direccion_fisica, int tamanio, int puntero_archivo, Interfaz* configuracion_fs)
{
    usleep(config_io->tiempo_unidad_trabajo * 1000);
    return;
}

void crear_archivos_gestion_fs()
{
    // CREO Y DEFINO EL TAMAÑO DEL ARCHIVO bloques.dat

    log_info(log_io, "Entré a CREAR ARCHIVOS DE GESTIÓN");

    FILE* bloques_dat = fopen("bloques.dat", "wb");

    log_info(log_io,"ya abri el bloques.dat");

    log_info(log_io, "El block size es: %d", config_io->block_size);
    log_info(log_io, "El block count es: %d", config_io->block_count);
    
    if (bloques_dat == NULL) 
    {
        perror("Error al abrir el archivo");
        return;
    }

    int tamanio = config_io->block_size * config_io->block_count;

    log_info(log_io, "Creé el tamaño: %d", tamanio);

    // Mueve el puntero al final del archivo para definir su tamaño
    if (fseek(bloques_dat, tamanio - 1, SEEK_SET) != 0) 
    {
        perror("Error al establecer el tamaño del archivo");
        fclose(bloques_dat);
        return;
    }

    // Escribe un byte en la última posición para establecer el tamaño
    fputc('\0', bloques_dat);

    log_info(log_io,"estableci el final del archivo");

    fclose(bloques_dat);

    log_info(log_io, "Ya cerré bloques_dat");

    // CREO EL ARCHIVO CON EL BITMAP --------------------------------------------------------

    FILE *bitmap_file = fopen("bitmap.dat", "wb");  // Abre el archivo en modo lectura y escritura binaria
    
    if (bitmap_file == NULL) 
    {
        perror("Error al abrir bitmap.dat");
        return;
    }
    
    log_info(log_io,"estoy por crear el bitmap");
    
    size_t bitmap_size_bytes = (config_io->block_count + 7) / 8;  // 1 bit por bloque
    
    char* bitmap_buffer = (char*)malloc(bitmap_size_bytes);

    log_info(log_io,"arme el bitmap buffer");

    if (bitmap_buffer == NULL) 
    {
        perror("Error al reservar memoria para el bitmap");
        fclose(bitmap_file);
        return;
    }
    
    memset(bitmap_buffer, 0, bitmap_size_bytes);
    // Limpio el bitmap -> lo seteo en 0
    t_bitarray* bitmap = bitarray_create_with_mode(bitmap_buffer, bitmap_size_bytes, MSB_FIRST);
    
    log_info(log_io,"cree el bitarray");

    fseek(bitmap_file, 0, SEEK_SET);  // Mueve el puntero al inicio del archivo
    fwrite(bitmap_buffer, sizeof(char), bitmap_size_bytes, bitmap_file);

    log_info(log_io, "Acabo de escribir sobre el bitmap");
   
    if (bitmap == NULL) 
    {
        fprintf(stderr, "Error al crear el bitmap\n");
        free(bitmap_buffer);
        fclose(bitmap_file);
        return;
    }

    free(bitmap_buffer);
    fclose(bitmap_file);
    
    log_info(log_io, "Cierro el bitmap");

    bitarray_destroy(bitmap);
    
    log_info(log_io,"Supuestamente ya cree los archivos de gestion");
    
    // bitarray_set_bit(bitmap, 0);  // Por ejemplo, establece el primer bloque como ocupado
    // bitarray_set_bit(bitmap, 1);  // Y el segundo bloque como ocupado
    // bitarray_clean_bit(bitmap, 0) // Pone el bit número 0 libre
    return;
}

// char* pasar_a_string(int valor)
// {
//     static char buffer[20]; 
//     snprintf(buffer, sizeof(buffer), "%d", valor);
//     return buffer;
// }

char* pasar_a_string(int valor)
{
    char* buffer = (char*)malloc(20 * sizeof(char));
    if (buffer != NULL)
    {
        snprintf(buffer, 20, "%d", valor);
    } else {
        log_info(log_io, "ERORRRRR");
    }
    return buffer;
}


int calcular_bloques_que_ocupa(int bytes)
{
    int bloques_que_ocupa = (bytes + config_io->block_size - 1) / config_io->block_size;
    
    return bloques_que_ocupa;
}

char* obtener_bitmap()
{
    FILE* bitmap_file = fopen("bitmap.dat", "rb"); 

    //bitarray_size = (config_io->block_count + 7) / 8;

    char *bitmap_buffer = (char *)malloc(bitarray_size);
    fread(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);
    return bitmap_buffer;
}

void escribir_archivo_con_bitmap(char* bitmap_buffer)
{

    log_info(log_io, "el bitmap antes de actualizar es:");
    
    FILE *file_2 = fopen("bitmap.dat", "rb");

   // log_info(log_io, "Voy a leer el bitmap");

    unsigned char byte_2;
    while (fread(&byte_2, sizeof(unsigned char), 1, file_2) == 1) {
        for (int i = 7; i >= 0; i--) {
            printf("%d", (byte_2 >> i) & 1);
        }
        printf(" "); // Para separar los bytes
    }

    fclose(file_2);

    FILE* bitmap_file = fopen("bitmap.dat", "wb"); 
    fwrite(bitmap_buffer, 1, bitarray_size, bitmap_file);
    fclose(bitmap_file);

    log_info(log_io, "ya actualice el bitmap ahora es:");

    FILE *file_3 = fopen("bitmap.dat", "rb");

    //log_info(log_io, "Voy a leer el bitmap");

    unsigned char byte_3;
    while (fread(&byte_3, sizeof(unsigned char), 1, file_3) == 1) {
        for (int i = 7; i >= 0; i--) {
            printf("%d", (byte_3 >> i) & 1);
        }
        printf(" "); // Para separar los bytes
    }
    fclose(file_3);


    //bitarray_destroy(bitmap);
}

int buscar_bloques_contiguos_desde_cierto_bloque(int bloque_inicial, int bloques_necesarios, t_bitarray* bitmap) 
{
    int cont_bloques_libres_consecutivos = 0;

    for (int i = bloque_inicial+1; i < config_io->block_count; i++) 
    {
        if (!bitarray_test_bit(bitmap, i)) 
        { // Si el bloque i está libre (bit es 0)
            
            cont_bloques_libres_consecutivos++; // Incrementa el cont de bloques libres consecutivos

            if (cont_bloques_libres_consecutivos == bloques_necesarios) 
            {
                return bloque_inicial; // Retorna bloque_inicial si se encontraron suficientes bloques libres consecutivos
            }
        } else {
            return -1; // Reinicia el cont si se encuentra un bloque ocupado
        }
    }

    return -1; // Retorna -1 si no se encuentran suficientes bloques libres contiguos
}

int buscar_bloques_contiguos(int bloques_necesarios, t_bitarray* bitmap){
    
    int contador_bloques_contiguos = 0;
    int bloque_inicio = -1; // Inicializa bloque_inicio

    for (int i = 0; i < config_io->block_count; i++) 
    {
        if (!bitarray_test_bit(bitmap, i)) 
        { 
            // Bloque libre
            if (contador_bloques_contiguos == 0) 
            {
                bloque_inicio = i;
                
            log_info(log_io, "El indice del bloque inicial es -> %d", i);

            }

            contador_bloques_contiguos++;

            log_info(log_io, "El contador es -> %d", contador_bloques_contiguos);
            log_info(log_io, "Bloques necesarios-> %d", bloques_necesarios);

            if (contador_bloques_contiguos == bloques_necesarios)
            {
                log_info(log_io, "Estoy a punto de devolver (buscar_bloques_contiguos): %d", bloque_inicio);
                
                return bloque_inicio;
            }
            
        } else {
            contador_bloques_contiguos = 0;
            bloque_inicio = -1;
        }
    }
    return -1; // No hay suficientes bloques contiguos disponibles 
}

int contar_bloques_libres(t_bitarray* bitmap)
{
    int bloques_libres = 0;

    for (int i = 0; i < config_io->block_count; i++)
    {
        if (!bitarray_test_bit(bitmap, i)) 
        { // Si el bit es 0, el bloque está libre,
            bloques_libres++;
        }
    }
    
    return bloques_libres;
}

char* leer_bloques(int bloque_inicial, int num_bloques) 
{
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

char *agregar_al_final(char *buffer, const char *informacion) 
{
    if (buffer == NULL) 
    {
        // Si el buffer es NULL, asigna memoria suficiente para la información
        buffer = malloc(strlen(informacion) + 1);  // +1 para el terminador nulo
        
        strcpy(buffer, informacion);
    } else {
        // Si el buffer ya contiene datos, realloca memoria para incluir la nueva información
        size_t tam_buffer = strlen(buffer);
        size_t tam_informacion = strlen(informacion);
        buffer = realloc(buffer, tam_buffer + tam_informacion + 1);  // +1 para el terminador nulo
        if (buffer == NULL) 
        {
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
    //hago un usleep por el tiempo q me dice la config
    usleep(config_io->retraso_compactacion * 1000); //multiplico por mil porque esta dado en miliseg y necesito microseg
    char* buffer;
    int contador_ocupados = 0;
    t_metadata* metadata = malloc(sizeof(t_metadata)); //metadata->nombre_archivo, ->BLOQUE_INICIAL, ->TAMANIO_ARCHIVO
    int bloques_ocupados;
    //i recorre cada bloque del bitmap i == un numero de bloque

    for (int i = 0; i < config_io->block_count; i++) 
    { 
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
    
    

    actualizar_archivo_bloques(buffer);//copio lo del buffer en el archivo de bloques.dat
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

    log_info(log_io, "El bloque inicial en metadata (int) es: %d", metadata->bloque_inicial);
    log_info(log_io, "El bloque inicial en metadata (string) es: %s", bloque_inicial);

    config_set_value(config_archivo, "BLOQUE_INICIAL", bloque_inicial);
    config_set_value(config_archivo, "TAMANIO_ARCHIVO", tamanio_bloque);

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

int bitarray_find_first_clear_bit(t_bitarray* bitmap)
{
    for (int i = 0; i < config_io->block_count; i++)
    {
        // bitarray_test_bit(bitmap, i) --> Si retorna 0, quiere decir que el bloque esta libre
        // !bitarray_test_bit(bitmap, i) --> 0 en C es FALSO, !0 es VERDADERO, entonces queremos que entre en el if para que 
        // retorne el bloque que esta libre

        if (!bitarray_test_bit(bitmap, i)) // Si está vacío el bitmap, entonces devuelvo el índice en el que me encuentro
        {
            log_info(log_io, "Voy a devolver el indice: %d", i);
            return i;
        }
    }
    return -1;
}

int min(int a, int b) {
    return a < b ? a : b;
}
