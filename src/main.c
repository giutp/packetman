#include <arpa/inet.h>
#include <net/ethernet.h>
#include <linux/if_packet.h>
#include <net/if.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "pellets.h"

// main criada puramente para testes, nao sera usada (depois vou apagar)
int main(){
    // AREA DE TESTES DO pellet.c

    pellet_t pellets[6];

    int status = bind_pellets_files(pellets);

    switch (status){
    case 0:
        printf("Todos foram atrelados!\n");
        for (int i = 0; i < 6; i++){
            printf("\nCaminho: %s\n", pellets[i].filepath);
            printf("Tipo: %d\n\n", pellets[i].arc_type);

            const char *xdg = "xdg-open ";
            int len = strlen(pellets[i].filepath) + strlen(xdg) + 1;
            char *path = (char *)malloc(len);
            snprintf(path, len, "%s%s", xdg, pellets[i].filepath);
            printf("%s\n", path);
            system(path);
            free(path);
        }
        break;
    
    case -2:
        printf("Erro -2: O diretorio files nao foi encontrado!\n");
        break;
    
    case -1:
        printf("Erro -1: Diretorio existe! Mas existe arquivos a mais, a menos ou invalidos\n");
        break;

    }

    return 0;
}