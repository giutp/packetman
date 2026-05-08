#ifndef PELLETS_H
#define PELLETS_H

#include "utils.h"

// Quantidade de pastilhas
#define NUM_PELLETS 6

// Tipo do arquivo enviado entre servidor-cliente pelas pastilhas
typedef enum {
    FILE_TXT,
    FILE_JPG,
    FILE_MP4
} file_type_t;

// Atributos das pastilhas
typedef struct pellet {
    coord_t position;               // coordenada cartesiana
    file_type_t arc_type;           // tipo do arquivo atrelado
    char *filepath;                 // caminho do arquivo atrelado
    int collected;                  // flag de pastilha coletada
} pellet_t;

// Atrela dois de cada tipo arquivo as pastilhas
// Retorna
// + -2: diretório inexistente
// + -1: arquivos insuficientes ou formato inválido
// +  0: todos foram atrelados com sucesso
int bind_pellets_files(pellet_t *pellets);

#endif