#ifndef INTERFAZSTDIN_H_
#define INTERFAZSTDIN_H_

#include "io_gestor.h"

void leer_consola();
void leer_configuracion_stdin(Interfaz *configuracion);
void iniciar_config_stdin(Interfaz *configuracion);
void liberar_configuracion_stdin(Interfaz* configuracion);
void recibir_operacion_stdin_de_kernel(STDIN, cod_op_io);

#endif