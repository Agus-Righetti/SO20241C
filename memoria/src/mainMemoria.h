#ifndef MEMORIA_MAIN_H_
#define MEMORIA_MAIN_H_

#include "m_gestor.h"

#include "configuracion.h"
#include "servidor.h"
#include "escucha.h"
#include "atenderCPU.h"
#include "atenderKERNEL.h"
#include "paginacion.h"

//************** VARIABLES GLOBALES *******************
//************** CONFIG Y LOG ******************
t_log* log_memoria;
memoria_config* config_memoria;

//******** SOCKET DE MI SERVIDOR: MEMORIA ***********
int socket_servidor_memoria;

//******** SOCKETS DE TODOS LOS CLIENTES ********
int socket_cliente_cpu;
int socket_cliente_kernel;
int socket_cliente_io;

//******** PROCESOS ***********
t_list* lista_procesos_recibidos;

//******** PAGINACIÓN ***********
void* espacio_usuario;
t_bitarray* bitmap_marcos;
int cant_marcos;
t_list* marcos_libres;

//******** SEMÁFOROS ***********
pthread_mutex_t mutex_bitmap_marcos;
pthread_mutex_t mutex_espacio_usuario;

#endif