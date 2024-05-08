#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/collections/list.h>
#include<commons/collections/queue.h>
#include<string.h>
#include<assert.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<pthread.h> // Biblioteca para hilos
#include<semaphore.h> // Biblioteca para semaforos
#include<unistd.h> // Para la función sleep()

// Server

typedef enum
{
	MENSAJE,
	PAQUETE,
	INSTRUCCION, 
    WAIT,
    SIGNAL, 
    CODIGO,
	INTERRUPCION,
	CPU,
	DESALOJO
}op_code;

void* recibir_buffer(int*, int);
int iniciar_servidor(char*, t_log*);
int esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
void recibir_mensaje(int, t_log*);
int recibir_operacion(int);
void iterator(char* value);
void recv_handshake(int socket_cliente);

// Client

typedef struct
{
	int size;
	void* stream;
} t_buffer;

typedef struct
{
	op_code codigo_operacion;
	t_buffer* buffer;
} t_paquete;

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);
void send_handshake(int socket_cliente);


// ************ ESTRUCTURA REGISTROS DEL CPU ************
typedef struct{
	uint32_t pc; // Program Counter
	uint8_t ax; // Acumulador
	uint8_t bx; // Base
	uint8_t cx; // Counter
	uint8_t dx; // Data
	uint32_t eax;
	uint32_t ebx;
	uint32_t ecx;
	uint32_t edx;
	uint32_t si; // Contiene la direccion logica de memoria de origen desde donde se va a copiar un string
	uint32_t di; // Contiene la direccion logica de memoria de destino a donde se va a copiar un string
}registros_cpu;

// ************ ESTRUCTURA PCB DEL KERNEL, TIENE REGISTROS DEL CPU ************

typedef enum {
	READY,
	BLOCKED,
	EXECUTE,
	NEW,
	EXIT,
}estados;

typedef struct {
    int pid;
	estados estado_del_proceso;
    int program_counter;
    int quantum;
    registros_cpu* registros; 
	char* direccion_instrucciones; //es el path que me mandan por consola
	pthread_mutex_t mutex_pcb;
	t_list* instrucciones;
}pcb;

#endif