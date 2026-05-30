#ifndef INTERFACE_H
#define INTERFACE_H

// Inicializa o ncurses
void init_interface();

// Imprime mensagens de log na janela de logs
void print_log(WINDOW *log_window, const char *log, ...);

// Limpa janela w e recoloca a borda
void wclear_with_box(WINDOW *win, int, int);

// Desenha mensagens ascii centralizado na janela win
void wdraw_center_ascii_art(WINDOW *win, const char **ascii, int h, int w, int lines);

#endif // INTERFACE_H