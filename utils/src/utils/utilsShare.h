#ifndef UTILS_H_
#define UTILS_H_

#include<stdio.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<unistd.h>
#include<netdb.h>
#include<commons/log.h>
#include<commons/error.h>
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
	DESALOJO,
	IO_GEN_SLEEP,
	GENERICA,
    STDIN,
    STDOUT,
    DIALFS,
	// KERNEL A MEMORIA
	CREACION_PROCESO_KERNEL_A_MEMORIA, // [path, PID] -> [Char, Int]

	// CPU A MEMORIA
	CPU_PIDE_INSTRUCCION_A_MEMORIA, // [PID, IP] -> [Int, Int]

	// MEMORIA A CPU
	CPU_RECIBE_INSTRUCCION_DE_MEMORIA, // [Instruccion] -> [Char, Char, Char, Char, Char]

	// INSTRUCCION
	// PCB de Kernel a CPU
	PCB_KERNEL_A_CPU,
	// PCB CPU a Kernel
	PCB_CPU_A_KERNEL
}op_code;

void* recibir_buffer(int*, int);
int iniciar_servidor(char*, t_log*);
int esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
void recibir_mensaje(int, t_log*);
int recibir_operacion(int);
void iterator(char* value);

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

// ************ ESTRUCTURA DE UNA INSTRUCCION *******
// ************** USADO POR MEMORIA Y CPU **********
typedef struct{
	char* mnemonico;
    char* primero_parametro;
    char* segundo_parametro;
    char* tercero_parametro;
    char* cuarto_parametro;
    char* quinto_parametro;
}t_instruccion_codigo;

//******* PAQUETES PERSONALIZADOS **********
t_paquete* crear_paquete_personalizado(op_code code_op);
void agregar_int_al_paquete_personalizado(t_paquete* paquete, int valor);
void agregar_string_al_paquete_personalizado(t_paquete* paquete, char* string);
void agregar_estructura_al_paquete_personalizado(t_paquete* paquete, void* estructura, int size);
t_buffer* recibiendo_paquete_personalizado(int socket_conexion);
int recibir_int_del_buffer(t_buffer* buffer);
char* recibir_string_del_buffer(t_buffer* buffer);
void* recibir_estructura_del_buffer(t_buffer* buffer);

#endif