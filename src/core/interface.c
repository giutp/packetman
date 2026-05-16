#include <ncurses.h>

void init_interface(){
    initscr();

    cbreak();
    noecho();
    keypad(stdscr, TRUE);

    curs_set(0);
    // nodelay(stdscr, TRUE);
}

void print_log(WINDOW *log_window, const char *log, ...){
    va_list args;
    
    va_start(args, log);
    vw_printw(log_window, log, args);
    va_end(args);

    wrefresh(log_window);
}