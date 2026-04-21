#include <stdlib.h>
#include <stdio.h>

#include "kermit.h"

int main(int argc, char **argv){
    if (argc < 2){
        fprintf(stderr, "Necessario informar interface da placa de rede\n");
        exit(-1);
    }

    int socket = create_raw_socket(argv[1]);
    
    return 0;
}