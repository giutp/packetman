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

// Cria o arquivo mandado pelo servidor para download
// Salva em path_arc o caminho do arquivo aberto
// Retorna:
// + Sucesso: Ponteiro para o arquivo criado
// + Erro: NULL
static FILE *create_arc(char *name_size_arc, char *path_arc, unsigned long *size_arc){
    FILE *arc;
    // size_arc ainda esta inutil, mas sera usado para log
    int range;
    char *ext, name_arc[16], *del;
    char *path_download = "../assets/download/";

    del = strchr(name_size_arc, '-');
    
    if (del != NULL){
        *del = '\0';
        range = strtoul(name_size_arc, NULL, 10);

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
        *size_arc = strtoul(del + 1, NULL, 10);

        arc = fopen(path_arc, "wb");
        return arc;
    }

    return NULL;
}

int main(int argc, char **argv){
    // =============================================================================
    // Inicalizações da rede

    if (argc < 2){
        fprintf(stderr, "Necessário informar a placa de rede!\n");
        return -1;
    }

    // Criação do soquete
    int socket = create_raw_socket(argv[1]);

    // =============================================================================
    // Definições de variáveis de rede/mensagens

    // Buffers usados em send() e rcv()
    uint8_t send_buffer[TAM_BUFFER];
    uint8_t rcv_buffer[TAM_BUFFER];

    // Ethernet
    uint8_t mac_orig[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t mac_dest[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t eth_type   = 0x8888;
    memcpy(send_buffer, mac_dest, 6);
    memcpy(send_buffer+6, mac_orig, 6);
    memcpy(send_buffer+12, &eth_type, 2);

    // Protocolo kermit
    kermit_t send_msg, rcv_msg;
    uint8_t curr_seq = 0, expected_seq = 0;


    // =============================================================================
    // Inializações do ncurses

    init_interface();

    // Checagem de tamanho de janela
    // x = colunas (largura da tela)
    // y = linhas (altura da tela)
    // 80x24 = 80 colunas e 24 linhas
    // ncurses ao contrario (igual matriz em C [linhas][colunas] = [y][x])
    int console_width, console_heght;
    getmaxyx(stdscr, console_heght, console_width);                                                         // tamanho da janela aberta (terminal)

    // TAMANHO MÍNIMO PARA RODAR O JOGO
    if (console_heght < MIN_H || console_width < MIN_W){
        endwin();
        
        fprintf(stderr, "Erro: janela do terminal abaixo do mínimo %dx%d\n", MIN_W, MIN_H);
        fprintf(stderr, "Atual: %dx%d\n", console_width, console_heght);
        fprintf(stderr, "Aumente o tamanho da janela e tente novamente!\n");
        return -1;
    }

    // Proporção das janelas (jogo e log)
    int game_w = (console_width * 50) / 80;
    int log_w = console_width - game_w;
    int game_h, log_h;
    game_h = log_h = console_heght;

    // Divisão do terminal em janelas
    refresh();
    WINDOW *game_window = newwin(game_h, game_w, 0, 0);
    box(game_window, 0, 0);
    wrefresh(game_window);
    
    WINDOW *box_log_window = newwin(log_h, log_w, 0, 0 + game_w);
    box(box_log_window, 0, 0);
    wrefresh(box_log_window);

    WINDOW *log_window = derwin(box_log_window, log_h - 2, log_w - 2, 1, 1);
    scrollok(log_window, TRUE);
    print_log(log_window, "Criado janelas com sucesso\n");

    // =============================================================================
    // Loop game 
    int flag_game = 0;                                                                                          // flag do loop game
    // Fluxo do cliente
    // recv -> verificação (crc+nack/ack) -> download/desenhar -> captar input -> criar mensagem -> enviar -> esperar (nack/ack do servidor) -> recv
    while(1){
        int flag_ntw = 0;                                                                                       // flag do loop de recepção
        uint32_t size_line_map = 0, offset_view = 0;                                                               // tamanho da linha e linha atual
        uint64_t size_arc = 0, total_donwloaded = 0;                                                       // tamanho do arquivo de download e total baixado
        char path_arc[1024];                                                                                    // caminho do arquivo de download
        FILE *arc = NULL;                                                                                       // arquivo a ser criado (download)
        while(1){
            // ---------------------------------------------------------------------
            // Etapa de recepção (rcv() + verificação + desenhar/download)
            recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
            print_log(
                log_window, 
                "Recebendo pacote. Sequência esperada: %d\n", 
                expected_seq
            );

            // Mensagem do servidor -- qualquer outras mensagens serão ignoradas
            // <<Falta tratar perda de ack/nack do ultimo pacote>>
            if (is_valid_protocol(rcv_buffer+14)){
                print_log(
                    log_window, 
                    "Marcador de início validado com sucesso (%02X)\n", 
                    START_MARKER
                );

                // Mensagem válida -- ACK
                if(is_valid_crc(rcv_buffer+14)){
                    print_log(
                        log_window, 
                        "CRC válido (%02X) e enviando ACK\n", 
                        rcv_buffer[18+rcv_buffer[15]]
                    );

                    deserialize_msg(rcv_buffer+14, &rcv_msg);                                                      // Monta a struct kermit
                    print_log(
                            log_window, 
                            "Mensagem deserializada, tamanho: %d | sequencia: %d | tipo: %s\n", 
                            rcv_msg.size, 
                            rcv_msg.sequence, 
                            enum_to_string(rcv_msg.type)
                    );
                    send_control_msgs(socket, &send_msg, ACK, rcv_msg.sequence, send_buffer);

                    // Mensagem recebida é a esperada
                    if (rcv_msg.sequence == expected_seq){     
                        print_log(
                            log_window, 
                            "Sequências iguais. Mensagem interceptada!\n"
                        );
                        expected_seq = (expected_seq + 1) % 32;

                        // Verificação de tipo de mensagem dentre todos os possíveis tipos de recepção do cliente
                        switch (rcv_msg.type){
                        // Visão do Pacman
                        
                        case RAIO:
                            size_line_map = *(uint32_t *)(rcv_msg.data) * 2 + 1;
                            offset_view = 0;
                            print_log(
                                log_window, 
                                "Raio recebido. Tamanho do mapa a ser desenhado: %ux%u\n", 
                                size_line_map,
                                size_line_map
                            );
                            break;

                        // Desenhar o mapa
                        case VISUALIZACAO:
                            if (size_line_map != 0){
                                for(unsigned int i = 0; i < rcv_msg.size; i++){
                                    int calc_y = offset_view / size_line_map;
                                    int calc_x = offset_view / size_line_map;

                                    mvwprintw(game_window, calc_y + 1, calc_x + 1, "%c", rcv_msg.data[i]);

                                    offset_view++;
                                }
                                wrefresh(game_window);
                            }
                            break;

                        // Criação e preparação de arquivo de download
                        case TXT:
                        case JPG:
                        case MP4:
                            arc = create_arc((char *)rcv_msg.data, path_arc, &size_arc);
                            if (arc != NULL) {
                                print_log(
                                    log_window, 
                                    "Arquivo criado com sucesso e pronto para download\n"
                                );
                            }
                            // essa parte precisa de uma atenção depois
                            else {
                                flag_ntw = 1;
                                print_log(
                                    log_window, 
                                    "Erro ao criar arquivo de donwload. Não será iniciado download\n"
                                );

                                
                                // Função fopen() não conseguiu criar arquivo. Não será feito download do arquivo pelo cliente
                                // Subrotina de tratamento de erro
                                int flag_err = 0;                                                               // flag de tratamento de erro
                                uint8_t aux_rcv_buffer[50];
                                kermit_t aux_rcv_msg;
                                
                                send_control_msgs(socket, &send_msg, ERROS, curr_seq, send_buffer);
                                while(1){
                                    if (recebe_mensagem(socket, 1000, aux_rcv_buffer, sizeof(aux_rcv_buffer)) != -1 && is_valid_crc(aux_rcv_buffer+14)){
                                        deserialize_msg(aux_rcv_buffer, &aux_rcv_msg);
                                        if (aux_rcv_msg.sequence == curr_seq && aux_rcv_msg.type == ACK){
                                            curr_seq = (curr_seq + 1) % 32;
                                            flag_err = 1;
                                        }
                                        free(aux_rcv_msg.data);
                                    }
                                    else send(socket, send_buffer, 19+send_buffer[15], 0);

                                    if (flag_err) break;
                                }
                            }
                            break;

                        // Download do arquivo
                        case DADOS:
                            if (arc != NULL) {
                                size_t bytes_write = fwrite(rcv_msg.data, 1, rcv_msg.size, arc);
                                total_donwloaded += bytes_write;
                                print_log(
                                    log_window, 
                                    "Download: %.2f%%\n", 
                                    ((float)total_donwloaded/size_arc)*100
                                );
                            }
                            break;

                        // Fim do pacote (mapa ou download)
                        case FIM_DA_TRANSMISSAO:
                            print_log(
                                log_window, 
                                "Fim do pacote\n"
                            );
                            if (arc != NULL){
                                fclose(arc);

                                print_log(
                                    log_window, 
                                    "Abrindo arquivo de donwload...\n"
                                );
                                char *xdg = "xdg-open ";
                                char cmd[2048]; 
                                sprintf(cmd, "%s%s", xdg, path_arc);
                                system(cmd);
                            }

                            flag_ntw = 1;
                            break;
                        
                        // Pacman coletou todas as pastilhas
                        case VITORIA:
                            print_log(
                                log_window, 
                                "Mensagem de vitoria\n"
                            );
                            flag_ntw = flag_game = 1;
                            break;
                        
                        // Pacman colidiu com fantasma
                        case DERROTA:
                            print_log(
                                log_window, 
                                "Mensagem de derrota\n"
                            );
                            flag_ntw = flag_game = 1;
                            break;
                        }

                        if (flag_ntw) break;
                    }
                    free(rcv_msg.data);
                }

                // Mensagem inválida -- NACK
                else{
                    print_log(
                        log_window, 
                        "CRC checado, inválido e enviando NACK (%02X != %02X)\n", 
                        rcv_buffer[18+rcv_buffer[15]], 
                        calculate_crc8(rcv_buffer+14, 4+rcv_buffer[15])
                    );
                    send_control_msgs(socket, &send_msg, NACK, expected_seq, send_buffer);
                }
            }
        }

        if (flag_game) break;

        // ---------------------------------------------------------------------
        // Etapa de captar input
        print_log(
            log_window, 
            "Capturando input do usuario\n"
        );

        int input = -1;
        do{
            int key = getch();                                                                                  // getch bloqueia fluxo
            switch(key){
            // Movimento
            case 'W': case 'w': case KEY_UP: input = CIMA; break;
            case 'D': case 'd': case KEY_RIGHT: input = DIREITA; break;
            case 'S': case 's': case KEY_DOWN: input = BAIXO; break;
            case 'A': case 'a': case KEY_LEFT: input = ESQUERDA; break;
            // Pause
            // (VOU CRIAR UMA INTERFACE MAIS BONITINHA AINDA)
            case 'Q': case 'q': flag_game = 1; break;
            }
        } while (input == -1);

        print_log(
            log_window, 
            "Input captado: %s\n", 
            enum_to_string(input)
        );

        // Fim do jogo
        // (TIPO SAIR)
        if (flag_game) break;

        // ---------------------------------------------------------------------
        // Etapa de envio (send() + ack/nack/timeout)
        print_log(
            log_window, 
            "Criando mensagem para o servidor. Seq atual: %d\n", 
            curr_seq
        );

        send_control_msgs(socket, &send_msg, input, curr_seq, send_buffer);
        int flag_rcv = 0;                                                                                       // flag de loop de espera de ack/nack
        while(1){
            // Timeout (de 1s -- por enquanto) + CRC
            if ((recebe_mensagem(socket, 1000, rcv_buffer, sizeof(rcv_buffer)) != -1) && (is_valid_crc(rcv_buffer))){
                print_log(
                    log_window, 
                    "Servidor recebeu mensagem\n"
                );
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
                print_log(
                    log_window, 
                    "TIMEOUT OU CRC INVÁLIDO. Reenviando mensagem...\n"
                );
                send(socket, send_buffer, 19+send_buffer[15], 0);
            }

            if (flag_rcv) break;
        }
    }
    
    endwin();
    return 0;
}