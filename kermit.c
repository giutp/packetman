#include <arpa/inet.h>          // htons(), htonl()
#include <net/ethernet.h>       // ETH_P_ALL
#include <linux/if_packet.h>    // sockaddr_ll, packet_mreq, PACKET_MR_PROMISC, PACKET_ADD_MEMBERSHIP
#include <net/if.h>             // if_nametoindex()
#include <sys/socket.h>         // socket(), bind(), setsockopt(), AF_PACKET, SOCK_RAW, SOL_PACKET
#include <string.h>             // memset()
#include <stdlib.h>             // exit(), NULL
#include <stdio.h>              // fprintf(), stderr
#include <stdint.h>             // uint8_t

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

uint8_t calculate_crc8(uint8_t *buffer, int size_buffer){
    uint8_t crc8 = 0x00;
    const uint8_t pol = 0x07; // Talvez mude depois

    for (int i = 0; i < size_buffer; i++){
        crc8 ^= buffer[i];
        for (int j = 0; j < 8; j++){
            if (crc8 & 0x80) crc8 = (crc8 << 1) ^ pol;
            else crc8 <<= 1;
        }
    }

    return crc8;
}

int serialize_msg(kermit_t *deserialize_msg, uint8_t *serialize_msg){
    if (!deserialize_msg || !serialize_msg) return -1;

    uint8_t size;
    size = (deserialize_msg->size_sequence_type >> 11) & 0x1f;
    if (size > 0 && !deserialize_msg->data) return -1;

    serialize_msg[0] = deserialize_msg->starter_marker;
    serialize_msg[1] = deserialize_msg->size_sequence_type >> 8;
    serialize_msg[2] = deserialize_msg->size_sequence_type;

    for (int i = 0; i < size; i++){
        serialize_msg[i+3] = deserialize_msg->data[i];
    }

    serialize_msg[size + 3] = calculate_crc8(serialize_msg, size + 3);

    // TODO: mudar para constantes esses valores talvez?
    return (3 + size + 1);
}

int deserialize_msg(uint8_t *serialize_msg, kermit_t *deserialize_msg){
    if (!serialize_msg || !deserialize_msg) return -1;
    
    if (serialize_msg[0] != 0x7e) return -2;

    deserialize_msg->starter_marker = serialize_msg[0];
    deserialize_msg->size_sequence_type = serialize_msg[1];
    deserialize_msg->size_sequence_type <<= 8;
    deserialize_msg->size_sequence_type |= serialize_msg[2];
    
    uint8_t size = (deserialize_msg->size_sequence_type >> 11) & 0x1f;
    if (size > 0 && !deserialize_msg->data) return -3;
    deserialize_msg->crc = serialize_msg[3 + size];
    if (calculate_crc8(serialize_msg, 3 + size) != deserialize_msg->crc) return -4;

    for (int i = 0; i < size; i++){
        deserialize_msg->data[i] = serialize_msg[i + 3];
    }

    return 0;
}

// Cria mensagem usando o procolo kermit
static void create_msg(kermit_t *msg, uint8_t size, uint8_t type, uint8_t seq, uint8_t *data){
    msg->starter_marker = 0x7e;
    msg->size_sequence_type = size & 0x1f;
    msg->size_sequence_type <<= 5;
    msg->size_sequence_type |= seq & 0x3f;
    msg->size_sequence_type <<= 5;
    msg->size_sequence_type |= type & 0x1f;
    msg->data = data;
}

void create_control_msg(kermit_t *msg, uint8_t type, uint8_t seq){
    create_msg(msg, 0, type, seq, NULL);
}

void create_data_msg(kermit_t *msg, uint8_t size, uint8_t type, uint8_t seq, uint8_t *data){
    create_msg(msg, size, type, seq, data);
}
