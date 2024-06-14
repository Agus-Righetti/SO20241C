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
#include<commons/string.h>
#include<commons/config.h>
#include<commons/bitarray.h> //Biblioteca para manejar bitarray (memoria)
#include<assert.h>
#include<readline/readline.h>
#include<pthread.h>          // Biblioteca para hilos
#include<semaphore.h>        // Biblioteca para semaforos
#include<math.h>             // Biblioteca para (ceil)
#include <time.h>            // Biblioteca para usar clock()


// ************* ESTRUCTURAS GLOBALES *************
// ************* CODIGOS DE OPERACION *************
typedef enum
{
	MENSAJE,
	PAQUETE,
	INSTRUCCION, 

	// Instrucciones
    WAIT,
    SIGNAL, 
	MOV_IN,
	MOV_OUT,
	OUT_OF_MEMORY, // CAMBIAR ESTO

    CODIGO,
	INTERRUPCION,
	CPU,
	DESALOJO, // Devolvemos PCB por interrupcion

	INTERRUPCION_KERNEL,
	DIRECCION_FISICA,
	DEVOLVER_TRADUCCION,

	// Instruccioniones de I/O
	IO_GEN_SLEEP, 
	IO_STDIN_READ,
	IO_STDOUT_WRITE,
	IO_FS_CREATE,
	IO_FS_DELETE,
	IO_FS_TRUNCATE,
	IO_FS_WRITE,
	IO_FS_READ,
	SOLICITAR_TRADUCCION,

	// Interfaces
	GENERICA,
    STDIN,
    STDOUT,
    DIALFS,
	
	// KERNEL A MEMORIA
	CREACION_PROCESO_KERNEL_A_MEMORIA,  // [path, PID] -> [String, Int]
	FINALIZAR_PROCESO_KERNEL_A_MEMORIA, // [PID] -> [Int] 

	// CPU A MEMORIA
	CPU_PIDE_INSTRUCCION_A_MEMORIA, // [PID, IP] -> [Int, Int]
	CPU_MANDA_RESIZE_A_MEMORIA,     // [PID, TAMAÑO] -> [Int, Int]
	CPU_PIDE_MARCO_A_MEMORIA,

	ACCESO_A_TABLA_DE_PAGINA,       // [PID, NUMERO PAG] -> [Int, Int]
	CPU_PIDE_LECTURA_MEMORIA,       // [Direccion Fisica, TAMAÑO, VALOR]
	CPU_PIDE_ESCRITURA_MEMORIA,     // [Direccion Fisica, TAMAÑO, VALOR]

	// MEMORIA A CPU
	CPU_RECIBE_INSTRUCCION_DE_MEMORIA,     // [Instruccion] -> [String, String, String, String, String]
	CPU_RECIBE_TAMAÑO_PAGINA_DE_MEMORIA,   // [Tam_pagina] -> [Int]
	CPU_RECIBE_OUT_OF_MEMORY_DE_MEMORIA,   // VACIO
	CPU_RECIBE_OK_DEL_RESIZE,              // VACIO -> falta implementar
	CPU_RECIBE_NUMERO_DE_MARCO_DE_MEMORIA, // [NUMERO DE MARCO] -> [Int]   ->falta implementar

	// IO A MEMORIA
	IO_PIDE_LECTURA_MEMORIA,   // [Direccion Fisica, TAMAÑO, VALOR] 
	IO_PIDE_ESCRITURA_MEMORIA, // [Direccion Fisica, TAMAÑO, VALOR]

	IO_RECIBE_TRADUCCION_DE_KERNEL,
	IO_RECIBE_RESPUESTA_DE_LECTURA_DE_MEM,

	PCB_KERNEL_A_CPU,
	PCB_CPU_A_KERNEL,
	
	CPU_TERMINA_EJECUCION_PCB //Flag para ver si el proceso ya se ejecuto del todo (1 o 0)
}op_code;

// ************* SERIALIZACION *************
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


// ************ REGISTROS DEL CPU ************
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
} estados;

typedef struct {
    int pid;
	estados estado_del_proceso;
    int program_counter;
    int quantum;
    registros_cpu* registros; 
	char* direccion_instrucciones; //es el path que me mandan por consola
	pthread_mutex_t mutex_pcb;
	t_list* instrucciones;
	t_queue* recursos_asignados;
} pcb;

// ************ DECLARACION DE FUNCIONES ************
// ************ SERIALIZACION Y CONEXIONES GENERALES ************
void* recibir_buffer(int*, int);
int iniciar_servidor(char*, t_log*);
int esperar_cliente(int, t_log*);
t_list* recibir_paquete(int);
void recibir_mensaje(int, t_log*);
int recibir_operacion(int);
void iterator(char* value);

int crear_conexion(char* ip, char* puerto);
void enviar_mensaje(char* mensaje, int socket_cliente);
t_paquete* crear_paquete(void);
void agregar_a_paquete(t_paquete* paquete, void* valor, int tamanio);
void enviar_paquete(t_paquete* paquete, int socket_cliente);
void liberar_conexion(int socket_cliente);
void eliminar_paquete(t_paquete* paquete);

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