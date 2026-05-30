#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>

#include "interface.h"

// main criada puramente para testes, nao sera usada (depois vou apagar)
int main(){
    // // AREA DE TESTES DO pellet.c
    // // Testado e funcionando (mais testes sao bem vindo)

    // pellet_t pellets[6];

    // int status = bind_pellets_files(pellets);

    // switch (status){
    // case 0:
    //     printf("Todos foram atrelados!\n");
    //     for (int i = 0; i < 6; i++){
    //         // printf("\nCaminho: %s\n", pellets[i].filepath);
    //         // printf("Tipo: %d\n\n", pellets[i].arc_type);
    //         // Desse jeito, memoria vai precisar ser desalocada
    //         // q
    //         // const char *xdg = "xdg-open ";
    //         // int len = strlen(pellets[i].filepath) + strlen(xdg) + 1;
    //         // char *path = (char *)malloc(len);
    //         // snprintf(path, len, "%s%s", xdg, pellets[i].filepath);
    //         // printf("%s\n", path);
    //         // system(path);
    //         // free(path);
    //     }
    //     break;
    
    // case -2:
    //     printf("Erro -2: O diretorio files nao foi encontrado!\n");
    //     break;
    
    // case -1:
    //     printf("Erro -1: Diretorio existe! Mas existe arquivos a mais, a menos ou invalidos\n");
    //     break;

    // }

    // init_interface();
    // char c = '#';

    // for (int line = 0; line < 10; line++)
    //     for (int col = 0; col < 20; col += 2){
    //         mvwprintw(stdscr, line, col, "%c ", c);
    //     }
    // refresh();
    // int k = getch();
    // endwin();

    init_interface();
    
                                  
    // const char *ascii[] = {
    //     "__   __         __      ___      ",
    //     "\\ \\ / /__ _  _  \\ \\    / (_)_ _  ",
    //     " \\ V / _ \\ || |  \\ \\/\\/ /| | ' \\ ",
    //     "  |_|\\___/\\_,_|   \\_/\\_/ |_|_||_|"
    // };

    const char *ascii[] = {
        " ___                 ___               ",
        "/ __|__ _ _ __  ___ / _ \\__ _____ _ _  ",
        "| (_ / _` | '  \\/ -_) (_) \\ V / -_) '_|",
        "\\___\\__,_|_|_|_\\___|\\___/ \\_/\\___|_|   "
    };
    
    
    int w, h;
    getmaxyx(stdscr, w, h);
    refresh();
    WINDOW *w1 = newwin(h/2, w, 0, 0);
    box(w1, 0, 0);
    wrefresh(w1);

    WINDOW *w2 = newwin(h/2, w, 0, 23+(w/2));
    box(w2, 0, 0);
    wrefresh(w2);


    for (int i = 0; i < 4; i++){
        mvwprintw(w1, i+1, 1, "%s", ascii[i]);
        wrefresh(w1);
    }


    getch();

    endwin();

    return 0;
}