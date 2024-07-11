#include "interfazGENERICA.h"


void realizar_sleep(int tiempo, int pid)
{
    // Implementa la lógica para dormir la cantidad de tiempo especificado
    log_info(log_io, "PID: %d - Operacion: Sleep", pid);
    usleep(tiempo * 1000); // Convertir de milisegundos a microsegundos
    //falta avisarle al kernel que termine de hacer sleep
    avisar_fin_io_a_kernel();
    return;
}
