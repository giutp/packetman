#include <ncurses.h>

void init_interface(){
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
}