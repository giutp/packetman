#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "kermit.h"

// main criada puramente para testes, nao sera usada (depois vou apagar)
int main(){
    // Testando envio e recebimento

    // Criando e abrindo soquete
    int sock = create_raw_socket("lo");

    // Enviando mensagem
    unsigned char msg[17];

    memset(msg, 0, 12);
    unsigned short ethType = htons(0x0800);
    memcpy(msg + 12, &ethType, 2);
    msg[14] = 'o';
    msg[15] = 'l';
    msg[16] = 'a';
    
    send(sock, msg, sizeof(msg), 0);

    // Recebendo mensagem
    unsigned char buffer[1025];
    size_t bytesRecv = recv(sock, buffer, sizeof(buffer) - 1, 0);
    buffer[bytesRecv] = '\0';
    printf("Mensagem recebida: %s\n", buffer+14);

    return 0;
}