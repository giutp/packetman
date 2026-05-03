#ifndef GAME_H
#define GAME_H

#define N 40

typedef enum {
    RED,
    BLUE,
    GREEN,
    YELLOW
} ghost_color_t;

typedef enum {
    FILE_TXT,
    FILE_JPG,
    FILE_MP4
} file_type_t;

typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} direction_t;

typedef struct coord {
    int x;
    int y;
} coord_t;

typedef struct pacman {
    coord_t position;
    int radius;
    int count_mov;
    int pellets;
} pacman_t;

typedef struct ghost {
    coord_t position;
    ghost_color_t color;
    direction_t prev_direc;
    direction_t curr_direc;
} ghost_t;

typedef struct pellet {
    coord_t position;
    file_type_t arc_type;
    char *filepath;
    int collected;
} pellet_t;


/* Ler um arquivo .csv e monta o mapa do jogo
    Retorna:
        * -1: falha em abrir o arquivo
        *  0: sucesso de leitura e criacao do mapa
*/
int read_map(char *filepath, char map[N][N]);

void init_entities(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets);

int update_pacman(char map[N][N], pacman_t *pacman, ghost_t *ghosts, pellet_t *pellets, direction_t direction);

void bind_pellets_files(pellet_t *pellets);

int check_pellets(pacman_t *pacman, pellet_t *pellets);

// IA de movimentação dos fantamas seguindo as regras de movimento de cada cor
void move_ghosts(char map[N][N], ghost_t *ghosts, int round);

void get_visible_map(char map[N][N], pacman_t *pacman, char visible_map[N][N]);

#endif