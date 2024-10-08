#ifndef M_GESTOR_H_
#define M_GESTOR_H_

// ************* INCLUDES GENERALES *************
#include <utils/hello.h>
#include <utils/utilsShare.h>
#include <math.h>

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
    int tamanio; 
	t_list* instrucciones;
    t_list* tabla_paginas;
    int tam_usado_ult_pag;
    sem_t sem_instrucciones;
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

// ************ ESTRUCTURA DE UNA FILA DE TABLA -> una PAGINA *********
typedef struct {
    int frame;
    // El bit de modificado es innecesario, no tengo memoria virtual
    // Idem, el bit de presencia
} t_pagina;

// ************ ESTRUCTURA DE UN MARCO, la tuve que hacer para la lista *********
typedef struct {
    int id;
} t_frame;

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
extern int cant_marcos; 

extern pthread_mutex_t mutex_bitmap_marcos;
extern pthread_mutex_t mutex_espacio_usuario;
extern sem_t sem_lista_procesos;
extern sem_t sem_primero;
extern pthread_mutex_t mutex_lista_procesos;

#endif
