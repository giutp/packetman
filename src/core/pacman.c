#include "../include/pacman.h"

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