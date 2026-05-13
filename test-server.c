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
    int socket = create_raw_socket(argv[1]);
    kermit_t rcv_msg, send_msg;                                                     // struct de mensagens
    int curr_seq = 0, expected_seq = 0;                                             // sequencia de mensagens
    uint8_t send_buffer[TAM_BUFFER];                                                // buffer de envia mensagem
    uint8_t rcv_buffer[TAM_BUFFER];                                                 // buffer de receber mensagem

    create_control_msg(&send_msg, VISUALIZACAO, curr_seq);
    int send_bytes = serialize_msg(&send_msg, send_buffer);
    send_with_ack(socket, send_buffer, send_bytes, rcv_buffer, &rcv_msg, &curr_seq);

    return 0;
}