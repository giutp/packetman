#ifndef PACMAN_H
#define PACMAN_H

#include "utils.h"

// Atributos do Pacman
typedef struct pacman {
    coord_t position;               // coordenada cartesiana
    int radius;                     // raio de visão
    int count_mov;                  // contador de passos
    int pellets;                    // contador de pastilhas coletadas
} pacman_t;

// Move o Pacman na direção desejada se for uma posição válida no mapa
void move_pacman(char map[N][N], pacman_t *pacman, direction_t direction);

#endif