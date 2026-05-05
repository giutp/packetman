#include <arpa/inet.h>          // htons(), htonl()
#include <net/ethernet.h>       // ETH_P_ALL
#include <linux/if_packet.h>    // sockaddr_ll, packet_mreq, PACKET_MR_PROMISC, PACKET_ADD_MEMBERSHIP
#include <net/if.h>             // if_nametoindex()
#include <sys/socket.h>         // socket(), bind(), setsockopt(), AF_PACKET, SOCK_RAW, SOL_PACKET
#include <string.h>             // memset()
#include <stdlib.h>             // exit(), NULL
#include <stdio.h>              // fprintf(), stderr
#include <stdint.h>             // uint8_t
#include <sys/time.h>           // struct timeval

#include "kermit.h"

int create_raw_socket(char *network_interface_name){
    int sk, ifindex;
    struct sockaddr_ll addr;
    struct packet_mreq mr;

    sk = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sk == -1){
        fprintf(stderr, "Error to create socket: check if you're root!\n");
        exit(-1);
    }

    ifindex = if_nametoindex(network_interface_name);

    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htonl(ETH_P_ALL);
    addr.sll_ifindex = ifindex;
    if (bind(sk, (struct sockaddr*) &addr, sizeof(addr)) == -1){
        fprintf(stderr, "Error to make bind in socket\n");
        exit(-1);
    }

    memset(&mr, 0, sizeof(mr));
    mr.mr_ifindex = ifindex;
    mr.mr_type = PACKET_MR_PROMISC;
    if (setsockopt(sk, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1){
        fprintf(stderr, "Error to make setsockopt: "
                        "Check if the network interface was correctly specified.\n");
        exit(-1);
    }

    return sk;
}

int is_valid_start_marker(uint8_t *buffer){
    return (buffer[0] == START_MARKER);
}

int is_valid_crc(uint8_t *buffer){

    uint16_t size_sequence_type;
    size_sequence_type = buffer[1];
    size_sequence_type <<= 8;
    size_sequence_type = buffer[2];

    uint8_t size = (size_sequence_type >> 11) & 0x1f;

    uint8_t crc_buffer, crc_check;
    crc_buffer = buffer[size+3];
    crc_check = calculate_crc8(buffer, 3 + size);
    return (crc_buffer == crc_check);
}

uint8_t calculate_crc8(uint8_t *buffer, int size_buffer){
    uint8_t crc8 = 0x00;
    const uint8_t pol = 0x07; // POLINOMIO ARBRITARIO, TALVEZ MUDE DEPOIS 

    // Algoritmo da divisao polinomial
    for (int i = 0; i < size_buffer; i++){
        crc8 ^= buffer[i];
        for (int j = 0; j < 8; j++){
            if (crc8 & 0x80) crc8 = (crc8 << 1) ^ pol;
            else crc8 <<= 1;
        }
    }

    return crc8;
}

int serialize_msg(kermit_t *send_msg, uint8_t *send_buffer){
    // if (!send_msg || !send_buffer) return -1;

    send_buffer[0] = send_msg->starter_marker;
    send_buffer[1] = send_msg->size;
    send_buffer[2] = send_msg->sequence;
    send_buffer[3] = send_msg->type;
    for (int i = 0; i < send_msg->size; i++){
        send_buffer[i + 3] = send_msg->data[i];
    }
    send_buffer[3 + send_msg->size] = calculate_crc8(send_buffer, 3 + send_msg->size);

    return (3 + send_msg->size + 1);

    // uint8_t size;
    // size = (deserialize_msg->size_sequence_type >> 11) & 0x1f;
    // if (size > 0 && !deserialize_msg->data) return -1;

    // serialize_msg[0] = deserialize_msg->starter_marker;
    // serialize_msg[1] = deserialize_msg->size_sequence_type >> 8;
    // serialize_msg[2] = deserialize_msg->size_sequence_type;

    // for (int i = 0; i < size; i++){
    //     serialize_msg[i+3] = deserialize_msg->data[i];
    // }

    // serialize_msg[size + 3] = calculate_crc8(serialize_msg, size + 3);

    // // TODO: mudar para constantes esses valores talvez? Apenas por legibilidade
    // return (3 + size + 1);
}

