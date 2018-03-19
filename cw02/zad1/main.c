#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 


void generate(int argc, char **argv, int *i){
    char *name = argv[*i + 1];
    int n = atoi(argv[*i + 2]);
    int size = atoi(argv[*i + 3]) + 2; // na \0 \n

    *i += 3;

    FILE *plik = fopen(name, "w");

    if(!plik){
        perror("generate() - couldn't open file");
        return;
    }

    char *record = (char *)calloc(size, sizeof(char));


    for(int i = 0; i < n; i++){
        for(int j=0; j < size-1; j++){ // bo na koncu jest \0
            record[j] = 'A' + (rand() % 26);
        }
        record[size - 2] = '\n';

        fwrite(record, sizeof(char), size, plik);
    }
    fclose(plik);
    free(record);
}

void sort_sys(int argc, char **argv, int *i){
    
    char *tmpName = argv[*i + 1];
    int n = atoi(argv[*i + 2]);
    int size = atoi(argv[*i +3]) + 2; // na \n i \0

    *i += 4;

    int in;
    in = open(tmpName, O_RDWR|O_CREAT);
    if(in < 0){
        perror("sort_sys() - nie udało się otworzyć pliku");
        return;
    }


    char *buf1 = (char *)calloc(size, sizeof(char));
    char *buf2 = (char *)calloc(size, sizeof(char));
    int readCounter = size;
    int ai;
    int aj;
    int j; // iterator po rekordach do porównywania
    
    for(int i = 1; i < n; i++){
        if(lseek(in, i * size, 0) < 0){
            perror("sort_sys() - nie udało się zmienić położenia wskaźnika");
            return;
        }

        readCounter = read(in, buf1, size);
       
        if(readCounter != size){ // EOF ?
            perror("sort_sys() - błąd podczas odczytu danych");
            return;
        }

        j = i - 1;

        if(lseek(in, j * size, 0) < 0){
            perror("sort_sys() - nie udało się zmienić położenia wskaźnika");
            return;
        }

        readCounter = read(in, buf2, size);

        if(readCounter != size){
            perror("sort_sys() - błąd podczas odczytu danych");
            return;
        }

        ai = buf1[0];
        aj = buf2[0];

        while(j >= 0 && aj > ai){
            
            write(in, buf2, size);

            if(lseek(in, j*size, 0) < 0){
                perror("sort_sys() - nie udalo sie zmienic polozenia wskaznika");
                return;
            }
            
            write(in, buf1, size);

            j = j - 1;
            if(j >= 0){

                if(lseek(in, j * size, 0) < 0){
                    perror("sort_sys() - nie udalo sie zmienic polozenia wskaznika");
                    return;
                }

                readCounter = read(in, buf2, size);

                if(readCounter != size){
                    perror("sort_sys() - błąd podczas odczytu danych");
                    return;
                }

                aj = buf2[0];
            }
        }
    }

    free(buf1);
    free(buf2);
}

void sort_lib(int argc, char **argv, int *i){
    
    char *name = argv[*i + 1];
    int n = atoi(argv[*i + 2]);
    int size = atoi(argv[*i + 3]) + 2; //na \n i \0
    
    *i += 4;

    FILE *plik = fopen(name, "r+");

    if(!plik){
        perror("sort_lib() - file doesn't exist");
        return;
    }

    char *buf1 = (char *)calloc(size, sizeof(char));
    char *buf2 = (char *)calloc(size, sizeof(char));
    int readCounter = size;
    int ai;
    int aj;
    int j; // iterator po rekordach do porównywania

    for(int i = 1; i < n; i++){ // zaczynam od 2 rekordu w tablicy i przechodzy po wszystkich
        
        if(fseek(plik, i * size, 0) != 0){
            perror("sort_lib() - nie udalo sie zmienic polozenia wskaznika");
            return;
        }

        readCounter = fread(buf1, sizeof(char), size, plik);

        if(readCounter != size){ // EOF ?
            perror("sort_lib() - błąd podczas odczytu danych");
            return;
        }

        j = i - 1;

        if(fseek(plik, j * size, 0) != 0){
            perror("sort_lib() - nie udalo sie zmienic polozenia wskaznika");
            return;
        }

        readCounter = fread(buf2, sizeof(char), size, plik);
        printf("%d\n", readCounter);
        if(readCounter != size){
            perror("sort_lib() - błąd podczas odczytu danych");
            return;
        }

        ai = buf1[0];
        aj = buf2[0];

        while(j >= 0 && aj > ai){
            
            fwrite(buf2, sizeof(char), size, plik);

            if(fseek(plik, j*size, 0) != 0){
                perror("sort_lib() - nie udalo sie zmienic polozenia wskaznika");
                return;
            }
            
            fwrite(buf1, sizeof(char), size, plik);

            j = j - 1;
            if(j >= 0){

                if(fseek(plik, j * size, 0) != 0){
                    perror("sort_lib() - nie udalo sie zmienic polozenia wskaznika");
                    return;
                }

                readCounter = fread(buf2, sizeof(char), size, plik);

                if(readCounter != size){
                    perror("sort_lib() - błąd podczas odczytu danych");
                    return;
                }

                aj = buf2[0];
            }
        }
    }

    free(buf1);
    free(buf2);
}

