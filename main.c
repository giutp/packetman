#include <stdio.h>
#include "rawSocket.h"

int main(){
    int sock = createRawSocket("enp42s0");
    printf("Socket opened: %d\n", sock);

    return 0;
}