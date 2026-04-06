#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int createRawSocket(char* networkInterfaceName){
    int sk, ifindex;
    struct sockaddr_ll addr;
    struct packet_mreq mr;

    sk = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    if (sk == -1){
        fprintf(stderr, "Error to create socket: check if you're root!\n");
        exit(-1);
    }

    ifindex = if_nametoindex(networkInterfaceName);

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
