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

void wclear_with_box(WINDOW *win, int v, int h){
    wclear(win);
    box(win, v, h);
}

void wdraw_center_ascii_art(WINDOW *win, const char **ascii, int h, int w, int lines){
    for (int i = 0; i < lines; i++){
        mvwprintw(win, ((h/2)-2)+i, (w/2)-16, "%s", ascii[i]);
    }
    wrefresh(win);
}