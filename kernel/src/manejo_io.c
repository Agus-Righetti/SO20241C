#include "manejo_io.h"


//Dejo mi idea de funcionamiento de manejo de io:
//Cada vez que se conecta un ainterfaz se arma un hilo q se queda escuchando a la interfaz, para mandarle cosas a cada interfaz lo haremos desde el lugar q corresponda segun las verificaciones q haya q hacer (si existe la interfaz, si acepta cierta instruccion, etc). Cada interfaz tendra una cola de procesos esperando para usarla.
//A cada interfaz accedemos desde la queue que arme q es global que se llama cola_interfaces_conectadas buscando la q corresponda por el nombre (el nombre es un ENUM)
//Al desconectarse una interfaz tenemos q sacarla de la cola de interfaces conectadas, borrar su cola de procesos en espera y poner los procesos en estado ready o lo que corresopnda nuevamente y luego hacer el free de la estructura.

void crear_interfaz(op_code interfaz_nueva, int socket, char* nombre_interfaz)
{
    //inicializo una nueva interfaz

	interfaz_kernel* nueva_interfaz = malloc(sizeof(interfaz_kernel)); 
	nueva_interfaz->tipo_interfaz= interfaz_nueva;
	nueva_interfaz->cola_de_espera = queue_create();
	nueva_interfaz->socket = socket;
	nueva_interfaz->en_uso = false;
    nueva_interfaz->nombre_interfaz = nombre_interfaz;

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

int io_gen_sleep(char* nombre_interfaz, int unidades_de_trabajo, pcb* proceso)
{
    //ante una petición van a esperar una cantidad de unidades de trabajo, cuyo valor va a venir dado en la petición desde el Kernel.
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, GENERICA);
    argumentos_para_io args;
    args.unidades_de_trabajo = unidades_de_trabajo;

    if(interfaz) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        //aca hago lo q tengo q hacer, es decir bloquear el proceso y madnarle la isntruccion a la interfaz
        pthread_mutex_lock(&mutex_enviando_instruccion_a_io);
        if(interfaz->en_uso == false) //la interfaz esta disponible para mandar
        {
            interfaz->en_uso = true;
            interfaz->proceso_en_interfaz = proceso;
            enviar_instruccion_io(interfaz->socket , IO_GEN_SLEEP, args);
            pasar_proceso_a_blocked(proceso);

        }else{// la interfaz esta ocupada, pongo al proceso en la cola de espera
            queue_push(interfaz->cola_de_espera, proceso);
            pasar_proceso_a_blocked(proceso);
        }
        pthread_mutex_unlock(&mutex_enviando_instruccion_a_io);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else return 1;//mando el proceso a exit

}

void enviar_instruccion_io(int socket, op_code instruccion, argumentos_para_io args)
{
    t_paquete* paquete_instruccion = crear_paquete_personalizado(instruccion); // Creo un paquete personalizado con un codop para que IO reconozca lo que le estoy mandando

    switch (instruccion)
    {
        case IO_GEN_SLEEP:
            agregar_int_al_paquete_personalizado(paquete_instruccion, args.unidades_de_trabajo);
            break;
        
        default:
            break;
    }
   

    enviar_paquete(paquete_instruccion, socket); // Envio el paquete a través del socket

    eliminar_paquete(paquete_instruccion); // Libero el paquete

    return;
}

void* verificar_interfaz(char* nombre_interfaz_buscada, op_code tipo_interfaz_buscada)
{
    //Esta funcion verifica q exista la interfaz con el nombre pasado por parametro y que su tipo se corresponda con el pasado por parametro
    interfaz_kernel* aux;

    if(queue_is_empty(cola_interfaces_conectadas) == false){ // Si la cola no está vacía

        aux  = queue_pop(cola_interfaces_conectadas);
        char* primer_nombre = aux->nombre_interfaz;

        while(aux->nombre_interfaz != nombre_interfaz_buscada) 
        {

            queue_push(cola_interfaces_conectadas, aux);
            aux = queue_pop(cola_interfaces_conectadas);
            if(aux->nombre_interfaz == primer_nombre)
            {
                queue_push(cola_interfaces_conectadas, aux);
                return NULL; //No esta la interfaz con ese nombre, retorno null
            }
        }
        //Si llegamos a este punto es porque se encontro la interfaz
        if(aux->tipo_interfaz == tipo_interfaz_buscada)
        {
            return aux; //La interfaz es del tipo q quiero, la devuelvo
        }else return NULL; //No coincide el tipo de interfaz, mando null
    }

    return NULL;
}
