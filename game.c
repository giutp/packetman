#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>             // uint8_t

#include "game.h"

char symbols[11] = {'P', 'R', 'B', 'G', 'Y', '1', '2', '3', '4', '5', '6'};

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

coord_t rand_coord(){
    coord_t coord;
    coord.x = rand()%N;
    coord.y = rand()%N;
    return coord;
}

int is_inside_P(coord_t coord){
    int x = coord.x;
    int y = coord.y;
    return (x >= 4 && x <= 9) && (y >= 7 && y <= 11);
}

int is_inside_R(coord_t coord){
    int x = coord.x;
    int y = coord.y;
    return (x >= 13 && x <= 24) && (y >= 7 && y <= 11);
}

int use_default_map(char map[N][N]){
    if (!read_map("default.csv", map))
        return -1;

    int i = 0;
    coord_t coord;
    while(i<11){
        coord = rand_coord();
        if(!is_inside_P(coord) || !is_inside_R(coord) || map[coord.y][coord.x] == '0'){
            map[coord.y][coord.x] = symbols[i];
            i++;
        }
    }
    
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

void bind_pellets_files(pellet_t *pellets){
    // O caminho de onde vai esta os arquivos ainda nao definir, entao pode mudar a atribuicao
    pellets[0].filepath = "1.txt";
    pellets[1].filepath = "2.txt";
    pellets[2].filepath = "3.jpg";
    pellets[3].filepath = "4.jpg";
    pellets[4].filepath = "5.mp4";
    pellets[5].filepath = "6.mp4";
}

static int check_wall_free(char map[N][N], coord_t ghost_coord, direction_t new_direction, coord_t *new_coord){
    switch (new_direction){
    case UP:
        ghost_coord.y--;
        break;
    
    case DOWN:
        ghost_coord.y++;
        break;

    case LEFT:
        ghost_coord.x--;
        break;

    case RIGHT:
        ghost_coord.x++;
        break;
    }

    if (((ghost_coord.y >= 0) && (ghost_coord.y < N)) 
        && ((ghost_coord.x >= 0) && (ghost_coord.x < N)) 
        && (map[ghost_coord.y][ghost_coord.x] != 'X')){
            *new_coord = ghost_coord;
            return 0;
        }

    return -1;
}

void move_ghosts(char map[N][N], ghost_t *ghosts, int round){
    for (int i = 0; i < 4; i++){
        coord_t new_cord = ghosts[i].position;

        // Todo fantasma tem um array de prioridade de movimento
        // TODO: Da para simplificar esse codigo com mais funcao auxiliar (depois faco)
        switch (ghosts[i].color){
        // Mão esquerda
        case RED:{  
            direction_t direction_relative_priority[4] = {
                (ghosts[i].curr_direc + LEFT) % 4, 
                (ghosts[i].curr_direc + UP) % 4, 
                (ghosts[i].curr_direc + RIGHT) % 4, 
                (ghosts[i].curr_direc + DOWN) % 4
            };

            int j = 0;
            while(j < 4 && check_wall_free(map, ghosts[i].position, direction_relative_priority[j], &new_cord) != 0) j++;

            if (j < 4){
                ghosts[i].prev_direc = ghosts[i].curr_direc;
                ghosts[i].curr_direc = direction_relative_priority[j];
                ghosts[i].position = new_cord;
            }

            break;
        }
        // Mão direita
        case BLUE:{  
            direction_t direction_relative_priority[4] = {
                (ghosts[i].curr_direc + RIGHT) % 4,
                (ghosts[i].curr_direc + UP) % 4, 
                (ghosts[i].curr_direc + LEFT) % 4, 
                (ghosts[i].curr_direc + DOWN) % 4
            };

            int j = 0;
            while(j < 4 && check_wall_free(map, ghosts[i].position, direction_relative_priority[j], &new_cord) != 0) j++;

            if (j < 4){
                ghosts[i].prev_direc = ghosts[i].curr_direc;
                ghosts[i].curr_direc = direction_relative_priority[j];
                ghosts[i].position = new_cord;
            }

            break;
        }
        // Alterna entre mão direita e esquerda usando paridade do grid
        case GREEN:{
            // Mão esquerda
            if(round%2 == 1){
                direction_t direction_relative_priority[4] = {
                    (ghosts[i].curr_direc + LEFT) % 4, 
                    (ghosts[i].curr_direc + UP) % 4, 
                    (ghosts[i].curr_direc + RIGHT) % 4, 
                    (ghosts[i].curr_direc + DOWN) % 4
                };

                int j = 0;
                while(j < 4 && check_wall_free(map, ghosts[i].position, direction_relative_priority[j], &new_cord) != 0) j++;

                if (j < 4){
                    ghosts[i].prev_direc = ghosts[i].curr_direc;
                    ghosts[i].curr_direc = direction_relative_priority[j];
                    ghosts[i].position = new_cord;
                }
            }
            // Mão direita
            else if(round%2 == 0){
                direction_t direction_relative_priority[4] = {
                    (ghosts[i].curr_direc + RIGHT) % 4,
                    (ghosts[i].curr_direc + UP) % 4, 
                    (ghosts[i].curr_direc + LEFT) % 4, 
                    (ghosts[i].curr_direc + DOWN) % 4
                };

                int j = 0;
                while(j < 4 && check_wall_free(map, ghosts[i].position, direction_relative_priority[j], &new_cord) != 0) j++;

                if (j < 4){
                    ghosts[i].prev_direc = ghosts[i].curr_direc;
                    ghosts[i].curr_direc = direction_relative_priority[j];
                    ghosts[i].position = new_cord;
                }
            }


            break;
        }

        // Aleatorio
        case YELLOW:{
            direction_t direction_relative_priority[4] = {0, 1, 2, 3};
            // Embaralha o array de prioridade
            for (int j = 3; j > 0; j--){
                int swap = rand()%(j+1);
                int tmp = direction_relative_priority[j];
                direction_relative_priority[j] = direction_relative_priority[swap];
                direction_relative_priority[swap] = tmp;
            }
            // Calcula as direcoes relativas com o array embralhado
            for (int j = 0; j < 4; j++)
                direction_relative_priority[j] = (ghosts[i].curr_direc + direction_relative_priority[j])%4;

            int j = 0;
            while(j < 4 && check_wall_free(map, ghosts[i].position, direction_relative_priority[j], &new_cord) != 0) j++;

            if (j < 4){
                ghosts[i].prev_direc = ghosts[i].curr_direc;
                ghosts[i].curr_direc = direction_relative_priority[j];
                ghosts[i].position = new_cord;
            }

            break;
            }
        }
    }
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

// essa funçaão vem de depois dos fantasmas
int update_pacman(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, direction_t direction){
    if(map[pacman->position.y][pacman->position.x] = 'P')
        map[pacman->position.y][pacman->position.x] = '0';

    move_pacman(map, pacman, direction);

    if(check_ghost_pacman_collision(pacman, ghosts)){
        return -1; // morto
    }

    int pellet = check_pellet_collision(pacman, pellets);

    if(pellet){
        pellets[pellet].collected = 1;
        pacman->pellets++;
    }

    map[pacman->position.y][pacman->position.x] = 'P';

    return pellet; // retorna pellet coletada, 0 se não coletou
}

void get_visible_map(char map[N][N], pacman_t *pacman, char visible_map[N][N]){
    int y_max = pacman->position.y + pacman->radius;
    int y_min = pacman->position.y - pacman->radius;
    int x_max = pacman->position.x + pacman->radius;
    int x_min = pacman->position.x - pacman->radius;
    
    for(int i = 0; i < N; i++){
        for(int j = 0; j < N; j++){
            if (i >= y_min && i <= y_max && j >= x_min && j <= x_max)
                visible_map[i][j] = map[i][j];
            else
                visible_map[i][j] = '?';
        }
    }
}