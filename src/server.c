#include <stdlib.h>
#include <stdio.h>

#include "kermit.h"
#include "game.h"

#define TAM_BUFFER 36

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
    direction_t direction;

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
    kermit_t rcv_msg, send_msg;                                                     // struct de mensagens
    int curr_seq = 0, expected_seq = 0;                                             // sequencia de mensagens
    uint8_t send_buffer[TAM_BUFFER];                                                // buffer de envia mensagem
    uint8_t rcv_buffer[TAM_BUFFER];                                                 // buffer de receber mensagem

    while(1){

        while(1){

            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            
            // TODO: incluir função do crc
            int error;

            if(error){
                create_control_msg(&send_msg, NACK, &send_msg);
            }

            switch(rcv_msg.type){
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
        }

        int game_response = update_world(map, &pacman, ghosts, pellets, direction);

        // Envio de resposta
        create_control_msg(&send_msg, direction, curr_seq);
        int send_byes = serialize_msg(&send_msg, send_buffer);
        send(socket, send_buffer, send_buffer, 0);

    }

    return 0;
}