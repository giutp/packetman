#include <string.h>     // strtok
#include <stdlib.h>     // rand()
#include <stdio.h>      // fopen, fclose

#include "game.h"

char symbols[11] = {'P', 'R', 'B', 'G', 'Y', '1', '2', '3', '4', '5', '6'};

// Sorteia uma coordenada dentro dos limites do mapa (NxN)
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
    while(i < 11){
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

/*
    Essas funções não foram testadas ainda
    A ordem de ações é atualizar fantasmas -> atualizar pacman
*/

int check_ghost_pacman_collision(pacman_t *pacman, ghost_t *ghosts){
    for(int i = 0; i < 4; i++)
        if((pacman->position.x == ghosts[i].position.x) && (pacman->position.y == ghosts[i].position.y))
            return 1;

    return 0;
}

int check_pellets_pacman_collision(pacman_t *pacman, pellet_t *pellets){
    for(int i = 0; i < 6; i++)
        if((!pellets[i].collected) && (pacman->position.x == pellets[i].position.x) && (pacman->position.y == pellets[i].position.y))
            return i+1;

    return 0;
}

// essa funçaão vem de depois dos fantasmas
int update_pacman(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, direction_t direction){
    if(map[pacman->position.y][pacman->position.x] == 'P')
        map[pacman->position.y][pacman->position.x] = '0';

    move_pacman(map, pacman, direction);

    if(check_ghost_pacman_collision(pacman, ghosts)){
        return -1; // morto
    }

    int pellet = check_pellets_pacman_collision(pacman, pellets);

    if(pellet){
        pellets[pellet].collected = 1;
        pacman->pellets++;
    }

    map[pacman->position.y][pacman->position.x] = 'P';

    return pellet; // retorna pellet coletada, 0 se não coletou
}