void copy_sys(int argc, char **argv, int *i){
    
    char *name1 = argv[*i + 1];
    char *name2 = argv[*i + 2];
    int n = atoi(argv[*i + 3]);
    int size = atoi(argv[*i + 4]) + 2; //na \n i \0
    
    *i += 5;


    int in, out;

    in = open(name1, O_RDONLY);
    if(in < 0){
        perror("copy_sys() - couldn't open file");
        return;
    }

    out = open(name2, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
    if(out < 0){
        perror("copy_sys() - couldn't create file to save");
        return;
    }

    char *buf1 = (char *)calloc(size, sizeof(char));
    int readCounter = 0;
    int writeCounter = 0;
    for(int i = 0; i < n; i++){

        readCounter = read(in, buf1, size);
        
        if(readCounter != size){ // EOF ?
            perror("copy_lib() - couldn't read all bytes");
            return;
        }

        writeCounter = write(out, buf1, size);
        if(writeCounter != size){ // EOF ?
            perror("copy_lib() - couldn't write all bytes");
            return;
        }
    }
}

void copy_lib(int argc, char **argv, int *i){
    
    char *name1 = argv[*i + 1];
    char *name2 = argv[*i + 2];
    int n = atoi(argv[*i + 3]);
    int size = atoi(argv[*i + 4]) + 2; //na \n i \0
    
    *i += 5;

    FILE *plik1 = fopen(name1, "r");

    if(!plik1){
        perror("copy_lib() - file doesn't exist");
        return;
    }
    
    FILE *plik2 = fopen(name2, "w");

    if(!plik2){
        perror("copy_lib() - couldn't create file");
        return;
    }

    char *buf1 = (char *)calloc(size, sizeof(char));
    int readCounter = 0;
    int writeCounter = 0;
    for(int i = 0; i < n; i++){

        readCounter = fread(buf1, sizeof(char), size, plik1);
        
        if(readCounter != size){ // EOF ?
            perror("copy_lib() - couldn't read all bytes");
            return;
        }

        writeCounter = fwrite(buf1, sizeof(char), size, plik2);
        if(writeCounter != size){ // EOF ?
            perror("copy_lib() - couldn't write all bytes");
            return;
        }
    }
}

int main(int argc, char ** argv){
    
    srand(time(NULL));    
    
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "generate") == 0)
            generate(argc, argv, &i);
        else if (strcmp(argv[i], "sort") == 0){
            if(strcmp(argv[i + 4], "lib") == 0)
                sort_lib(argc, argv, &i);
            else if(strcmp(argv[i + 4], "sys") == 0)
                sort_sys(argc, argv, &i);
            else {
                printf("Podano zle argumenty");
                break;
            };
        }
        else if (strcmp(argv[i], "copy") == 0){         
            if(strcmp(argv[i + 5], "lib") == 0)
                copy_lib(argc, argv, &i);
            else if(strcmp(argv[i + 5], "sys") == 0)
                copy_sys(argc, argv, &i);
            else {
                printf("Podano zle argumenty");
                break;
            };
        }
    }
    
    return 0;
}

