#ifndef GHOSTS_H
#define GHOSTS_H

#include "utils.h"

// Quantidade de fantasmas
#define NUM_GHOSTS 4

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

// IA de movimentação dos fantasmsas, seguindo as seguintes regras:
// + Vermelho: regra da mão esquerda
// + Azul: regra da mão direita
// + Verde: alterna entre mão esquerda e direita
// + Amarelo: aleatório
void move_ghosts(char map[N][N], ghost_t *ghosts);

#endif // GHOSTS_H