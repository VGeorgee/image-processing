#include <string.h>


int get_index_of_param(char **argv, const char *param) {
    for(int i = 1; i < 6; i++){
        if(!strcmp(argv[i], param)){
            return i +1;
        }
    }
    return -1;
}

char *crop_filename(char *filename){
    int i = strlen(filename) - 1;
    while(filename[i--] != '\\' && i);
    return filename + i + 2;
}

void trim_dir_backslash(char *dir){
    while(*dir++);
    dir--;
    if(*--dir == '\\') *dir = '\0';
}