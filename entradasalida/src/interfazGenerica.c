#include "interfazGenerica.h"

io_config* leer_configuracion(const char* nombre_archivo) 
{
    FILE* archivo = fopen(nombre_archivo, "r");
    if (archivo == NULL) 
    {
        // Manejar error si no se puede abrir el archivo
        perror("Error al abrir el archivo de configuración");
        return NULL;
    }

    io_config* configuracion = malloc(sizeof(io_config));
    if (configuracion == NULL) 
    {
        // Manejar error si no se puede asignar memoria
        perror("Error de asignación de memoria");
        fclose(archivo);
        return NULL;
    }

    // Inicializar las propiedades a valores predeterminados o nulos
    configuracion->tipo_interfaz = NULL;
    configuracion->tiempo_unidad_trabajo = 0;
    configuracion->ip_kernel = NULL;
    configuracion->puerto_kernel = 0;
    configuracion->ip_memoria = NULL;
    configuracion->puerto_memoria = 0;
    configuracion->path_base_dialfs = NULL;
    configuracion->block_size = 0;
    configuracion->block_count = 0;
    configuracion->retraso_compactacion = 0;

    char linea[100]; // Tamaño suficiente para leer cada línea del archivo

    // Leer cada línea del archivo
    while (fgets(linea, sizeof(linea), archivo) != NULL) 
    {
        // Analizar la línea y extraer las propiedades
        char propiedad[50]; // Tamaño suficiente para el nombre de la propiedad
        if (sscanf(linea, "%[^=]=%s", propiedad, valor) != 2) {
            // Ignorar líneas que no tengan el formato correcto (propiedad=valor)
            continue;
        }

        // Comparar el nombre de la propiedad y asignar el valor correspondiente
        if (strcmp(propiedad, "TIPO_INTERFAZ") == 0) {
            configuracion->tipo_interfaz = strdup(valor);
        } else if (strcmp(propiedad, "TIEMPO_UNIDAD_TRABAJO") == 0) {
            configuracion->tiempo_unidad_trabajo = atoi(valor);
        } else if (strcmp(propiedad, "IP_KERNEL") == 0) {
            configuracion->ip_kernel = strdup(valor);
        } else if (strcmp(propiedad, "PUERTO_KERNEL") == 0) {
            configuracion->puerto_kernel = atoi(valor);
        } else if (strcmp(propiedad, "IP_MEMORIA") == 0) {
            configuracion->ip_memoria = strdup(valor);
        } else if (strcmp(propiedad, "PUERTO_MEMORIA") == 0) {
            configuracion->puerto_memoria = atoi(valor);
        } else if (strcmp(propiedad, "PATH_BASE_DIALFS") == 0) {
            configuracion->path_base_dialfs = strdup(valor);
        } else if (strcmp(propiedad, "BLOCK_SIZE") == 0) {
            configuracion->block_size = atoi(valor);
        } else if (strcmp(propiedad, "BLOCK_COUNT") == 0) {
            configuracion->block_count = atoi(valor);
        } else if (strcmp(propiedad, "RETRASO_COMPACTACION") == 0) {
            configuracion->retraso_compactacion = atoi(valor);
        }
    }

    fclose(archivo);
    return configuracion;
}


// Función para configurar la interfaz genérica
int configurar_interfaz_generica(const char* nombre_archivo) {
    // Leer la configuración desde el archivo
    io_config* configuracion = leer_configuracion(nombre_archivo);

    if (configuracion == NULL) {
        // Manejar error de lectura de configuración
        return EXIT_FAILURE;
    }

    // Inicializar la estructura InterfazGenerica
    InterfazGenerica* interfaz_generica = malloc(sizeof(InterfazGenerica));
    
    if (interfaz_generica == NULL) {
        // Manejar error de asignación de memoria
        free(configuracion);
        return EXIT_FAILURE;
    }

    // Asignar el nombre y la configuración a la interfaz genérica
    interfaz_generica->interfaz.nombre = "Interfaz Genérica";
    interfaz_generica->interfaz.archivo = configuracion;
    interfaz_generica->tiempo_unidad_trabajo = configuracion->tiempo_unidad_trabajo;

    // Aquí puedes realizar cualquier operación adicional necesaria para configurar la interfaz

    // Liberar la memoria asignada
    free(configuracion);
    free(interfaz_generica);
    
    return EXIT_SUCCESS;
}
