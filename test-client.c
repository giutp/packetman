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
    int expected_seq = 0;                                             // sequencia de mensagens
    uint8_t send_buffer[TAM_BUFFER];                                                // buffer de envia mensagem
    uint8_t rcv_buffer[TAM_BUFFER];                                                 // buffer de receber mensagem
    uint8_t mac_dest[6] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    uint8_t mac_orig[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint16_t eth_type = 0x8888;
    memcpy(send_buffer, mac_dest, 6);
    memcpy(send_buffer+6, mac_orig, 6);
    memcpy(send_buffer+12, &eth_type, 2);

    int flag_ntw = 0;
    FILE *arc = NULL;
    unsigned long size_arc = 0;
    unsigned long total_donwloaded = 0;
    while(1){

        recv(socket, rcv_buffer, sizeof(rcv_buffer), 0);
        if (is_valid_protocol(rcv_buffer+14)){
            // CRC válido
            if(is_valid_crc(rcv_buffer+14)){
                deserialize_msg(rcv_buffer+14, &rcv_msg);
                // Mensagem recebida é a esperada
                if (rcv_msg.sequence == expected_seq){                       
                    create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                    int send_bytes = serialize_msg(&send_msg, send_buffer+14); 
                    send(socket, send_buffer, send_bytes+14, 0);               
                    expected_seq = (expected_seq + 1) % 32;
                    if(rcv_msg.type == TXT){
                        char arc_name[31];
                        memcpy(arc_name, rcv_msg.data, rcv_msg.size);
                        arc_name[rcv_msg.size] = '\0';

                        char *del = strchr(arc_name, '-');
                        if (del != NULL){
                            *del = '\0';
                            size_arc = atoi(del+1);
                            arc = fopen("assets/files/1.txt", "wb");

                            if (arc != NULL){
                                printf("\n Arquivo %s criado! Tamanho: %ld\n", arc_name, size_arc);
                            }
                        }
                        
                    }
                    else if (rcv_msg.type == DADOS){
                        if (arc != NULL){
                            size_t bytes_writed = fwrite(rcv_msg.data, 1, rcv_msg.size, arc);
                            total_donwloaded +=  bytes_writed;
                            if (size_arc > 0){
                                printf("Download: %.2f%%\n", ((float)total_donwloaded/size_arc)*100);
                            }
                        }

                    }
                    else if (rcv_msg.type == FIM_DA_TRANSMISSAO){
                        if (arc != NULL){
                            fclose(arc);
                            printf("Download completo com sucesso\n");
                            
                        }
                        flag_ntw = 1;
                    }
                }
                // Mensagem repetida
                else{
                    create_control_msg(&send_msg, ACK, rcv_msg.sequence);
                    int send_bytes = serialize_msg(&send_msg, send_buffer+14);
                    send(socket, send_buffer, send_bytes+14, 0);
                }
                free(rcv_msg.data);
            }
            // CRC inválido
            else{
                create_control_msg(&send_msg, NACK, expected_seq);
                int send_bytes = serialize_msg(&send_msg, send_buffer+14);
                send(socket, send_buffer, send_bytes+14, 0);
            }
        }
        if (flag_ntw) break;
    }

    return 0;
}