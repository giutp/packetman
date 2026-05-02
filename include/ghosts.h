#ifndef GHOSTS_H
#define GHOSTS_H

#include "utils.h"


// Cor dos fantasmas
typedef enum {
    RED,
    BLUE,
    GREEN,
    YELLOW
} ghost_color_t;

// Atributos dos fantasmas
typedef struct ghost {
    coord_t position;               // coordenada cartesiana
    ghost_color_t color;            // cor do fanstamas
    direction_t prev_direc;         // direção anterior
    direction_t curr_direc;         // direção atual
} ghost_t;


#endif 