#include <string.h>
#include <stdio.h>

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

static void init_ghost(ghost_t *ghost, int x, int y, ghost_color_t color){
    ghost->position.x = x;
    ghost->position.y = y;
    ghost->color = color;
    ghost->curr_direc = UP;
}

static void init_pellet(pellet_t *pellet, int x, int y, file_type_t type){
    pellet->position.x = x;
    pellet->position.y = y;
    pellet->arc_type = type;
    pellet->collected = 0;
}

void init_entities(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets){
    for (int i = 0; i < N; i++){
        for  (int j = 0; j < N; j++){
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