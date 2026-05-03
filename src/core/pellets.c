#define _DEFAULT_SOURCE

#include <string.h>     // strcmp, strlen, strrchr
#include <dirent.h>     // opendir, readdir, closedir
#include <stdlib.h>     // malloc, free
#include <stdio.h>      // snprintf

#include "pellets.h"

int bind_pellets_files(pellet_t *pellets){
    const char *base_path = "assets/files/";

    DIR *direc;
    if (!(direc = opendir(base_path))) return -2;

    int count_txt = 0, count_jpg = 0, count_mp4 = 0;
    char *path_txt[2], *path_jpg[2], *path_mp4[2];

    // Varre o diretório aberto
    int valid = 1;
    struct dirent *file;
    while(((file = readdir(direc)) != NULL) && (valid)){
        // Arquivo regular
        if (file->d_type == DT_REG){
            // Checa extensão do arquivo
            char *ext = strrchr(file->d_name, '.');

            // Arquivo sem extensão (inválido)
            if (!ext) valid = 0;
            else{
                int len = strlen(base_path) + strlen(file->d_name) + 1;
                
                // Checa o tipo da extensão do arquivo (txt, jpg e mp4)
                // Se contador dos path estorar o loop será interrompido
                if ((strcmp(ext, ".txt") == 0)){
                    if (count_txt == 2) valid = 0;
                    else{
                        path_txt[count_txt] = (char *)malloc(len);
                        snprintf(path_txt[count_txt], len, "%s%s", base_path, file->d_name);
                        count_txt++;
                    }
                }
                else if((strcmp(ext, ".jpg") == 0)){
                    if (count_jpg == 2) valid = 0;
                    else{
                        path_jpg[count_jpg] = (char *)malloc(len);
                        snprintf(path_jpg[count_jpg], len, "%s%s", base_path, file->d_name);
                        count_jpg++;
                    }
                }
                else if((strcmp(ext, ".mp4") == 0)){
                    if (count_mp4 == 2) valid = 0;
                    else{
                        path_mp4[count_mp4] = (char *)malloc(len);
                        snprintf(path_mp4[count_mp4], len, "%s%s", base_path, file->d_name);
                        count_mp4++;
                    }
                }
                else valid = 0;
            }
        }
    }

    closedir(direc);

    // Libera memória alocada em caso de erro
    if (count_txt != 2 || count_jpg != 2 || count_mp4 != 2 || !valid) {
        for (int i = 0; i < count_txt; i++) free(path_txt[i]);
        for (int i = 0; i < count_jpg; i++) free(path_jpg[i]);
        for (int i = 0; i < count_mp4; i++) free(path_mp4[i]);

        return -1;
    }

    // Atrela o caminho dos arquivos às pastilhas
    for (int i = 0; i < 2; i++){
        pellets[i].filepath = path_txt[i];
        pellets[i].arc_type = FILE_TXT;
        
        pellets[i+2].filepath = path_jpg[i];
        pellets[i+2].arc_type = FILE_JPG;

        pellets[i+4].filepath = path_mp4[i];
        pellets[i+4].arc_type = FILE_MP4;
    }

    return 0;
}