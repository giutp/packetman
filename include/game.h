#ifndef GAME_H
#define GAME_H

// Tamanho da matriz (mapa) do jogo
#define N 40

// Cores dos fantasmas
typedef enum {
    RED,
    BLUE,
    GREEN,
    YELLOW
} ghost_color_t;

// Tipo de arquivos enviados entre servidor-cliente
typedef enum {
    FILE_TXT,
    FILE_JPG,
    FILE_MP4
} file_type_t;


// Direção das entidades andarem (pacman e fantasmas)
typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} direction_t;

// Coordenada cartesiana
typedef struct coord {
    int x;
    int y;
} coord_t;

// Atributos do pacman
typedef struct pacman {
    coord_t position;
    int radius;
    int count_mov;
    int pellets;
} pacman_t;


// atributos dos fantasmas
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

int check_pellets_collision(pacman_t *pacman, pellet_t *pellets);

// IA de movimentação dos fantamas seguindo as regras de movimento de cada cor
void move_ghosts(char map[N][N], ghost_t *ghosts, int round);

void move_pacman(char map[N][N], pacman_t *pacman, direction_t direction);

int check_ghost_pacman_collision(pacman_t *pacman, ghost_t *ghosts);

#endif