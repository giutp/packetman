#ifndef PELLETS_H
#define PELLETS_H

#include "utils.h"
#include "kermit.h"

// Quantidade de pastilhas
#define NUM_PELLETS 6

// Atributos das pastilhas
typedef struct pellet {
    coord_t position;               // coordenada cartesiana
    types_t arc_type;               // tipo do arquivo atrelado
    char *filepath;                 // caminho do arquivo atrelado
    int collected;                  // flag de pastilha coletada
} pellet_t;

// Atrela dois de cada tipo arquivo as pastilhas
// Retorna
// + -2: diretório inexistente
// + -1: arquivos insuficientes ou formato inválido
// +  0: todos foram atrelados com sucesso
int bind_pellets_files(pellet_t *pellets);

#endif // PELLETS_H