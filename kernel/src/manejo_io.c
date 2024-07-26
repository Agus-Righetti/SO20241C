#include "manejo_io.h"


//Dejo mi idea de funcionamiento de manejo de io:
//Cada vez que se conecta un ainterfaz se arma un hilo q se queda escuchando a la interfaz, para mandarle cosas a cada interfaz lo haremos desde el lugar q corresponda segun las verificaciones q haya q hacer (si existe la interfaz, si acepta cierta instruccion, etc). Cada interfaz tendra una cola de procesos esperando para usarla.
//A cada interfaz accedemos desde la queue que arme q es global que se llama cola_interfaces_conectadas buscando la q corresponda por el nombre (el nombre es un ENUM)
//Al desconectarse una interfaz tenemos q sacarla de la cola de interfaces conectadas, borrar su cola de procesos en espera y poner los procesos en estado ready o lo que corresopnda nuevamente y luego hacer el free de la estructura.

void crear_interfaz(op_code interfaz_nueva, int socket, char* nombre_interfaz)
{
    //inicializo una nueva interfaz
    log_info(log_kernel, "Se conecto una interfaz nueva, hola! %s \n", nombre_interfaz);
	interfaz_kernel* nueva_interfaz = malloc(sizeof(interfaz_kernel)); 
	nueva_interfaz->tipo_interfaz= interfaz_nueva;
	nueva_interfaz->cola_de_espera = queue_create();
	nueva_interfaz->socket = socket;
    nueva_interfaz->nombre_interfaz = nombre_interfaz;
    nueva_interfaz->conectada = true;

    sem_init(&nueva_interfaz->sem_puedo_mandar_operacion, 0,1);
    sem_init(&nueva_interfaz->sem_hay_procesos_esperando,0,0); 
    
    pthread_mutex_init(&(nueva_interfaz->mutex_cola), NULL);

    //agrego la interfaz a la cola de las q estan conectadas
    pthread_mutex_lock(&mutex_cola_de_interfaces);
	queue_push(cola_interfaces_conectadas, nueva_interfaz); 
    pthread_mutex_unlock(&mutex_cola_de_interfaces);

    pthread_create(&nueva_interfaz->hilo_de_escucha_interfaz, NULL, (void*)escucha_interfaz,(void*)nueva_interfaz);
    pthread_create(&nueva_interfaz->hilo_de_envio_a_interfaz, NULL, (void*)envio_interfaz,(void*)nueva_interfaz);

    //hago detatch para q nadie espere a q este finalice, el cancel del otro lo hago cuando termina este hilo
    pthread_detach(nueva_interfaz->hilo_de_escucha_interfaz);
    pthread_detach(nueva_interfaz->hilo_de_envio_a_interfaz);

    return;
}

void envio_interfaz(void* args)
{
    interfaz_kernel* interfaz = (interfaz_kernel*) args;
    argumentos_para_io* args_mandar_a_io;

    while(interfaz->conectada)
    {
        sem_wait(&interfaz->sem_hay_procesos_esperando);
        sem_wait(&interfaz->sem_puedo_mandar_operacion);

        pthread_mutex_lock(&interfaz->mutex_cola);
        args_mandar_a_io = queue_pop(interfaz->cola_de_espera);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        interfaz->proceso_en_interfaz = args_mandar_a_io->proceso;

        enviar_instruccion_io(interfaz->socket, args_mandar_a_io);
    }
}

void escucha_interfaz(void* args)
{
    interfaz_kernel* interfaz = (interfaz_kernel*) args;
    op_code cod_op;
    t_buffer* buffer;

    while(interfaz->conectada)
    {
        //se queda esperando que la interfaz me mande algo
        cod_op = recibir_operacion(interfaz->socket); //me habla la interfaz
        
        switch (cod_op) { 
            case FIN_OP_IO:

                if(interfaz->proceso_en_interfaz->pid == pid_eliminar)
                {
                    accionar_segun_estado(interfaz->proceso_en_interfaz,1, INTERRUPTED_BY_USER) ; //lo mando a exit

                }else {
                    accionar_segun_estado(interfaz->proceso_en_interfaz,0, -1);
                } // lo mando a ready 

                //flag 0 para q lo mande a la cola de ready
                sem_post(&interfaz->sem_puedo_mandar_operacion);
                
                buffer = recibiendo_paquete_personalizado(interfaz->socket);

                free(buffer);

                //argumentos->interfaz->conectada = false;
                
                break;
            case -1://se desconecta la interfaz

                interfaz->conectada = false;
                desconectar_interfaz(interfaz);

				break;
			default:
            
				log_warning(log_kernel,"Estamos en manejo io escucha interfaz: Operacion desconocida. No quieras meter la pata");

			    break;
	    };
	}
    return;
}

