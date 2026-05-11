#include <stdlib.h>

#include "utils.h"
#include "kermit.h"

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
    case ERROS: return "ERROS"; break;
    case FIM_DA_TRANSMISSAO: return "FIM_DA_TRANSMISSAO"; break;
    case VITORIA: return "VITORIA"; break;
    case DERROTA: return "DERROTA"; break;
    }

    return NULL;
}