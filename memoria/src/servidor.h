#ifndef SERVIDOR_H_
#define SERVIDOR_H_

#include "m_gestor.h"
#include "atenderCPU-IO.h"


// *************** DECLARACIÓN DE FUNCIONES **********
// ********* INICIO MEMORIA COMO SERVIDOR *********
void inicializar_servidor();

// ********* ESPERO QUE MODULOS SE CONECTEN COMO CLIENTES *********
void server_para_cpu();
void server_para_kernel();
void server_para_io();

// ********* APENAS ESTABLEZCO CONEXION CON CPU ENVIO TAM DE MARCOS *********
void enviar_tam_marco_a_cpu();

// ********* APENAS SE CONECTA UNA INTERFAZ ARMO UN HILO Q SE QUEDE ESCUCHANDO LO Q PIDE *********
void crear_hilo_escucha_interfaz(int socket_io);
void escucha_interfaz(void* arg);


#endif