void desconectar_interfaz(interfaz_kernel* interfaz)
{
    interfaz_kernel* interfaz_aux;
    pthread_mutex_lock(&mutex_cola_de_interfaces);
    interfaz_aux = queue_pop(cola_interfaces_conectadas);
    while(interfaz_aux->nombre_interfaz != interfaz->nombre_interfaz)
    {
        queue_push(cola_interfaces_conectadas, interfaz_aux);
        interfaz_aux = queue_pop(cola_interfaces_conectadas);
    }
    pthread_mutex_unlock(&mutex_cola_de_interfaces);
 
    //aca adentro hay procesos, pero no importa porque nunca van a evaluar la posibilidad de q se 
    //desconecte la interfaz y tenga procesos en la cola, hago el if por si pasa eso (lo cual seria un error porq no tiene q pasar nunca)

    if(queue_size(interfaz_aux->cola_de_espera) > 0)
    {
        log_warning(log_kernel, "Ojo al piojo! Esta interfaz tenia procesos en la cola de espera!!");
    }
    queue_destroy(interfaz_aux->cola_de_espera);

    sem_destroy(&interfaz_aux->sem_puedo_mandar_operacion);
    sem_destroy(&interfaz_aux->sem_hay_procesos_esperando);
    pthread_mutex_destroy(&interfaz_aux->mutex_cola);

    log_info(log_kernel, "Interfaz %s desconectada", interfaz_aux->nombre_interfaz);

    free(interfaz_aux);
    return;
}

void enviar_instruccion_io(int socket, argumentos_para_io* args)
{
    t_paquete* paquete_instruccion = crear_paquete_personalizado(args->operacion); // Creo un paquete personalizado con un codop para que IO reconozca lo que le estoy mandando
    t_direccion_fisica* dir;
    
    agregar_int_al_paquete_personalizado(paquete_instruccion, args->proceso->pid); //le mando el pid del proceso
    
    switch (args->operacion)
    {
        case IO_GEN_SLEEP:

            agregar_int_al_paquete_personalizado(paquete_instruccion, args->unidades_de_trabajo);
            break;

        case IO_STDIN_READ:
        case IO_STDOUT_WRITE:

            agregar_int_al_paquete_personalizado(paquete_instruccion, args->registro_tamano); 
            agregar_lista_al_paquete_personalizado(paquete_instruccion, args->direcciones_fisicas, sizeof(t_direccion_fisica));
            
            //libero memoria
            for(int i = 0; i < list_size(args->direcciones_fisicas); i++)
            {   
                dir = list_get(args->direcciones_fisicas, i);
                free(dir);
            }

            list_destroy(args->direcciones_fisicas);
            break;
        case IO_FS_CREATE:
        case IO_FS_DELETE:
            agregar_string_al_paquete_personalizado(paquete_instruccion, args->nombre_archivo);
            break;
        case IO_FS_TRUNCATE:
            agregar_string_al_paquete_personalizado(paquete_instruccion, args->nombre_archivo);
            agregar_int_al_paquete_personalizado(paquete_instruccion, args->registro_tamano);
            break;
        case IO_FS_WRITE:
        case IO_FS_READ:

            agregar_string_al_paquete_personalizado(paquete_instruccion, args->nombre_archivo);
            agregar_int_al_paquete_personalizado(paquete_instruccion, args->registro_tamano);
            agregar_int_al_paquete_personalizado(paquete_instruccion, args->registro_puntero_archivo);

            agregar_lista_al_paquete_personalizado(paquete_instruccion, args->direcciones_fisicas, sizeof(t_direccion_fisica));
            
            //libero memoria
            for(int i = 0; i < list_size(args->direcciones_fisicas); i++)
            {   
                dir = list_get(args->direcciones_fisicas, i);
                free(dir);
            }

            list_destroy(args->direcciones_fisicas);
            
            break;
        default:
            break;
    }
   
    free(args);

    enviar_paquete(paquete_instruccion, socket); // Envio el paquete a través del socket

    eliminar_paquete(paquete_instruccion); // Libero el paquete

    return;
}

interfaz_kernel* verificar_interfaz(char* nombre_interfaz_buscada, op_code tipo_interfaz_buscada)
{
    //Esta funcion verifica q exista la interfaz con el nombre pasado por parametro y que su tipo se corresponda con el pasado por parametro
    interfaz_kernel* aux;

    if(queue_is_empty(cola_interfaces_conectadas) == false){ // Si la cola no está vacía

        pthread_mutex_lock(&mutex_cola_de_interfaces);
        aux  = queue_pop(cola_interfaces_conectadas);
        char* primer_nombre = aux->nombre_interfaz;

        while(strcmp(aux->nombre_interfaz,nombre_interfaz_buscada) !=0) 
        {

            queue_push(cola_interfaces_conectadas, aux);
            aux = queue_pop(cola_interfaces_conectadas);
            if(strcmp(aux->nombre_interfaz, primer_nombre) == 0)
            {
                queue_push(cola_interfaces_conectadas, aux);
                aux->nombre_interfaz = "nada";
                return aux; //No esta la interfaz con ese nombre, retorno null
            }
        }
        queue_push(cola_interfaces_conectadas, aux);
        pthread_mutex_unlock(&mutex_cola_de_interfaces);
        //Si llegamos a este punto es porque se encontro la interfaz
        if(aux->tipo_interfaz == tipo_interfaz_buscada)
        {
            return aux; //La interfaz es del tipo q quiero, la devuelvo

        }else{
            aux->nombre_interfaz = "nada";
            return aux; //No esta la interfaz con ese nombre, retorno null//No coincide el tipo de interfaz, mando null
        }
    }
    
    return aux;
}
//********************************FUNCIONES DE INSTRUCCIONES DE IO**********************

