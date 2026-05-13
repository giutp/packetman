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

    int flag_ntw = 0;
    while(1){

        recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
        
        if (is_valid_protocol(rcv_buffer)){
            // CRC válido
            if(is_valid_crc(rcv_buffer)){
                deserialize_msg(rcv_buffer, &rcv_msg);
                // Mensagem recebida é a esperada
                if (rcv_msg.sequence == expected_seq){                       
                    create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                    int send_bytes = serialize_msg(&send_msg, send_buffer); 
                    send(socket, send_buffer, send_bytes, 0);               
                    expected_seq = (expected_seq + 1) % 32;
                    if(rcv_msg.type == VISUALIZACAO)
                        printf("SUCESSO");
                }
                // Mensagem repetida
                else{
                    create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                    int send_bytes = serialize_msg(&send_msg, send_buffer);
                    send(socket, send_buffer, send_bytes, 0);
                }
                free(rcv_msg.data);
            }
            // CRC inválido
            else{
                create_control_msg(&send_msg, NACK, expected_seq);
                int send_bytes = serialize_msg(&send_msg, send_buffer);
                send(socket, send_buffer, send_bytes, 0);
            }
        }
    }
}