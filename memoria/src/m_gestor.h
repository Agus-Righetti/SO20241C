#ifndef M_GESTOR_H_
#define M_GESTOR_H_

// ************* INCLUDES GENERALES *************
#include <utils/hello.h>
#include <utils/utilsShare.h>

// ************* ESTRUCTURAS GLOBALES *************
typedef struct{
    char* puerto_escucha;
    int tam_memoria;
    int tam_pagina;
    char* path_instrucciones;
    int retardo_respuesta;
} memoria_config;

typedef struct{
	int pid;
    char* path;
	t_list* instrucciones;
    t_list* tabla_paginas;
} t_proceso;

// ************ ESTRUCTURA DE UNA INSTRUCCION *******
typedef struct{
	char* mnemonico;
    char* primero_parametro;
    char* segundo_parametro;
    char* tercero_parametro;
    char* cuarto_parametro;
    char* quinto_parametro;
} t_instruccion_codigo;

// ************ ESTRUCTURA DE UNA FILA DE TABLA *********
typedef struct {
    int frame;
    bool presencia;
    bool modificado;
    // Seguramente tengo que agregar más datos
} t_fila_tabla_paginas;

// ************* VARIABLES GLOBALES *************
extern t_log* log_memoria;
extern memoria_config* config_memoria;

extern int socket_servidor_memoria;
extern int socket_cliente_cpu;
extern int socket_cliente_kernel;
extern int socket_cliente_io;

extern t_list* lista_procesos_recibidos;

extern void* espacio_usuario;
extern t_bitarray* bitmap_marcos;

extern pthread_mutex_t mutex_bitmap_marcos;

#endif