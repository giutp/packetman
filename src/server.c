#include <stdlib.h>
#include <stdio.h>

#include "kermit.h"
#include "game.h"

int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "Necessario informar interface da placa de rede\n");
        exit(-1);
    }

    // Inicialização variáveis do jogo
    char map[N][N];
    pacman_t pacman;
    ghost_t ghosts[4];
    pellet_t pellets[6];

    // TODO: colocar mensagem de erro
    if (argc == 2){
        randomizer_entities(map);
    }

    // TODO: colocar mensagem de erro
    else {
        FILE *f = fopen(argv[2], "r");
        read_map(f, map);
    }

    init_entities(map, &pacman, ghosts, pellets);

    int socket = create_raw_socket(argv[1]);
    kermit_t rcv_msg, send_msg;
    int curr_seq = 0, expected_seq = 0;

    while(1){

        // TODO: incluir função do crc
        int error;

        if(error){
            create_control_msg(&send_msg, NACK, &send_msg);
        }

        direction_t direction;
        switch(rcv_msg.size_sequence_type){
            case CIMA: 
                direction = UP;
                break;
            case ESQUERDA:
                direction = LEFT;
                break;
            case DIREITA:
                direction = RIGHT;
                break; 
            case BAIXO:
                direction = DOWN;
                break;    
        }

        update_world(map, &pacman, ghosts, pellets, direction);
    }

    return 0;
}