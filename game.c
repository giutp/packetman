#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>             // uint8_t

#include "game.h"

int read_map(char *filepath, char map[N][N]){
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

static void init_ghost(ghost_t *ghost, int y, int x, ghost_color_t color){
    ghost->position.x = x;
    ghost->position.y = y;
    ghost->color = color;
    ghost->curr_direc = (direction_t)(rand()%4);
}

static void init_pellet(pellet_t *pellet, int y, int x, file_type_t type){
    pellet->position.x = x;
    pellet->position.y = y;
    pellet->arc_type = type;
    pellet->collected = 0;
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
                break;
            case 'R':
                init_ghost(&ghosts[0], i, j, RED);
                break;
            case 'B':
                init_ghost(&ghosts[1], i, j, BLUE);
                break;
            case 'G':
                init_ghost(&ghosts[2], i, j, GREEN);
                break;
            case 'Y':
                init_ghost(&ghosts[3], i, j, YELLOW);
                break;
            case '1':
                init_pellet(&pellets[0], i, j, FILE_TXT);
                break;
            case '2':
                init_pellet(&pellets[1], i, j, FILE_TXT);
                break;        
            case '3':
                init_pellet(&pellets[2], i, j, FILE_JPG);
                break;
            case '4':
                init_pellet(&pellets[3], i, j, FILE_JPG);
                break;
            case '5':
                init_pellet(&pellets[4], i, j, FILE_MP4);
                break;
            case '6':
                init_pellet(&pellets[5], i, j, FILE_MP4);
                break;
            }
        }
    }
}

/*
    Essas funções não foram testadas ainda
    A ordem de ações é atualizar fantasmas -> atualizar pacman
*/

// muda a posição do pacman com base no input de movimento
void mov_pacman(char map[N][N], pacman_t *pacman, uint8_t movtype){
    if((movtype == 'w') && (pacman->position.y > 0) && (map[pacman->position.x][pacman->position.y - 1] != 'X'))
        pacman->position.y--;
    else if((movtype == 'd') && (pacman->position.x < N-1) && (map[pacman->position.x + 1][pacman->position.y] != 'X'))
        pacman->position.x++;
    else if((movtype == 's') && (pacman->position.y < N-1) && (map[pacman->position.x][pacman->position.y + 1] != 'X'))
        pacman->position.y++;
    else if((movtype == 'a') && (pacman->position.x > 0) && (map[pacman->position.x - 1][pacman->position.y] != 'X'))
        pacman->position.x--;
}

// verifica e retorna se houve colisão do pacman com algum fantasma
int check_ghost_pacman_collision(pacman_t *pacman, ghost_t *ghosts){
    for(int i = 0; i < 4; i++)
        if((pacman->position.x == ghosts[i].position.x) && (pacman->position.y == ghosts[i].position.y))
            return 1;

    return 0;
}

// verifica e retorna se houve colisão do pacman com alguma 
int check_pellet_collision(pacman_t *pacman, pellet_t *pellets){
    for(int i = 0; i < 6; i++)
        if((!pellets[i].collected) && (pacman->position.x == pellets[i].position.x) && (pacman->position.y == pellets[i].position.y))
            return i+1;

    return 0;

}

int update_pacman(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, uint8_t movtype){
    map[pacman->position.x][pacman->position.y] = '0';
    mov_pacman(map, pacman, movtype);

    if(check_ghost_pacman_collision(pacman, ghosts)){
        return -1; // morto
    }

    int pellet = check_pellet_collision(pacman, pellets);

    if(pellet)
        pellets[pellet].collected = 1;

    map[pacman->position.x][pacman->position.y] = 'P';

    return pellet; // retorna pellet coletada, 0 se não coletou
}

void move_pacman(char map[N][N], pacman_t *pacman, direction_t direction){
    coord_t new_coord;
    new_coord.x = pacman->position.x;
    new_coord.y = pacman->position.y;

    switch (direction){
    case UP:
        new_coord.y--;
        break;
    case DOWN:
        new_coord.y++;
        break;
    case LEFT:
        new_coord.x--;
        break;
    case RIGHT:
        new_coord.x++;
        break;
    }

    if (((new_coord.y >= 0) && (new_coord.y < N)) 
        && ((new_coord.x >= 0) && (new_coord.x < N)) 
        && (map[new_coord.y][new_coord.x] != 'X')){
        pacman->position.x = new_coord.x;
        pacman->position.y = new_coord.y;

        pacman->count_mov++;
        if (pacman->count_mov % 5 == 0) pacman->radius++;
    }
}