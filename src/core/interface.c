#include <ncurses.h>

void init_interface(){
    initscr();

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    curs_set(0);
    // nodelay(stdscr, TRUE);
}