//Funcao vai mudar
int deserialize_msg(uint8_t *rcv_buffer, kermit_t *rcv_msg){
    // if (!rcv_buffer || !rcv_msg) return -1;

    rcv_msg->starter_marker = rcv_buffer[0];
    rcv_msg->size = rcv_buffer[1];
    rcv_msg->sequence = rcv_buffer[2];
    rcv_msg->type = rcv_buffer[3];
    for(int i = 0; i < rcv_buffer[1]; i++){
        rcv_msg->data[i] = rcv_buffer[i + 3];
    }
    rcv_msg->crc = rcv_buffer[3 + rcv_buffer[1]];

    return (3 + rcv_buffer[1] + 1);
    
    // Inserir constante tvlz?
    // 0x7e: marcador de inicio
    // Provavelmente vai sair daqui pois esta fora do escopo da funcao
    // if (serialize_msg[0] != 0x7e) return -2;

    // deserialize_msg->starter_marker = serialize_msg[0];
    // deserialize_msg->size_sequence_type = serialize_msg[1];
    // deserialize_msg->size_sequence_type <<= 8;
    // deserialize_msg->size_sequence_type |= serialize_msg[2];
    
    // uint8_t size = (deserialize_msg->size_sequence_type >> 11) & 0x1f;

    // Essas checagens talvez saiam daqui devido escopo da funcao
    // ser apenas deserializar mensagem
    // deserialize_msg->crc = serialize_msg[3 + size];
    // if (calculate_crc8(serialize_msg, 3 + size) != deserialize_msg->crc) return -4;
    
    // if (size > 0 && !deserialize_msg->data) return -3;
    // for (int i = 0; i < size; i++){
    //     deserialize_msg->data[i] = serialize_msg[i + 3];
    // }

    // return (3 + size + 1);
}

// Cria mensagem usando o procolo kermit
static void create_msg(kermit_t *msg, uint8_t size, uint8_t type, uint8_t seq, uint8_t *data){
    msg->starter_marker = START_MARKER;
    msg->size = size;
    msg->sequence = seq;
    msg->type = type;
    msg->data = data;

    // msg->starter_marker = 0x7e;
    // msg->size_sequence_type = size & 0x1f;
    // msg->size_sequence_type <<= 5;
    // msg->size_sequence_type |= seq & 0x3f;
    // msg->size_sequence_type <<= 5;
    // msg->size_sequence_type |= type & 0x1f;
    // msg->data = data;
}

void create_control_msg(kermit_t *msg, uint8_t type, uint8_t seq){
    create_msg(msg, 0, type, seq, NULL);
}

void create_data_msg(kermit_t *msg, uint8_t size, uint8_t type, uint8_t seq, uint8_t *data){
    create_msg(msg, size, type, seq, data);
}

// TODO: mudar o nome dessas funcoes e organizar se possivel, apenas copiei do site do professor
long long timestamp(){
    struct timeval tp;
    gettimeofday(&tp, NULL);
    return tp.tv_sec*1000 + tp.tv_usec/1000;
}

int protocolo_e_valido(char *buffer, int tamanho_buffer){
    if (tamanho_buffer <= 0) return 0;

    return buffer[0] == 0x7e;
}

int recebe_mensagem(int soquete, int timeoutMillis, char *buffer, int tamanho_buffer){
    long long comeco = timestamp();
    struct timeval timeout = {
        .tv_sec = timeoutMillis/1000,
        .tv_usec = (timeoutMillis%1000) * 1000
    };
    setsockopt(soquete, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    int bytes_lidos;
    do {
        bytes_lidos = recv(soquete, buffer, tamanho_buffer, 0);
        if (protocolo_e_valido(buffer, bytes_lidos)) return bytes_lidos;
    } while (timestamp() - comeco <= timeoutMillis);

    return -1;
}