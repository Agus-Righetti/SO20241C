#ifndef PAGINACION_H_
#define PAGINACION_H_

#include "m_gestor.h"

// *************** DECLARACIÓN DE FUNCIONES **********
//**************** TODAVIA FALTA ARREGLAR *******************
void ocupar_marco(t_frame* un_frame);
void liberar_marco(t_frame* un_frame);
t_pagina* crear_pagina(t_frame* un_frame);
void agregar_pag_a_tabla (t_proceso* proceso, t_pagina* pagina);
bool hay_lugar_en_memoria(int paginasNecesarias);
t_list* buscar_marcos_libres();
void eliminar_lista(t_list* lista);
void eliminar_algo(void* algo);

#endif