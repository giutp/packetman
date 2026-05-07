#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <sys/socket.h>

#include "kermit.h"
#include "interface.h"

#define MIN_W 80
#define MIN_H 24
#define TAM_BUFFER 36

int main(int argc, char **argv){
    // Inicalizações da rede
    if (argc < 2){
        fprintf(stderr, "Necessário informar a placa de rede!\n");
        return -1;
    }

    // Criacao do soquete
    int socket = create_raw_socket(argv[1]);

    uint8_t send_buffer[TAM_BUFFER];                                                // buffer de envia mensagem
    uint8_t rcv_buffer[TAM_BUFFER];                                                 // buffer de receber mensagem
    kermit_t send_msg, rcv_msg;                                                     // struct de mensagens
    uint8_t curr_seq = 0, expected_seq = 0;                                         // sequencia de mensagens


    // Inializações do ncurses
    init_interface();
    // x = colunas (largura da tela)
    // y = linhas (altura da tela)
    // 80x24 = 80 colunas e 24 linhas
    // ncurses ao contrario (igual matriz em C [linhas][colunas] = [y][x])
    int console_width, console_heght;
    getmaxyx(stdscr, console_heght, console_width);                                 // tamanho da janela aberta (terminal)
    // tamanho minimo para o jogo inicializar
    if (console_heght < MIN_H || console_width < MIN_W){
        endwin();
        
        fprintf(stderr, "Erro: janela do terminal abaixo do mínimo %dx%d\n", MIN_W, MIN_H);
        fprintf(stderr, "Atual: %dx%d\n", console_width, console_heght);
        fprintf(stderr, "Aumente o tamanho da janela e tente novamente!\n");
        return -1;
    }

    // proporcao das janelas (game e log)
    int game_w = (console_width * 50) / 80;
    int log_w = console_width - game_w;
    int game_h, log_h;
    game_h = log_h = console_heght;
    // printf("Larguras definidas\n");
    // printf("game: %d\n", game_w);
    // printf("log: %d\n", log_w);

    refresh();
    WINDOW *game_window = newwin(game_h, game_w, 0, 0);
    WINDOW *log_window = newwin(log_h, log_w, 0, 0 + game_w);
    box(game_window, 0, 0);
    box(log_window, 0, 0);

    wrefresh(game_window);
    wrefresh(log_window);

    int flag = 0;
    // Loop game
    // Fluxo do cliente (pelo menos por agora ate eu ver que errei alguma parte)
    // recv -> verificacao (crc+nack/ack) -> download/desenhar -> input -> cria mensagem -> envia -> espera (nack/ack do servidor) -> recv
    while(1){
        // recv (loop de recepcao)
        // + ACK e NACK
        while(1){
            // ========================
            // Etapa de recepcao
            // recebe mensagem do servidor
            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            
            // Mensagem do servidor (marcador de inicio)
            if (is_valid_start_marker(rcv_buffer)){
                // Mensagem valida (CRC certo)
                if(is_valid_crc(rcv_buffer)){
                    deserialize_msg(rcv_buffer+14, &rcv_msg);                       //
                    if (rcv_msg.sequence == expected_seq){
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        int send_bytes = serialize_msg(&send_msg, send_buffer+14);
                        send(socket, send_buffer, send_bytes+14, 0);
                        curr_seq = expected_seq;
                        expected_seq = (expected_seq + 1) % 32;

                        // Desenhar mapa
                        if (rcv_msg.type == VISUALIZACAO){
                            
                        }
                        // Download
                        else if (rcv_msg.type == DADOS){

                        }
                        // Fim do pacote
                        else if (rcv_msg.type == FIM_DA_TRANSMISSAO){
                            break;
                        }
                    }
                    else{
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        int send_bytes = serialize_msg(&send_msg, send_buffer+14);
                        send(socket, send_buffer, send_bytes+14, 0);
                    }
                }
                // crc invalido envia NACK
                else{
                    create_control_msg(&send_msg, NACK, expected_seq);
                    int send_bytes = serialize_msg(&send_msg, send_buffer+14);
                    send(socket, send_buffer, send_bytes+14, 0);
                }
            }
            
            
            if (deserialize_msg(rcv_buffer+14, &rcv_msg)){
                uint8_t size, sequence, type;
            }
        }

        // Monta a struct e verifica o tipo
        // Desenha ou download
        
        // input

        // 


        int key = getch();
        
        switch(key){
        case 'W':
        case 'w':
        case KEY_UP:
            break;
        
        case 'D':
        case 'd':
        case KEY_RIGHT:
            break;

        case 'S':
        case 's':
        case KEY_DOWN:
            break;
        
        case 'A':
        case 'a':
        case KEY_LEFT:
            break;
        
        // Pause
        case 'Q':
        case 'q':
            flag = 1;
            break;
        }

        if (flag) break;

        // cria mensagem


        // send (loop de envio)
        // espera NACK E ACK
    }
    
    endwin();
    return 0;
    
}