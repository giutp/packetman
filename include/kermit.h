#ifndef KERMIT_H
#define KERMIT_H

#include <stdint.h>

#define START_MARKER 0x7e
#define TAM_BUFFER 50

/* TODO: 
    * melhorar os comentarios
    * talvez mudar nome das funcoes e parametros
*/
// Tipo de mensagem do protocolo kermit
typedef enum kermit_types_t {
    ACK,                                // mensagem recebida
    NACK,                               // mensagem recebida com erro
    VISUALIZACAO,                       // mapa do jogo
    DADOS,                              // arquivos de download
    TXT,                                // arquivo .txt
    JPG,                                // arquivo .jpg
    MP4,                                // arquivo .mp4
    RAIO,                               // raio de visao do pacman
    SAIR,                               // parar o jogo
    DIREITA,                            // movimento para direita
    ESQUERDA,                           // movimento para esquerda
    CIMA,                               // movimento para cima
    BAIXO,                              // movimento para baixo
    NFILE,                              // a definir
    ERROS,                              // erros do cliente (permissao/espaco)
    FIM_DA_TRANSMISSAO,                 // fim de mensagem do mapa
    VITORIA,                            // jogador venceu
    DERROTA                             // jogador perdeu
    /* 
        TODO: Preenchido 12 dos 32 tipos
        * Definir os outros 20 tipos
    */
} types_t;

// Estrutura do protocolo kermit (modificado)
typedef struct protocol_kermit_t {
    uint8_t starter_marker;             // 8 bits
    uint8_t size: 5;                    // 5 bits
    uint8_t sequence: 6;                // 6 bits
    uint8_t type: 5;                    // 5 bits
    uint8_t *data;                      // n bytes
    uint8_t crc;                        // 8 bits
} __attribute__((packed)) kermit_t;

// Cria o raw scoket baseado no nome da interface
// Retorna:
// + -1: erro de criação, bind ou setsockopt
// + socket: canal do socket aberto             
int create_raw_socket(char *network_interface_name);

// Calcula o CRC passado
// Retorna:
// + Resto da divisão polinomial
// - - Cliente: calcula o crc da mensagem enviada a ser enviada
// - - Server: calcula o crc do pacote recebido
uint8_t calculate_crc8(uint8_t *buffer, int size_buffer);

// Serializa a mensagem
// Retorna:
// + Tamanho do pacote serializado 
int serialize_msg(kermit_t *send_msg, uint8_t *send_buffer);

// Deserializa a mensagem
// Retorna:
// + Tamanho do pacote deserializado 
int deserialize_msg(uint8_t *rcv_buffer, kermit_t *rcv_msg);

// Função wrapper: cria mensagem sem dados chamando create_msg()
void create_control_msg(kermit_t *msg, uint8_t type, uint8_t seq);

// Função wrapper: cria, serializa e envia mensagem
void send_control_msgs(int socket, kermit_t *send_msg, types_t type, uint8_t seq, uint8_t *send_buffer);

// Função wrapper: cria mensagem com dados chamando create_msg()
void create_data_msg(kermit_t *msg, uint8_t size, uint8_t type, uint8_t seq, uint8_t *data);

// Válida o marcador de início da mensagem recebida
// Retorna:
// + "True": se o começo da mensagem for igual a START_MARKER
// + "False": caso contrário
int is_valid_protocol(uint8_t *buffer);

// Válida o CRC da mensagem recebida
// Retorna:
// + "True": se o campo CRC da mensagem for igual ao CRC calculado
// + "False": caso contrário
int is_valid_crc(uint8_t *buffer);

long long timestamp();

int protocolo_e_valido(uint8_t *buffer, int tamanho_buffer);

int recebe_mensagem(int soquete, int timeoutMillis, uint8_t *buffer, int tamanho_buffer);

#endif // KERMIT_H