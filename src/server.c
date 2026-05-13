#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "kermit.h"
#include "game.h"

#define TAM_BUFFER 36
#define MAX_DATA 32

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
        int flag_ntw = 0;
        while(1){

            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            
            if (is_valid_protocol(rcv_buffer)){
                // CRC válido
                if(is_valid_crc(rcv_buffer)){
                    deserialize_msg(rcv_buffer, &rcv_msg);
                    // Mensagem recebida é a esperada
                    if (rcv_msg.sequence == expected_seq){                       
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        int send_bytes = serialize_msg(&send_msg, send_buffer); 
                        send(socket, send_buffer, send_bytes, 0);               
                        expected_seq = (expected_seq + 1) % 32;

                        // Precisa de flag se sempre é direção?
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
                    // Mensagem repetida
                    else{
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        int send_bytes = serialize_msg(&send_msg, send_buffer);
                        send(socket, send_buffer, send_bytes, 0);
                    }
                    free(rcv_msg.data);
                }
                // CRC inválido
                else{
                    create_control_msg(&send_msg, NACK, expected_seq);
                    int send_bytes = serialize_msg(&send_msg, send_buffer);
                    send(socket, send_buffer, send_bytes, 0);
                }
            }
        }

        int game_response = update_world(map, &pacman, ghosts, pellets, direction);

        FILE *pellet_file;
        switch (game_response)
        {
            case 1:
                pellet_file = fopen("../assets/files/1.txt", "r");
                break;
            case 2:
                pellet_file = fopen("../assets/files/2.txt", "r");
                break;
            case 3:
                pellet_file = fopen("../assets/files/3.jpg", "r");
                break;
            case 4:
                pellet_file = fopen("../assets/files/4.jpg", "r");
                break;
            case 5:
                pellet_file = fopen("../assets/files/5.mp4", "r");
                break;
            case 6:
                pellet_file = fopen("../assets/files/6.mp4", "r");
                break;
            default:
                break;
        }

        if(game_response >= 1 && game_response <= 6){
            // Envio do identificador da pastilha
            create_control_msg(&send_msg, direction, curr_seq);
            int send_bytes = serialize_msg(&send_msg, send_buffer);
            send(socket, send_buffer, send_bytes, 0);

            uint8_t file_buffer[MAX_DATA];
            size_t bytes_read;

            // envio dos dados parciais
            while ((bytes_read = fread(file_buffer, 1, MAX_DATA, pellet_file)) > 0) {
                create_data_msg( &send_msg, DADOS, curr_seq, file_buffer, bytes_read);
                int send_bytes = serialize_msg(&send_msg, send_buffer);
                send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);
            }
        }

        // envio do mapa

        // Envio do identificado de visualização
        create_control_msg(&send_msg, VISUALIZACAO, curr_seq);
        int send_bytes = serialize_msg(&send_msg, send_buffer);
        send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

        // Envio do raio
        create_control_msg(&send_msg, RAIO, curr_seq);
        int send_bytes = serialize_msg(&send_msg, send_buffer);
        send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

        // Envio da área visível
        int side = 2 * pacman.radius + 1;
        int total = side * side;

        uint8_t *submatrix_buffer = malloc(total*sizeof(uint8_t));
        build_submatrix(map, &pacman, ghosts, pellets, submatrix_buffer);
       
        int offset = 0;

        while (offset < total) {
            int chunk_size = MAX_DATA;
            if (total - offset < MAX_DATA)
                chunk_size = total - offset;

            create_data_msg(&send_msg, DADOS, curr_seq, submatrix_buffer + offset, chunk_size);
            int send_bytes = serialize_msg(&send_msg, send_buffer);
            send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

            offset += chunk_size;
        }

        free(submatrix_buffer);

        create_control_msg(&send_msg, FIM_DA_TRANSMISSAO, curr_seq);
        int send_bytes = serialize_msg(&send_msg, send_buffer);
        send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);
    }

    return 0;
}