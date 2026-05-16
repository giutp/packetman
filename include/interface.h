#ifndef INTERFACE_H
#define INTERFACE_H

// Inicializa o ncurses
void init_interface();

// Imprime mensagens de log na janela de logs
void print_log(WINDOW *log_window, const char *log, ...);

#endif // INTERFACE_H