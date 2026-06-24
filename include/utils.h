#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>

#define N 40                                                            // Tamanho do mapa (matriz) do jogo
#define RECV_BUFFER (rcv_buffer+14)

#include "kermit.h"

// Direção de movimento das entidades (pacman e fantasmas)
typedef enum {
    UP,
    RIGHT,
    DOWN,
    LEFT
} direction_t;

// Coordenada cartesiana (posição da matriz) das entidades
typedef struct coord {
    int x;
    int y;
} coord_t;

// Converte o valor do enum para a string correspondente
// Retorna:
// + String correspondente ao type
char *enum_to_string(types_t type);

// Cria o arquivo mandado pelo servidor para download
// Salva em path_arc o caminho do arquivo aberto
// Retorna:
// + Sucesso: Ponteiro para o arquivo criado
// + Erro: NULL
FILE *create_arc(char *name_size_arc, char *path_arc, unsigned long *size_arc);

// Envia mensagem e faz o controle de ACK e NACK
void send_with_ack(int socket, uint8_t *send_buffer, int send_bytes, uint8_t *rcv_buffer, kermit_t *rcv_msg, int *curr_seq);

#endif // UTILS_H