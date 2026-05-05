#ifndef KERMIT_H
#define KERMIT_H

#include <stdint.h>

#define START_MARKER 0x7e

/* TODO: 
    * melhorar os comentarios
    * talvez mudar nome das funcoes e parametros
*/
// Tipo de mensagem do protocolo kermit
typedef enum kermit_types_t {
    ACK,
    NACK,
    VISUALIZACAO,
    DADOS,
    TXT,
    JPG,
    MP4,
    NULL0,
    NULL1,
    DIREITA,
    ESQUERDA,
    CIMA,
    BAIXO,
    NULL2,
    ERROS,
    FIM_DA_TRANSMISSAO
    /* 
        TODO: Preenchido 12 dos 32 tipos
        * Definir os outros 20 tipos
    */
} types_t;

// Estrutura do protocolo kermit (modificado)
typedef struct protocol_kermit_t {
    uint8_t starter_marker;             // 8 bits
    // uint16_t size_sequence_type;        // 5 + 6 + 5 = 16 bits
    uint8_t size: 5;
    uint8_t sequence: 6;
    uint8_t type: 5;
    uint8_t *data;                      // n bytes
    uint8_t crc;                        // 8 bits
} kermit_t;


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
// TODO: a definir os codigos de erros
int deserialize_msg(uint8_t *rcv_buffer, kermit_t *rcv_msg);

// Função wrapper: cria mensagem sem dados chamando create_msg()
void create_control_msg(kermit_t *msg, uint8_t type, uint8_t seq);

// Função wrapper: cria mensagem com dados chamando create_msg()
void create_data_msg(kermit_t *msg, uint8_t size, uint8_t type, uint8_t seq, uint8_t *data);

int is_valid_start_marker(uint8_t *buffer);

int is_valid_crc(uint8_t *buffer);

long long timestamp();

int protocolo_e_valido(char *buffer, int tamanho_buffer);

int recebe_mensagem(int soquete, int timeoutMillis, char *buffer, int tamanho_buffer);

#endif