#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rawSocket.h"

int main(){
    // Testando envio e recebimento
    int sock = createRawSocket("lo");
    unsigned char mac[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    // -------------------- Envio --------------------
    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(addr));
    addr.sll_family = AF_PACKET;
    addr.sll_protocol = htonl(ETH_P_ALL);
    addr.sll_ifindex = if_nametoindex("lo");
    addr.sll_halen = sizeof(mac);
    memcpy(addr.sll_addr, mac, sizeof(mac));

    unsigned char sendBuffer[22] = {0};
    sendBuffer[12] = 0x88;
    sendBuffer[13] = 0xB5;
    for (int i = 0; i < 7; i++){
        sendBuffer[14 + i] = 'a';
    }
    
    sendto(
        sock, sendBuffer, sizeof(sendBuffer), 
        0, (struct sockaddr *)&addr, sizeof(addr)
        );
    printf("pacote de %lu bytes disparado\n", sizeof(sendBuffer));
    // -----------------------------------------------
    
    // -------------------- Recebimento --------------------
    char recvBuffer[1024] = {0};

    while(1){
        int tam = recvfrom(
            sock, recvBuffer, sizeof(recvBuffer), 0, 
            NULL, NULL
            );
        if (tam < 0) continue;

        if (tam >= 22 && memcmp(sendBuffer + 14, recvBuffer + 14, 8) == 0){
            printf("MAC Destino Recebido: %02x:%02x:%02x:%02x:%02x:%02x\n", 
                    recvBuffer[0], recvBuffer[1], recvBuffer[2], 
                    recvBuffer[3], recvBuffer[4], recvBuffer[5]);

            printf("Mensagem: %s\n", recvBuffer + 14);
            
            break;
        }
    }

    return 0;
}