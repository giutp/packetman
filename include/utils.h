#ifndef UTILS_H
#define UTILS_H

// Tamanho do mapa (matriz) do jogo
#define N 40

#include "kermit.h"

// Direção de movimento das entidades (pacman e fantasmas)
typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} direction_t;

// Coordenada cartesiana (posição da matriz) das entidades
typedef struct coord {
    int x;
    int y;
} coord_t;

// Converte o valor do enum para a string correspondente
// Retorna:
// + String correspondente ao type
char *enum_to_string(types_t type);

#endif