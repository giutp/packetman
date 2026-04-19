#include <string.h>
#include <stdio.h>

#include "game.h"

int read_map(char *filepath, char map[N][N]){
    FILE *arc = fopen(filepath, "r");

    if (!arc) return -1;
    
    for(int i = 0; i < N; i++){
        char line[80];
        fgets(line, N + N-1 + 1, arc);

        for(int j = 0; j < N; j++) {
            char *token = strtok(j == 0 ? line : NULL, ";");
            map[i][j] = token[0];
        }
    }

    fclose(arc);

    return 0;
}