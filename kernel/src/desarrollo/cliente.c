#include "../include/general.h"

//********* DESARROLLO DE LA CONEXIÓN KERNEL - CPU *****
int conexion_a_cpu(t_log* log_kernel, kernel_config* config_kernel){
    int conexion_kernel_cpu = crear_conexion(config_kernel->ip_cpu, config_kernel->puerto_cpu_dispatch);
    if(conexion_kernel_cpu == -1){
        log_info(log_kernel , "ERROR: No se pudo establecer la conexión Kernel-CPU");
        exit(1);
    }
    log_info(log_kernel , "Conexión con el servidor CPU creada con éxito");
    enviar_mensaje("Hola CPU, soy Kernel",conexion_kernel_cpu);
    return conexion_kernel_cpu;
}

//********* DESARROLLO DE LA INTERRUPCION KERNEL - CPU *****
int interrupcion_a_cpu(t_log* log_kernel, kernel_config* config_kernel){
    int interrupcion_kernel_cpu = crear_conexion(config_kernel->ip_cpu, config_kernel->puerto_cpu_interrupt);
    if(interrupcion_kernel_cpu == -1){
        log_info(log_kernel , "ERROR: No se pudo establecer la interrupcion Kernel-CPU");
        exit(1);
    }
    log_info(log_kernel , "Interrupcion de Kernel a CPU");
    enviar_mensaje("Interrupcion de Kernel a CPU",interrupcion_kernel_cpu);
    return interrupcion_kernel_cpu;
}

//********* DESARROLLO DE LA CONEXIÓN KERNEL - MEMORIA *****
 int conexion_a_memoria(t_log* log_kernel, kernel_config* config_kernel){
    int conexion_kernel_memoria = crear_conexion(config_kernel->ip_memoria, config_kernel->puerto_memoria);
    if(conexion_kernel_memoria == -1){
        log_info(log_kernel , "ERROR: No se pudo establecer la conexión Kernel-Memoria");
        exit(1);
    }
    log_info(log_kernel , "Conexión con el servidor Memoria creada con éxito");
    enviar_mensaje("Hola Memoria, soy Kernel",conexion_kernel_memoria);
    return conexion_kernel_memoria;
}