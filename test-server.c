#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <ncurses.h>
#include <sys/socket.h>
#include <string.h>

#include "kermit.h"
#include "interface.h"
#include "utils.h"

int main(int argc, char **argv){
    if (argc < 2) return -1;
    int socket = create_raw_socket(argv[1]);
    kermit_t rcv_msg, send_msg;                                                     // struct de mensagens
    uint8_t curr_seq = 0, expected_seq = 0;                                             // sequencia de mensagens
    uint8_t send_buffer[TAM_BUFFER];                                                // buffer de envia mensagem
    uint8_t rcv_buffer[TAM_BUFFER];                                                 // buffer de receber mensagem
    uint8_t mac_dest[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t mac_orig[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t eth_type = 0x8888;
    memcpy(send_buffer, mac_dest, 6);
    memcpy(send_buffer+6, mac_orig, 6);
    memcpy(send_buffer+12, &eth_type, 2);

    FILE *arc = fopen("assets/files/1.txt", "rb");
    fseek(arc, 0, SEEK_END);
    unsigned long file_size = ftell(arc);
    rewind(arc);
    uint8_t name_arc[31];
    sprintf("1-%ld", file_size);
    

    create_data_msg(&send_msg, strlen(name_arc), TXT, curr_seq, name_arc);
    int send_bytes = serialize_msg(&send_msg, send_buffer+14);
    send_with_ack(socket, send_buffer, send_bytes+14, rcv_buffer, &rcv_msg, &curr_seq);

    uint8_t chunk[31];
    size_t bytes_lidos;

    while((bytes_lidos = fread(chunk, 1, sizeof(chunk), arc)) > 0){
        create_data_msg(&send_msg, DADOS, curr_seq, chunk, bytes_lidos);
        send_bytes = serialize_msg(&send_msg, send_buffer+14);
        send_with_ack(socket, send_buffer, send_bytes + 14, rcv_buffer, &rcv_msg, &curr_seq);
    }

    fclose(arc);

    create_control_msg(&send_msg, FIM_DA_TRANSMISSAO, curr_seq);
    send_bytes = serialize_msg(&send_msg, send_buffer+14);
    send_with_ack(socket, send_buffer, send_bytes + 14, rcv_buffer, &rcv_buffer, &curr_seq);

    return 0;
}