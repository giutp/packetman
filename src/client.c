#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <sys/socket.h>

#include "kermit.h"
#include "interface.h"

int main(int argc, char **argv){
    // if (argc < 2){
    //     fprintf(stderr, "Necessario informar interface da placa de rede\n");
    //     exit(-1);
    // }

    init_interface();
    refresh();

    uint8_t seq = 0;
    kermit_t send_msg, rcv_msg;
    uint8_t buffer[36];

    int lin, col;
    
    int min_lin = 40, min_col = 40;
    
    do{
        getmaxyx(stdscr, lin, col);
        clear();
        mvprintw(0, 0, "Terminal muito pequeno!");
        mvprintw(1, 0, "Minimo: %d linhas x %d colunas", min_lin, min_col);
        mvprintw(2, 0, "Atual: %d linhas x %d colunas", lin, col);
        refresh();

        int key = getch();
        if (key == 'q'){
            endwin();
            return 0;
        }
    }while(lin < min_lin || col < min_col);
    

    
    WINDOW *win1 = newwin(lin, col/2, 0, 0);
    WINDOW *win2 = newwin(lin, col/2, 0, col/2);
    box(win1, 0, 0);
    box(win2, 0, 0);
    
    wprintw(win1, "col: %d", col/2);
    wprintw(win2, "col: %d", col/2);
    wrefresh(win1);
    wrefresh(win2);

    refresh();

    int key = getch();
    // while (1)
    // {
    //     if (key != ERR) break;
    // }
    

    endwin();
    return 0;

    // uint8_t seq = 0;
    // kermit_t send_msg, rcv_msg;
    // uint8_t buffer[36];

    // int socket = create_raw_socket(argv[1]);

    // /*  ================================================
    //     Tentativa de criar a logica do client, mas sem o servidor
    //     para ficar testando e debbugando ficou dificil. Por isso
    //     partir para criar o servidor.
    //     ================================================ */
    // init_interface();
    // refresh();

    // WINDOW *game_window = newwin(20, 30, 0 , 0);
    // WINDOW *log_window = newwin(20, 40, 0, 32);
    // box(game_window, 0, 0);
    // box(log_window, 0, 0);
    // wrefresh(game_window);
    // wrefresh(log_window);
    
    // // Isso ta uma bagunca, vou iniciar o servidor para ter como testar algo
    // while(1){
    //     int key = getch();
    //     if (key != ERR){
    //         if (key == 'q') break;

    //         int valid = 0;
    //         switch (key){
    //         case 'w':
    //             valid = 1;
    //             create_control_msg(&send_msg, CIMA, seq);
    //             break;
    //         case 'a':
    //             valid = 1;
    //             create_control_msg(&send_msg, ESQUERDA, seq);
    //             break;
    //         case 's':
    //             valid = 1;
    //             create_control_msg(&send_msg, BAIXO, seq);
    //             break;
    //         case 'd':
    //             valid = 1;
    //             create_control_msg(&send_msg, DIREITA, seq);
    //             break;
    //         }
            
    //         mvwprintw(log_window, 1, 1, "Ultima tecla: %c", key);
    //         wrefresh(log_window);

    //         if (valid){
    //             int flag = 0;
    //             do{
    //                 int num_bytes = serialize_msg(&send_msg, buffer);
    //                 send(socket, buffer, num_bytes, 0);

    //                 // VALOR DE TIMEOUT AINDA NAO DEFINIDO -- 2000 POR ENQUANTO
    //                 if (recebe_mensagem(socket, 2000, buffer, 36) != -1){
    //                     if(deserialize_msg(buffer, &rcv_msg) == 0 && ((rcv_msg.size_sequence_type & 0x1f) == ACK) && (((rcv_msg.size_sequence_type >> 5) & 0x3f) == seq)){

    //                         flag = 1;
    //                         seq++;
    //                         if (seq > 63) seq = 0;
    //                     }
    //                 }
    //             } while (!flag);     
    //         }   
    //     }
    // }
    
    // endwin();
}