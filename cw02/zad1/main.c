#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h> 
#include <sys/times.h>



// zmienne zegarowe
static clock_t st_time;
static clock_t en_time;
static struct tms st_cpu;
static struct tms en_cpu;

void develop_data(){

    static long int clkTicksPS = 0; // clock ticks per second
    clkTicksPS = sysconf(_SC_CLK_TCK);

    FILE *plik = fopen("wyniki.txt", "a+");
    if(!plik){
        perror("develop_data() - couldn't open file");
        return;
    }

    char **info = (char **)calloc(2, sizeof(char*));
    info[0] = "user time: ";
    info[1] = " system time: ";

    double measurements[2];
    measurements[0] = (en_cpu.tms_utime - st_cpu.tms_utime) / (double)clkTicksPS;
    measurements[1] = (en_cpu.tms_stime - st_cpu.tms_stime) / (double)clkTicksPS;

    for(int i = 0; i < 2; i++){    
        if(fprintf(plik, "%s", info[i]) < 0 || fprintf(plik, "%f", measurements[i]) < 0){
            perror("develop_data() - couldn't write to file");
            return;
        }
    }

    if(fclose(plik) == -1){
        perror("develop_data() - couldn't exit file");
        return;
    }

    free(info);   
}

void st_clock(){
    st_time = times(&st_cpu);
}

void en_clock(){
    en_time = times(&en_cpu);
    develop_data();
}

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
    int writeCounter = 0;

    for(int i = 0; i < n; i++){
        for(int j=0; j < size-1; j++){ // bo na koncu jest \0 -2 powinno byc
            record[j] = 'A' + (rand() % 26);
        }
        record[size - 2] = '\n';

        writeCounter = fwrite(record, sizeof(char), size, plik);
        if(writeCounter != size){
            printf("generate() - couldn't write all bytes\n");
            return;
        }
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
        perror("sort_sys() - couldn't open file");
        return;
    }


    char *buf1 = (char *)calloc(size, sizeof(char));
    char *buf2 = (char *)calloc(size, sizeof(char));
    int readCounter = 0;
    int writeCounter = 0;
    int ai;
    int aj;
    int j; // iterator po rekordach do porównywania
    
    for(int i = 1; i < n; i++){
        if(lseek(in, i * size, 0) < 0){
            perror("sort_sys() - couldn't change location of pointer");
            return;
        }

        readCounter = read(in, buf1, size);
        if(readCounter != size){ // EOF ?
            printf("sort_sys() - couldn't read all bytes\n");
            return;
        }

        j = i - 1;

        if(lseek(in, j * size, 0) < 0){
            perror("sort_sys() - nie udało się zmienić położenia wskaźnika");
            return;
        }

        readCounter = read(in, buf2, size);\
        if(readCounter != size){
            printf("sort_sys() - couldn't read all bytes\n");
            return;
        }

        ai = buf1[0];
        aj = buf2[0];

        while(j >= 0 && aj > ai){
            
            writeCounter = write(in, buf2, size);
            if(writeCounter != size){
                printf("sort_sys() - couldn't write all bytes\n");
                return;
            }

            if(lseek(in, j*size, 0) < 0){
                perror("sort_sys() - couldn't change location of pointer");
                return;
            }
            
            writeCounter = write(in, buf1, size);
            if(writeCounter != size){
                printf("sort_sys() - couldn't write all bytes\n");
                return;
            }

            j = j - 1;
            if(j >= 0){

                if(lseek(in, j * size, 0) < 0){
                    perror("sort_sys() - couldn't change location of pointer");
                    return;
                }

                readCounter = read(in, buf2, size);

                if(readCounter != size){
                    printf("sort_sys() - couldn't read all bytes\n");
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
    int readCounter = 0;
    int writeCounter = 0;
    int ai;
    int aj;
    int j; // iterator po rekordach do porównywania

    for(int i = 1; i < n; i++){ // zaczynam od 2 rekordu w tablicy i przechodzy po wszystkich
        
        if(fseek(plik, i * size, 0) != 0){
            perror("sort_lib() - couldn't change location of pointer");
            return;
        }

        readCounter = fread(buf1, sizeof(char), size, plik);
        if(readCounter != size){ // EOF ?
            printf("sort_lib() - couldn't write all bytes\n");
            return;
        }

        j = i - 1;

        if(fseek(plik, j * size, 0) != 0){
            perror("sort_lib() - couldn't change location of pointer");
            return;
        }

        readCounter = fread(buf2, sizeof(char), size, plik);
        if(readCounter != size){
            printf("sort_lib() - coulnd't read all bytes\n");
            return;
        }

        ai = buf1[0];
        aj = buf2[0];

        while(j >= 0 && aj > ai){
            
            writeCounter = fwrite(buf2, sizeof(char), size, plik);
            if(writeCounter != size){ // EOF ?
                printf("copy_lib() - couldn't write all bytes\n");
                return;
            }

            if(fseek(plik, j*size, 0) != 0){
                perror("sort_lib() - couldn't change location of pointer");
                return;
            }
            
            writeCounter = fwrite(buf1, sizeof(char), size, plik);
            if(writeCounter != size){ // EOF ?
                printf("copy_lib() - couldn't write all bytes\n");
                return;
            }

            j = j - 1;
            if(j >= 0){

                if(fseek(plik, j * size, 0) != 0){
                    perror("sort_lib() - couldn't change location of pointer");
                    return;
                }

                readCounter = fread(buf2, sizeof(char), size, plik);
                if(readCounter != size){
                    printf("sort_lib() - couldn't read all bytes\n");
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
            printf("copy_lib() - couldn't read all bytes\n");
            return;
        }

        writeCounter = write(out, buf1, size);
        if(writeCounter != size){ // EOF ?
            printf("copy_lib() - couldn't write all bytes\n");
            return;
        }
    }

    free(buf1);
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
            printf("copy_lib() - couldn't read all bytes\n");
            return;
        }

        writeCounter = fwrite(buf1, sizeof(char), size, plik2);
        if(writeCounter != size){ // EOF ?
            printf("copy_lib() - couldn't write all bytes\n");
            return;
        }
    }
    
    free(buf1);
}

int main(int argc, char ** argv){
    
    srand(time(NULL));    
    
    for (int i = 0; i < argc; i++){
        if (strcmp(argv[i], "generate") == 0)
            generate(argc, argv, &i);
        else if (strcmp(argv[i], "sort") == 0){
            if(strcmp(argv[i + 4], "lib") == 0){
                st_clock();
                sort_lib(argc, argv, &i);
                en_clock();
            }
            else if(strcmp(argv[i + 4], "sys") == 0){
                st_clock();
                sort_sys(argc, argv, &i);
                en_clock();
            }
            else {
                printf("Podano zle argumenty");
                break;
            };
        }
        else if (strcmp(argv[i], "copy") == 0){         
            if(strcmp(argv[i + 5], "lib") == 0){
                st_clock();
                copy_lib(argc, argv, &i);
                en_clock();
            }
            else if(strcmp(argv[i + 5], "sys") == 0){
                st_clock();
                copy_sys(argc, argv, &i);
                en_clock();
            }
            else {
                printf("Podano zle argumenty");
                break;
            };
        }
    }
    
    return 0;
}

