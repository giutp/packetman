#include <stdlib.h>     // rand()
#include "ghosts.h"

// Checa se a nova coordenada é válida (dentro do mapa e não-parede)
// Retorna:
// + -1: posição estrapola o limite do mapa ou é parede ('X')
// +  0: coordenada válida
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

void move_ghosts(char map[N][N], ghost_t *ghosts){
    for (int i = 0; i < 4; i++){
        coord_t new_cord = ghosts[i].position;

        // Todo fantasma tem um array de prioridade de movimento (seguindo suas regras)
        // Todo fantasma checa pelo grau de prioridade se a direção desejada é válida
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
            if((ghosts[i].position.x + ghosts[i].position.y) % 2 == 1){
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
            else if((ghosts[i].position.x + ghosts[i].position.y) % 2 == 0){
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