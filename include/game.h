#ifndef GAME_H
#define GAME_H

#include "utils.h"
#include "pacman.h"
#include "ghosts.h"
#include "pellets.h"

// Ler um arquivo .csv e monta o mapa do jogo
// Retorna:
// + -1: falha em abrir o arquivo (mapa)
// +  0: mapa inicializado com sucesso
//
int read_map(char *filepath, char map[N][N]);

// Inicializa as entidades em lugares aleatórios válidos
// Retorna:
// + -1: o mapa default não existe
// + 0: entidades inicializadas com sucesso
int randomizer_entities(char map[N][N]);

// Inicializa as entidades do jogo (Pacman, fantasmas e pastilhas)
// Atribui as coordenadas iniciais com base no mapa (map[N][N])
// Limpa todos os simbolos após inicialização
void init_entities(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets);

int update_pacman(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, direction_t direction);

// Checa se o Pacman e algum fantasma se cruzaram
// Retorna:
// + 0: não se cruzaram
// + 1: se cruzaram
int check_ghost_pacman_collision(pacman_t *pacman, ghost_t *ghosts);

// Checa se o Pacman encontrou alguma pastilha
// Retorna:
// +  0: não encontrou
// + id: o id da pastilha encontrada (1..6)
int check_pellets_pacman_collision(pacman_t *pacman, pellet_t *pellets);

#endif