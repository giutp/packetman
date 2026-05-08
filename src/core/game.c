#include <string.h>     // strtok
#include <stdlib.h>     // rand()
#include <stdint.h>     // uint_8
#include <stdio.h>      // fopen, fclose

#include "game.h"

char symbols[11] = {'P', 'R', 'B', 'G', 'Y', '1', '2', '3', '4', '5', '6'};

// Sorteia uma coordenada dentro dos limites do mapa (NxN)
// Retorna:
// + Coordenada válida [0..N-1]
static coord_t rand_coord(){
    coord_t coord;
    coord.x = rand()%N;
    coord.y = rand()%N;
    return coord;
}

// Inicializa um fantasma, atribuindo os atributos iniciais
static void init_ghost(ghost_t *ghost, int y, int x, ghost_color_t color){
    ghost->position.x = x;
    ghost->position.y = y;
    ghost->color = color;
    ghost->curr_direc = (direction_t)(rand()%4);
}

// Inicializa uma pastilha, atribuindo os atributos iniciais (sem o path e type)
static void init_pellet(pellet_t *pellet, int y, int x){
    pellet->position.x = x;
    pellet->position.y = y;
    pellet->collected = 0;
}

// Calcula se alguma entidade está no campo de visão do Pacman
// Retorna:
// + 0: não está no campo de visão 
// + 1: está no campo de visão
static int is_inside_camera(coord_t e, coord_t st_c, coord_t en_c){
    if (e.y >= st_c.y && e.y < en_c.y && e.x >= st_c.x && e.x < en_c.x) return 1;

    return 0;
}

// Calcula o índice relativo da entidade dentro do buffer da submatriz
// Retorna:
// + índice no vetor
static int index_relative(coord_t e, coord_t st_c, int size_grid){
    return (e.y - st_c.y) * size_grid + (e.x - st_c.x);
}

int read_map(char *filepath, char map[N][N]){
    // TODO: 
    // vale a pena checar se o arquivo eh um .csv?
    // vale a pena checar se esta sendo inserindo o quantidade de entidades permitidas?p
    FILE *arc = fopen(filepath, "r");

    if (!arc) return -1;

    for(int i = 0; i < N; i++){
        char line[128];
        fgets(line, 128, arc);

            for(int j = 0; j < N; j++) {
                char *token = strtok(j == 0 ? line : NULL, ";");
                if (!token) return -1;
                map[i][j] = token[0];
            }
    }

    fclose(arc);
    
    return 0;
}

int randomizer_entities(char map[N][N]){
    if (!read_map("../assets/map_default/default.csv", map))
    return -1;
    
    int i = 0;
    coord_t coord;
    while(i < NUM_ENT){
        coord = rand_coord();
        if (map[coord.y][coord.x] == '0'){
            map[coord.y][coord.x] = symbols[i];
            i++;
        }
    }
    
    return 0;
}

void init_entities(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets){
    for (int i = 0; i < N; i++){
        for (int j = 0; j < N; j++){
            switch (map[i][j]){
                case 'P':
                pacman->position.y = i;
                pacman->position.x = j;
                pacman->pellets = 0;
                pacman->count_mov = 0;
                pacman->radius = 1;
                map[i][j] = '0';
                break;
                case 'R':
                init_ghost(&ghosts[0], i, j, RED);
                map[i][j] = '0';
                break;
                case 'B':
                init_ghost(&ghosts[1], i, j, BLUE);
                map[i][j] = '0';
                break;
                case 'G':
                init_ghost(&ghosts[2], i, j, GREEN);
                map[i][j] = '0';
                break;
            case 'Y':
                init_ghost(&ghosts[3], i, j, YELLOW);
                map[i][j] = '0';
                break;
            case '1':
                init_pellet(&pellets[0], i, j);
                map[i][j] = '0';
                break;
            case '2':
                init_pellet(&pellets[1], i, j);
                map[i][j] = '0';
                break;        
            case '3':
                init_pellet(&pellets[2], i, j);
                map[i][j] = '0';
                break;
            case '4':
                init_pellet(&pellets[3], i, j);
                map[i][j] = '0';
                break;
            case '5':
                init_pellet(&pellets[4], i, j);
                map[i][j] = '0';
                break;
            case '6':
                init_pellet(&pellets[5], i, j);
                map[i][j] = '0';
                break;
            }
        }
    }
}

int check_ghost_pacman_collision(pacman_t *pacman, ghost_t *ghosts){
    for(int i = 0; i < NUM_GHOSTS; i++)
        if((pacman->position.x == ghosts[i].position.x) && (pacman->position.y == ghosts[i].position.y))
            return 1;

    return 0;
}

int check_pellets_pacman_collision(pacman_t *pacman, pellet_t *pellets){
    for(int i = 0; i < NUM_PELLETS; i++)
        if((!pellets[i].collected) && (pacman->position.x == pellets[i].position.x) && (pacman->position.y == pellets[i].position.y))
            return i+1;

    return 0;
}

void build_submatrix(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, uint8_t *buffer){
    // Posições relativas à visão do Pacman
    int square_size = (pacman->radius * 2) + 1;

    coord_t coord_start, coord_end;
    coord_start.y = pacman->position.y - pacman->radius;
    coord_start.x = pacman->position.x - pacman->radius;
    coord_end.y = coord_start.y + square_size;
    coord_end.x = coord_start.x + square_size;

    int i_buffer = 0;
    // Preenche a submatriz com vazio, parede ou chão
    for (int i = coord_start.y; i < coord_end.y; i++){
        for (int j = coord_start.x; j < coord_end.x; j++){
            if (i < 0 || i >= N || j < 0 || j >= N) buffer[i_buffer] = '#';
            else buffer[i_buffer] = map[i][j];
            i_buffer++;
        }
    }

    // Inclui as pastilhas dentro da submatriz
    for (int i = 0; i < NUM_PELLETS; i++){
        if (!pellets[i].collected && is_inside_camera(pellets[i].position, coord_start, coord_end)){
            i_buffer = index_relative(pellets[i].position, coord_start, square_size);
            buffer[i_buffer] = '0' + i;
        }
    }

    // Inclui os fantasmas dentro da submatriz
    for (int i = 0; i < NUM_GHOSTS; i++){
        if (is_inside_camera(ghosts[i].position, coord_start, coord_end)){
            i_buffer = index_relative(ghosts[i].position, coord_start, square_size);
            switch (ghosts[i].color){
            case RED:
                buffer[i_buffer] = 'R';
                break;
            
            case BLUE:
                buffer[i_buffer] = 'B';
                break;

            case GREEN:
                buffer[i_buffer] = 'G';
                break;

            case YELLOW:
                buffer[i_buffer] = 'Y';
                break;
            }
        }
    }

    // Inclui o Pacman na submatriz (centro)
    i_buffer = pacman->radius * square_size + pacman->radius;
    buffer[i_buffer] = 'P';
}

int update_world(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, direction_t direction){
    move_pacman(map, pacman, direction);
    if (check_ghost_pacman_collision(pacman, ghosts)) return -1;

    int id_pellet = check_pellets_pacman_collision(pacman, pellets);
    if (id_pellet != 0){
        pellets[id_pellet-1].collected = 1;
        pacman->pellets++;
        if (pacman->pellets >= 6) return 10;

        return id_pellet;
    }

    move_ghosts(map, ghosts);
    if (check_ghost_pacman_collision(pacman, ghosts)) return -1;

    return 0;
}