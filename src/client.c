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
static FILE *create_arc(char *name_size_arc, char *path_arc, unsigned long *size_arc){
    FILE *arc;
    // size_arc ainda esta inutil, mas sera usado para log
    int range;
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
        *size_arc = atoi(del + 1);

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

    // inicializacoes da janelas
    refresh();
    WINDOW *game_window = newwin(game_h, game_w, 0, 0);
    box(game_window, 0, 0);
    wrefresh(game_window);
    
    WINDOW *box_log_window = newwin(log_h, log_w, 0, 0 + game_w);
    box(box_log_window, 0, 0);
    wrefresh(box_log_window);

    WINDOW *log_window = derwin(box_log_window, log_h - 2, log_w - 2, 1, 1);
    scrollok(log_window, TRUE);
    wprintw(log_window, "Criado janelas com sucesso\n");
    wrefresh(log_window);

    int flag = 0;
    // Loop game
    // Fluxo do cliente (pelo menos por agora ate eu ver que errei alguma parte)
    // recv -> verificacao (crc+nack/ack) -> download/desenhar -> input -> criar mensagem -> enviar -> esperar (nack/ack do servidor) -> recv
    while(1){
        int flag_ntw = 0;                                                           // flag do loop de recepcao
        int size_line_map = 0, line = 1;                                            // tamanho da linha e qual linha esta
        unsigned long size_arc = 0, total_donwloaded = 0;                           // tamanho do arquivo de download
        char path_arc[1024];                                                        // caminho do arquivo de download
        FILE *arc = NULL;                                                           // arquivo a ser criado
        while(1){
            // ========================
            // Etapa de recepcao (rcv + verificacao + download/desenhar)
            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            wprintw(log_window, "Recebendo pacote. Sequencia esperada: %d\n", expected_seq);
            wrefresh(log_window);
            
            // Mensagem do servidor (marcador de inicio) -- Qualquer outras mensagens serao ignoradas
            // <<Falta tratar perda de ack/nack do ultimo pacote>>
            if (is_valid_protocol(rcv_buffer)){
                wprintw(log_window, "Marcador de início validado com sucesso (%02X)\n", START_MARKER);
                wrefresh(log_window);
                // Mensagem valida (CRC certo -- ACK)
                if(is_valid_crc(rcv_buffer)){
                    wprintw(log_window, "CRC valido com sucesso e enviado ACK (%02X)\n", rcv_buffer[4+rcv_buffer[1]]);
                    wrefresh(log_window);
                    deserialize_msg(rcv_buffer, &rcv_msg);                          // Monta a struct
                    wprintw(log_window, "Mensagem deserializada, tamanho: %d | sequencia: %d | tipo: %s \n", rcv_msg.size, rcv_msg.sequence, enum_to_string(rcv_msg.type));
                    wrefresh(log_window);
                    create_control_msg(&send_msg, ACK, rcv_msg.sequence);           // Cria mensagem (coloca valores na struct)
                    int send_bytes = serialize_msg(&send_msg, send_buffer);         // serializa a struct (coloca no buffer)
                    send(socket, send_buffer, send_bytes, 0);                       // envia
                    // Mensagem recebida eh a esperada
                    if (rcv_msg.sequence == expected_seq){     
                        wprintw(log_window, "Sequencias iguais! Mensagem interceptada\n");
                        wrefresh(log_window);
                        expected_seq = (expected_seq + 1) % 32;                     // atualiza sequencia
                        switch (rcv_msg.type){
                        case RAIO:
                            size_line_map = *(int *)(rcv_msg.data) * 2 + 1;
                            wprintw(log_window, "Raio recebido. Tamanho do mapa a ser desenhado: %dx%d (%d elementos)\n", size_line_map, size_line_map, size_line_map*size_line_map);
                            wrefresh(log_window);
                            break;
                        case VISUALIZACAO:
                            if (size_line_map != 0){
                                wprintw(log_window, "Linha atual: %d\n", line);
                                wrefresh(log_window);
                                for(int i = 0; i < size_line_map; i++){
                                    mvwprintw(game_window, line, i+1, "%c", rcv_msg.data[i]);
                                    wrefresh(game_window);
                                }
                                line++;
                                wprintw(log_window, "Próxima linha: %d\n", line);
                                wrefresh(log_window);
                            }
                            break;
                        case DADOS:
                            if (arc != NULL) {
                                size_t bytes_write = fwrite(rcv_msg.data, 1, rcv_msg.size, arc);
                                total_donwloaded += bytes_write;
                                wprintw(log_window, "Download: %.2f%%\n", ((float)total_donwloaded/size_arc)*100);
                                wrefresh(log_window);
                            }
                            break;
                        case TXT:
                        case JPG:
                        case MP4:
                            arc = create_arc((char *)rcv_msg.data, path_arc, &size_arc);
                            if (arc != NULL) {
                                wprintw(log_window, "Arquivo criado com sucesso e pronto para download\n");
                                wrefresh(log_window);
                            }
                            else {
                                flag_ntw = 1;

                                wprintw(log_window, "Erro ao criar arquivo de donwload. Nao sera iniciado download\n");
                                wrefresh(log_window);
                                create_control_msg(&send_msg, ERROS, curr_seq);
                                send_bytes = serialize_msg(&send_msg, send_buffer);
                                send(socket, send_buffer, send_bytes, 0);

                                int flag_err = 0;
                                uint8_t aux_rcv_buffer[50];
                                kermit_t aux_rcv_msg;
                                while(1){
                                    if (recebe_mensagem(socket, 1000, aux_rcv_buffer, sizeof(aux_rcv_buffer)) != -1 && is_valid_crc(aux_rcv_buffer)){
                                        deserialize_msg(aux_rcv_buffer, &aux_rcv_msg);
                                        if (aux_rcv_msg.sequence == curr_seq && aux_rcv_msg.type == ACK){
                                            curr_seq = (curr_seq + 1) % 32;
                                            flag_err = 1;
                                        }
                                        free(aux_rcv_msg.data);
                                    }
                                    else send(socket, send_buffer, send_bytes, 0);

                                    if (flag_err) break;
                                }
                            }
                            break;
                        case FIM_DA_TRANSMISSAO:
                            wprintw(log_window, "Fim do pacote\n");
                            wrefresh(log_window);
                            if (arc != NULL){
                                fclose(arc);
                                wprintw(log_window, "Abrindo arquivo de donwload...\n");
                                wrefresh(log_window);
                                char *xdg = "xdg-open ";
                                char cmd[2048]; 
                                sprintf(cmd, "%s%s", xdg, path_arc);
                                system(cmd);

                            }

                            flag_ntw = 1;
                            break;
                        case VITORIA:
                            wprintw(log_window, "Mensagem de vitoria\n");
                            wrefresh(log_window);
                            flag_ntw = flag = 1;
                            break;
                        case DERROTA:
                            wprintw(log_window, "Mensagem de derrota\n");
                            wrefresh(log_window);
                            flag_ntw = flag = 1;
                            break;
                        }
                        if (flag_ntw) break;
                    }
                    free(rcv_msg.data);
                }
                // Mensagem invalida (CRC errado)
                else{
                    wprintw(log_window, "CRC checado, inválido e enviado NACK (%02X <> %02X)\n", rcv_buffer[4+rcv_buffer[1]], calculate_crc8(rcv_buffer, 4+rcv_buffer[1]));
                    wrefresh(log_window);
                    create_control_msg(&send_msg, NACK, expected_seq);
                    int send_bytes = serialize_msg(&send_msg, send_buffer);
                    send(socket, send_buffer, send_bytes, 0);
                }
            }
        }

        if (flag) break;

        // ========================
        // Etapa de input
        wprintw(log_window, "Capturando input do usuario\n");
        wrefresh(log_window);
        int type = -1;
        do{
            int key = getch();                                                          // getch bloqueia fluxo
            // Etapa de captar input
            switch(key){
            case 'W':
            case 'w':
            case KEY_UP:
                type = CIMA;
                break;
            
            case 'D':
            case 'd':
            case KEY_RIGHT:
                type = DIREITA;
                break;
    
            case 'S':
            case 's':
            case KEY_DOWN:
                type = BAIXO;
                break;
            
            case 'A':
            case 'a':
            case KEY_LEFT:
                type = ESQUERDA;
                break;
            
            // Pause
            // (VOU CRIAR UMA INTERFACE MAIS BONITINHA AINDA)
            case 'Q':
            case 'q':
                flag = 1;
                break;
            }
        } while (type == -1);

        wprintw(log_window, "Input captado: %s\n", enum_to_string(type));
        wrefresh(log_window);

        // Fim do jogo
        // SERA CRIADO MAIS UM TIPO
        if (flag) break;

        // ========================
        // Etapa de envio (send + ack/nack/timeout)
        
        wprintw(log_window, "Criando mensagem para o servidor. Seq atual: %d\n", curr_seq);
        wrefresh(log_window);
        create_control_msg(&send_msg, type, curr_seq);                              // Cria a mensagem com o input do usuario
        int send_bytes = serialize_msg(&send_msg, send_buffer);                     // Serializa a mensagem
        send(socket, send_buffer, send_bytes, 0);                                   // envia
        int flag_rcv = 0;                                                           // Loop de recebimento de ack/nack
        while(1){
            // Timeout (de 1s -- por enquanto) + CRC
            if ((recebe_mensagem(socket, 1000, rcv_buffer, sizeof(rcv_buffer)) != -1) && (is_valid_crc(rcv_buffer))){
                wprintw(log_window, "Servidor recebeu mensagem\n");
                wrefresh(log_window);
                deserialize_msg(rcv_buffer, &rcv_msg);
                if (rcv_msg.sequence == curr_seq && rcv_msg.type == ACK){
                    wprintw(log_window, "Pacote recebido com sucesso pelo servidor\n");
                    wrefresh(log_window);
                    curr_seq = (curr_seq + 1) % 32;
                    flag_rcv = 1;
                }
                free(rcv_msg.data);
            }
            else {
                wprintw(log_window, "TIMEOUT OU CRC INVALIDO. Reenviando mensagem...\n");
                wrefresh(log_window);
                send(socket, send_buffer, send_bytes, 0);
            }

            if (flag_rcv) break;
        }
    }
    
    endwin();
    return 0;
    
}