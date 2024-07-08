#include "manejo_io.h"


//Dejo mi idea de funcionamiento de manejo de io:
//Cada vez que se conecta un ainterfaz se arma un hilo q se queda escuchando a la interfaz, para mandarle cosas a cada interfaz lo haremos desde el lugar q corresponda segun las verificaciones q haya q hacer (si existe la interfaz, si acepta cierta instruccion, etc). Cada interfaz tendra una cola de procesos esperando para usarla.
//A cada interfaz accedemos desde la queue que arme q es global que se llama cola_interfaces_conectadas buscando la q corresponda por el nombre (el nombre es un ENUM)
//Al desconectarse una interfaz tenemos q sacarla de la cola de interfaces conectadas, borrar su cola de procesos en espera y poner los procesos en estado ready o lo que corresopnda nuevamente y luego hacer el free de la estructura.

void crear_interfaz(op_code interfaz_nueva, int socket)
{
    //inicializo una nueva interfaz

	interfaz_kernel* nueva_interfaz = malloc(sizeof(interfaz_kernel)); 
	nueva_interfaz->nombre_interfaz= interfaz_nueva;
	nueva_interfaz->cola_de_espera = queue_create();
	nueva_interfaz->socket = socket;
	nueva_interfaz->en_uso = false;

    //agrego la interfaz a la cola de las q estan conectadas
	queue_push(cola_interfaces_conectadas, nueva_interfaz); 


	pthread_t hilo_de_escucha_interfaz; // Creo un hilo
    thread_args_escucha_io args_hilo = {nueva_interfaz}; // En sus args le cargo el la interfaz

    //este hilo se quedara escuchando lo que le devuelva la interfaz y hara lo q corresponda con cada proceso q la llamo al recibirlo de nuevo, tendriamos q chequear si la interfaz devuelve un proceso u otra cosa, en cuyo caso tendriamos q sacar el proceso de otro lado para volverlo a poner en ready o lo q sea
    pthread_create(&hilo_de_escucha_interfaz, NULL, (void*)escucha_interfaz,(void*)&args_hilo);
    pthread_join(hilo_de_escucha_interfaz, NULL); 

    return;
	
}

void escucha_interfaz(thread_args_escucha_io* args)
{
    bool interfaz_conectada = true;
    op_code cod_op;
    interfaz_kernel* interfaz = args->interfaz;

    while(interfaz_conectada)
    {
        //se queda esperando que la interfaz me mande algo
        cod_op = recibir_operacion(interfaz->socket); //me habla la interfaz

		switch (cod_op) { 
            //aca habria que agregar los posibles codigos de operacion q pueden llegar, dsp me fijo bien
			case -1://se desconecta la interfaz
                interfaz_conectada = false;
                desconectar_interfaz(interfaz);
				break;
			default:
				log_warning(log_kernel,"Operacion desconocida. No quieras meter la pata");
				break;

	    };
	}
    return;
}

void desconectar_interfaz(interfaz_kernel* interfaz)
{
    
}
