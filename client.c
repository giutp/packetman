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

    char buffer[2] = {0};
    unsigned char mac[6] = {0x34, 0x5a, 0x60, 0xee, 0x1b, 0xeb};

    int ifindex = if_nametoindex("enp42s0");

    struct sockaddr_ll addr;
    memset(&addr, 0, sizeof(struct sockaddr_ll));
    addr.sll_family = AF_PACKET;
    addr.sll_ifindex = ifindex;
    addr.sll_halen = 6;
    addr.sll_protocol = htons(ETH_P_ALL);
    memcpy(addr.sll_addr, mac, 6);
    
    while(1){
        scanf("%c ", &buffer[0]);
        sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*)&addr, sizeof(addr));
    }

    return 0;
}