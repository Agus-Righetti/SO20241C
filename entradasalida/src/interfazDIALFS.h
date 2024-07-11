#ifndef INTERFAZFS_H_
#define INTERFAZFS_H_

#include "io_gestor.h"
#include "conexion.h"




void leer_configuracion_dialfs(Interfaz *configuracion);
void iniciar_config_dialfs(Interfaz *configuracion);
void liberar_configuracion_dialfs(Interfaz* configuracion);
void recibir_operacion_dialfs_de_kernel(Interfaz* configuracion_fs, op_code codigo);
void manejar_creacion_archivo(char* nombre_archivo, int pid);
void manejar_eliminacion_archivo(char* nombre_archivo, int pid);
void manejar_truncado_archivo(char* nombre_archivo, int nuevo_tamanio, int pid);
void manejar_escritura_archivo(Interfaz* configuracion, char* nombre_archivo, int direccion_logica, int tamanio, int puntero_archivo); 
void manejar_lectura_archivo(char* nombre_archivo, int direccion, int tamano, int puntero_archivo, Interfaz* configuracion_fs);
char* pasar_a_string(int valor);

void crear_archivos_gestion_fs();
int buscar_bloques_contiguos_desde_cierto_bloque(int bloque_inicial, int bloques_necesarios, t_bitarray* bitmap);
int buscar_bloques_contiguos(int bloques_necesarios, t_bitarray* bitmap);
char* leer_bloques(int bloque_inicial, int num_bloques);
void actualizar_metadata(t_metadata metadata);
t_metadata buscar_archivo_que_inicia_en_bloque(int nro_bloque);
void actualizar_archivo_bloques(char* buffer);
void compactar(t_bitarray* bitmap);
char *agregar_al_final(char *buffer, const char *informacion);
void escribir_archivo_con_bitmap(t_bitarray* bitmap);
int agregar_info_en_cierto_bloque(int bloque_inicial_nuevo, int cant_bloques , char* buffer);
char* obtener_bitmap();

#endif