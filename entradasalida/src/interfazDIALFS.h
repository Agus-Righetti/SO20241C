#ifndef INTERFAZFS_H_
#define INTERFAZFS_H_

#include "io_gestor.h"
#include "conexion.h"

void leer_configuracion_dialfs(Interfaz *configuracion);
void iniciar_config_dialfs(Interfaz *configuracion);
void liberar_configuracion_dialfs(Interfaz* configuracion);
void recibir_operacion_dialfs_de_kernel(Interfaz* configuracion_fs, op_code codigo);
void manejar_creacion_archivo(char* nombre_archivo, int pid);
void manejar_eliminacion_archivo(char* nombre_archivo, Interfaz* configuracion_fs);
void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, Interfaz* configuracion_fs);
void manejar_escritura_archivo(Interfaz* configuracion, char* nombre_archivo, int direccion_logica, int tamanio, int puntero_archivo); 
void manejar_lectura_archivo(char* nombre_archivo, int direccion, int tamano, int puntero_archivo, Interfaz* configuracion_fs);
char* pasar_a_string(int valor);

void crear_archivos_gestion_fs();
int buscar_bloque_libre();
FILE* bloques_dat;

t_bitarray *bitmap;

#endif