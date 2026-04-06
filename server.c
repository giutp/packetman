#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "rawSocket.h"

int main(){
    int sock = createRawSocket("enp42s0");
    unsigned char buffer[1024];
    struct sockaddr_ll addr;
    socklen_t addr_len = sizeof(struct sockaddr_ll);
    unsigned char mac[6] = {0x34, 0x5a, 0x60, 0xee, 0x1b, 0xeb};

    while(1){
        recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, &addr_len);
        printf("MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", 
            addr.sll_addr[0],
            addr.sll_addr[1],
            addr.sll_addr[2],
            addr.sll_addr[3],
            addr.sll_addr[4],
            addr.sll_addr[5]
        );
        if (memcmp(addr.sll_addr, mac, sizeof(mac)) == 0)
            printf("buffer: %16x\n", buffer[14]);
    }

    return 0;
}