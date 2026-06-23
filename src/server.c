#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include "kermit.h"
#include "game.h"

#define MAX_DATA 31

int main(int argc, char **argv){
    srand(time(NULL));

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
        printf("Mapa padrão e entidades aleatórias\n");
        randomizer_entities(map);
    }

    // TODO: colocar mensagem de erro
    else 
        read_map(argv[2], map);

    printf("Criado mapa\n");

    init_entities(map, &pacman, ghosts, pellets);

    bind_pellets_files(pellets);

    printf("Inicializado entidades\n");

    // Inicialização variáveis de rede
    int socket = create_raw_socket(argv[1]);
    kermit_t rcv_msg, send_msg;                                                     // struct de mensagens
    int curr_seq = 0, expected_seq = 0;                                             // sequencia de mensagens
    uint8_t send_buffer[TAM_BUFFER];                                                // buffer de envia mensagem
    uint8_t rcv_buffer[TAM_BUFFER];                                                 // buffer de receber mensagem
    int send_bytes;

    // Ethernet
    uint8_t mac_orig[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t mac_dest[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t eth_type   = 0x8888;
    memcpy(send_buffer, mac_dest, 6);
    memcpy(send_buffer+6, mac_orig, 6);
    memcpy(send_buffer+12, &eth_type, 2);

    // inicializado com valor para ser ignorado
    int game_message_type = -1;
    int game_response = -10;
    int flag_quit = 0;
    FILE *pellet_file;     

    while(1){
        if(game_response >= 1 && game_response <= 6){
            printf("Pastilha pega: %d\n", game_response);

            pellet_file = fopen(pellets[game_response-1].filepath, "r");
            game_message_type = pellets[game_response-1].arc_type;

            // Fluxo de nao enviar arquivo
            if (!pellet_file){
                printf("Arquivo inexistente. Nao sera enviando para cliente DADOS\n");
                create_control_msg(&send_msg, NFILE, curr_seq);
                send_bytes = serialize_msg(&send_msg, send_buffer+14);
                send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);
            }
            else{
                fseek(pellet_file, 0, SEEK_END);
                uint64_t size_arc = ftell(pellet_file);
                char data[32];
                snprintf(data, sizeof(data), "%d-%ld", game_response, size_arc);
                printf("Tamanho do arquivo a ser enviado: %lu\n", size_arc);

                // Envio do identificador da pastilha
                create_data_msg(&send_msg, strlen(data)+1, game_message_type, curr_seq, (uint8_t *)data);
                send_bytes = serialize_msg(&send_msg, send_buffer+14);
                send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

                if(rcv_msg.type == ERROS){
                    printf("Cliente nao conseguiu criar arquivo, download nao sera feito\n");

                    create_control_msg(&send_msg, ACK, curr_seq);
                    send_bytes = serialize_msg(&send_msg, send_buffer+14);
                    send(socket, send_buffer, send_bytes+14, 0);
                }
                else{
                uint8_t file_buffer[MAX_DATA];
                size_t bytes_read;

                // envio dos dados parciais
                rewind(pellet_file);
                while ((bytes_read = fread(file_buffer, 1, MAX_DATA, pellet_file)) > 0) {
                    printf("[DEBUG SERVER] fread leu %zu bytes do arquivo.\n", bytes_read);

                    create_data_msg( &send_msg, bytes_read, DADOS, curr_seq, file_buffer);
                    send_bytes = serialize_msg(&send_msg, send_buffer+14);
                    send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);
                }

                create_control_msg(&send_msg, FIM_DA_TRANSMISSAO, curr_seq);
                send_bytes = serialize_msg(&send_msg, send_buffer+14);
                send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

                }
                fclose(pellet_file);
            }

            if (pacman.pellets == 6){
                create_control_msg(&send_msg, VITORIA, curr_seq);
                send_bytes = serialize_msg(&send_msg, send_buffer+14);
                send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

                break;
            }
        }
        else if(game_response == -1){
            game_message_type = DERROTA;

            create_control_msg(&send_msg, game_message_type, curr_seq);
            send_bytes = serialize_msg(&send_msg, send_buffer+14);
            send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);
            break;
        }

        printf("Início envio servidor\n");

        // Envio do raio
        create_data_msg(&send_msg, (uint8_t)sizeof(uint32_t), RAIO, curr_seq, (uint8_t *)&pacman.radius);
        printf("Raio enviado: %d\n", pacman.radius);
        send_bytes = serialize_msg(&send_msg, send_buffer+14);
        printf("Bytes serializados: %d\n", send_bytes);
        send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);
        
        printf("Mensagem de raio enviada\n");

        // Envio da área visível
        int side = 2 * pacman.radius + 1;
        int total = side * side;

        uint8_t *submatrix_buffer = malloc(total*sizeof(uint8_t));
        build_submatrix(map, &pacman, ghosts, pellets, submatrix_buffer);


        printf("Submatriz construida\n");
        for (int i = 0; i < side; i++){
            for (int j = 0; j < side; j++){
                printf("%c ", submatrix_buffer[(i*side)+j]);
            }
            printf("\n");
        }


        for (int i = 0; i < side; i++){
            int offset_line = i * side;
            int offset = 0;

            while (offset < side){
                int chunk_size = side - offset < MAX_DATA ? side - offset : MAX_DATA;

                create_data_msg(&send_msg, chunk_size, VISUALIZACAO, curr_seq, submatrix_buffer+offset+offset_line);
                send_bytes = serialize_msg(&send_msg, send_buffer+14);
                send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

                offset += chunk_size;
                printf("Chunk de linha enviada\n");
            }
            
        }

        printf("Mapa enviado\n");

        free(submatrix_buffer);

        create_control_msg(&send_msg, FIM_DA_TRANSMISSAO, curr_seq);
        send_bytes = serialize_msg(&send_msg, send_buffer+14);
        send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

        while(1){

            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            
            if (is_valid_protocol(rcv_buffer+14)){
                // CRC válido
                if(is_valid_crc(rcv_buffer+14)){
                    deserialize_msg(rcv_buffer+14, &rcv_msg);
                    // Mensagem recebida é a esperada
                    if (rcv_msg.sequence == expected_seq){
                        printf("Enviando ACK\n");                       
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        send_bytes = serialize_msg(&send_msg, send_buffer+14); 
                        send(socket, send_buffer, send_bytes+14, 0);               
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
                            case SAIR:
                                flag_quit = 1;
                                break;
                        }

                        break;
                    }
                    // Mensagem repetida
                    else{
                        printf("Enviando ACK de mensagem repetida\n");
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        send_bytes = serialize_msg(&send_msg, send_buffer+14);
                        send(socket, send_buffer, send_bytes+14, 0);
                    }
                    free(rcv_msg.data);
                }
                // CRC inválido
                else{
                    printf("Enviando NACK\n");
                    create_control_msg(&send_msg, NACK, expected_seq);
                    send_bytes = serialize_msg(&send_msg, send_buffer+14);
                    send(socket, send_buffer, send_bytes, 0);
                }
            }
        }

        if (flag_quit) break;

        game_response = update_world(map, &pacman, ghosts, pellets, direction);
    }

    return 0;
}