int io_gen_sleep(char* nombre_interfaz, int unidades_de_trabajo, pcb* proceso)
{

    //ante una petición van a esperar una cantidad de unidades de trabajo, cuyo valor va a venir dado en la petición desde el Kernel.
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, GENERICA);

    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->unidades_de_trabajo = unidades_de_trabajo;
    args->proceso = proceso;
    args->operacion = IO_GEN_SLEEP;

    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        //aca hago lo q tengo q hacer, es decir bloquear el proceso y madnarle la isntruccion a la interfaz
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        sem_post(&interfaz->sem_hay_procesos_esperando);

        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);

        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }

}

int io_stdin_read(char* nombre_interfaz, t_list* direcciones_fisicas, uint32_t registro_tamano, pcb* proceso)
{
    
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, STDIN);
    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->direcciones_fisicas = direcciones_fisicas;
    args->registro_tamano = registro_tamano;
    args->proceso = proceso;
    args->operacion = IO_STDIN_READ;

    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }
}

int io_stdout_write(char* nombre_interfaz, t_list* direcciones_fisicas, uint32_t registro_tamano, pcb* proceso)
{
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, STDOUT);
    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->direcciones_fisicas = direcciones_fisicas;
    args->registro_tamano = registro_tamano;
    args->proceso = proceso;
    args->operacion = IO_STDOUT_WRITE;

    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }
}

int io_fs_create(char* nombre_interfaz, char* nombre_archivo, pcb* proceso)
{
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, DIALFS);
    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->proceso = proceso;
    args->nombre_archivo = nombre_archivo;
    args->operacion = IO_FS_CREATE;

    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }
}

int io_fs_delete(char* nombre_interfaz, char* nombre_archivo, pcb* proceso)
{
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, DIALFS);
    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->proceso = proceso;
    args->nombre_archivo = nombre_archivo;
    args->operacion = IO_FS_DELETE;
    
    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }
}

int io_fs_truncate(char* nombre_interfaz, char* nombre_archivo, int registro_tamano, pcb* proceso)
{
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, DIALFS);
    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->nombre_archivo = nombre_archivo;
    args->proceso = proceso;
    args->registro_tamano = registro_tamano;
    args->operacion = IO_FS_TRUNCATE;
    
    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }
}

int io_fs_write(char* nombre_interfaz, char* nombre_archivo, t_list*  registro_direccion, int registro_tamano, int registro_puntero_archivo, pcb* proceso)
{
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, DIALFS);
    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));
    args->proceso = proceso;
    args->nombre_archivo = nombre_archivo;
    args->registro_tamano = registro_tamano;
    args->direcciones_fisicas = registro_direccion;
    args->registro_puntero_archivo = registro_puntero_archivo;
    args->operacion = IO_FS_WRITE;
    
    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else{
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return -1;//no hago nada porq ya lo finalizo yo
    }
}

int io_fs_read(char* nombre_interfaz, char* nombre_archivo, t_list* direcciones_fisicas, int registro_tamano, int registro_puntero_archivo, pcb* proceso)
{
    interfaz_kernel* interfaz = verificar_interfaz(nombre_interfaz, DIALFS);

    argumentos_para_io* args = malloc(sizeof(argumentos_para_io));

    args->nombre_archivo = nombre_archivo;
    args->registro_tamano = registro_tamano;
    args->proceso = proceso;
    args->direcciones_fisicas = direcciones_fisicas;
    args->registro_puntero_archivo = registro_puntero_archivo;
    args->operacion = IO_FS_READ;
    
    if(strcmp(interfaz->nombre_interfaz, nombre_interfaz) == 0) // si no devuelve null es porq encontro la interfaz q queria y es del tipo q queria
    {
        pthread_mutex_lock(&interfaz->mutex_cola);
        queue_push(interfaz->cola_de_espera, args);
        pthread_mutex_unlock(&interfaz->mutex_cola);
        log_info(log_kernel, "PID: <%d> - Bloqueado por: <%s>", proceso->pid, nombre_interfaz);
        pasar_proceso_a_blocked(proceso);
        sem_post(&interfaz->sem_hay_procesos_esperando);
        return -1; //que no haga nada porq ya lo bloquie yo

    }else {
        pasar_proceso_a_exit(proceso, INVALID_INTERFACE);
        return 1;//mando el proceso a exit
    }
}



