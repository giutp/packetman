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

    char sendBuffer[8] = {0};
    for (int i = 0; i < 7; i++){
        sendBuffer[i] = 'a';
    }
    
    // -----------------------------------------------
    
    // -------------------- Recebimento --------------------
    char recvBuffer[1024] = {0};
    struct sockaddr_ll recvAddr;
    socklen_t recvAddrLen = sizeof(recvAddr);
    
    do{
        sendto(
            sock, sendBuffer, sizeof(sendBuffer), 
            0, (struct sockaddr *)&addr, sizeof(addr)
            );
        recvfrom(
            sock, recvBuffer, sizeof(recvBuffer), 0, 
            (struct sockaddr *)&recvAddr, &recvAddrLen
            );
        printf("recvAddr.sll_addr: %s\n", recvAddr.sll_addr);
        for (int i = 0; i < 64; i++){
            printf("recvBuffer[%d]: %02x ", i, recvBuffer[i]);
        }
        printf("\n");
    }while(memcmp(mac, recvAddr.sll_addr, sizeof(mac)) || memcmp(sendBuffer, recvBuffer + 14, sizeof(sendBuffer)));

    // protocolo ethernet tem 14 bytes de header
    printf("msg: %s\n", recvBuffer + 14);

    // -----------------------------------------------

    return 0;
}