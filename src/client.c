#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <string.h>

#include "kermit.h"
#include "interface.h"
#include "utils.h"

#define MIN_W 80
#define MIN_H 24
#define TAM_BUFFER 36

// Cria o arquivo mandado pelo servidor para download
// Salva em path_arc o caminho do arquivo aberto
// Retorna:
// + Ponteiro para o arquivo criado
static FILE *create_arc(char *name_size_arc, char *path_arc){
    FILE *arc;
    // size_arc ainda esta inutil, mas sera usado para log
    int size_arc, range;
    char *ext, name_arc[16], *del;
    char *path_download = "../assets/download/";

    del = strchr(name_size_arc, '-');
    
    if (del != NULL){
        *del = '\0';
        range = atoi(name_size_arc);

        switch (range){
        case 1:
        case 2:
            ext = ".txt";
            break;
        case 3:
        case 4:
            ext = ".jpg";
            break;
        case 5:
        case 6:
            ext = ".mp4";
            break;
        }
        sprintf(name_arc, "%s%s", name_size_arc, ext);
        sprintf(path_arc, "%s%s", path_download, name_arc);
        size_arc = atoi(del + 1);

        arc = fopen(path_arc, "wb");
        return arc;
    }

    return NULL;
}


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
        int flag_ntw = 0;
        int size_line_map = 0, line = 1;
        char path_arc[1024];
        FILE *arc = NULL;
        while(1){
            // ========================
            // Etapa de recepcao (rcv + verificacao + download/desenhar)
            // recebe mensagem do servidor
            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            
            // Mensagem do servidor (marcador de inicio)
            // Qualquer outras mensagens serao ignoradas
            // Falta tratar perda de ack/nack do ultimo pacote
            if (is_valid_protocol(rcv_buffer)){
                // Mensagem valida (CRC certo -- ACK)
                if(is_valid_crc(rcv_buffer)){
                    deserialize_msg(rcv_buffer, &rcv_msg);                       // Monta a struct
                    // Mensagem recebida eh a esperada
                    if (rcv_msg.sequence == expected_seq){                       
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);   // Cria mensagem (coloca valores na struct)
                        int send_bytes = serialize_msg(&send_msg, send_buffer); // serializa a struct (coloca no buffer)
                        send(socket, send_buffer, send_bytes, 0);               // envia
                        expected_seq = (expected_seq + 1) % 32;                 // atualiza sequencia

                        switch (rcv_msg.type){
                        // RAIO PRECISA ACONTECER ANTES DE VISUALIZACAO SEMPRE
                        case RAIO:
                            size_line_map = *(int *)(rcv_msg.data) * 2 + 1;
                            break;
                        case VISUALIZACAO:
                            if (size_line_map != 0){
                                for(int i = 0; i < size_line_map; i++){
                                    mvwprintw(game_window, line, i+1, "%c", rcv_msg.data[i]);
                                }
                                line++;
                            }
                            break;
                        // Download
                        case DADOS:
                            if (arc != NULL) fwrite(rcv_msg.data, 1, rcv_msg.size, arc);
                            break;
                        // [nome]-[tamanho]
                        case TXT:
                        case JPG:
                        case MP4:
                            arc = create_arc((char *)rcv_msg.data, path_arc);
                            break;
                        case FIM_DA_TRANSMISSAO:
                            if (arc != NULL){
                                fclose(arc);

                                char *xdg = "xdg-open ";
                                char cmd[1024]; 
                                sprintf(cmd, "%s%s", xdg, path_arc);
                                system(cmd);

                            }

                            flag_ntw = 1;
                            break;
                        }
                        if (flag_ntw) break;
                    }
                    // Mensagem repetida
                    else{
                        create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                        int send_bytes = serialize_msg(&send_msg, send_buffer);
                        send(socket, send_buffer, send_bytes, 0);
                    }
                    free(rcv_msg.data);
                }
                // Mensagem invalida (CRC errado -- NACK)
                else{
                    create_control_msg(&send_msg, NACK, expected_seq);
                    int send_bytes = serialize_msg(&send_msg, send_buffer);
                    send(socket, send_buffer, send_bytes, 0);
                }
            }
        }

        // ========================
        // Etapa de input
        int direction = -1;
        do{
            int key = getch();                                                          // getch bloqueia fluxo
            // Etapa de captar input
            switch(key){
            case 'W':
            case 'w':
            case KEY_UP:
                direction = CIMA;
                break;
            
            case 'D':
            case 'd':
            case KEY_RIGHT:
                direction = DIREITA;
                break;
    
            case 'S':
            case 's':
            case KEY_DOWN:
                direction = BAIXO;
                break;
            
            case 'A':
            case 'a':
            case KEY_LEFT:
                direction = ESQUERDA;
                break;
            
            // Pause
            // (VOU CRIAR UMA INTERFACE MAIS BONITINHA AINDA)
            case 'Q':
            case 'q':
                flag = 1;
                break;
            }
        } while (direction == -1);

        // Fim do jogo
        // SERA CRIADO MAIS UM TIPO
        if (flag) break;

        // ========================
        // Etapa de envio (send + ack/nack/timeout)
        create_control_msg(&send_msg, direction, curr_seq);                         // Cria a mensagem com o input do usuario
        int send_bytes = serialize_msg(&send_msg, send_buffer);                     // Serializa a mensagem
        send(socket, send_buffer, send_bytes, 0);                                   // envia
        int flag_rcv = 0;                                                           // Loop de recebimento de ack/nack
        while(1){
            // Timeout (de 1s -- por enquanto) + CRC
            if ((recebe_mensagem(socket, 1000, rcv_buffer, sizeof(rcv_buffer)) != -1) && (is_valid_crc(rcv_buffer))){
                deserialize_msg(rcv_buffer, &rcv_msg);
                if (rcv_msg.sequence == curr_seq && rcv_msg.type == ACK){
                    curr_seq = (curr_seq + 1) % 32;
                    flag_rcv = 1;
                }
                free(rcv_msg.data);
            }

            if (flag_rcv) break;
            else send(socket, send_buffer, send_bytes, 0);
        }
    }
    
    endwin();
    return 0;
    
}