#ifndef UTILS_H
#define UTILS_H

// Tamanho do mapa (matriz) do jogo
#define N 40

// Direção de movimento das entidades (pacman e fantasmas)
typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} direction_t;

// Coordenada cartesiana (matriz) das entidades
typedef struct coord {
    int x;
    int y;
} coord_t;



#endif