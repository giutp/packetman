#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>

#include "utils.h"

char *enum_to_string(types_t type){
    switch (type){
    case ACK: return "ACK"; break;
    case NACK: return "NACK"; break;
    case VISUALIZACAO: return "VISUALIZACAO"; break;
    case DADOS: return "DADOS"; break;
    case TXT: return "TXT"; break;
    case JPG: return "JPG"; break;
    case MP4: return "MP4"; break;
    case RAIO: return "RAIO"; break;
    case SAIR: return "SAIR"; break;
    case DIREITA: return "DIREITA"; break;
    case ESQUERDA: return "ESQUERDA"; break;
    case CIMA: return "CIMA"; break;
    case BAIXO: return "BAIXO"; break;
    case NFILE: return "NFILE"; break;
    case ERROS: return "ERROS"; break;
    case FIM_DA_TRANSMISSAO: return "FIM_DA_TRANSMISSAO"; break;
    case VITORIA: return "VITORIA"; break;
    case DERROTA: return "DERROTA"; break;
    }

    return NULL;
}

void send_with_ack(int socket, uint8_t *send_buffer, int send_bytes, uint8_t *rcv_buffer, kermit_t *rcv_msg, int *curr_seq){
    while (1) {
        send(socket, send_buffer, send_bytes + 14, 0);

        if ((recebe_mensagem(socket, 3000, rcv_buffer, TAM_BUFFER) != -1) && is_valid_crc(rcv_buffer+14)) {
            deserialize_msg(rcv_buffer+14, rcv_msg);

            if (rcv_msg->sequence == *curr_seq && rcv_msg->type == ACK) {
                printf("Recebido ACK com sucesso\n");
                *curr_seq = (*curr_seq + 1) % 32;
                free(rcv_msg->data);
                break;
            }
            free(rcv_msg->data);
        }
        else printf("TIMEOUT OU CRC INVALIDO\n");
    }
}