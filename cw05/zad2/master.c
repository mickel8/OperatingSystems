#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#define MEMBERS 4096

int main(int argc, char **argv){

    char *line = calloc(MEMBERS, sizeof(char));

    if(argc != 2){
        printf("Bad number of arguments");
        exit(EXIT_FAILURE);
    }

    char *pipe = argv[1];

    if(mkfifo(pipe, S_IRUSR | S_IWUSR) == -1){
        perror("mkfifo()");
        exit(EXIT_FAILURE);
    }

    FILE *file = fopen(pipe, "r");
    if(file == NULL){
        perror("fopen()");
        exit(EXIT_FAILURE);
    }

    while(fgets(line, MEMBERS, file) != NULL)
        printf("%s\n", line);

    if(errno){
        perror("fgets()");
        exit(EXIT_FAILURE);
    }

    if(fclose(file) == EOF){
        perror("fclose()");
        exit(EXIT_FAILURE);
    }

    return 0;
}