#include "interfazSTDIN.h"

void leer_consola()
{
	char* leido;

	// La primera te la dejo de yapa
	leido = readline("Ingrese el texto: > ");
	log_info(log_io, ">>%s", leido);

	// El resto, las vamos leyendo y logueando hasta recibir un string vacío
	while(strcmp(leido, "") != 0)
    {
		free(leido);
		leido = readline("> ");
		log_info(log_io, ">> %s", leido);
	}

	// ¡No te olvides de liberar las lineas antes de regresar!
	free(leido);
}